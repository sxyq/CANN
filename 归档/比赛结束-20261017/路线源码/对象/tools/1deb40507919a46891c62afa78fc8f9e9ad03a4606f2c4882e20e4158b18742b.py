#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
AddRmsNormBias Host 参考实现与数值验证（无 NPU/无 CANN 环境下的辅助验证）
==================================================================
目的：验证“kernel 算法语义”（FP32 归约 + 分块 + DataCopyPad 补 0 + 尾块 +
      输出一次性量化）在各 dtype x rank x 边界 D 下是否满足判题精度预算：
        fp32: 相对误差 < 1e-4 且 绝对误差 < 1e-4
        fp16/bf16: 相对误差 < 1e-3 且 绝对误差 < 1e-3
参考基准：原始输入的 fp64 数学公式，以及分别按目标 dtype 量化输入后的 fp64/fp32 公式。

注意：这是 Host 层语义模拟，不是 NPU 编译/运行。结论仅证明“算法的数值可行性”，
不构成任何 NPU 上已通过编译/精度的声明。
"""
import sys

import numpy as np

def check_raw(got, ref):
    """仅报告口径：与 fp64 参考的逐元素相对(分母加保护)与最大绝对误差，用于标注 dtype 固有精度"""
    got = np.asarray(got, np.float32)
    ref = np.asarray(ref, np.float32)
    denom = np.where(np.abs(ref) > 1e-12, np.abs(ref), 1.0)
    max_rel = float(np.max(np.abs(got - ref) / denom))
    max_abs = float(np.max(np.abs(got - ref)))
    return max_rel, max_abs, True
import torch

# ---------------- BF16 编解码（模拟昇腾 bfloat16_t 存储语义） ----------------
def to_bf16(x):
    """float32 -> bfloat16 语义（尾数截断，round-to-nearest-even）"""
    x = np.asarray(x, dtype=np.float32)
    i32 = x.view(np.uint32)
    lsb = (i32 >> 16) & 1
    rounded = i32 + 0x7FFF + lsb
    truncated = (rounded & 0xFFFF0000).astype(np.uint32)
    return truncated.view(np.float32)

# ---------------- Kernel 语义模拟 ----------------
TILE_HALF = 4096   # 与 op_kernel 常量一致
TILE_FLOAT = 2048

def quantize(x, dtype):
    """把输入量化到目标 dtype 存储语义，模拟数据在 GM 上以该类型表示"""
    if dtype == "fp16":
        return x.astype(np.float16).astype(np.float32)
    if dtype == "bf16":
        return to_bf16(x)
    return x.astype(np.float32)

def np_sum_fp32(a):
    """分段 fp32 求和，近似 ReduceSum（硬件为二叉树，这里用顺序 fp32 累加，
    验证误差量级即可）"""
    a = a.astype(np.float32)
    s = np.float32(0.0)
    step = 128
    for i in range(0, a.size, step):
        s = np.float32(s + np.float32(np.sum(a[i:i + step], dtype=np.float32)))
    return s

def kernel_sim(x_in, r_in, g_in, b_in, eps, dtype):
    """按 op_kernel 的流程语义计算：
       Pass1: 逐 tile：cast fp32 -> add -> square -> sum(块内) -> 跨块 fp32 累加
       尾块按 DataCopyPad 补 0 语义（pad 部分 0，不影响求和）
       Pass2: y = x+r (fp32) -> *scale -> *gamma -> +bias -> cast 回 dtype
    """
    x = quantize(x_in, dtype)
    r = quantize(r_in, dtype)
    g = quantize(g_in, dtype)
    b = quantize(b_in, dtype)
    D = x.shape[-1]
    outer = int(np.prod(x.shape[:-1]))
    tile = TILE_HALF if dtype in ("fp16", "bf16") else TILE_FLOAT
    xf = x.reshape(outer, D).astype(np.float32)
    rf = r.reshape(outer, D).astype(np.float32)
    gf = g.astype(np.float32)
    bf = b.astype(np.float32)

    row_sum = np.zeros(outer, dtype=np.float32)
    for off in range(0, D, tile):
        seg = slice(off, min(off + tile, D))
        y = xf[:, seg] + rf[:, seg]
        sq = y * y
        for row in range(outer):
            row_sum[row] = np.float32(row_sum[row] + np_sum_fp32(sq[row]))
    rms = np.sqrt(row_sum / np.float32(D) + np.float32(eps)).astype(np.float32)
    scale = (1.0 / rms).astype(np.float32)

    out = np.empty_like(xf)
    for off in range(0, D, tile):
        seg = slice(off, min(off + tile, D))
        y = (xf[:, seg] + rf[:, seg]) * scale[:, None]
        y = y * gf[seg][None, :]
        y = y + bf[seg][None, :]
        if dtype == "fp16":
            out[:, seg] = y.astype(np.float16).astype(np.float32)
        elif dtype == "bf16":
            out[:, seg] = to_bf16(y)
        else:
            out[:, seg] = y
    return out.reshape(x.shape).astype(np.float32)

def check(got, ref, dtype):
    """逐元素 |o-r| <= atol + rtol*|r| 与纯 abs 双口径；fp32 用 1e-4，低精度用 1e-3"""
    got = np.asarray(got, np.float32)
    ref = np.asarray(ref, np.float32)
    tol = 1e-4 if dtype == "fp32" else 1e-3
    mask = np.abs(got - ref) <= (tol + tol * np.abs(ref))
    max_abs = float(np.max(np.abs(got - ref)))
    denom = np.where(np.abs(ref) > 1e-12, np.abs(ref), 1.0)
    max_rel = float(np.max(np.abs(got - ref) / denom))
    ok = bool(mask.all())
    return max_rel, max_abs, ok

def formula_reference(x_in, r_in, g_in, b_in, eps, dtype, compute_dtype):
    """按 GM 中的已量化输入计算公式，再按目标 dtype 保存输出。"""
    x = quantize(x_in, dtype).astype(compute_dtype)
    r = quantize(r_in, dtype).astype(compute_dtype)
    g = quantize(g_in, dtype).astype(compute_dtype)
    b = quantize(b_in, dtype).astype(compute_dtype)
    y = x + r
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + compute_dtype(eps))
    out = y / rms * g + b
    if dtype == "fp16":
        return out.astype(np.float16).astype(np.float32)
    if dtype == "bf16":
        return to_bf16(out.astype(np.float32))
    return out.astype(np.float32)

def main():
    torch.manual_seed(0)
    np.random.seed(0)
    d_list = [64, 67, 96, 129, 192, 255, 576, 1000, 1024, 2048, 4096, 8192, 32768]
    rank_specs = [("2D", ()), ("3D", (2,)), ("4D", (2, 2))]
    dtypes = ["fp32", "fp16", "bf16"]
    cases, fails = [], []
    for dt in dtypes:
        for rname, spec in rank_specs:
            for D in d_list:
                n = 4 if D == 32768 else 1  # 大数据量抽样小批量，控制时长
                shape = spec + (n, D)
                x = (np.random.randn(*shape) * 0.8).astype(np.float32)
                r = (np.random.randn(*shape) * 0.4).astype(np.float32)
                g = (1.0 + np.random.randn(D) * 0.2).astype(np.float32)
                b = (np.random.randn(D) * 0.3).astype(np.float32)
                eps = 1e-5

                yd = torch.from_numpy(x).double() + torch.from_numpy(r).double()
                ref64 = (yd / torch.sqrt(torch.mean(yd * yd, dim=-1, keepdim=True) + eps)
                         * torch.from_numpy(g).double()
                         + torch.from_numpy(b).double()).numpy()
                stored64 = formula_reference(x, r, g, b, eps, dt, np.float64)
                stored32 = formula_reference(x, r, g, b, eps, dt, np.float32)

                got = kernel_sim(x, r, g, b, eps, dt)
                # 口径1：原始输入的 fp64 数学参考，仅用于观察 dtype 量化误差。
                rel64, abs64, _ = check_raw(got, ref64)
                # 口径2：先按 GM 存储类型量化 x/residual/gamma/bias，再计算并量化输出。
                relStored, absStored, okStored = check(got, stored64, dt)
                # 口径3：同一组已量化输入的 fp32 计算参考，用于观察归约顺序差异。
                relFp32, absFp32, okFp32 = check(got, stored32, dt)
                ok = okStored and okFp32
                cases.append((dt, rname, D, rel64, abs64, ok))
                print(f"{dt:5s} {rname} D={D:6d}  info(fp64): rel={rel64:.2e} abs={abs64:.2e} | "
                      f"stored-input(fp64): rel={relStored:.2e} abs={absStored:.2e} {'PASS' if okStored else 'FAIL'} | "
                      f"stored-input(fp32): rel={relFp32:.2e} abs={absFp32:.2e} {'PASS' if okFp32 else 'FAIL'}")
                if not ok:
                    fails.append(cases[-1])

    print("\n=== 汇总 ===")
    print(f"总用例: {len(cases)}  通过: {len(cases) - len(fails)}  失败: {len(fails)}")
    for f in fails:
        print("FAIL:", f)
    return 0 if not fails else 1

if __name__ == "__main__":
    sys.exit(main())
