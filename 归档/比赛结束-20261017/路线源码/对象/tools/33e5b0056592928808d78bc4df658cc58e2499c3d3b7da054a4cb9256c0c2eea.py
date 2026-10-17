#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""CANNJudge 只读工具：结果轮询。

严格边界（写死在代码里，不要改）：
  1. 只发 GET。本文件不含 POST / PUT / PATCH / DELETE 任何代码路径。
  2. 不发 Cookie、不读浏览器配置、不保存任何凭据。所有请求都是匿名请求。
  3. 本工具不执行提交。线上提交必须由用户在已登录浏览器内手动完成。

两条子命令：
  poll <submissionId> 轮询 GET /api/submissions/{id}，终态后输出总状态与逐点结果
  problem [token]    只读：解析题目的 problemId 与 15 个测试点公开基准 tbest

用法示例：
  python3 脚本/cannjudge.py poll <submissionId> --interval 2
  python3 脚本/cannjudge.py poll <submissionId> --official
  python3 脚本/cannjudge.py problem --token addrmsnormbias
"""

from __future__ import annotations

import argparse
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
        description="CANNJudge 只读工具：结果轮询（本工具不提交、不保存凭据）")
    sub = p.add_subparsers(dest="cmd", required=True)

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
