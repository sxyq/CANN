#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""CANNJudge 只读工具：提交前核对 + 结果轮询。

严格边界（写死在代码里，不要改）：
  1. 只发 GET。本文件不含 POST / PUT / PATCH / DELETE 任何代码路径。
  2. 不发 Cookie、不读浏览器配置、不保存任何凭据。所有请求都是匿名请求。
  3. 本工具不执行提交。线上提交必须由用户在已登录浏览器内手动完成。

三条子命令：
  preflight <路径>   本地只读：版本路径、入口文件名、行数、字节数、SHA-256、模板约定体检
  poll <submissionId> 轮询 GET /api/submissions/{id}，终态后输出总状态与逐点结果
  problem [token]    只读：解析题目的 problemId 与 15 个测试点公开基准 tbest

用法示例：
  python3 脚本/cannjudge.py preflight /absolute/path/kernel.txt
  python3 脚本/cannjudge.py poll <submissionId> --interval 2
  python3 脚本/cannjudge.py poll <submissionId> --official
  python3 脚本/cannjudge.py problem --token addrmsnormbias
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import sys
import time
import urllib.error
import urllib.request

BASE = "https://cannjudge.cn"
DEFAULT_PROBLEM_TOKEN = "addrmsnormbias"
UA = "cannjudge-readonly-cli/1.0 (+GET only; no credentials)"

# 平台提交入口固定文件名为 kernel.asc
SUBMISSION_ENTRY_NAME = "kernel.asc"

# 终态 / 非终态关键词。前端 ui/shared.js 的 statusKey() 把未知值都当作 waiting，
# 这里为了不无限轮询，额外把明显的错误/结束类词也视为终态。
TERMINAL_HINTS = (
    "pass", "accepted", "skip", "wrong", "fail", "compile", "runtime",
    "time limit", "error", "reject", "invalid", "cancel", "finish",
    "done", "success", "complete",
)
PENDING_HINTS = (
    "wait", "running", "pending", "queue", "judging", "compiling",
    "preparing", "init", "judge",
)


# --------------------------------------------------------------------------- #
# HTTP：只有 GET
# --------------------------------------------------------------------------- #
def _opener(proxy: str | None) -> urllib.request.OpenerDirector:
    handlers = []
    if proxy:
        handlers.append(urllib.request.ProxyHandler({"http": proxy, "https": proxy}))
    else:
        # 默认直连：本机沙箱会注入 HTTP_PROXY，可能把请求劫持到本地代理端口
        handlers.append(urllib.request.ProxyHandler({}))
    return urllib.request.build_opener(*handlers)


def http_get_json(path_or_url: str, *, proxy: str | None = None, timeout: float = 20.0):
    """只读 GET，返回 (status_code, body_dict_or_text)。"""
    url = path_or_url if path_or_url.startswith("http") else BASE + path_or_url
    req = urllib.request.Request(url, method="GET")
    req.add_header("User-Agent", UA)
    req.add_header("Accept", "application/json")
    # 刻意不设置 Cookie / Authorization / Referer
    try:
        with _opener(proxy).open(req, timeout=timeout) as resp:
            raw = resp.read().decode("utf-8", "replace")
            code = resp.status
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode("utf-8", "replace")
        code = exc.code
    except Exception as exc:  # 网络层
        return 0, {"_transport_error": str(exc)}
    try:
        return code, json.loads(raw) if raw else {}
    except json.JSONDecodeError:
        return code, {"_raw": raw[:2000]}


# --------------------------------------------------------------------------- #
# 评分：平台公开公式的本地复算
# --------------------------------------------------------------------------- #
def case_score(t: float, tbest: float) -> float | None:
    """s_i = 100 / (1 + log_1.5(t_i / T_i))；T_i<=0 或 t_i<=0 时不可算。"""
    try:
        t = float(t)
        tbest = float(tbest)
    except (TypeError, ValueError):
        return None
    if t <= 0 or tbest <= 0:
        return None
    return 100.0 / (1.0 + math.log(t / tbest, 1.5))


def status_key(status: str) -> str:
    s = str(status or "").lower()
    if "pass" in s or "accepted" in s or s == "ac":
        return "pass"
    if "skip" in s:
        return "skipped"
    if "wrong" in s:
        return "wrong"
    if any(k in s for k in ("fail", "runtime", "compile", "time limit", "error")):
        return "fail"
    return "waiting"


def is_terminal(status: str) -> bool:
    s = str(status or "").lower().strip()
    if not s:
        return False
    if any(k in s for k in PENDING_HINTS):
        return False
    if status_key(s) != "waiting":
        return True
    return any(k in s for k in TERMINAL_HINTS)


# --------------------------------------------------------------------------- #
# preflight：本地只读核对，不联网
# --------------------------------------------------------------------------- #
def cmd_preflight(args) -> int:
    target = os.path.abspath(args.path)
    if os.path.isdir(target):
        cand = os.path.join(target, "kernel.txt")
        if not os.path.exists(cand):
            print(f"[错误] 目录内未找到 kernel.txt：{target}", file=sys.stderr)
            return 2
        target = cand
    if not os.path.exists(target):
        print(f"[错误] 文件不存在：{target}", file=sys.stderr)
        return 2

    with open(target, "rb") as fh:
        blob = fh.read()
    text = blob.decode("utf-8", "replace")
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines = lines[:-1]

    sha = hashlib.sha256(blob).hexdigest()
    name = os.path.basename(target)

    # 模板约定体检（只提示，不阻断）
    checks = [
        ("首行为 #include <cmath>", bool(lines) and lines[0].strip() == "#include <cmath>"),
        ("末行为 }", bool(lines) and lines[-1].strip() == "}"),
        ('含 extern "C" void run_kernel', 'extern "C" void run_kernel' in text),
        ("入口限定符 __global__ __vector__", "__global__ __vector__" in text),
        ("无聚合初始化 DataCopyPadExtParams{", "DataCopyPadExtParams" in text
         and "DataCopyPadExtParams<" in text and "= {" not in text),
    ]

    info = {
        "版本目录": os.path.dirname(target),
        "本机源码文件": target,
        "本机文件名": name,
        "平台提交入口名": SUBMISSION_ENTRY_NAME,
        "行数": len(lines),
        "字节数": len(blob),
        "sha256": sha,
        "模板约定体检": {k: ok for k, ok in checks},
    }

    if args.json:
        print(json.dumps(info, ensure_ascii=False, indent=2))
        return 0

    print("=== CANNJudge 提交前核对（本地只读，未联网）===")
    print(f"版本目录      : {info['版本目录']}")
    print(f"本机源码文件  : {info['本机源码文件']}")
    print(f"本机文件名    : {name}")
    print(f"平台入口名    : {SUBMISSION_ENTRY_NAME}"
          + ("" if name == "kernel.txt" else "   ← ⚠ 本机未使用 .txt 命名；平台提交入口固定文件名为 kernel.asc"))
    print(f"行数          : {info['行数']}")
    print(f"字节数        : {info['字节数']}")
    print(f"SHA-256       : {sha}")
    print("模板约定体检（仅提示，请人工复核）：")
    for k, ok in checks:
        print(f"  [{'通过' if ok else '需确认'}] {k}")
    print()
    print("提示：把本段行数/字节数/SHA-256 写进版本结果记录，提交后用于人工确认"
          "平台上跑的就是这一份文件（结果接口对非本人源码只返回占位内容，无法远程反查）。")
    return 0


# --------------------------------------------------------------------------- #
# poll：结果轮询
# --------------------------------------------------------------------------- #
def _render_submission(sub: dict, official_score: float | None, problem_token: str) -> dict:
    rows = sub.get("result") if isinstance(sub.get("result"), list) else []
    status = sub.get("status")
    per_case, scores = [], []
    for idx, row in enumerate(rows, 1):
        t = row.get("time")
        tbest = row.get("best_time")
        ratio = row.get("precision_ratio")
        try:
            err_pct = (1.0 - float(ratio)) * 100.0 if ratio is not None else None
        except (TypeError, ValueError):
            err_pct = None
        s = case_score(t, tbest)
        if s is not None:
            scores.append(s)
        per_case.append({
            "序号": idx,
            "testcase_id": row.get("testcase_id"),
            "状态": row.get("testcase_status"),
            "失配率%": None if err_pct is None else round(err_pct, 4),
            "耗时us": t,
            "基准us": tbest,
            "单点分(复算)": None if s is None else round(s, 2),
            "msg": (row.get("msg") or "").strip()[:400],
        })
    local_score = round(sum(scores) / len(scores), 3) if scores else None
    hidden = sum(1 for r in rows if r.get("testcase_status") == "Hidden")
    return {
        "submission_id": sub.get("_id"),
        "ID": sub.get("ID"),
        "status": status,
        "status_key": status_key(status),
        "valid": sub.get("valid"),
        "create_time": sub.get("create_time"),
        "user_id": sub.get("user_id"),
        "problem_id": sub.get("problem_id"),
        "题目": (sub.get("problem") or {}).get("name") or problem_token,
        "通过点数": sum(1 for r in rows if status_key(r.get("testcase_status")) == "pass"),
        "隐藏点数": hidden,
        "点位数": len(rows),
        "总分(本地复算)": local_score,
        "总分(公开排行官方值)": official_score,
        "can_view_code": sub.get("can_view_code"),
        "逐点": per_case,
    }


def _fetch_official_score(problem_id: str, submission_id: str, proxy: str | None) -> float | None:
    """公开排行 GET /api/problems/{pid}/ranking，按 submission_id 找官方总分。

    排行只收录 Pass 状态的提交；找不到只代表"未上榜"，不代表失败。
    """
    if not problem_id:
        return None
    page, size = 1, 100
    seen_pages = 0
    while seen_pages < 10:
        code, body = http_get_json(
            f"/api/problems/{problem_id}/ranking?page={page}&size={size}", proxy=proxy)
        if code != 200 or not isinstance(body, dict):
            return None
        rows = body.get("rows") or []
        for row in rows:
            if str(row.get("submission_id")) == str(submission_id):
                try:
                    return round(float(row.get("score")), 3)
                except (TypeError, ValueError):
                    return None
        pages = int(body.get("pages") or 1)
        if page >= pages or not rows:
            return None
        page += 1
        seen_pages += 1
    return None


def cmd_poll(args) -> int:
    sub_id = args.submission_id.strip()
    if not sub_id:
        print("[错误] submissionId 不能为空", file=sys.stderr)
        return 2

    deadline = time.monotonic() + args.max_wait
    attempt = 0
    last_key = None
    while True:
        attempt += 1
        code, body = http_get_json(f"/api/submissions/{sub_id}", proxy=args.proxy)
        stamp = time.strftime("%H:%M:%S")

        if code == 404:
            print(f"[{stamp}] 第 {attempt} 次：404 未找到该 submissionId（{sub_id}）")
            return 2
        if code == 401 or code == 403:
            print(f"[{stamp}] 第 {attempt} 次：{code} 被拒绝。结果接口本身是公开的，"
                  f"出现鉴权拒绝说明 id 形态或站点策略与预期不同，请人工核对。")
            return 2
        if code != 200:
            msg = body.get("_transport_error") or body.get("message") or body
            print(f"[{stamp}] 第 {attempt} 次：HTTP {code} {msg}", file=sys.stderr)
            if time.monotonic() >= deadline:
                return 2
            time.sleep(max(1.0, args.interval))
            continue

        status = body.get("status")
        rows = body.get("result") if isinstance(body.get("result"), list) else []
        key = status_key(status)
        if args.json and not is_terminal(status):
            # JSON 模式下静默等待，只在终态输出
            pass
        elif key != last_key:
            print(f"[{stamp}] 第 {attempt} 次：status={status!r}  已有 {len(rows)} 个点位结果")
            last_key = key
        else:
            print(f"[{stamp}] 第 {attempt} 次：status={status!r} … 继续等待")

        if is_terminal(status):
            official = None
            if args.official:
                official = _fetch_official_score(body.get("problem_id"), sub_id, args.proxy)
            report = _render_submission(body, official, args.problem_token)
            if args.out:
                out_path = os.path.abspath(args.out)
                with open(out_path, "w", encoding="utf-8") as fh:
                    json.dump(report, fh, ensure_ascii=False, indent=2)
                print(f"\n原始结果已写入：{out_path}")
            if args.json:
                print(json.dumps(report, ensure_ascii=False, indent=2))
            else:
                _print_report(report)
            return 0 if report["status_key"] == "pass" else 1

        if time.monotonic() >= deadline:
            print(f"\n[超时] 已等待 {args.max_wait}s，status 仍为 {status!r}（非终态）。"
                  f"可用更大的 --max-wait 继续，或稍后重跑同一 submissionId。")
            return 3
        if args.once:
            print("\n[--once] 只查一次，未到终态，退出。")
            return 3
        time.sleep(max(1.0, args.interval))


def _print_report(rep: dict) -> None:
    print()
    print("=" * 78)
    print(f"提交 ID        : {rep['submission_id']}   (平台 ID {rep['ID']})")
    print(f"题目           : {rep['题目']}    problem_id={rep['problem_id']}")
    print(f"总状态         : {rep['status']}   [判定 {rep['status_key']}]"
          f"{'   有效提交' if rep['valid'] else '   ⚠ 已封禁/无效'}")
    print(f"通过点位       : {rep['通过点数']} / {rep['点位数']}"
          + (f"（其中隐藏 {rep['隐藏点数']} 点）" if rep['隐藏点数'] else ""))
    print(f"提交时间       : {rep['create_time']}")
    if rep["总分(公开排行官方值)"] is not None:
        print(f"官方总分       : {rep['总分(公开排行官方值)']}   ← 来自公开排行榜，可作收敛依据")
    if rep["总分(本地复算)"] is not None:
        print(f"总分(本地复算) : {rep['总分(本地复算)']}   ← 按平台公开公式复算，"
              f"仅作诊断，不得直接写成官方分")
    print("-" * 78)
    print(f"{'#':>2}  {'状态':<14} {'失配率%':>9} {'耗时us':>10} {'基准us':>10} {'单点分(复算)':>12}")
    for row in rep["逐点"]:
        err = "-" if row["失配率%"] is None else f"{row['失配率%']:.4f}"
        t = "-" if row["耗时us"] is None else f"{row['耗时us']:.2f}"
        tb = "-" if row["基准us"] is None else f"{row['基准us']:.2f}"
        sc = "-" if row["单点分(复算)"] is None else f"{row['单点分(复算)']:.2f}"
        print(f"{row['序号']:>2}  {str(row['状态']):<14} {err:>9} {t:>10} {tb:>10} {sc:>12}")
        if row["msg"]:
            print(f"    msg: {row['msg']}")
    print("=" * 78)
    print("说明：本工具只做只读查询。「总分(本地复算)」是按平台公开公式 "
          "s_i=100/(1+log_1.5(t_i/T_i)) 的复算值；")
    print("      已用公开排行样本验证与官方 score 一致，但按项目约定，"
          "性能收敛只认排行榜里的官方总分。")


# --------------------------------------------------------------------------- #
# problem：只读解析题目
# --------------------------------------------------------------------------- #
def cmd_problem(args) -> int:
    token = (args.token or DEFAULT_PROBLEM_TOKEN).strip()
    code, body = http_get_json(f"/api/problems/name/{token}", proxy=args.proxy)
    if code != 200 or not isinstance(body, dict):
        print(f"[错误] 解析题目失败：HTTP {code} {body}", file=sys.stderr)
        return 2
    pid = body.get("_id") or body.get("id")
    print(f"题目 token     : {token}")
    print(f"标题           : {body.get('title')}")
    print(f"problemId      : {pid}")
    print(f"平台题目编号   : {body.get('ID')}")
    print(f"contest_id     : {body.get('contest_id')}")
    print(f"code_template  : {body.get('code_template')}")

    if pid:
        rc, rb = http_get_json(f"/api/problems/{pid}/ranking?page=1&size=1", proxy=args.proxy)
        tcs = rb.get("testcases") if isinstance(rb, dict) else None
        if isinstance(tcs, list) and tcs:
            print(f"测试点数量     : {len(tcs)}（公开 tbest 基准）")
            for i, t in enumerate(tcs, 1):
                print(f"  T{i:>2}  {t.get('_id')}  tbest={t.get('tbest')}  "
                      f"baseline={t.get('baseline')}  type={t.get('type')}")
        else:
            print(f"测试点基准     : 未取到（HTTP {rc}）")
    return 0


# --------------------------------------------------------------------------- #
def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="cannjudge.py",
        description="CANNJudge 只读工具：提交前核对 + 结果轮询（本工具不提交、不保存凭据）")
    sub = p.add_subparsers(dest="cmd", required=True)

    pf = sub.add_parser("preflight", help="本地只读：版本路径/入口名/行数/字节数/SHA-256/模板体检")
    pf.add_argument("path", help="版本目录或本机源码 .txt 路径")
    pf.add_argument("--json", action="store_true", help="输出 JSON")
    pf.set_defaults(func=cmd_preflight)

    po = sub.add_parser("poll", help="轮询 GET /api/submissions/{id} 直到终态")
    po.add_argument("submission_id", help="提交后从页面 URL 取到的 submissionId")
    po.add_argument("--interval", type=float, default=2.0, help="轮询间隔秒，默认 2")
    po.add_argument("--max-wait", type=float, default=1800.0, help="最长等待秒，默认 1800")
    po.add_argument("--once", action="store_true", help="只查一次，不等终态")
    po.add_argument("--official", action="store_true",
                    help="额外读取公开排行榜以取得官方总分（只收录 Pass 提交）")
    po.add_argument("--problem-token", default=DEFAULT_PROBLEM_TOKEN)
    po.add_argument("--out", help="把完整逐点结果另存为 JSON 文件")
    po.add_argument("--json", action="store_true", help="终态后以 JSON 输出")
    po.add_argument("--proxy", help="显式代理，例如 http://127.0.0.1:7897；默认直连")
    po.set_defaults(func=cmd_poll)

    pr = sub.add_parser("problem", help="只读解析题目 problemId 与公开测试点基准")
    pr.add_argument("--token", default=DEFAULT_PROBLEM_TOKEN)
    pr.add_argument("--proxy")
    pr.set_defaults(func=cmd_problem)
    return p


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
