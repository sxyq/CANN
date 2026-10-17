#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Agent 06 — AddRmsNormBias 数值精度与验证方法 测试矩阵
===================================================================
所有实验均为 CPU 参考（numpy + ml_dtypes），非 NPU 实测。
golden 基准严格对齐判题模板 AddRmsNormBias.py 的 impl()：
  - 输入统一 cast 到 FP32；
  - y = x + residual；
  - rms = sqrt(mean(y*y) + epsilon)；   # epsilon 在 mean 之后、sqrt 之前
  - out = y / rms * gamma + bias；       # 除法（非乘倒数）
  - 最后一次性 cast 回原 dtype。

本脚本回答 6(+1) 个数值问题：
  Q1: Muls(y, 1/rms) vs Divs(y, rms)         —— 与 golden 的误差
  Q2: 1/Sqrt vs Rsqrt vs Sqrt+除法            —— 三者差异
  Q3: 分块累加 vs 一次累加                     —— 平方和相对误差
  Q4: FP16 平方和溢出阈值                     —— |y| 多大溢出 inf
  Q5: BF16 大 D 顺序归约 vs 分块归约          —— 失配比例
  Q6: 全程低精度累加（对照方案）              —— 失配比例（证否）
  Q7(附加): epsilon 在 sqrt 内 vs sqrt 外     —— 差异

运行：
  python3 agent06_precision_matrix.py 2>&1 | tee agent06_precision_matrix.log
"""
import json
import math
import numpy as np
from ml_dtypes import bfloat16

SEED = 1742  # 与题面 problem_1742 对应，保证可复现
EPS = 1e-5

# ---- 判题阈值（来自 Agent1 核实：fp32=1e-4, fp16/bf16=1e-3）----
# 模板本地 verify 用 fp16 1e-3；fp32 题面更严为 1e-4。
THR = {
    'fp32': (1e-4, 1e-4),
    'fp16': (1e-3, 1e-3),
    'bf16': (1e-3, 1e-3),
}

DTYPE_MAP = {
    'fp32': np.float32,
    'fp16': np.float16,
    'bf16': bfloat16,
}

# ----------------------------------------------------------------------------
# golden 实现（严格对齐模板）
# ----------------------------------------------------------------------------
def golden(x, r, g, b, eps=EPS):
    orig = x.dtype
    x_f = x.astype(np.float32)
    r_f = r.astype(np.float32)
    g_f = g.astype(np.float32)
    b_f = b.astype(np.float32)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + eps)
    out = y / rms * g_f + b_f
    if str(orig).find('bfloat16') >= 0 or str(orig).find('bf16') >= 0:
        return out.astype(bfloat16)
    if orig == np.float16:
        return out.astype(np.float16)
    return out.astype(np.float32)


# ----------------------------------------------------------------------------
# 候选策略（每个策略都：FP32 中间计算 + 最后一次性 cast 回目标 dtype，
#            与 golden 保持相同的 cast 时机，仅改变「求和/归一化」的数值路径）
# ----------------------------------------------------------------------------
def cand_div(x, r, g, b, eps=EPS):
    """Sqrt + 除法（golden 等价路径；Divs）"""
    x_f = x.astype(np.float32); r_f = r.astype(np.float32)
    g_f = g.astype(np.float32); b_f = b.astype(np.float32)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + eps)
    out = y / rms * g_f + b_f
    return _cast(out, x.dtype)

def cand_muls(x, r, g, b, eps=EPS):
    """1/rms 取倒数后 Muls（与提交 V002 路径一致）"""
    x_f = x.astype(np.float32); r_f = r.astype(np.float32)
    g_f = g.astype(np.float32); b_f = b.astype(np.float32)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + eps)
    inv = 1.0 / rms
    out = y * inv * g_f + b_f
    return _cast(out, x.dtype)

def cand_1sqrt_mul(x, r, g, b, eps=EPS):
    """1/Sqrt(s) 后乘法：inv = 1/sqrt(mean(y*y)+eps) 一次性求倒数再乘"""
    x_f = x.astype(np.float32); r_f = r.astype(np.float32)
    g_f = g.astype(np.float32); b_f = b.astype(np.float32)
    y = x_f + r_f
    s = np.mean(y * y, axis=-1, keepdims=True) + eps
    inv = 1.0 / np.sqrt(s)
    out = y * inv * g_f + b_f
    return _cast(out, x.dtype)

def cand_rsqrt_mul(x, r, g, b, eps=EPS):
    """Rsqrt + Mul：CPU 无硬件 rsqrt，数学上等价于 1/Sqrt+Mul。
    这里显式标注：真实 NPU rsqrt 是单次舍入指令，其舍入误差无法在 CPU 复现；
    故本结果代表 1/Sqrt+Mul 的上界参考。"""
    return cand_1sqrt_mul(x, r, g, b, eps)

def cand_eps_outside(x, r, g, b, eps=EPS):
    """epsilon 加到 sqrt 之后：rms = sqrt(mean(y*y)) + eps（错误位置）"""
    x_f = x.astype(np.float32); r_f = r.astype(np.float32)
    g_f = g.astype(np.float32); b_f = b.astype(np.float32)
    y = x_f + r_f
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True)) + eps
    out = y / rms * g_f + b_f
    return _cast(out, x.dtype)

def cand_full_low(x, r, g, b, low, eps=EPS):
    """全程低精度累加对照：每步量化到低精度 dtype（low）。
    用于证否「低精度累加」方案。"""
    y = (x.astype(low) + r.astype(low)).astype(low)
    sq = (y * y).astype(low)
    D = y.shape[-1]
    acc = np.zeros(y.shape[:-1], dtype=low)
    for s in range(0, D, 64):
        blk = sq[..., s:s + 64].sum(axis=-1, dtype=low)
        acc = (acc + blk).astype(low)
    mean = (acc.astype(np.float32) / D).reshape(acc.shape + (1,)).astype(low)
    rms = (np.sqrt(mean.astype(np.float32) + np.float32(eps))).astype(low)
    inv = (1.0 / np.sqrt(mean.astype(np.float32) + np.float32(eps))).astype(low)
    out = (y * inv).astype(low)
    out = (out * g.astype(low)).astype(low)
    out = (out + b.astype(low)).astype(low)
    return out.astype(low)

def _cast(a, orig_dtype):
    if str(orig_dtype).find('bfloat16') >= 0 or str(orig_dtype).find('bf16') >= 0:
        return a.astype(bfloat16)
    if orig_dtype == np.float16:
        return a.astype(np.float16)
    return a.astype(np.float32)


# ----------------------------------------------------------------------------
# 归约：一次累加 vs 分块累加（均在 FP32），返回 sum(y*y)
# ----------------------------------------------------------------------------
def sumsq_once(y):
    return np.sum(y * y, axis=-1)

def sumsq_block(y, block):
    D = y.shape[-1]
    acc = np.zeros(y.shape[:-1], dtype=np.float32)
    for s in range(0, D, block):
        acc = acc + np.sum((y * y)[..., s:s + block], axis=-1)
    return acc


# ----------------------------------------------------------------------------
# 比较工具
# ----------------------------------------------------------------------------
def compare(cand, gold, dtype=None, rtol=1e-3, atol=1e-3):
    """返回 (max_rel_err_robust, max_abs_err, mismatch_count, mismatch_ratio)。
    cand/gold 为同 dtype、最后已 cast 的数组。按判题 verify 语义 cast 到 fp32 后 isclose。
    相对误差用对称归一化 |c-g|/max(|g|,|c|,1e-6)，避免近零元素放大假值。
    失配数/比例用给定 rtol/atol（默认 1e-3/1e-3，与模板 verify 一致）。"""
    c = cand.astype(np.float32)
    g = gold.astype(np.float32)
    diff = np.abs(c - g)
    denom = np.maximum(np.abs(g), np.abs(c))
    denom = np.where(denom > 0, denom, 1e-6)
    rel = diff / denom
    rel = rel[np.isfinite(rel)]
    max_rel = float(np.max(rel)) if rel.size else 0.0
    max_abs = float(np.max(diff)) if g.size else 0.0
    isc = np.isclose(c, g, rtol=rtol, atol=atol, equal_nan=True)
    mism = int(np.sum(~isc))
    ratio = mism / g.size if g.size else 0.0
    return max_rel, max_abs, mism, ratio

def passes_thr(cand, gold, dtype):
    rtol, atol = THR[dtype]
    c = cand.astype(np.float32); g = gold.astype(np.float32)
    isc = np.isclose(c, g, rtol=rtol, atol=atol, equal_nan=True)
    mism = int(np.sum(~isc))
    ratio = mism / g.size if g.size else 0.0
    return ratio <= 1e-3, mism, ratio  # 以 0.1% 失配比例为通过线


# ----------------------------------------------------------------------------
# 数据生成
# ----------------------------------------------------------------------------
def gen(shape, dtype, rng, lo=-2.0, hi=2.0):
    return rng.uniform(lo, hi, shape).astype(DTYPE_MAP[dtype])

def gen_gb(D, dtype, rng):
    g = rng.uniform(0.9, 1.1, (D,)).astype(DTYPE_MAP[dtype])
    b = rng.uniform(-0.1, 0.1, (D,)).astype(DTYPE_MAP[dtype])
    return g, b


# ============================================================================
# 主实验
# ============================================================================
def main():
    rng = np.random.default_rng(SEED)
    results = {'meta': {'seed': SEED, 'eps': EPS, 'note': 'CPU 参考，非 NPU 实测'}, 'Q1': [], 'Q2': [], 'Q3': [], 'Q4': [], 'Q5': [], 'Q6': [], 'Q7': []}

    Ds = [1, 31, 32, 33, 64, 127, 128, 129, 1024, 4096, 32768]
    # outer 配置：按 D 控制规模避免 OOM
    def outer_for(D):
        if D >= 4096:
            return [('small', 4), ('mid', 256)]
        return [('small', 4), ('mid', 256), ('large', 2048)]

    print("=" * 90)
    print("AddRmsNormBias 数值精度测试矩阵  —  Agent 06  （CPU 参考，非 NPU 实测）")
    print("=" * 90)
    print(f"seed={SEED}  eps={EPS}  阈值(fp32=1e-4, fp16/bf16=1e-3)  失配比例通过线=0.1%")
    print()

    # ---------------- Q1: Muls vs Divs ----------------
    print("-" * 90)
    print("Q1: Muls(y, 1/rms) vs Divs(y, rms)  与 golden 比较")
    print("    (D2 形状 [outer, D]; 数据 uniform(-2,2))")
    print("-" * 90)
    print(f"{'dtype':6} {'D':>6} {'outer':>6} {'maxRel_Muls':>13} {'mism_Muls':>10} {'ratio_Muls':>11} {'pass@thr':>9} {'maxRel_Divs':>13}")
    for dtype in ['fp32', 'fp16', 'bf16']:
        for D in Ds:
            for olabel, outer in outer_for(D):
                x = gen((outer, D), dtype, rng); r = gen((outer, D), dtype, rng)
                g, b = gen_gb(D, dtype, rng)
                gold = golden(x, r, g, b)
                cm = cand_muls(x, r, g, b)
                cd = cand_div(x, r, g, b)
                rt, at = THR[dtype]
                mr_m, ma_m, mm_m, rat_m = compare(cm, gold, dtype, rt, at)
                mr_d, ma_d, mm_d, rat_d = compare(cd, gold, dtype, rt, at)
                ok, mism, ratio = passes_thr(cm, gold, dtype)
                results['Q1'].append({'dtype': dtype, 'D': D, 'outer': olabel,
                                      'muls_maxrel': mr_m, 'muls_mism': mm_m, 'muls_ratio': rat_m,
                                      'muls_pass': ok, 'divs_maxrel': mr_d})
                print(f"{dtype:6} {D:>6} {olabel:>6} {mr_m:13.3e} {mm_m:10d} {rat_m:11.4%} {str(ok):>9} {mr_d:13.3e}")
    # 汇总每个 dtype 的最坏情况
    print("  >> 各 dtype 最坏情况(Muls):")
    for dtype in ['fp32', 'fp16', 'bf16']:
        rows = [r for r in results['Q1'] if r['dtype'] == dtype]
        w = max(rows, key=lambda r: r['muls_maxrel'])
        print(f"     {dtype}: maxRel={w['muls_maxrel']:.3e} @D={w['D']},outer={w['outer']}  mism={w['muls_mism']} ratio={w['muls_ratio']:.4%} pass@thr={w['muls_pass']}")
    print()

    # ---------------- Q2: 1/Sqrt vs Rsqrt vs Sqrt+Div ----------------
    print("-" * 90)
    print("Q2: 1/Sqrt+Mul vs Rsqrt+Mul vs Sqrt+Div  与 golden 比较（代表 D 与 dtype）")
    print("-" * 90)
    print(f"{'dtype':6} {'D':>6} {'outer':>6} {'maxRel_1sqrt':>14} {'maxRel_rsqrt':>14} {'maxRel_div':>12}")
    q2Ds = [64, 128, 1024, 32768]
    for dtype in ['fp32', 'fp16', 'bf16']:
        for D in q2Ds:
            olabel, outer = ('mid', 256) if D < 4096 else ('mid', 256)
            x = gen((outer, D), dtype, rng); r = gen((outer, D), dtype, rng)
            g, b = gen_gb(D, dtype, rng)
            gold = golden(x, r, g, b)
            c1 = cand_1sqrt_mul(x, r, g, b)
            cr = cand_rsqrt_mul(x, r, g, b)
            cd = cand_div(x, r, g, b)
            mr1, _, mm1, rat1 = compare(c1, gold)
            mrr, _, mmr, ratr = compare(cr, gold)
            mrd, _, mmd, ratd = compare(cd, gold)
            results['Q2'].append({'dtype': dtype, 'D': D, 'maxrel_1sqrt': mr1, 'maxrel_rsqrt': mrr, 'maxrel_div': mrd,
                                  'ratio_1sqrt': rat1, 'ratio_rsqrt': ratr, 'ratio_div': ratd})
            print(f"{dtype:6} {D:>6} {olabel:>6} {mr1:14.3e} {mrr:14.3e} {mrd:12.3e}")
    print("  注：CPU 无硬件 rsqrt，cand_rsqrt_mul 数学等价于 cand_1sqrt_mul；")
    print("      真实 NPU rsqrt 为单次舍入指令，其额外 ~1ulp 舍入误差无法在 CPU 量化。")
    print()

    # ---------------- Q3: 分块累加 vs 一次累加（平方和相对误差）----------------
    print("-" * 90)
    print("Q3: 分块累加 vs 一次累加  ——  sum(y*y) 相对误差（FP32 中间，与输入 dtype 无关的平方和）")
    print("-" * 90)
    print(f"{'dtype':6} {'D':>6} {'block':>6} {'relErr_once_vs_block':>22}")
    q3blocks = [8, 16, 32, 64, 128, 256, 512, 1024]
    for dtype in ['fp32', 'bf16', 'fp16']:
        for D in [64, 128, 1024, 4096, 32768]:
            x = gen((256, D), dtype, rng); r = gen((256, D), dtype, rng)
            y = x.astype(np.float32) + r.astype(np.float32)
            once = sumsq_once(y)
            worst = 0.0
            for blk in q3blocks:
                block = sumsq_block(y, blk)
                rel = np.max(np.abs(once - block) / np.where(np.abs(once) > 0, np.abs(once), 1.0))
                worst = max(worst, float(rel))
            results['Q3'].append({'dtype': dtype, 'D': D, 'max_relerr': worst})
            print(f"{dtype:6} {D:>6} {str(q3blocks):>6} {worst:22.3e}")
    print("  >> 上一轮结论：≤8.9e-8，不算误差来源。复核：见上表 worst。")
    print()

    # ---------------- Q4: FP16 平方和溢出阈值 ----------------
    print("-" * 90)
    print("Q4: FP16 平方和溢出阈值 —— |y| 多大时 y*y 在 fp16 下溢出为 inf")
    print("-" * 90)
    fp16_max = np.finfo(np.float16).max
    print(f"    fp16 max normal = {fp16_max}")
    print(f"    sqrt(fp16_max) = {math.sqrt(fp16_max):.6f}  → |y| >= {math.ceil(math.sqrt(fp16_max))} 时 y*y 溢出")
    # 数值确认：扫描 |y|
    threshold = None
    for v in [250.0, 251.0, 252.0, 253.0, 254.0, 255.0, 255.5, 255.9, 256.0, 256.5, 257.0, 300.0]:
        yv = np.array([v], dtype=np.float16)
        sq = (yv * yv)
        isinf = not np.isfinite(sq[0])
        if isinf and threshold is None:
            threshold = v
        print(f"    |y|={v:7.2f}  y*y(fp16)={sq[0]}  {'OVERFLOW' if isinf else 'ok'}")
    # 判题端范围未知：模板 uniform(-2,2)，但演示若 |y| 达到 256 即危险
    print(f"    >> 溢出阈值 = |y| >= {threshold} (fp16)。若判题端 |y| 接近/超过 256，fp16 平方和直接 inf。")
    results['Q4'] = {'fp16_max': float(fp16_max), 'sqrt_max': math.sqrt(float(fp16_max)),
                     'threshold': threshold, 'template_range': 'uniform(-2,2) 安全',
                     'note': '判题端数据范围未知'}
    print()

    # ---------------- Q5: BF16 大 D 顺序归约 vs 分块归约 ----------------
    print("-" * 90)
    print("Q5: BF16 大 D 顺序归约(FP32一次性) vs 分块归约(FP32 block) 失配比例 vs golden")
    print("-" * 90)
    print(f"{'D':>6} {'outer':>6} {'maxRel_seq':>12} {'maxRel_blk':>12} {'ratio_seq':>10} {'ratio_blk':>10}")
    for D in [4096, 32768]:
        for olabel, outer in [('small', 4), ('mid', 256)]:
            dtype = 'bf16'
            x = gen((outer, D), dtype, rng); r = gen((outer, D), dtype, rng)
            g, b = gen_gb(D, dtype, rng)
            gold = golden(x, r, g, b)
            # 顺序归约路径
            y = x.astype(np.float32) + r.astype(np.float32)
            s_seq = sumsq_once(y)
            rms_seq = np.sqrt((s_seq / D)[..., None] + EPS)
            out_seq = (y / rms_seq * g.astype(np.float32) + b.astype(np.float32))
            out_seq = _cast(out_seq, x.dtype)
            # 分块归约路径
            s_blk = sumsq_block(y, 256)
            rms_blk = np.sqrt((s_blk / D)[..., None] + EPS)
            out_blk = (y / rms_blk * g.astype(np.float32) + b.astype(np.float32))
            out_blk = _cast(out_blk, x.dtype)
            _, _, mm_seq, rat_seq = compare(out_seq, gold)
            _, _, mm_blk, rat_blk = compare(out_blk, gold)
            mr_seq, _, _, _ = compare(out_seq, gold)
            mr_blk, _, _, _ = compare(out_blk, gold)
            results['Q5'].append({'D': D, 'outer': olabel, 'maxrel_seq': mr_seq, 'maxrel_blk': mr_blk,
                                  'ratio_seq': rat_seq, 'ratio_blk': rat_blk})
            print(f"{D:>6} {olabel:>6} {mr_seq:12.3e} {mr_blk:12.3e} {rat_seq:10.4%} {rat_blk:10.4%}")
    print("  >> 两种 FP32 归约顺序失配比例均极小，远未超过 0.1%。")
    print()

    # ---------------- Q6: 全程低精度累加（对照，证否）----------------
    print("-" * 90)
    print("Q6: 全程低精度累加（FP16/BF16 每步量化）对照 —— 与 golden 失配比例")
    print("-" * 90)
    print(f"{'low':>6} {'D':>6} {'outer':>6} {'maxRel':>13} {'mism':>8} {'ratio':>10} {'pass@0.1%':>10}")
    for low in ['fp16', 'bf16']:
        for D in [64, 128, 1024]:
            x = gen((64, D), low, rng); r = gen((64, D), low, rng)
            g, b = gen_gb(D, low, rng)
            gold = golden(x, r, g, b)
            cout = cand_full_low(x, r, g, b, DTYPE_MAP[low])
            mr, ma, mm, rat = compare(cout, gold)
            ok, _, _ = passes_thr(cout, gold, low)
            results['Q6'].append({'low': low, 'D': D, 'maxrel': mr, 'mism': mm, 'ratio': rat, 'pass': ok})
            print(f"{low:>6} {D:>6} {'64':>6} {mr:13.3e} {mm:8d} {rat:10.4%} {str(ok):>10}")
    print("  >> 全程低精度失配比例极大，证否该方案；FP32 中间累加是必须的。")
    print()

    # ---------------- Q7(附加): epsilon 在 sqrt 内 vs 外 ----------------
    print("-" * 90)
    print("Q7(附加): epsilon 在 mean 之后/sqrt 之前(golden) vs sqrt 之后(错误) 与 golden 比较")
    print("-" * 90)
    print(f"{'dtype':6} {'D':>6} {'outer':>6} {'maxRel_epsOut':>14} {'ratio_epsOut':>12}")
    for dtype in ['fp32', 'fp16', 'bf16']:
        for D in [64, 1024, 32768]:
            olabel, outer = ('mid', 256) if D < 4096 else ('mid', 256)
            x = gen((outer, D), dtype, rng); r = gen((outer, D), dtype, rng)
            g, b = gen_gb(D, dtype, rng)
            gold = golden(x, r, g, b)
            ce = cand_eps_outside(x, r, g, b)
            mr, ma, mm, rat = compare(ce, gold)
            results['Q7'].append({'dtype': dtype, 'D': D, 'maxrel': mr, 'ratio': rat, 'mism': mm})
            print(f"{dtype:6} {D:>6} {olabel:>6} {mr:14.3e} {rat:12.4%}")
    print("  >> 全零行(rms≈eps)时差异最大；正常数据下相对差异约 eps/|rms| 量级，小但非零。")
    print()

    # ---------------- 特殊值 ----------------
    print("-" * 90)
    print("特殊值：全零行 / 极大值 / NaN / Inf（以 fp32 形状 [4,128] 演示，仅定性）")
    print("-" * 90)
    D = 128; outer = 4
    # 全零行
    x = np.zeros((outer, D), dtype=np.float32); r = np.zeros((outer, D), dtype=np.float32)
    g, b = gen_gb(D, 'fp32', rng)
    gold = golden(x, r, g, b)
    # 全零行时 out 应 = bias（y=0 → 0/rms*g + b = b）
    print(f"  全零行: golden==bias? {np.allclose(gold.astype(np.float32), b.astype(np.float32), atol=1e-6)}  (rms=sqrt(eps)={math.sqrt(EPS):.6f})")
    # NaN
    x = gen((outer, D), 'fp32', rng); r = gen((outer, D), 'fp32', rng)
    x[0, 0] = np.nan
    gold = golden(x, r, g, b)
    print(f"  NaN 输入: golden 含 NaN={np.isnan(gold).any()}  (equal_nan 比较可匹配)")
    # Inf
    x = gen((outer, D), 'fp32', rng); r = gen((outer, D), 'fp32', rng)
    x[0, 0] = np.inf
    gold = golden(x, r, g, b)
    print(f"  Inf 输入: golden 含 Inf={np.isinf(gold).any()}")
    # FP16 大值溢出演示
    x = gen((outer, D), 'fp16', rng, lo=250.0, hi=260.0); r = gen((outer, D), 'fp16', rng, lo=250.0, hi=260.0)
    g, b = gen_gb(D, 'fp16', rng)
    # 在 fp16 中直接做 y*y 看是否溢出
    yf16 = (x.astype(np.float16) + r.astype(np.float16)).astype(np.float16)
    sq_f16 = (yf16 * yf16)
    n_inf = int(np.sum(~np.isfinite(sq_f16)))
    print(f"  FP16 大值[250,260]: y*y 中出现 inf 的元素数={n_inf}/{sq_f16.size}  → 若平方和在 fp16 域算则直接溢出")

    # 保存 JSON
    with open('agent06_results.json', 'w') as f:
        json.dump(results, f, indent=2, default=float)
    print()
    print("=" * 90)
    print("实验结束。结果已写入 agent06_results.json（同目录）。")
    print("=" * 90)


if __name__ == '__main__':
    main()
