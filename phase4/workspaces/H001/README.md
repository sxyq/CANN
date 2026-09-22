# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## Status

- V004: 15/15, 12.54
- V005: 0/15 TLE — depth-2 TQue Alloc-before-Free; do not retry
- V006: 15/15, 12.55 (fused ApplyRow — flat)
- V007: 15/15, **23.29 best** — ReduceSum in hot `ApplyRow` (T03/T04/T06–T08/T10/T14 collapsed)
- V008: V007 + **one** change: `ReduceSum` in wide `ChunkSumSquares` (T05/T09/T11–T13/T15)

## V008 only delta vs V007

`ChunkSumSquares` (wide pass1) uses `Mul`+`ReduceSum<float,true>` instead of the scalar GetValue loop. Same partial/reduceTmp as hot path. Everything else identical to V007.

## Template (npu_kernel_dev / B001)

`src/add_rms_norm_bias_kernel.cpp` → `kernel.asc`. dtype 0=FP32, 1=FP16, 2=BF16.

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-11.log`.
