#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
主代理独立复核脚本（调研2 / 2026-09-12）
目的：不依赖 Agent 06 的脚本，独立复现「首版推荐数值链」与若干备选链的精度差异。

说明：
- 全部为 CPU 参考（numpy + ml_dtypes），**不是 NPU 实测**。
- golden 直接 import 平台下发的模板实现，保证口径与判题一致。
- 比较口径：np.isclose(rtol, atol, equal_nan=True) 逐元素；阈值按题面：
  fp32 -> (1e-4, 1e-4)；fp16/bf16 -> (1e-3, 1e-3)
  另统计失配比例（本地 verify_result.py 的 tol=1e-3 口径）。
"""
import os
import sys
import json
import numpy as np
from ml_dtypes import bfloat16

TEMPLATE_SCRIPTS = "/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts"
sys.path.insert(0, TEMPLATE_SCRIPTS)
from AddRmsNormBias import impl as golden_impl  # noqa: E402

SEED = 1742
EPS = 1e-5


def to_f32(a):
    return a.astype(np.float32)


def back_cast(a, dt):
    if dt == bfloat16:
        return a.astype(bfloat16)
    if dt == np.float16:
        return a.astype(np.float16)
    return a.astype(np.float32)


def kernel_chain(x, r, g, b, eps, mode, block=None):
    """模拟 kernel 内部的数值链。mode 决定使用哪种操作组合。"""
    xf, rf, gf, bf = to_f32(x), to_f32(r), to_f32(g), to_f32(b)
    y = xf + rf
    D = y.shape[-1]

    if block is None:
        sq = np.sum(y * y, axis=-1, keepdims=True)
    else:  # 分块累加：模拟 UB 分块求和后再合并
        acc = np.zeros(y.shape[:-1] + (1,), dtype=np.float32)
        for s in range(0, D, block):
            e = min(s + block, D)
            acc = acc + np.sum(y[..., s:e] * y[..., s:e], axis=-1, keepdims=True)
        sq = acc

    mean = sq * np.float32(1.0 / D) + np.float32(eps)  # eps 在 mean 之后、开方之前
    if mode == "sqrt_divs":
        rms = np.sqrt(mean)
        out = y / rms * gf + bf                 # Divs：先除
    elif mode == "sqrt_muls":
        rms = np.sqrt(mean)
        out = y * (np.float32(1.0) / rms) * gf + bf   # Muls：乘 1/rms
    elif mode == "rsqrt_mul":
        rstd = np.float32(1.0) / np.sqrt(mean)
        out = y * rstd * gf + bf                # Rsqrt 等价路径
    elif mode == "eps_outside":
        rms = np.sqrt(sq * np.float32(1.0 / D)) + np.float32(eps)  # 反例：eps 在开方外
        out = y / rms * gf + bf
    else:
        raise ValueError(mode)
    return out


def low_precision_chain(x, r, g, b, eps, dt):
    """反例：全程低精度中间累加（S4）。"""
    acc_dtype = np.float32 if dt == np.float32 else dt
    xd, rd, gd, bd = x.astype(acc_dtype), r.astype(acc_dtype), g.astype(acc_dtype), b.astype(acc_dtype)
    y = (xd.astype(np.float32) + rd.astype(np.float32))
    if dt != np.float32:
        y = y.astype(acc_dtype)
    sq = np.sum(y * y, axis=-1, keepdims=True).astype(np.float32)
    mean = sq * np.float32(1.0 / x.shape[-1]) + np.float32(eps)
    rms = np.sqrt(mean)
    out = y.astype(np.float32) / rms * gd.astype(np.float32) + bd.astype(np.float32)
    return out


def compare(dt, shape, mode, block=None, lowp=False):
    rng = np.random.default_rng(SEED)
    x = rng.uniform(-2, 2, shape).astype(dt)
    r = rng.uniform(-2, 2, shape).astype(dt)
    g = rng.uniform(0.8, 1.2, (shape[-1],)).astype(dt)
    b = rng.uniform(-0.3, 0.3, (shape[-1],)).astype(dt)

    gold = golden_impl(x, r, g, b, epsilon=EPS)
    if lowp:
        cand32 = low_precision_chain(x, r, g, b, EPS, dt)
    else:
        cand32 = kernel_chain(x, r, g, b, EPS, mode, block)
    cand = back_cast(cand32, dt)

    gc, cc = to_f32(gold), to_f32(cand)
    rtol, atol = (1e-4, 1e-4) if dt == np.float32 else (1e-3, 1e-3)
    close = np.isclose(cc, gc, rtol=rtol, atol=atol, equal_nan=True)
    n_bad = int(np.sum(~close))
    total = int(gc.size)
    return {
        "dtype": "fp32" if dt == np.float32 else ("bf16" if dt == bfloat16 else "fp16"),
        "shape": list(shape),
        "mode": mode if not lowp else "low_precision",
        "block": block,
        "mismatch": n_bad,
        "total": total,
        "rate": n_bad / total if total else 0.0,
        "max_abs": float(np.max(np.abs(cc - gc))) if total else 0.0,
        "pass_tol": bool(n_bad / total <= 1e-3) if total else True,
    }


def main():
    results = []
    dtypes = [np.float16, bfloat16, np.float32]
    shapes_2d = [(8, 64), (8, 70), (64, 1024), (1, 32768), (8, 4096)]
    shapes_nd = [(2, 3, 4, 8), (4, 8, 128), (2, 2, 2, 129)]

    print("=" * 78)
    print("主代理独立复核：数值链精度对比（CPU 参考，非 NPU 实测）")
    print("=" * 78)

    # A. 主对比：三种归一化路径 + 分块
    for dt in dtypes:
        for shp in shapes_2d:
            for mode in ["sqrt_divs", "sqrt_muls", "rsqrt_mul"]:
                res = compare(dt, shp, mode)
                results.append(res)

    # B. 分块累加 vs 一次累加（block=1024 / 4096）
    for dt in dtypes:
        for shp in [(8, 4096), (1, 32768)]:
            for blk in [1024, 4096]:
                results.append(compare(dt, shp, "sqrt_divs", block=blk))

    # C. 3D/4D 覆盖
    for dt in dtypes:
        for shp in shapes_nd:
            results.append(compare(dt, shp, "sqrt_divs"))

    # D. 反例：eps 外置
    for dt in dtypes:
        results.append(compare(dt, (64, 1024), "eps_outside"))

    # E. 反例：低精度中间累加（S4）
    for dt in dtypes:
        results.append(compare(dt, (64, 1024), "sqrt_divs", lowp=True))

    # 汇总打印
    print("\n[1] 三种归一化路径 —— 失配元素数 / 失配比例")
    for mode in ["sqrt_divs", "sqrt_muls", "rsqrt_mul"]:
        print(f"\n  mode = {mode}")
        for dtname in ["fp16", "bf16", "fp32"]:
            rows = [r for r in results if r["mode"] == mode and r["dtype"] == dtname and r["block"] is None
                    and r["shape"] in [list(s) for s in shapes_2d]]
            tot_bad = sum(r["mismatch"] for r in rows)
            tot = sum(r["total"] for r in rows)
            mx = max((r["max_abs"] for r in rows), default=0.0)
            print(f"    {dtname:5s} 失配 {tot_bad:6d} / {tot:8d}  = {tot_bad/tot*100 if tot else 0:.6f}%   最大绝对差 {mx:.3e}")

    print("\n[2] 分块累加（首版推荐 block<=4096） vs 一次累加")
    for dtname in ["fp16", "bf16", "fp32"]:
        for blk in [1024, 4096]:
            rows = [r for r in results if r["mode"] == "sqrt_divs" and r["dtype"] == dtname and r["block"] == blk]
            tot_bad = sum(r["mismatch"] for r in rows)
            tot = sum(r["total"] for r in rows)
            print(f"    {dtname:5s} block={blk:5d} 失配 {tot_bad:6d} / {tot:8d} = {tot_bad/tot*100 if tot else 0:.6f}%")

    print("\n[3] 3D/4D 覆盖（sqrt_divs）")
    for r in [r for r in results if r["shape"] in [list(s) for s in shapes_nd]]:
        print(f"    {r['dtype']:5s} {str(r['shape']):18s} 失配 {r['mismatch']:5d}/{r['total']:7d} = {r['rate']*100:.6f}%")

    print("\n[4] 反例：epsilon 外置（开方之后才加 eps）")
    for r in [r for r in results if r["mode"] == "eps_outside"]:
        print(f"    {r['dtype']:5s} 失配 {r['mismatch']:6d}/{r['total']:7d} = {r['rate']*100:.6f}%  pass_tol={r['pass_tol']}")

    print("\n[5] 反例：低精度中间累加（S4）")
    for r in [r for r in results if r["mode"] == "low_precision"]:
        print(f"    {r['dtype']:5s} 失配 {r['mismatch']:6d}/{r['total']:7d} = {r['rate']*100:.6f}%  pass_tol={r['pass_tol']}")

    out_json = os.path.join(os.path.dirname(os.path.abspath(__file__)), "main_precision_check.json")
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(results, f, ensure_ascii=False, indent=2)
    print(f"\n结果已写入 {out_json}")
    print("=" * 78)


if __name__ == "__main__":
    main()
