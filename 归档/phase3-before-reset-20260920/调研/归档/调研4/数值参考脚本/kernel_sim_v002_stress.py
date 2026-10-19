#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kernel_sim_v002_stress.py — 针对 V002 数值链的定向压力测试（CPU / numpy 口径）

背景
----
`kernel_sim_v002.py` 的 21 组用例显示：V002 的算法结构（两遍扫描 + FP32 中间 +
分块 ReduceSum + 标量累加 + CAST_RINT）与官方 golden 在数值上完全等价（失配 0%），
但有两处**离群**值得单独量化：

  1. fp16、D=4097 时「vs golden 最大相对误差」达 9.17e-4，逼近 1e-3 判定阈值；
  2. fp16、D=32768 时达 7.66e-4。

本脚本隔离潜在成因，并用随机种子扫描找出更坏情况：

  实验 A：`Muls(y, 1/rms)` 与 `y / rms` 的差异（V002 用前者，golden 用后者）
  实验 B：分块累加（tile=4096/2048）与整体累加的顺序差异
  实验 C：多随机种子 × D 边界 × 数值量级的最坏相对误差扫描
  实验 D：fp16 平方和溢出边界（|y| 阈值）

声明：全部为 CPU/numpy 口径，**不是 NPU 实测**，不能替代真机精度验证。

用法
----
    /usr/bin/python3 kernel_sim_v002_stress.py
"""

import numpy as np
from ml_dtypes import bfloat16 as BF16

F32 = np.float32


def golden(x, r, g, b, eps=1e-5):
    y = x.astype(F32) + r.astype(F32)
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + F32(eps))
    out = y / rms * g.astype(F32) + b.astype(F32)
    return out.astype(x.dtype)


def golden_recip(x, r, g, b, eps=1e-5):
    """把 golden 的 `/ rms` 换成 `* (1/rms)`，其余完全一致。"""
    y = x.astype(F32) + r.astype(F32)
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + F32(eps))
    out = y * (F32(1.0) / rms) * g.astype(F32) + b.astype(F32)
    return out.astype(x.dtype)


def row_sum_tiled(y_f32, tile):
    """按 tile 分块求平方和，块间 fp32 标量累加（模拟 V002）。"""
    y2 = (y_f32 * y_f32).astype(F32)
    total = F32(0.0)
    for i in range(0, y2.size, tile):
        total = F32(total + F32(np.sum(y2[i:i + tile], dtype=F32)))
    return total


def rel_stats(a, ref):
    af = a.astype(F32).astype(np.float64)
    rf = ref.astype(F32).astype(np.float64)
    d = np.abs(af - rf)
    rel = d / np.maximum(np.abs(rf), 1e-30)
    return float(rel.max()), float(np.median(rel))


def part_a():
    print("=" * 96)
    print("实验 A：`Muls(y, 1/rms)`  vs  golden 的 `y / rms`   —— 只看这一步的差异")
    print("=" * 96)
    print(f"{'D':>8}{'dtype':>10}{'最大相对':>14}{'中位相对':>14}   说明")
    print("-" * 96)
    rng = np.random.default_rng(7)
    for D in [64, 129, 1000, 4096, 4097, 8192, 32768]:
        for dt in [np.float16, BF16, np.float32]:
            x = rng.uniform(-2, 2, (1, D)).astype(dt)
            r = rng.uniform(-2, 2, (1, D)).astype(dt)
            g = rng.uniform(0.8, 1.2, (D,)).astype(dt)
            b = rng.uniform(-0.3, 0.3, (D,)).astype(dt)
            a = golden(x, r, g, b)
            c = golden_recip(x, r, g, b)
            mx, md = rel_stats(c, a)
            same = "完全相同" if mx == 0 else ""
            print(f"{D:>8}{np.dtype(dt).name:>10}{mx:>14.3e}{md:>14.3e}   {same}")
    print("\n结论：该差异在 fp16/bf16 上的量级直接贡献了 D 较大时的离群相对误差。\n")


def part_b():
    print("=" * 96)
    print("实验 B：分块累加（tile=4096）vs 整体累加 —— 只看平方和这一步")
    print("=" * 96)
    print(f"{'D':>8}{'dtype':>10}{'sum 相对差':>16}{'由此产生的 rms 相对差':>26}")
    print("-" * 96)
    rng = np.random.default_rng(11)
    for D in [4096, 4097, 8192, 8193, 32768]:
        for dt in [np.float16, BF16, np.float32]:
            x = rng.uniform(-2, 2, (1, D)).astype(dt)
            r = rng.uniform(-2, 2, (1, D)).astype(dt)
            y = (x.astype(F32) + r.astype(F32))
            s_tiled = row_sum_tiled(y[0], 4096)
            s_whole = F32(np.sum((y * y)[0], dtype=F32))
            dsum = abs(float(s_tiled) - float(s_whole)) / max(abs(float(s_whole)), 1e-30)
            rms_t = np.sqrt(F32(F32(s_tiled) / F32(D) + F32(1e-5)), dtype=F32)
            rms_w = np.sqrt(F32(F32(s_whole) / F32(D) + F32(1e-5)), dtype=F32)
            drms = abs(float(rms_t) - float(rms_w)) / max(abs(float(rms_w)), 1e-30)
            print(f"{D:>8}{np.dtype(dt).name:>10}{dsum:>16.3e}{drms:>26.3e}")
    print()


def rel_stats_filtered(a, ref, floor=1e-3):
    """只看 |ref| >= floor 的元素的相对误差 —— 避免近零元素把相对误差放大到无意义。"""
    af = a.astype(F32).astype(np.float64)
    rf = ref.astype(F32).astype(np.float64)
    d = np.abs(af - rf)
    m = np.abs(rf) >= floor
    if not np.any(m):
        return 0.0, float(d.max())
    return float((d[m] / np.abs(rf[m])).max()), float(d.max())


def part_c(trials=40):
    print("=" * 96)
    print("实验 C：多随机种子扫描 —— V002 完整模拟(即 *1/rms 版) vs golden")
    print("")
    print("列含义：")
    print("  · 『最大绝对』= 全部元素的最大绝对误差（真正决定 isclose 能否过的量）")
    print("  · 『最大相对(|ref|>=1e-3)』= 过滤掉近零元素后的相对误差，排除放大假象")
    print("  · 『失配率>0.1% 的种子数』= 按 isclose(rtol=1e-3, atol=1e-3) 判定是否真的会失败")
    print("=" * 96)
    print(f"{'D':>8}{'dtype':>10}{'量级':>6}{'最大绝对':>14}{'最大相对(|ref|>=1e-3)':>24}"
          f"{'失配率>0.1%的种子数':>22}")
    print("-" * 96)
    for scale in [1.0, 10.0, 100.0]:
        for D in [1000, 4096, 4097, 8192, 8193, 16384, 32768]:
            for dt in [np.float16, BF16]:
                worst_abs = 0.0
                worst_rel = 0.0
                bad = 0
                for t in range(trials):
                    rng = np.random.default_rng(5000 + t)
                    x = rng.uniform(-2, 2, (1, D)).astype(F32)
                    r = rng.uniform(-2, 2, (1, D)).astype(F32)
                    x = (x * F32(scale)).astype(dt)
                    r = (r * F32(scale)).astype(dt)
                    g = rng.uniform(0.8, 1.2, (D,)).astype(dt)
                    b = rng.uniform(-0.3, 0.3, (D,)).astype(dt)
                    gd = golden(x, r, g, b)
                    rc = golden_recip(x, r, g, b)
                    rel, ab = rel_stats_filtered(rc, gd)
                    worst_rel = max(worst_rel, rel)
                    worst_abs = max(worst_abs, ab)
                    ok = np.isclose(rc.astype(F32).astype(np.float64),
                                    gd.astype(F32).astype(np.float64),
                                    rtol=1e-3, atol=1e-3, equal_nan=True)
                    if np.sum(~ok) / ok.size > 0.001:
                        bad += 1
                flag = "  <== 相对超阈" if worst_rel >= 1e-3 else ""
                print(f"{D:>8}{np.dtype(dt).name:>10}{scale:>6.0f}{worst_abs:>14.3e}"
                      f"{worst_rel:>24.3e}{bad:>22}{flag}")
        print("-" * 96)
    print()


def part_d():
    print("=" * 96)
    print("实验 D：fp16 平方和的溢出边界")
    print("=" * 96)
    fp16_max = 65504.0
    print(f"  fp16 最大正规数 = {fp16_max}")
    print(f"  若在 fp16 域平方：|y| > sqrt(65504) ≈ {np.sqrt(fp16_max):.1f} 时 y^2 即上溢为 inf")
    for v in [200.0, 250.0, 255.0, 256.0, 300.0, 1000.0]:
        y16 = np.array([v], dtype=np.float16)
        y32 = np.array([v], dtype=F32)
        sq16 = y16.astype(F32) * y16.astype(F32)          # fp16 域平方后升精度观察
        sq16_native = (y16 * y16)                          # 直接在 fp16 域平方
        print(f"  |y|={v:>7.1f}  fp16 域 y*y = {float(sq16_native[0]):>12}   "
              f"fp32 域 y*y = {float(y32[0] * y32[0]):>12.1f}")
    print("\n结论：FP32 域累加彻底消除平方上溢；低精度域（方案 S4）在 |y|>256 时必然失败。\n")


def main():
    part_a()
    part_b()
    part_c(trials=40)
    part_d()
    print("=" * 96)
    print("全部为 CPU / numpy 口径。未在真实 CANN / 昇腾 NPU 上验证。")
    print("=" * 96)


if __name__ == "__main__":
    main()
