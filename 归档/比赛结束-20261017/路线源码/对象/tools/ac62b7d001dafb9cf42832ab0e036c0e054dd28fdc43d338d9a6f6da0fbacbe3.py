#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kernel_sim_v002_divpath.py — 隔离「归一化除法路径」对输出舍入的影响（CPU / numpy 口径）

问题
----
官方 golden（`scripts/AddRmsNormBias.py`）计算：

    out = y / rms * gamma + bias

而当前候选 `提交/V002/kernel.asc` 计算：

    rms   = sqrtf(sum/D + eps)      // 标量
    scale = 1.0f / rms              // 标量取倒数
    out   = Muls(y, scale) * gamma + bias   // 先乘倒数

这两条路径在实数域等价，但在浮点域**不是同一个舍入路径**：
先取倒数再相乘会引入一次额外舍入，并且当最终结果量化到 fp16/bf16 时，
可能落在量化分界的另一侧，导致输出相差 1~3 个 ulp。

本脚本量化这个差异，并给出「改成 Divs 直接除」是否能消除它。

三个对照路径
------------
  P1  y / rms              —— golden 路径（参考）
  P2  y * (1/rms)          —— V002 当前路径
  P3  y * rsqrt(mean+eps)  —— 另一常见实现路径（向量 Rsqrt 近似）

判据
----
与判题模板 `verify_result.py` 同构：np.isclose(rtol, atol, equal_nan=True)，
并统计「不完全相等的元素数」与「失配率是否 > 0.1%」。

声明：全部为 CPU / numpy 口径，**不是 NPU 实测**。numpy 的 reciprocal 与
昇腾向量的 Rsqrt 指令近似误差不同，P3 仅用于说明「不同倒数实现会产生不同舍入」，
不能当作昇腾 Rsqrt 的精度结论。

用法
----
    /usr/bin/python3 kernel_sim_v002_divpath.py
"""

import numpy as np
from ml_dtypes import bfloat16 as BF16

F32 = np.float32
EPS = F32(1e-5)


def paths(x, r, g, b):
    y = (x.astype(F32) + r.astype(F32)).astype(F32)
    mean = F32(np.mean((y * y).astype(F32), dtype=F32))
    base = F32(mean + EPS)

    rms = F32(np.sqrt(base, dtype=F32))
    inv_exact = F32(F32(1.0) / rms)                 # 精确倒数（fp32 一次除法）
    rsqrt_approx = F32(np.reciprocal(np.sqrt(base.astype(np.float64))))  # 高精度 rsqrt

    gg = g.astype(F32)
    bb = b.astype(F32)

    p1 = ((y / rms).astype(F32) * gg + bb).astype(F32)
    p2 = ((y * inv_exact).astype(F32) * gg + bb).astype(F32)
    p3 = ((y * rsqrt_approx).astype(F32) * gg + bb).astype(F32)
    return p1, p2, p3


def quantize(a, dt):
    return a.astype(dt)


def report(tag, a, ref, rtol, atol):
    af = a.astype(F32).astype(np.float64)
    rf = ref.astype(F32).astype(np.float64)
    d = np.abs(af - rf)
    neq = int(np.sum(a != ref))
    mismatch = int(np.sum(~np.isclose(af, rf, rtol=rtol, atol=atol, equal_nan=True)))
    rate = mismatch / ref.size
    m = np.abs(rf) >= 1e-3
    rel = (d[m] / np.abs(rf[m])).max() if np.any(m) else 0.0
    return (f"{tag:<26}{neq:>10}{mismatch:>10}{rate:>11.4%}"
            f"{d.max():>12.3e}{rel:>14.3e}")


def main():
    print("=" * 112)
    print("归一化除法路径对输出舍入的影响 —— CPU / numpy 口径，非 NPU 实测")
    print("=" * 112)
    print("列含义：")
    print("  『不等元素数』= 与 P1(golden 路径) 逐元素不完全相等的元素个数（含 1 ulp 差异）")
    print("  『isclose 失配』= 按 rtol=atol=1e-3/1e-4 判定真正失配的元素数")
    print("  『最大绝对』『最大相对(|ref|>=1e-3)』同前")
    print()
    for dt, (rtol, atol) in [(np.float16, (1e-3, 1e-3)), (BF16, (1e-3, 1e-3)),
                             (np.float32, (1e-4, 1e-4))]:
        print("-" * 112)
        print(f"dtype = {np.dtype(dt).name}   判据 rtol={rtol}, atol={atol}")
        print("-" * 112)
        print(f"{'D / 路径':<26}{'不等元素数':>10}{'isclose 失配':>10}{'失配率':>11}"
              f"{'最大绝对':>12}{'最大相对':>14}")
        for D in [64, 129, 1000, 4096, 8192, 32768]:
            rng = np.random.default_rng(20260911)
            x = rng.uniform(-2, 2, (4, D)).astype(dt)
            r = rng.uniform(-2, 2, (4, D)).astype(dt)
            g = rng.uniform(0.8, 1.2, (D,)).astype(dt)
            b = rng.uniform(-0.3, 0.3, (D,)).astype(dt)
            p1, p2, p3 = paths(x, r, g, b)
            q1, q2, q3 = quantize(p1, dt), quantize(p2, dt), quantize(p3, dt)
            print(report(f"D={D}  P2 乘1/rms", q2, q1, rtol, atol))
            print(report(f"D={D}  P3 乘rsqrt", q3, q1, rtol, atol))
        print()

    print("=" * 112)
    print("解读与建议（本机 CPU 口径，需真机确认）")
    print("=" * 112)
    print("""  1. 只要把 `Muls(y, 1/rms)` 换成 `Divs(y, rms)`（或先 `Muls(y, y)`… 即与 golden 同链的
     『先除后乘』），输出与 golden 的逐元素差异应当收敛到 0（见上表 P2 的『不等元素数』列）。
  2. P2（V002 当前写法）在 bf16 上产生的差异多为量化分界翻转，量级约 1~3 个 bf16 ulp
     （bf16 ulp ≈ 2^-8 ≈ 3.9e-3），单个元素绝对误差可达 1e-2 量级。
  3. 因此『判题是否允许少量元素失配』成为本方案的关键未知量：
       · 若判题等价于模板 verify_result.py（rtol/atol=1e-3 + 失配率 ≤0.1%）→ 可通过；
       · 若判题要求**全部元素**严格满足 <1e-3 → bf16 上存在失败风险。
    该口径必须由真机/平台确认（见 research-report.md 的未确认事项）。""")
    print()


if __name__ == "__main__":
    main()
