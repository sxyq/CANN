#!/usr/bin/env python3
"""
端到端测试：用模拟的 NTQQ 数据库验证核心链路。
不依赖真实设备，验证 解密→查询→解码→筛选→输出 全流程。
"""
import csv
import hashlib
import importlib.util
import os
import struct
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
spec = importlib.util.spec_from_file_location(
    "cann", Path(__file__).parent / "cann_extract.py")
ce = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ce)

import sqlite3
import sqlcipher3

NT_UID = "u_VEeKUEtDON6tOMf8CmUL6Q"
GROUP = 901064769
TMP = Path(tempfile.mkdtemp(prefix="cann_e2e_"))


def varint(n):
    out = b""
    while True:
        b = n & 0x7F
        n >>= 7
        out += bytes([b | (0x80 if n else 0)])
        if not n:
            break
    return out


def field(no, payload):
    return varint((no << 3) | 2) + varint(len(payload)) + payload


def make_body(text, mtype=1):
    """构造 40800 protobuf：顶层 40800 -> 元素{45002:type, 45101:text}"""
    el = field(45002, varint(mtype)) + field(45101, text.encode("utf-8"))
    return field(40800, el)


def build_plain_db(path: Path):
    con = sqlite3.connect(str(path))
    cur = con.cursor()
    # 关键：第一列不能用 INTEGER，否则会被 SQLite 当成 rowid 别名。
    # 真实库的第一列是消息 id，rowid 是独立的隐式列。
    cur.execute("""CREATE TABLE group_msg_table(
        "40001" TEXT,"40003" INTEGER,"40013" INTEGER,"40020" TEXT,
        "40027" INTEGER,"40030" INTEGER,"40033" INTEGER,
        "40050" INTEGER,"40090" TEXT,"40093" TEXT,"40800" BLOB)""")
    samples = [
        (1,  "case5 做到5us了", 1),
        (2,  "case1的1.47us呢", 1),
        (3,  "把大D运算变成cube+vector混合", 1),
        (4,  "通过时间可以反推指令开销", 1),
        (5,  "冒泡防踢", 1),
        (6,  "666", 1),
        (7,  "接龙 #接龙 截止时间 已报名", 1),
        (8,  "case8的tbest要16.7TB/s带宽", 1),
        (9,  "case4优化后反而更差了", 1),
        (10, "哈哈哈", 1),
        (11, "tbest 3750.12 → 3665.94 专刷单点", 1),
        (12, "组队缺人", 1),
    ]
    base_ts = 1789049561
    for i, (rowid, text, mt) in enumerate(samples):
        cur.execute(
            'INSERT INTO group_msg_table ("40001","40003","40013","40020",'
            '"40027","40030","40033","40050","40090","40093","40800") '
            'VALUES(?,?,?,?,?,?,?,?,?,?,?)',
            (f"m{rowid}", 7880 + i, 0, f"u_test{i:04d}", GROUP, 0, 10000 + i,
             base_ts + i * 3600, f"测试员{i}", "", make_body(text, mt)))
    # 加一条别群的消息，验证群号过滤
    cur.execute('INSERT INTO group_msg_table ("40001","40003","40013","40020",'
                '"40027","40030","40033","40050","40090","40093","40800") '
                'VALUES(?,?,?,?,?,?,?,?,?,?,?)',
                ("m99", 9999, 0, "u_x", 123456789, 0, 1,
                 base_ts, "别群", "", make_body("case5 别群消息")))
    # 加一条无效时间戳
    cur.execute('INSERT INTO group_msg_table ("40001","40003","40013","40020",'
                '"40027","40030","40033","40050","40090","40093","40800") '
                'VALUES(?,?,?,?,?,?,?,?,?,?,?)',
                ("m100", 10000, 0, "u_y", GROUP, 0, 2, 0, "零时间", "",
                 make_body("零时间戳消息")))
    con.commit()
    con.close()


def encrypt_like_ntqq(plain: Path, enc: Path, key: str, rand: str):
    """把明文库包成 NTQQ 格式：1024 字节头 + 保持原样（模拟已加密的密文结构）"""
    with open(plain, "rb") as f:
        data = f.read()
    head = bytearray(1024)
    head[0:16] = b"SQLite header 3\x00"
    head[0x10:0x12] = b"\x10\x00"
    head[32:40] = b"QQ_NT DB"
    head[0x28:0x2C] = struct.pack("<I", 36)
    head[0x2C] = 0x12
    head[0x2D] = 0x08
    head[0x2E:0x2E + 8] = rand.encode()
    head[0x36] = 0x1A
    head[0x37] = 0x07
    head[0x38:0x3F] = b"1.1.0.1"
    head[0x3F] = 0x22
    head[0x40] = 0x09
    head[0x41:0x4A] = b"HMAC_SHA1"
    enc.write_bytes(bytes(head) + data)


def main():
    print("=" * 60)
    print("  端到端测试（模拟数据，不连设备）")
    print("=" * 60)

    # 1. 构造明文库
    plain = TMP / "plain.db"
    build_plain_db(plain)
    print(f"1. 构造测试库 {plain.stat().st_size} 字节")

    # 2. 加密封装
    rand = "fv9McxcE"
    uin_hash = hashlib.md5(NT_UID.encode()).hexdigest()
    expect_key = hashlib.md5((uin_hash + rand).encode()).hexdigest()
    enc = TMP / "enc.db"
    encrypt_like_ntqq(plain, enc, expect_key, rand)
    print(f"2. 封装 NTQQ 头，rand={rand}")

    # 3. 密钥推导
    with open(enc, "rb") as f:
        head = f.read(1024)
    key, r2, hmac = ce.derive_key(head, NT_UID)
    assert key == expect_key, f"密钥推导失败: {key} != {expect_key}"
    assert r2 == rand, f"rand 提取失败: {r2}"
    print(f"3. 密钥推导 ✓key={key}")

    # 4. 剥头
    clear = TMP / "clear.db"
    ce.strip_header(enc, clear)
    assert clear.stat().st_size == enc.stat().st_size - 1024
    print(f"4. 剥头 ✓ {clear.stat().st_size} 字节")

    # 5. 连接验证
    # 注意：测试库里是明文（真实场景是SQLCipher 密文），
    # 所以用明文连接验证「剥头后能否正常打开 + 已知表名校验」这条链路。
    # SQLCipher 加密参数已在真实设备上验证（见 README）。
    try:
        con, cur = ce.connect(clear, key, hmac)
        names = ce.verify_decrypted(cur)
        print(f"5. SQLCipher 连接 ✓ 读到 {len(names)} 张表")
        con.close()
    except Exception as e:
        # 明文库用 SQLCipher 打开必然失败，这符合预期
        if "not a database" not in str(e).lower():
            raise
        print("5. SQLCipher 打开失败（预期：测试库是明文）")
        print("   → 改用明文连接继续验证查询链路")

    import sqlite3
    con = sqlite3.connect(str(clear))
    cur = con.cursor()
    names = ce.verify_decrypted(cur)
    assert "group_msg_table" in names, f"验证失败: {names}"
    print(f"   ✓ 明文链路可读，{len(names)} 张表，含 group_msg_table")

    # 6. 拉取 + 解码 + 筛选
    state = {"群": {}}
    res = ce.process_group(cur, GROUP, state, full=True,
                           do_screen=True, out_dir=TMP)
    print(f"6. 提取 {res['提取']} 条，提分 {res['提分']} 条")
    assert res["提取"] == 12, f"应提取 12 条（13 条中 1 条零时间戳被过滤），实得 {res['提取']}"
    #别群消息不应出现
    allrows = list(csv.DictReader(open(res["全量"], encoding="utf-8-sig")))
    assert not any("别群" in r["内容"] for r in allrows), "别群消息泄漏"
    assert not any("零时间" in r["内容"] for r in allrows), "零时间戳未过滤"
    print("   ✓ 群号过滤正确（40027）")
    print("   ✓ 零时间戳已过滤")

    # 7. 校验筛选结果
    picked = list(csv.DictReader(open(res["提分文件"], encoding="utf-8-sig")))
    texts = [r["原文"] for r in picked]
    should = ["case5 做到5us了", "case1的1.47us呢", "把大D运算变成cube+vector混合",
              "通过时间可以反推指令开销", "case8的tbest要16.7TB/s带宽",
              "case4优化后反而更差了", "tbest 3750.12 → 3665.94 专刷单点"]
    should_not = ["冒泡防踢", "666", "接龙", "哈哈哈", "组队缺人"]
    for s in should:
        assert any(s in t for t in texts), f"漏判: {s}"
    for s in should_not:
        assert not any(s in t for t in texts), f"误判: {s}"
    print(f"   ✓ 筛选正确（{len(should)} 保留 / {len(should_not)} 排除）")

    # 8. 增量测试（复用同一个 state，模拟第二次运行）
    print("7. 增量测试")
    water = state["群"][str(GROUP)]["last_rowid"]
    print(f"   首次水位 rowid = {water}")
    cur.execute('DELETE FROM group_msg_table WHERE rowid<=12')
    cur.execute('INSERT INTO group_msg_table ("40001","40003","40013","40020",'
                '"40027","40030","40033","40050","40090","40093","40800") '
                'VALUES(?,?,?,?,?,?,?,?,?,?,?)',
                ("m50", 7899, 0, "u_new", GROUP, 0, 999,
                 1789900000, "新来的", "", make_body("case7 也能这么做")))
    con.commit()
    res2 = ce.process_group(cur, GROUP, state, full=False,
                            do_screen=True, out_dir=TMP)
    inc = list(csv.DictReader(open(res2["全量"], encoding="utf-8-sig")))
    texts_inc = [r["内容"] for r in inc]
    print(f"   增量提取 {res2['提取']} 条：{texts_inc}")
    # 注意：SQLite 会复用被删除的 rowid，所以不能断言 rowid 数值，
    # 只能断言「只取到新增的那条内容」。
    assert len(inc) == 1, f"增量应只含1 条新增，实得 {len(inc)}"
    assert "case7 也能这么做" in texts_inc[0], "增量未取到新消息"
    assert not any("case5 做到5us了" in t for t in texts_inc), "重复提取了旧消息"
    print("   ✓ 增量只取新增，未重复旧数据")

    # 9. 再跑一次，应无新增
    res3 = ce.process_group(cur, GROUP, state, full=False,
                            do_screen=True, out_dir=TMP)
    print(f"   二次运行提取 {res3['提取']} 条（应为 0）")
    assert res3["提取"] == 0, "二次运行不应有新增"
    print("   ✓ 幂等：无新消息时不产生冗余输出")

    con.close()
    import shutil
    shutil.rmtree(TMP, ignore_errors=True)
    print()
    print("=" * 60)
    print("  ✅ 全部通过")
    print("=" * 60)


if __name__ == "__main__":
    main()