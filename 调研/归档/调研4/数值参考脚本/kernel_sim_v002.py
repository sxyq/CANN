#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kernel_sim_v002.py — 对 `提交/V002/kernel.asc` 算法语义的 CPU 等价模拟

目的
----
本机无 CANN / 无昇腾 NPU，无法编译或运行 Ascend C。因此用 numpy 逐位复现
V002 的**算法结构与数值链**，与官方 golden（`scripts/AddRmsNormBias.py` 的 impl）
逐元素比对，用于：

  1. 判断 V002 的算法结构（两遍扫描 + FP32 中间 + ReduceSum 分块 + 标量累加 + CAST_RINT）
     是否与 golden 在数值上等价；
  2. 定位差异来源（分块累加顺序、1/rms 与 /rms 的差异、sqrt 精度、输出舍入）；
  3. 给出「本机 CPU 口径」的误差表，作为真机验证的对照基线。

重要声明
--------
本脚本的结论**全部是 CPU/numpy 口径**，不是 NPU 实测。
它不能证明 NPU 上的编译、精度或性能任何一项。numpy 的 float32 语义与
昇腾向量单元的 FP32 语义（例如是否使用 FMA、ReduceSum 的树形归约顺序）
不保证逐位一致，因此本脚本只能验证「算法结构是否合理」，不能替代真机精度验证。

用法
----
    /Users/sunyiyang/.workbuddy/binaries/python/envs/default/bin/python kernel_sim_v002.py
"""

import sys
import numpy as np

try:
    import ml_dtypes
    from ml_dtypes import bfloat16 as BF16
except ImportError:  # pragma: no cover
    ml_dtypes = None
    BF16 = None

F32 = np.float32

# ---------------------------------------------------------------------------
# V002 的常量（与 提交/V002/kernel.asc 第 7-12 行一致）
# ---------------------------------------------------------------------------
TILE_HALF = 4096      # fp16/bf16 每块元素数
TILE_FLOAT = 2048     # fp32 每块元素数
MAX_DIM = 32768


def tile_len_for(dtype):
    """V002: tile_len_ = (sizeof(T) == sizeof(float)) ? TILE_FLOAT : TILE_HALF"""
    return TILE_FLOAT if dtype == np.float32 else TILE_HALF


def to_f32(a, dtype):
    """V002 MakeValue: fp16/bf16 先 CAST_NONE 到 fp32（精确）；fp32 直接使用。"""
    if dtype == np.float32:
        return a.astype(F32)
    return a.astype(F32)   # fp16->fp32 与 bf16->fp32 都是无损/精确扩展


def cast_out(a_f32, dtype):
    """V002 StoreOutput: CAST_RINT（= RNE，四舍六入五成双），与 numpy astype 一致。"""
    if dtype == np.float32:
        return a_f32.astype(F32)
    if dtype == np.float16:
        return a_f32.astype(np.float16)
    return a_f32.astype(BF16)


def rne_f32(x):
    """把 python float 标量按 float32 舍入（模拟标量寄存器为 fp32）。"""
    return F32(x)


def v002_row_pass1(row_x, row_r, D, dtype, tile_len):
    """
    V002 Process/ReduceRow + CopyAndReduce：
      total = 0.0f
      每个 tile：y = cast(x) + cast(res);  y2 = y*y;  s = ReduceSum(y2, count=calc_len)
                 total += s            # 标量累加，fp32
    返回 fp32 标量 total。
    """
    total = F32(0.0)
    offset = 0
    while offset < D:
        valid = min(tile_len, D - offset)
        # DataCopyPad 右侧补 0 到 32B 对齐；补 0 不影响平方和，但参与 ReduceSum 的 count
        elem_per_block = 32 // np.dtype(dtype).itemsize
        calc = ((valid + elem_per_block - 1) // elem_per_block) * elem_per_block
        seg_x = to_f32(row_x[offset:offset + valid], dtype)
        seg_r = to_f32(row_r[offset:offset + valid], dtype)
        y = (seg_x + seg_r).astype(F32)          # Add（fp32）
        y2 = (y * y).astype(F32)                 # Mul（就地平方）
        if calc > valid:                          # 补零部分
            y2 = np.concatenate([y2, np.zeros(calc - valid, dtype=F32)])
        # ReduceSum：昇腾为树形/二叉树归约；numpy np.sum 在 float32 上使用 pairwise
        # 求和（也是树形），量级相近但顺序不保证逐位一致。
        s = np.sum(y2, dtype=F32)
        total = F32(total + F32(s))              # 标量累加
        offset += valid
    return total


def v002_row_pass2(row_x, row_r, g, b, D, dtype, tile_len, scale):
    """
    V002 NormalizeRow：
      y = cast(x)+cast(res)
      Mul(y, y, scale)                       # Muls(y, 1/rms)：先归一化
      Mul(y, y, gamma_f32); Add(y, y, bias_f32)
      CAST_RINT 回原 dtype
    """
    out = np.empty(D, dtype=dtype)
    offset = 0
    while offset < D:
        valid = min(tile_len, D - offset)
        seg_x = to_f32(row_x[offset:offset + valid], dtype)
        seg_r = to_f32(row_r[offset:offset + valid], dtype)
        y = (seg_x + seg_r).astype(F32)
        y = (y * F32(scale)).astype(F32)                       # Muls(y, scale)
        gg = to_f32(g[offset:offset + valid], dtype)
        bb = to_f32(b[offset:offset + valid], dtype)
        y = (y * gg).astype(F32)                               # Mul(y, gamma)
        y = (y + bb).astype(F32)                               # Add(y, bias)
        out[offset:offset + valid] = cast_out(y, dtype)
        offset += valid
    return out


def v002_forward(x, residual, gamma, bias, epsilon):
    """完整模拟 V002 的 forward。x/residual: (..., D)"""
    dtype = x.dtype
    D = x.shape[-1]
    tile_len = tile_len_for(dtype)
    outer = int(np.prod(x.shape[:-1])) if x.ndim > 1 else 1
    x2 = x.reshape(outer, D)
    r2 = residual.reshape(outer, D)
    out = np.empty((outer, D), dtype=dtype)
    for row in range(outer):
        total = v002_row_pass1(x2[row], r2[row], D, dtype, tile_len)
        # rms = sqrtf(sum / D + eps)   —— V002 是标量 sqrtf
        mean = F32(F32(total) / F32(D))
        rms = F32(np.sqrt(F32(mean + F32(epsilon)), dtype=F32))
        scale = F32(F32(1.0) / rms)     # V002: 1.0f / rms
        out[row] = v002_row_pass2(x2[row], r2[row], gamma, bias, D, dtype,
                                  tile_len, scale)
    return out.reshape(x.shape)


def golden(x, residual, gamma, bias, epsilon=1e-5):
    """官方 golden：scripts/AddRmsNormBias.py 的 impl（numpy 组合实现）。"""
    orig = x.dtype
    x_f = x.astype(F32)
    r_f = residual.astype(F32)
    g_f = gamma.astype(F32)
    b_f = bias.astype(F32)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + F32(epsilon))
    out = y / rms * g_f
    out = out + b_f
    if orig == np.float16:
        return out.astype(np.float16)
    if orig == BF16:
        return out.astype(BF16)
    return out.astype(F32)


def truth_fp64(x, residual, gamma, bias, epsilon=1e-5):
    """高精度参考（fp64），用来看 golden 离真值有多远。"""
    x_f = x.astype(np.float64)
    r_f = residual.astype(np.float64)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + np.float64(epsilon))
    return (y / rms) * gamma.astype(np.float64) + bias.astype(np.float64)


def stats(a, b):
    """逐元素相对/绝对误差（b 为参考）。"""
    a_f = a.astype(F32).astype(np.float64)
    b_f = b.astype(F32).astype(np.float64)
    absd = np.abs(a_f - b_f)
    denom = np.maximum(np.abs(b_f), 1e-30)
    rel = absd / denom
    return float(absd.max()), float(np.median(absd)), float(rel.max()), float(np.median(rel))


def isclose_rate(a, b, rtol, atol):
    a_f = a.astype(F32).astype(np.float64)
    b_f = b.astype(F32).astype(np.float64)
    ok = np.isclose(a_f, b_f, rtol=rtol, atol=atol, equal_nan=True)
    return int(np.sum(~ok)), ok.size


# ---------------------------------------------------------------------------
# 测试矩阵
# ---------------------------------------------------------------------------
def make_case(shape, D, dtype, seed):
    rng = np.random.default_rng(seed)
    xs = list(shape) + [D]
    x = rng.uniform(-2, 2, size=xs).astype(dtype)
    r = rng.uniform(-2, 2, size=xs).astype(dtype)
    g = rng.uniform(0.8, 1.2, size=(D,)).astype(dtype)
    b = rng.uniform(-0.3, 0.3, size=(D,)).astype(dtype)
    return x, r, g, b


CASES = [
    # (标签, rank/形状前缀, D, dtype)
    ("2D-小D-64",       (2,),      64,   np.float16),
    ("2D-非32倍数-67",  (2,),      67,   np.float16),
    ("2D-非32倍数-129", (2,),      129,  np.float16),
    ("2D-96",           (3,),      96,   np.float16),
    ("3D-192",          (2, 3),    192,  np.float16),
    ("3D-576",          (2, 2),    576,  np.float16),
    ("2D-1024",         (4,),      1024, np.float16),
    ("2D-4096",         (2,),      4096, np.float16),
    ("2D-4097分块跨界", (2,),      4097, np.float16),
    ("2D-32768",        (2,),      32768, np.float16),
    ("4D-1000",         (2, 2, 2), 1000, np.float16),
    ("2D-64-bf16",      (2,),      64,   BF16),
    ("2D-129-bf16",     (2,),      129,  BF16),
    ("2D-1024-bf16",    (2,),      1024, BF16),
    ("2D-4096-bf16",    (2,),      4096, BF16),
    ("2D-32768-bf16",   (1,),      32768, BF16),
    ("2D-64-fp32",      (2,),      64,   np.float32),
    ("2D-129-fp32",     (2,),      129,  np.float32),
    ("2D-2048-fp32",    (2,),      2048, np.float32),
    ("2D-2049-fp32",    (2,),      2049, np.float32),
    ("2D-32768-fp32",   (1,),      32768, np.float32),
]

# 判题口径（模板 verify_result.py + 题面精度表）
TOL = {np.float32: (1e-4, 1e-4), np.float16: (1e-3, 1e-3)}
if BF16 is not None:
    TOL[BF16] = (1e-3, 1e-3)


def main():
    if ml_dtypes is None:
        print("WARNING: ml_dtypes 未安装，bf16 用例将被跳过（pip install ml_dtypes）")
    print("=" * 108)
    print("V002 算法结构 CPU 等价模拟 vs 官方 golden  —— 本机 numpy 口径，非 NPU 实测")
    print("=" * 108)
    hdr = (f"{'用例':<18}{'dtype':<9}{'D':>7}  "
           f"{'vs golden 最大相对':>18}{'vs golden 失配':>16}  "
           f"{'golden vs fp64 最大相对':>24}{'判定':>7}")
    print(hdr)
    print("-" * 108)

    fails = []
    for i, (tag, prefix, D, dt) in enumerate(CASES):
        if dt is BF16 and ml_dtypes is None:
            continue
        if D > MAX_DIM:
            continue
        x, r, g, b = make_case(prefix, D, dt, seed=1000 + i)
        gd = golden(x, r, g, b)
        sim = v002_forward(x, r, g, b, 1e-5)
        tr = truth_fp64(x, r, g, b)

        rtol, atol = TOL[dt]
        miss, total = isclose_rate(sim, gd, rtol, atol)
        _, _, sim_rel_max, _ = stats(sim, gd)
        _, _, gd_rel_max, _ = stats(gd, tr)

        rate = miss / total
        verdict = "PASS" if rate <= 0.001 else "CHECK"
        if verdict == "CHECK":
            fails.append(tag)
        print(f"{tag:<18}{np.dtype(dt).name:<9}{D:>7}  "
              f"{sim_rel_max:>18.3e}{rate:>15.4%}  "
              f"{gd_rel_max:>24.3e}{verdict:>7}")

    print("-" * 108)
    print(f"合计 {len(CASES)} 组；判定 CHECK 的用例：{fails if fails else '无'}")
    print()
    print("口径说明：")
    print("  · 'vs golden 失配' = 按 isclose(rtol=1e-3/1e-4, atol=1e-3/1e-4) 统计的失配元素比例；")
    print("    守门线取 0.1%，与模板 verify_result.py 的 tol 语义一致（判题端是否相同未证实）。")
    print("  · 'golden vs fp64 最大相对' 说明 golden 自身离真值的距离，用于区分")
    print("    『实现误差』与『dtype 表示误差』。bf16 该值必然在 1e-3 量级，属表示误差而非实现误差。")
    print("  · 本模拟不建模 DMA 对齐、UB 容量、指令调度、FMA 融合与 ReduceSum 的真实树形顺序，")
    print("    因此不能替代真机精度验证。")


if __name__ == "__main__":
    sys.exit(main())
