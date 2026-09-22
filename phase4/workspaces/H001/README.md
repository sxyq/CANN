# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## Status

- V004: **15/15**, score 12.54 (H001 best). Keep as correctness baseline.
- V005: 0/15 TLE — depth-2 TQue Alloc-before-Free deadlock. Reverted.
- V006: V004 structure + **one** change: fused `ApplyRow` (compute `u=x+res` once, reuse for sum and normalize). No ReduceSum, no depth-2 queues.

## V006 only delta vs V004

`ApplyRow` no longer re-casts x/residual for the normalize pass. ProcessHot/ProcessWide/Init/queues identical to V004.

## Template (npu_kernel_dev / B001 shape)

`src/add_rms_norm_bias_kernel.cpp` → `kernel.asc`. dtype 0=FP32, 1=FP16, 2=BF16.

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-09.log`.
