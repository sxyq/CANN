# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## Status

- V004: 15/15, 12.54
- V005: 0/15 TLE (depth-2 TQue deadlock) — do not retry Alloc-before-Free
- V006: 15/15, 12.55 (fused ApplyRow only — flat)
- V007: V006 + **one** change: `ReduceSum` for `sum(u*u)` in hot `ApplyRow` only (wide `ChunkSumSquares` still scalar)

## V007 only delta vs V006

Hot `ApplyRow`: `Mul(u,u)` + `ReduceSum<float,true>` replaces the scalar GetValue loop.
New `partialBuf_` (32B) + `reduceTmpBuf_` (8KiB), disjoint from workA/workB. Queues stay depth 1.

## Template (npu_kernel_dev / B001)

`src/add_rms_norm_bias_kernel.cpp` → `kernel.asc`. dtype 0=FP32, 1=FP16, 2=BF16.

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-10.log`.
