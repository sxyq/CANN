#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
AddRmsNormBias 数值精度实验（Agent 06）
运行环境：/usr/bin/python3  (numpy 2.0.2 + ml_dtypes 0.5.4)
所有数字均为本机 CPU / numpy 口径，非 NPU 实测。

运行：
  /usr/bin/python3 numerics_experiments.py

设计要点（对照 kernel.asc V002）：
  - 候选实现：fp16/bf16 先 CAST_NONE 到 FP32（精确，无损失），Add 得 y，
    Mul(y,y) 平方（FP32），ReduceSum 分块累加到标量 FP32，
    sqrtf(标量 FP32) 得 rms，Muls(1/rms) 归一化，末尾 CAST_RINT(RNE) 回原类型。
  - golden：统一 astype(float32)，rms=sqrt(mean(y*y)+eps)，out=y/rms*gamma+bias，astype(原dtype)。
"""
import numpy as np
from ml_dtypes import bfloat16

RNG = np.random.default_rng(20260911)
EPS = 1e-5


def is_bf16(dt):
    return 'bfloat16' in str(dt) or 'bf16' in str(dt)


# ---------- 参考实现 ----------

def golden(x, residual, gamma, bias, epsilon=EPS):
    """精确复刻 AddRmsNormBias.py 的 impl（float32 内部计算）。"""
    orig = x.dtype
    xf = x.astype(np.float32)
    rf = residual.astype(np.float32)
    gf = gamma.astype(np.float32)
    bf = bias.astype(np.float32)
    y = xf + rf
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + epsilon)
    out = y / rms * gf + bf
    if is_bf16(orig):
        return out.astype(bfloat16)
    if orig == np.float16:
        return out.astype(np.float16)
    return out.astype(np.float32)


def truth64(x, residual, gamma, bias, epsilon=EPS):
    """FP64 高精度真值（作为误差上界基准）。"""
    xd = x.astype(np.float64)
    rd = residual.astype(np.float64)
    gd = gamma.astype(np.float64)
    bd = bias.astype(np.float64)
    y = xd + rd
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + epsilon)
    return y / rms * gd + bd


def npu_emu(x, residual, gamma, bias, epsilon=EPS, block=4096, odtype=None):
    """模拟候选 kernel 的 FP32 计算链 + 分块归约 + RNE 回写。"""
    if odtype is None:
        odtype = x.dtype
    xf = x.astype(np.float32)
    rf = residual.astype(np.float32)
    gf = gamma.astype(np.float32)
    bf = bias.astype(np.float32)
    y = xf + rf                     # Add（FP32）
    ysq = y * y                     # 平方（FP32，先做 Cast 再算，无 fp16 溢出）
    flat = ysq.reshape(-1, ysq.shape[-1])
    D = flat.shape[-1]
    totals = np.zeros(flat.shape[0], dtype=np.float32)
    for i in range(flat.shape[0]):
        row = flat[i]
        s = np.float32(0.0)
        o = 0
        while o < D:
            vl = D - o if D - o < block else block
            part = row[o:o + vl]
            psum = np.float32(0.0)
            for v in part:          # 块内顺序累加（模拟 ReduceSum 标量输出）
                psum = psum + v
            s = s + psum            # 块间顺序累加到标量
            o += vl
        totals[i] = s
    mean = totals / np.float32(D)   # square_sum / dim
    rms = np.sqrt(mean + np.float32(epsilon))   # sqrtf（标量 FP32）
    scale = np.float32(1.0) / rms
    scale3 = scale.reshape(-1, 1)
    out = y * scale3 * gf + bf      # Muls + ApplyAffine（FP32）
    if is_bf16(odtype):
        return out.astype(bfloat16)
    if odtype == np.float16:
        return out.astype(np.float16)
    return out.astype(np.float32)


# ---------- 工具 ----------

def rel_err(a, b):
    """相对误差（elementwise），处理 0。"""
    a = np.asarray(a, dtype=np.float64)
    b = np.asarray(b, dtype=np.float64)
    denom = np.maximum(np.abs(b), 1e-12)
    return np.abs(a - b) / denom


def make_case(shape, D, dtype, scale=1.0, seed=None):
    rg = np.random.default_rng(seed if seed is not None else RNG.integers(0, 2**31))
    x = (rg.uniform(-1, 1, shape) * scale).astype(dtype)
    r = (rg.uniform(-1, 1, shape) * scale).astype(dtype)
    g = rg.uniform(0.9, 1.1, (D,)).astype(dtype)
    b = rg.uniform(-0.1, 0.1, (D,)).astype(dtype)
    return x, r, g, b


print("=" * 78)
print("实验 A：golden 复现 + 测试矩阵跑通（多种 dtype/rank/D/outer）")
print("=" * 78)
matrix = [
    ("FP32", np.float32, (8, 64)),
    ("FP16", np.float16, (8, 64)),
    ("BF16", bfloat16, (8, 64)),
    ("FP32", np.float32, (4, 16, 128)),
    ("FP16", np.float16, (2, 3, 4, 32)),
    ("BF16", bfloat16, (2, 3, 4, 32)),
    ("FP32", np.float32, (512, 32768)),
    ("BF16", bfloat16, (512, 32768)),
]
for nm, dt, shp in matrix:
    D = shp[-1]
    x, r, g, b = make_case(shp, D, dt, seed=hash((nm, shp)) & 0x7fffffff)
    out = golden(x, r, g, b)
    assert out.shape == shp and (out.dtype == dt or is_bf16(out.dtype) and is_bf16(dt)), f"shape/dtype fail {nm} {shp}"
    # 与 FP64 真值比对（同口径：都转 fp32 看相对误差）
    t = truth64(x, r, g, b)
    re = rel_err(out.astype(np.float32), t.astype(np.float32))
    print(f"  {nm:4s} shape={str(shp):18s} D={D:6d} -> maxRelErr(vs FP64)={re.max():.3e} meanRelErr={re.mean():.3e}")


print()
print("=" * 78)
print("实验 B：两种判定口径（NPU 模拟 vs FP64 真值 / vs 同 dtype golden）")
print("=" * 78)
for nm, dt, shp, blk in [
    ("FP32", np.float32, (8, 1024), 4096),
    ("FP16", np.float16, (8, 1024), 4096),
    ("BF16", bfloat16, (8, 1024), 4096),
    ("BF16", bfloat16, (512, 32768), 4096),
    ("FP16", np.float16, (512, 32768), 4096),
]:
    D = shp[-1]
    x, r, g, b = make_case(shp, D, dt, seed=hash((nm, "B")) & 0x7fffffff)
    g_out = golden(x, r, g, b)
    n_out = npu_emu(x, r, g, b, block=blk, odtype=dt)
    t = truth64(x, r, g, b)
    # 口径1：vs FP64 真值
    re_true = rel_err(n_out.astype(np.float32), t.astype(np.float32))
    # 口径2：vs 同 dtype golden（judge 做法，isclose(rtol=1e-3,atol=1e-3)）
    cmp_n = n_out.astype(np.float32) if is_bf16(dt) else n_out
    cmp_g = g_out.astype(np.float32) if is_bf16(dt) else g_out
    isc = np.isclose(cmp_n, cmp_g, rtol=1e-3, atol=1e-3, equal_nan=True)
    exact = np.sum(cmp_n == cmp_g)
    fail = np.sum(~isc)
    print(f"  {nm:4s} D={D:6d} blk={blk:5d}: vsFP64 maxRel={re_true.max():.3e} meanRel={re_true.mean():.3e}"
          f" | vsGolden exact={exact}/{n_out.size}({100*exact/n_out.size:.2f}%) fail={fail} "
          f"maxAbsDiff={np.abs(cmp_n-cmp_g).max():.3e}")


print()
print("=" * 78)
print("实验 C：D=32768 不同分块大小的平方和归约相对误差（vs FP64 真值）")
print("=" * 78)
D = 32768
x, r, g, b = make_case((64, D), D, np.float32, seed=777)
xf = x.astype(np.float32) + r.astype(np.float32)
ysq = (xf * xf).reshape(-1, D)
truth_sum = ysq.astype(np.float64).sum(axis=1)          # FP64 真值
print(f"  D={D}, rows={ysq.shape[0]}, truth_sum 量级 ~ {truth_sum.mean():.3e}")
for blk in [512, 1024, 2048, 4096, 8192, 32768]:
    totals = np.zeros(ysq.shape[0], dtype=np.float32)
    for i in range(ysq.shape[0]):
        row = ysq[i]
        s = np.float32(0.0)
        o = 0
        while o < D:
            vl = D - o if D - o < blk else blk
            psum = np.float32(0.0)
            for v in row[o:o + vl]:
                psum = psum + v
            s = s + psum
            o += vl
        totals[i] = s
    re = rel_err(totals, truth_sum)
    print(f"  block={blk:6d}: maxRelErr={re.max():.3e}  meanRelErr={re.mean():.3e}  maxAbsErr={np.abs(totals-truth_sum).max():.3e}")
# 参考：numpy 自带 float32 sum（用 pairwise-ish 内部实现）
ref = ysq.astype(np.float32).sum(axis=1)
re_ref = rel_err(ref, truth_sum)
print(f"  numpy.float32.sum: maxRelErr={re_ref.max():.3e}  meanRelErr={re_ref.mean():.3e}")


print()
print("=" * 78)
print("实验 D：sqrt 与 rsqrt 近似误差（目标预算 1e-3）")
print("=" * 78)
# s = mean+eps 的若干代表值（覆盖不同量级）
s_vals = np.float32([1e-6, 1e-4, 1e-2, 0.1, 1.0, 10.0, 100.0, 1000.0])
r_exact = 1.0 / np.sqrt(s_vals.astype(np.float64))            # FP64 真值
r_sqrt32 = (1.0 / np.sqrt(s_vals)).astype(np.float32)         # kernel 实际路径：1/sqrtf
re_sqrt = np.abs(r_sqrt32.astype(np.float64) - r_exact) / r_exact
print("  s 值          1/sqrt(s) FP64       1/sqrt(s) FP32       relErr(FP32路径)")
for sv, re32, ex in zip(s_vals, re_sqrt, r_exact):
    print(f"  {float(sv):12.4g}  {ex:18.10g}  {float(r_sqrt32[list(s_vals).index(sv)]):18.10g}  {re32:.3e}")
print(f"  -> FP32 sqrt 路径最大相对误差 = {re_sqrt.max():.3e}  (<< 1e-3, 充足)")
# 模拟硬件 rsqrt 原始近似（相对误差上界 2^-20 ≈ 9.54e-7），并演示 1 次 Newton 迭代收敛
delta_raw = 2.0 ** -20
r_raw = (r_sqrt32.astype(np.float64) * (1.0 + delta_raw)).astype(np.float32)  # 人为加 2^-20 偏置模拟原始 rsqrt
# 1 次 Newton: x = x*(1.5 - 0.5*s*x*x)
x_n = (r_raw * (np.float32(1.5) - np.float32(0.5) * s_vals * r_raw * r_raw)).astype(np.float32)
re_raw = np.abs(r_raw.astype(np.float64) - r_exact) / r_exact
re_newton = np.abs(x_n.astype(np.float64) - r_exact) / r_exact
print(f"  原始 rsqrt 近似(偏置 2^-20) 最大 relErr = {re_raw.max():.3e}")
print(f"  1 次 Newton 迭代后  最大 relErr = {re_newton.max():.3e}")
print(f"  结论：即便用 2^-20 精度的 rsqrt，输出 = y*scale*gamma，scale 误差乘性传递，")
print(f"        输出相对误差 <= 2^-20 ≈ 9.5e-7 << 1e-3，预算充足。")


print()
print("=" * 78)
print("实验 E：epsilon 位置写错（sqrt(mean)+eps vs sqrt(mean+eps)）的偏差")
print("=" * 78)
def out_formula(y, g, b, eps, wrong_pos=False):
    y = y.astype(np.float64)
    g = g.astype(np.float64)
    b = b.astype(np.float64)
    sq = (y * y).mean(axis=-1, keepdims=True)
    if wrong_pos:
        rms = np.sqrt(sq) + eps
    else:
        rms = np.sqrt(sq + eps)
    return y / rms * g + b

# 普通量级
x, r, g, b = make_case((256, 1024), 1024, np.float32, seed=42)
y = (x.astype(np.float32) + r.astype(np.float32))
gd = g.astype(np.float64)
bd = b.astype(np.float64)
correct = out_formula(y, gd, bd, EPS, wrong_pos=False)
wrong = out_formula(y, gd, bd, EPS, wrong_pos=True)
re_normal = rel_err(wrong, correct)
print(f"  普通输入(|y|~1): maxRelErr(错位置) = {re_normal.max():.3e}  meanRelErr = {re_normal.mean():.3e}")
# 极端：输入极小（mean << eps），此时错位置影响巨大
x2 = (np.full((256, 1024), 1e-4, dtype=np.float32)
      + np.float32(1e-6) * np.random.default_rng(1).standard_normal((256, 1024)).astype(np.float32))
r2 = np.float32(1e-5) * np.random.default_rng(2).standard_normal((256, 1024)).astype(np.float32)
g2 = np.ones((1024,), dtype=np.float64)
b2 = np.zeros((1024,), dtype=np.float64)
c2 = out_formula(x2.astype(np.float32) + r2, g2, b2, EPS, wrong_pos=False)
w2 = out_formula(x2.astype(np.float32) + r2, g2, b2, EPS, wrong_pos=True)
re_tiny = rel_err(w2, c2)
print(f"  极小输入(mean<<eps): maxRelErr(错位置) = {re_tiny.max():.3e}  meanRelErr = {re_tiny.mean():.3e}")
# 量化示例：单值
for mv, eps in [(0.5, EPS), (1e-3, EPS), (1e-8, EPS)]:
    rc = np.sqrt(mv + eps)
    rw = np.sqrt(mv) + eps
    print(f"    mean={mv:.2e}, eps={eps:.0e}: 正确rms={rc:.8f} 错误rms={rw:.8f} rms相对差={(rw-rc)/rc:+.3e}")


print()
print("=" * 78)
print("实验 F：FP16 平方溢出边界（|y|^2 > 65504）")
print("=" * 78)
fp16_max = 65504.0
bound = np.sqrt(fp16_max)
print(f"  fp16 最大有限值 = {fp16_max}; 平方溢出阈值 |y| = sqrt(65504) = {bound:.4f}")
print(f"  -> 输入 |y| >= {bound:.2f} 时，若直接在 FP16 中做 Mul(y,y) 会得 Inf。")
print(f"  候选 kernel 先 CAST_NONE -> FP32 再 Mul：FP32 最大 ~3.4e38，")
# 演示：|y|=300（fp16 可表示，平方=90000>65504）在 fp32 中安全
for yv in [100.0, 256.0, 300.0, 1000.0, 1e4]:
    ysq_fp16 = np.float16(yv) * np.float16(yv)
    ysq_fp32 = np.float32(yv) * np.float32(yv)
    print(f"    |y|={yv:9.1f}: fp16 平方={float(ysq_fp16):.6g}  fp32 平方={float(ysq_fp32):.6g}  "
          f"fp32溢出={'是' if not np.isfinite(ysq_fp32) else '否'}")
print(f"  结论：CAST_NONE 到 FP32 后平方完全消除 FP16 溢出风险（只要不在 FP16 中做 Mul）。")


print()
print("=" * 78)
print("实验 G：NaN / Inf 输入传播行为（公式 vs 实现）")
print("=" * 78)
shp = (4, 64)
D = 64
x, r, g, b = make_case(shp, D, np.float32, seed=99)
# 1) 单个 NaN
x_nan = x.copy()
x_nan.flat[10] = np.float32('nan')
g_nan = golden(x_nan, r, g, b)
any_nan = np.any(np.isnan(g_nan))
print(f"  含 1 个 NaN 输入 -> golden 输出是否含 NaN: {any_nan}; "
      f"verify 用 equal_nan=True 故 NaN==NaN 判过。")
# 2) Inf 输入
x_inf = x.copy()
x_inf.flat[0] = np.float32('inf')
g_inf = golden(x_inf, r, g, b)
print(f"  含 Inf 输入 -> golden 输出含 NaN={np.any(np.isnan(g_inf))} 含Inf={np.any(np.isinf(g_inf))}")
print(f"  理论：y=Inf -> y^2=Inf -> sum=Inf -> rms=Inf -> y/rms=Inf/Inf=NaN -> 整行 NaN。")
# 3) 全 0 输入（eps 默认 1e-5 保驾）
x0 = np.zeros(shp, dtype=np.float32)
g0 = golden(x0, x0, g, b)
print(f"  全 0 输入 -> 输出 = bias (gamma*0 + bias) 均={g0.mean():.4f}, 应等于 bias 均值 {b.astype(np.float32).mean():.4f}")
# 4) 风险写法：rsqrt(Inf) 被某些实现返回 0（而非 NaN/Inf）
print(f"  风险：若实现用 rsqrt 且 rsqrt(Inf)=0（部分硬件/近似如此），则 y*0=0（非 NaN），")
print(f"        与 golden 的 NaN 不一致 -> 该元素判不过（equal_nan 要求 NaN）。需真机验证 rsqrt(Inf)。")
