#!/usr/bin/env python3
"""
CANN 群聊增量提取器
====================

从 Android 手机 QQ 数据库提取指定群的聊天记录，支持增量。

设计要点（均为本机实测踩坑后修正）：
 1. 群号在 group_msg_table 的 40027 列，不是官方文档说的 40030（后者是空列）
 2. 密钥 = md5(md5(nt_uid) + rand)，rand 在文件头 offset 0x2E（不是 0x2A）
 3. 必须剥掉 1024 字节自定义头，否则 SQLCipher 报 file is not a database
 4. 库有损坏页，必须用 rowid 游标分页，不能全表扫描
 5. 错误 key 时 count(*) FROM sqlite_master 返回 0 而不报错，
    验证必须实际读到已知表名

用法:
    python3 cann_extract.py                    # 增量提取 + 筛选
    python3 cann_extract.py --full             # 全量重新提取
    python3 cann_extract.py --group 901064769  # 指定群号
    python3 cann_extract.py --no-screen        # 只提取，不做提分筛选
    python3 cann_extract.py --status # 查看增量状态
    python3 cann_extract.py --reset            # 清除增量状态

依赖:
    pip install sqlcipher3   （macOS 需 arm64 预编译轮子）
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path

# ============================ 配置 ============================

CONFIG_PATH = Path(__file__).parent / "config.json"
STATE_PATH = Path(__file__).parent / "状态" / "incremental_state.json"
OUT_DIR = Path(__file__).parent / "输出"

ANDROID_PKG = "com.tencent.mobileqq"
DB_REL = f"/data/user/0/{ANDROID_PKG}/databases/nt_db"
UID_REL = f"/data/user/0/{ANDROID_PKG}/files/uid"

# 本机实测的 SQLCipher 参数（QQ 9.2.5 / CANN 9.0.0）
SQLCIPHER = {
    "cipher_page_size": 4096,
    "kdf_iter": 4000,
    "cipher_hmac_algorithm": "HMAC_SHA1",
    "cipher_kdf_algorithm": "PBKDF2_HMAC_SHA512",
}

# 已知表名（用于验证解密成功，避免 count(*) 假阳性）
KNOWN_TABLES = {"group_msg_table", "c2c_msg_table"}

DEFAULT_CONFIG = {
    "群号": ["901064769"],
    "群名映射": {"901064769": "CANN挑战赛-西南赛区"},
    "数据库相对路径": DB_REL,
    "uid 相对路径": UID_REL,
    "临时目录": None,
    "保留解密库": False,
}


def load_config() -> dict:
    if CONFIG_PATH.exists():
        with open(CONFIG_PATH, encoding="utf-8") as f:
            return {**DEFAULT_CONFIG, **json.load(f)}
    return DEFAULT_CONFIG.copy()


# ============================ 工具 ============================

def log(msg: str, level: str = "info") -> None:
    icons = {"info": "·", "ok": "✓", "warn": "!", "err": "✗", "step": "▸"}
    print(f"{icons.get(level, '·')} {msg}", flush=True)


def run(cmd: list[str], timeout: int = 300) -> tuple[int, str, str]:
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    return p.returncode, p.stdout.strip(), p.stderr.strip()


def human(n: float) -> str:
    for unit in ("B", "KB", "MB", "GB", "TB"):
        if n < 1024:
            return f"{n:.1f}{unit}"
        n /= 1024
    return f"{n:.1f}PB"


# ============================ 设备 ============================

def check_adb() -> str:
    if not shutil.which("adb"):
        sys.exit("✗ 未找到 adb。请先安装 Android Platform Tools 并加入 PATH。")
    code, out, _ = run(["adb", "devices"])
    devices = [ln.split("\t")[0] for ln in out.splitlines()[1:]
                if ln.strip() and "\tdevice" in ln]
    if not devices:
        sys.exit("✗ 未检测到已连接的 Android 设备。\n"
                 "  请：1) 用数据线连手机  2) 在手机「开发者选项」开启 USB 调试  "
                 "3) 在弹窗中点「允许」")
    if len(devices) > 1:
        log(f"检测到 {len(devices)} 台设备，默认使用第一台：{devices[0]}", "warn")
    return devices[0]


def check_root(serial: str) -> None:
    code, out, _ = run(["adb", "-s", serial, "shell", "id"])
    if "uid=0" in out:
        log("root 权限正常", "ok")
        return
    code, out, _ = run(["adb", "-s", serial, "shell", "su", "-c", "id"])
    if "uid=0" in out:
        log("root 权限正常（su）", "ok")
        return
    sys.exit("✗ 未获得 root 权限。\n"
             "  本工具需要 root 读取 QQ 私有数据库。\n"
             "  请确认已安装 KernelSU / Magisk 并对 shell 授予授权。")


def shell_su(serial: str, cmd: str, timeout: int = 600) -> tuple[int, str]:
    """以 su 身份执行 shell 命令"""
    p = subprocess.run(
        ["adb", "-s", serial, "shell", f"su -c '{cmd}'"],
        capture_output=True, text=True, timeout=timeout)
    return p.returncode, (p.stdout + p.stderr).strip()


# ============================ 密钥推导 ============================

def derive_key(db_head: bytes, nt_uid: str) -> tuple[str, str, str]:
    """
    从文件头推导 SQLCipher 密钥。

    文件头布局（本机三个账号实测一致）：
      0x20 "QQ_NT DB" | 0x28 24 00 00 00 | 0x2C 12 08
      0x2E <8 bytes rand> ← 密钥随机串 | 0x42 "HMAC_SHA1"

    Returns: (key, rand, hmac_algorithm)
    """
    if db_head[:16] != b"SQLite header 3\x00":
        raise ValueError("不是 SQLite 文件")
    if db_head[32:40] != b"QQ_NT DB":
        raise ValueError("缺少 QQ_NT DB 标记，不是 NTQQ 数据库")

    rand = db_head[0x2E:0x2E + 8].decode("ascii", "replace")
    algo = None
    for cand in (b"HMAC_SHA512", b"HMAC_SHA256", b"HMAC_SHA1"):
        if cand in db_head:
            algo = cand.decode()
            break
    if not algo:
        algo = "HMAC_SHA1"

    uin_hash = hashlib.md5(nt_uid.encode()).hexdigest()
    key = hashlib.md5((uin_hash + rand).encode()).hexdigest()
    return key, rand, algo


def derive_path_hash(nt_uid: str) -> str:
    return hashlib.md5((hashlib.md5(nt_uid.encode()).hexdigest()
                        + "nt_kernel").encode()).hexdigest()


def find_accounts(serial: str, uid_rel: str) -> list[dict]:
    """列出设备上所有 QQ 账号及其 nt_uid"""
    code, out = shell_su(serial, f"ls {uid_rel} 2>/dev/null")
    if code != 0 or not out:
        return []
    accounts = []
    for line in out.splitlines():
        line = line.strip()
        if "###" not in line:
            continue
        uin, nt_uid = line.split("###", 1)
        if uin.isdigit() and nt_uid:
            accounts.append({"uin": uin, "nt_uid": nt_uid,
                             "path_hash": derive_path_hash(nt_uid)})
    return accounts


# ============================ 拉取 ============================

def fetch_database(serial: str, remote: str, dest: Path) -> Path:
    """root 复制到共享存储再 pull（adb pull 无 root 会 Permission denied）"""
    log(f"复制 {Path(remote).name} …")
    t0 = time.time()
    shell_su(serial,
             f"cp '{remote}' /sdcard/Download/_cann_tmp.db && "
             f"cp '{remote}-wal' /sdcard/Download/_cann_tmp.db-wal 2>/dev/null; "
             f"cp '{remote}-shm' /sdcard/Download/_cann_tmp.db-shm 2>/dev/null; "
             f"chmod 644 /sdcard/Download/_cann_tmp.db*")
    for suffix in ("", "-wal", "-shm"):
        r = Path(f"/sdcard/Download/_cann_tmp.db{suffix}")
        try:
            code, _, _ = run(["adb", "-s", serial, "shell", f"ls -la {r}"], 60)
            if code != 0:
                continue
            run(["adb", "-s", serial, "pull", str(r), f"{dest}{suffix}"], 1800)
        finally:
            run(["adb", "-s", serial, "shell", f"rm -f {r}"], 60)

    if not dest.exists():
        sys.exit(f"✗ 数据库拉取失败：{remote}")

    size = dest.stat().st_size
    dt = time.time() - t0
    log(f"已拉取 {human(size)}（{dt:.0f}s）", "ok")
    return dest


def strip_header(src: Path, dst: Path) -> None:
    """剥掉 1024 字节 QQ 自定义头"""
    total = src.stat().st_size
    with open(src, "rb") as fi, open(dst, "wb") as fo:
        fi.seek(1024)
        done = 0
        while chunk := fi.read(1 << 24):
            fo.write(chunk)
            done += len(chunk)
            print(f"\r  剥头 {done/(total-1024)*100:5.1f}%  "
                  f"{human(done)}", end="", flush=True)
    print()


# ============================ 解密与查询 ============================

def connect(clear_db: Path, key: str, hmac: str):
    """建立 SQLCipher 连接"""
    try:
        import sqlcipher3
    except ImportError:
        sys.exit("✗ 缺少 sqlcipher3 库。\n"
                 "  安装：pip install sqlcipher3\n"
                 "  macOS 若无预编译轮子，可用：\n"
                 "    pip install --no-cache-dir \\\n"
                 "      https://files.pythonhosted.org/packages/56/0d/"
                 "2cee40de57d47245de09382c64e649c8cc8e86fa549ecba7591633fabf20/"
                 "sqlcipher3-0.6.2-cp313-cp313-macosx_11_0_arm64.whl")

    con = sqlcipher3.connect(str(clear_db))
    cur = con.cursor()
    cur.execute(f"PRAGMA cipher_page_size = {SQLCIPHER['cipher_page_size']}")
    cur.execute(f"PRAGMA key = '{key}'")
    cur.execute(f"PRAGMA kdf_iter = {SQLCIPHER['kdf_iter']}")
    cur.execute(f"PRAGMA cipher_hmac_algorithm = {hmac}")
    cur.execute(f"PRAGMA cipher_kdf_algorithm = "
                f"{SQLCIPHER['cipher_kdf_algorithm']}")
    return con, cur


def verify_decrypted(cur) -> set[str]:
    """
    验证解密成功。必须实际读到已知表名——
    错误 key 时 count(*) FROM sqlite_master 返回 0 而不报错，会造成假阳性。
    """
    names = {r[0] for r in cur.execute(
        "SELECT name FROM sqlite_master WHERE type='table'").fetchall()}
    if not (names & KNOWN_TABLES):
        raise ValueError(
            f"解密失败：未找到已知表 {KNOWN_TABLES}（实际读到 {len(names)} 张表）")
    return names


def fetch_group_messages(cur, group: int, since_rowid: int = 0):
    """
    按 rowid 游标分页拉取指定群消息。

    两个必须注意的点：
      - 40027 才是群号（40030 是空列，官方文档有误）
      - 库有损坏页，不能全表扫描；rowid 游标 + 坏页跳过

    Yields: (rowid, time, sender_qq, card, nick, direction, body_bytes)
    """
    SQL = (
        'SELECT rowid, "40050", "40013", "40033", "40090", "40093", "40800" '
        f'FROM group_msg_table WHERE "40027"=? AND rowid>? '
        f'AND ("40050" IS NULL OR "40050">0) '
        f'ORDER BY rowid LIMIT ?'
    )
    last = since_rowid
    batch = 2000
    skipped = 0
    while True:
        try:
            rows = cur.execute(SQL, (group, last, batch)).fetchall()
        except Exception:
            skipped += 1
            if skipped > 500:
                log("损坏页过多，提前停止", "warn")
                break
            last += 1          # 跳过当前坏页
            continue
        if not rows:
            break
        for r in rows:
            yield r
        last = rows[-1][0]


# ============================ 消息解析 ============================

def read_varint(buf, i):
    shift = val = 0
    n = len(buf)
    while i < n:
        b = buf[i]
        val |= (b & 0x7F) << shift
        i += 1
        if not b & 0x80:
            return val, i
        shift += 7
        if shift > 63:
            break
    return val, i


def parse_fields(buf):
    out, i = {}, 0
    while i < len(buf):
        k, i = read_varint(buf, i)
        fn, wt = k >> 3, k & 7
        if wt == 0:
            v, i = read_varint(buf, i)
            out.setdefault(fn, []).append(("v", v))
        elif wt == 2:
            ln, i = read_varint(buf, i)
            if ln < 0 or i + ln > len(buf):
                break
            out.setdefault(fn, []).append(("b", buf[i:i + ln]))
            i += ln
        elif wt == 5:
            out.setdefault(fn, []).append(("f", buf[i:i + 4])); i += 4
        elif wt == 1:
            out.setdefault(fn, []).append(("f", buf[i:i + 8])); i += 8
        else:
            break
    return out


TYPE_NAMES = {1: "文本", 2: "图片", 3: "文件", 4: "语音", 5: "视频", 6: "表情",
              7: "回复", 8: "系统提示", 10: "应用", 11: "自定义表情",
              16: "分享", 21: "电话", 26: "动态消息"}


def decode_content(body: bytes) -> tuple[list[str], set[int]]:
    """从 40800 protobuf 提取文本与消息类型"""
    texts, types = [], set()
    for kind, val in parse_fields(body or b"").get(40800, []):
        if kind != "b":
            continue
        el = parse_fields(val)
        for k2, v2 in el.get(45002, []):
            if k2 == "v":
                types.add(v2)
        for fn in (45101, 48214, 48271):
            for k2, v2 in el.get(fn, []):
                if k2 == "b":
                    try:
                        s = v2.decode("utf-8").strip()
                        if s:
                            texts.append(s)
                    except UnicodeDecodeError:
                        pass
    return texts, types


def ts(v) -> str:
    try:
        v = int(v)
        return datetime.fromtimestamp(v).strftime("%Y-%m-%d %H:%M:%S")
    except Exception:
        return ""


# ============================ 提分筛选 ============================

EXCLUDE = re.compile(r"""
    违规|作弊|封禁|禁赛|取消.*成绩|异常提交|探针|probe|空\s*kernel|空marker|
    死代码|占位|绕过|篡改|组队|队友|人数限制|报名|接龙|抽奖|获奖名单|卡时|
    资源申请|申请算力|宣讲会|秋招|群管家|截止时间|邀请链接|单人成队|队长管理|
    守门员|斩杀线|冒泡|防踢|哈基米|宝宝|666|摸鱼|奶茶|脱单|熬夜|睡觉|好吃|
    外卖|运气|玄学|抽卡|彩票|astra大人|aatra|优化清单|求带|带我飞|收我|吹牛|
    招聘|简历|岗位|薪资|投递|实习生|校园招聘|就业|图灵业务部|昇腾社区|微软|
    绿卡|面经|面试|机考|Premium|Subscribe|【Kernel】|哈基米的吟唱|核爆一般|
    人民政府|新闻发布会|_byte|新闻发布|市长|书记|调研|考察|座谈|莅临|指导工作
""", re.I | re.X)

NOISE = re.compile(r"""
    秘密报告|opus5|fable会回退|左脑互搏|vscode风格|物理学不存在|放我一条生路|
    活路喵|口牙|唯一的优势|纯plus聊天|大家用gpt|全都pass才算|我怕伏笔|多少加速比|
    大肥鱼|家宽注册|Ask Grok|小红书|Opus不是写|榜单都是|分号|；$|^[.。，,]|^$|^\s+$|
    好的?$|收到$|^ok$
""", re.I | re.X)

CASE = re.compile(r"case\s*(\d{1,2})|case(\d{1,2})|测点\s*(\d+)|测试点\s*(\d+)", re.I)

KERNEL = re.compile(r"""
    归约|reduce|归一化|normaliz|rms|tile|TILE|tiling|分块|块化|对齐|align|尾块|
    tail|pad|精度|舍入|CAST_RINT|rint|fp32|fp16|bf16|溢出|误差|流水线|pipeline|
    双缓冲|重叠|overlap|融合|fuse|DataCopy|搬运|带宽|bandwidth|l2|L2|缓存|hbm|HBM|
    handoff|barrier|PipeBarrier|指令|instruction|向量核|核数|核分配|算子|kernel|
    op_kernel|AscendC|直调|注册|编译|时延|耗时|吞吐|加速|开销|性能|极限|物理|
    平台期|瓶颈|调度|放置|dtype|形状|shape|写死|特判|分支|展开|复用|驻留|UB|
    寄存器|metadata|cube|vector|prof|msProf|加速比|并行|流水
""", re.I | re.X)

SOLUTION = re.compile(r"""
    优化|改进|方案|思路|做法|实现|尝试|改成|换成|拆|合并|复用|预处理|后处理|提取|
    分离|跳过|消除|降低|减少|避免|利用|借助|通过|根据|针对|混合|反推|探测|打表|
    穷举|试出来|测出来|观察|对比|分析|定位|排查|验证|实测|试出来
""", re.I | re.X)

PITFALL = re.compile(r"""
    反而|更差|掉了|下降|退化|回退|没用|不起作用|没改善|没提升|没变化|无效|失败|
    浪费|白费|方向不对|思路错|坑|误杀|被否|卡住|卡在|瓶颈期|不理想|达不到
""", re.I | re.X)

BENCH = re.compile(r"\d+\.?\d*\s*(us|µs|微妙|微秒|ms|ns)", re.I)
# 平台最优/标杆数值（如 "平台最优 3750.12"、"稳定1.7左右"）也算标杆值
BENCH2 = re.compile(r"(平台\s*最优|最优|best|tbest|标杆|最好)\s*[:：]?\s*\d+\.?\d*"
                    r"|\d+\.\d+\s*(左右|以上|以内)", re.I)
TBEST = re.compile(r"tbest|计分机制|噪声带|上限|榜单|刷分|通缉|分能反馈", re.I)


def screen(text: str) -> tuple[bool, str, int, str]:
    """返回 (是否提分相关, case, 得分, 标签)"""
    t = text.strip()
    if not t or len(t) < 6:
        return False, "", 0, ""
    clean = re.sub(r"@[^@\s]{1,32}(（[^）]*）)?", "", t)   # 剥离 @昵称（已报名）
    clean = re.sub(r"（已报名）|（未报名）|\(已报名\)", "", clean)
    if EXCLUDE.search(clean) or NOISE.search(clean):
        return False, "", 0, ""

    score, tags, case = 0, [], ""
    m = CASE.search(t)
    if m:
        num = m.group(1) or m.group(2) or m.group(3) or m.group(4)
        if num and 1 <= int(num) <= 15:
            case = f"case{num}"
            score += 3
            tags.append(case)
    if KERNEL.search(clean):
        ops = len(set(x.group(0).lower() for x in KERNEL.finditer(clean)))
        score += min(6, ops * 2)
        tags.append(f"算子×{ops}")
    if TBEST.search(clean):
        score += 3
        tags.append("计分机制")
    if SOLUTION.search(clean):
        score += 2
        tags.append("解法")
    if PITFALL.search(clean):
        score += 2
        tags.append("避坑")
    if BENCH.search(clean) or BENCH2.search(clean):
        score += 2
        tags.append("标杆值")
    if len(t) < 10 and not (case or BENCH.search(clean) or BENCH2.search(clean)):
        score -= 3
    elif len(t) > 50:
        score += 1
    return score >= 4, case, score, "、".join(tags)


# ============================ 增量状态 ============================

def load_state() -> dict:
    if STATE_PATH.exists():
        with open(STATE_PATH, encoding="utf-8") as f:
            return json.load(f)
    return {"群": {}, "首次运行": None}


def save_state(state: dict) -> None:
    STATE_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(STATE_PATH, "w", encoding="utf-8") as f:
        json.dump(state, f, ensure_ascii=False, indent=2)


# ============================ 主流程 ============================

def process_group(cur, group: int, state: dict, full: bool,
                  do_screen: bool, out_dir: Path) -> dict:
    """处理单个群，返回统计"""
    gkey = str(group)
    gstate = state["群"].get(gkey, {})
    since = 0 if full else int(gstate.get("last_rowid", 0))
    # 双重保险：rowid 可能因删除被复用（SQLite 会复用已删的最大 rowid），
    # 因此同时用时间戳兜底——只有 rowid 更大且时间不早于上次的才算新增。
    since_ts = 0 if full else int(gstate.get("last_time", 0))

    log(f"群 {group}：{'全量提取' if full else f'增量（rowid > {since}）'}")

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    all_csv = out_dir / f"group_{group}_all_{stamp}.csv"
    picked_csv = out_dir / f"group_{group}_提分_{stamp}.csv"

    cols = ["rowid", "时间", "发送者", "方向", "所属case", "标签", "内容"]
    n_all = n_pick = 0
    max_rowid = since
    max_time = since_ts
    ts_range = [None, None]

    fa = open(all_csv, "w", encoding="utf-8-sig", newline="")
    fp = open(picked_csv, "w", encoding="utf-8-sig", newline="")
    wa = csv.writer(fa);wa.writerow(cols)
    wp = csv.writer(fp); wp.writerow(["case", "时间", "发送者", "标签", "得分", "原文"])

    try:
        for row in fetch_group_messages(cur, group, since):
            rowid, t, direction, sqq, card, nick, body = row
            texts, types = decode_content(body or b"")
            t_s = ts(t)
            name = card or nick or ("系统" if direction == 3 else "")
            if not texts:
                tn = "/".join(sorted(TYPE_NAMES.get(x, f"type{x}")
                                     for x in types)) or "空"
                texts = [f"[{tn}]"]

            max_rowid = max(max_rowid, rowid)
            try:
                max_time = max(max_time, int(t))
            except (TypeError, ValueError):
                pass
            if ts_range[0] is None:
                ts_range[0] = t_s
            ts_range[1] = t_s

            joined = " / ".join(texts)
            ok, case, score, tag = screen(joined) if do_screen else (False, "", 0, "")
            wa.writerow([rowid, t_s, name, direction, case, tag, joined])
            n_all += 1
            if ok:
                wp.writerow([case, t_s, name, tag, score, joined])
                n_pick += 1
    finally:
        fa.close(); fp.close()

    state["群"][gkey] = {
        "last_rowid": max_rowid,
        "last_time": max_time,
        "last_run": datetime.now().isoformat(timespec="seconds"),
        "累计提取": gstate.get("累计提取", 0) + n_all,
        "上次新增": n_all,
        "上次提分": n_pick,
        "全量文件": str(all_csv),
        "提分文件": str(picked_csv),
    }
    log(f"  提取 {n_all} 条，其中提分相关 {n_pick} 条", "ok")
    return {"群": group, "提取": n_all, "提分": n_pick,
            "时间范围": ts_range, "全量": all_csv, "提分文件": picked_csv}


def main() -> None:
    ap = argparse.ArgumentParser(
        description="CANN 群聊增量提取器",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__)
    ap.add_argument("--group", type=int, action="append",
                    help="群号，可重复指定；默认用config.json 里的全部")
    ap.add_argument("--full", action="store_true", help="全量重新提取（忽略增量水位）")
    ap.add_argument("--no-screen", action="store_true", help="只提取，不做提分筛选")
    ap.add_argument("--keep-db", action="store_true", help="保留解密后的数据库")
    ap.add_argument("--status", action="store_true", help="查看增量状态")
    ap.add_argument("--reset", action="store_true", help="清除增量状态")
    args = ap.parse_args()

    cfg = load_config()

    if args.reset:
        if STATE_PATH.exists():
            STATE_PATH.unlink()
            print("✓ 增量状态已清除")
        return

    if args.status:
        st = load_state()
        if st["首次运行"] is None and not st["群"]:
            print("尚无运行记录")
            return
        print(f"首次运行: {st.get('首次运行') or '—'}")
        for g, s in st["群"].items():
            print(f"\n群 {g}:")
            print(f"  水位 rowid : {s.get('last_rowid')}")
            print(f"  上次运行   : {s.get('last_run')}")
            print(f"  累计提取   : {s.get('累计提取')}")
            print(f"  上次新增   : {s.get('上次新增')}")
            print(f"  上次提分   : {s.get('上次提分')}")
        return

    print("=" * 56)
    print("  CANN 群聊增量提取器")
    print("=" * 56)

    log("检查设备…", "step")
    serial = check_adb()
    log(f"设备 {serial}")
    check_root(serial)

    log("查找 QQ 账号…", "step")
    accounts = find_accounts(serial, cfg["uid 相对路径"])
    if not accounts:
        sys.exit("✗ 未找到 QQ 账号。请确认手机上已登录 QQ，且应用包名为 "
                 f"{ANDROID_PKG}")
    for a in accounts:
        log(f"  {a['uin']}  nt_qq_{a['path_hash'][:16]}…")

    groups = args.group or cfg["群号"]
    state = load_state()
    if state.get("首次运行") is None:
        state["首次运行"] = datetime.now().isoformat(timespec="seconds")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    tmp_dir = Path(cfg["临时目录"]) if cfg.get("临时目录") \
        else Path(tempfile.mkdtemp(prefix="cann_extract_"))

    try:
        # 找出需要的库目录
        for acct in accounts:
            db_dir = f"{cfg['数据库相对路径']}/nt_qq_{acct['path_hash']}"
            code, _ = shell_su(serial, f"ls {db_dir} >/dev/null 2>&1")
            if code != 0:
                continue
            if not (state["群"] or args.full):
                pass
            log(f"账号 {acct['uin']} 数据库目录存在", "ok")

            # 找到含目标群的库：先看 group_info
            log("拉取群信息库以定位群号…", "step")
            gi = fetch_database(serial, f"{db_dir}/group_info.db", tmp_dir / "gi.db")
            with open(gi, "rb") as f:
                head = f.read(1024)
            try:
                gi_key, _, gi_algo = derive_key(head, acct["nt_uid"])
            except ValueError:
                continue
            clear = tmp_dir / "gi.clear.db"
            strip_header(gi, clear)
            con, cur = connect(clear, gi_key, gi_algo)
            cols = [d[0] for d in cur.execute(
                "SELECT * FROM group_list LIMIT 1").description]
            uin_c, name_c = "60001", "60007"
            if uin_c in cols and name_c in cols:
                ui, ni = cols.index(uin_c), cols.index(name_c)
                for r in cur.execute("SELECT * FROM group_list").fetchall():
                    g = r[ui]
                    if g in groups:
                        cfg["群名映射"][str(g)] = r[ni] or ""
                        log(f"  群 {g} = {r[ni]}", "ok")
            con.close()
            gi.unlink(missing_ok=True); clear.unlink(missing_ok=True)

            # 拉主库
            log(f"拉取主数据库（可能需要几分钟）…", "step")
            db = fetch_database(serial, f"{db_dir}/nt_msg.db", tmp_dir / "msg.db")
            with open(db, "rb") as f:
                head = f.read(1024)
            key, rand, hmac = derive_key(head, acct["nt_uid"])
            log(f"rand={rand}  key={key[:16]}…  hmac={hmac}")

            clear_db = tmp_dir / "msg.clear.db"
            log("剥除 1024 字节头…", "step")
            strip_header(db, clear_db)

            con, cur = connect(clear_db, key, hmac)
            try:
                names = verify_decrypted(cur)
            except ValueError as e:
                log(str(e), "err")
                con.close()
                continue
            log(f"解密成功，{len(names)} 张表", "ok")

            for g in groups:
                res = process_group(cur, g, state, args.full,
                                    not args.no_screen, OUT_DIR)
                name = cfg["群名映射"].get(str(g), "")
                print()
                print(f"  ── 群 {g} {name} ──")
                print(f"     提取 {res['提取']} 条｜提分相关 {res['提分']} 条")
                if res["时间范围"][0]:
                    print(f"     时间   {res['时间范围'][0]} ~ {res['时间范围'][1]}")
                print(f"     输出   {res['全量'].name}")
                print(f"            {res['提分文件'].name}")
            con.close()

            if not cfg.get("保留解密库") and not args.keep_db:
                db.unlink(missing_ok=True)
                clear_db.unlink(missing_ok=True)
                for s in ("-wal", "-shm"):
                    Path(f"{db}{s}").unlink(missing_ok=True)
                    Path(f"{clear_db}{s}").unlink(missing_ok=True)
    finally:
        if not cfg.get("保留解密库") and not args.keep_db:
            shutil.rmtree(tmp_dir, ignore_errors=True)

    save_state(state)
    total_all = sum(s.get("上次提取", 0) for s in state["群"].values())
    total_pick = sum(s.get("上次提分", 0) for s in state["群"].values())
    print()
    print("=" * 56)
    print(f"  完成：本次提取 {total_all} 条，提分相关 {total_pick} 条")
    print(f"  输出目录：{OUT_DIR}")
    print(f"  增量状态：{STATE_PATH}")
    print("=" * 56)
    print("\n下次直接运行同一命令即可，只提取新增消息。")


if __name__ == "__main__":
    main()

# ============================ 清理 ============================

def purge(paths) -> None:
    """删除文件或目录，忽略不存在的情况。"""
    for p in paths:
        try:
            p = Path(p)
            if p.is_dir():
                shutil.rmtree(p, ignore_errors=True)
            elif p.exists():
                p.unlink(missing_ok=True)
        except Exception as e:
            log(f"清理失败 {p}: {e}", "warn")


def purge_device(serial: str) -> None:
    """清理设备端临时文件。"""
    for name in ("_cann_tmp.db", "_cann_tmp.db-wal", "_cann_tmp.db-shm"):
        run(["adb", "-s", serial, "shell", f"rm -f /sdcard/Download/{name}"], 60)


# ============================ 主流程 ============================

def main() -> None:
    ap = argparse.ArgumentParser(
        description="CANN 群聊增量提取器（只提取指定群，用完即删）",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__)
    ap.add_argument("--group", type=int, action="append",
                    help="群号，可重复指定；默认用 config.json 里的")
    ap.add_argument("--full", action="store_true", help="全量重新提取（忽略增量水位）")
    ap.add_argument("--no-screen", action="store_true", help="只提取，不做提分筛选")
    ap.add_argument("--keep-db", action="store_true",
                    help="保留解密后的数据库（调试用，占 4GB+）")
    ap.add_argument("--status", action="store_true", help="查看增量状态")
    ap.add_argument("--reset", action="store_true", help="清除增量状态")
    args = ap.parse_args()

    cfg = load_config()

    if args.reset:
        if STATE_PATH.exists():
            STATE_PATH.unlink()
            print("✓ 增量状态已清除")
        return

    if args.status:
        st = load_state()
        if st.get("首次运行") is None and not st["群"]:
            print("尚无运行记录")
            return
        print(f"首次运行: {st.get('首次运行') or '—'}")
        for g, s in st["群"].items():
            name = cfg["群名映射"].get(g, "")
            print(f"\n群 {g} {name}:")
            print(f"  水位 rowid : {s.get('last_rowid')}")
            print(f"  水位 时间   : {s.get('last_time')}")
            print(f"  上次运行   : {s.get('last_run')}")
            print(f"  累计提取   : {s.get('累计提取')}")
            print(f"  上次新增   : {s.get('上次新增')}")
            print(f"  上次提分   : {s.get('上次提分')}")
        return

    groups = args.group or cfg["群号"]
    if not groups:
        sys.exit("✗ 未指定群号。请用 --group <群号> 或在 config.json 里配置。")

    print("=" * 58)
    print("  CANN 群聊增量提取器")
    print("=" * 58)
    log(f"目标群: {groups}")

    log("检查设备…", "step")
    serial = check_adb()
    log(f"设备 {serial}")
    check_root(serial)

    log("查找 QQ 账号…", "step")
    accounts = find_accounts(serial, cfg["uid 相对路径"])
    if not accounts:
        sys.exit("✗ 未找到 QQ 账号。请确认手机已登录 QQ。")
    for a in accounts:
        log(f"  {a['uin']}  nt_qq_{a['path_hash'][:16]}…")

    state = load_state()
    if state.get("首次运行") is None:
        state["首次运行"] = datetime.now().isoformat(timespec="seconds")

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    tmp_dir = Path(cfg["临时目录"]) if cfg.get("临时目录") \
        else Path(tempfile.mkdtemp(prefix="cann_extract_"))
    keep = bool(cfg.get("保留解密库")) or args.keep_db

    #所有需要在结束时清理的本地文件
    temp_files = list(tmp_dir.glob("*")) if tmp_dir.exists() else []
    done_groups = []

    try:
        for acct in accounts:
            db_dir = f"{cfg['数据库相对路径']}/nt_qq_{acct['path_hash']}"
            code, _ = shell_su(serial, f"ls {db_dir} >/dev/null 2>&1")
            if code != 0:
                continue

            # ---- 先用群信息库筛出「哪个账号有目标群」，避免白拉 4.3GB ----
            log(f"检查账号 {acct['uin']} 是否包含目标群…", "step")
            gi = fetch_database(serial, f"{db_dir}/group_info.db",
                                tmp_dir / "gi.db")
            temp_files += [gi, tmp_dir / "gi.db-wal", tmp_dir / "gi.db-shm"]
            with open(gi, "rb") as f:
                head = f.read(1024)
            try:
                gi_key, _, gi_algo = derive_key(head, acct["nt_uid"])
            except ValueError:
                continue
            gi_clear = tmp_dir / "gi.clear.db"
            temp_files.append(gi_clear)
            strip_header(gi, gi_clear)
            con, cur = connect(gi_clear, gi_key, gi_algo)
            try:
                cols = [d[0] for d in cur.execute(
                    "SELECT * FROM group_list LIMIT 1").description]
                present = set()
                if "60001" in cols and "60007" in cols:
                    ui, ni = cols.index("60001"), cols.index("60007")
                    for r in cur.execute("SELECT * FROM group_list").fetchall():
                        present.add(r[ui])
                        if r[ui] in groups:
                            cfg["群名映射"][str(r[ui])] = r[ni] or ""
            finally:
                con.close()
            # 群信息库已用完，立即删
            purge([gi, gi_clear, tmp_dir / "gi.db-wal", tmp_dir / "gi.db-shm"])

            hit = [g for g in groups if g in present]
            if not hit:
                log(f"账号 {acct['uin']} 不含目标群，跳过", "warn")
                continue

            for g in hit:
                log(f"  群 {g} = {cfg['群名映射'].get(str(g), '')}", "ok")

            # ---- 拉主库（只针对含目标群的账号）----
            log("拉取主数据库（大文件，需几分钟）…", "step")
            db = fetch_database(serial, f"{db_dir}/nt_msg.db",
                                tmp_dir / "msg.db")
            temp_files += [db, tmp_dir / "msg.db-wal", tmp_dir / "msg.db-shm",
                           tmp_dir / "msg.clear.db", tmp_dir / "msg.clear.db-wal",
                           tmp_dir / "msg.clear.db-shm"]

            with open(db, "rb") as f:
                head = f.read(1024)
            key, rand, hmac = derive_key(head, acct["nt_uid"])
            log(f"rand={rand}  key={key[:16]}…  hmac={hmac}")

            clear_db = tmp_dir / "msg.clear.db"
            log("剥除 1024 字节头…", "step")
            strip_header(db, clear_db)

            con, cur = connect(clear_db, key, hmac)
            try:
                try:
                    names = verify_decrypted(cur)
                except ValueError as e:
                    log(str(e), "err")
                    continue
                log(f"解密成功，{len(names)} 张表", "ok")

                for g in hit:
                    res = process_group(cur, g, state, args.full,
                                        not args.no_screen, OUT_DIR)
                    name = cfg["群名映射"].get(str(g), "")
                    print()
                    print(f"  ── 群 {g} {name} ──")
                    print(f"     提取 {res['提取']} 条｜提分相关 {res['提分']} 条")
                    if res["时间范围"][0]:
                        print(f"     时间   {res['时间范围'][0]} ~ "
                              f"{res['时间范围'][1]}")
                    print(f"     输出   {res['全量'].name}")
                    print(f"            {res['提分文件'].name}")
                    done_groups.append(g)
            finally:
                con.close()

            if keep:
                log(f"--keep-db 已启用，保留解密库于 {clear_db}", "warn")
            else:
                # 拉下来的密文+剥头副本，用完即删
                purge([db, clear_db, tmp_dir / "msg.db-wal",
                       tmp_dir / "msg.db-shm", tmp_dir / "msg.clear.db-wal",
                       tmp_dir / "msg.clear.db-shm"])
                log("本次数据库已删除", "ok")

    finally:
        # 无论成功、失败还是 Ctrl-C，都清理干净
        if not keep:
            purge(temp_files)
            if tmp_dir.exists():
                shutil.rmtree(tmp_dir, ignore_errors=True)
            purge_device(serial)
            log("临时文件已清理", "ok")
        else:
            log(f"临时文件保留于 {tmp_dir}", "warn")

    save_state(state)

    if not done_groups:
        print()
        print("⚠ 未提取到任何目标群消息。请确认群号是否正确。")
        return

    n_all = sum(state["群"][str(g)].get("上次新增", 0) for g in done_groups)
    n_pick = sum(state["群"][str(g)].get("上次提分", 0) for g in done_groups)
    print()
    print("=" * 58)
    print(f"  完成：本次提取 {n_all} 条，提分相关 {n_pick} 条")
    print(f"  输出目录：{OUT_DIR}")
    print(f"  增量状态：{STATE_PATH}")
    if not keep:
        print("  数据库：已用完即删（未留副本）")
    print("=" * 58)
    print("\n下次直接运行同一命令即可，只提取新增消息。")


if __name__ == "__main__":
    main()
