# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## Status

- V004: 15/15, 12.54
- V005: 0/15 TLE — depth-2 TQue Alloc-before-Free; do not retry
- V006: 15/15, 12.55 (fused ApplyRow — flat)
- V007: 15/15, 23.29 — ReduceSum in hot ApplyRow
- V008: 15/15, **29.04 best** — ReduceSum in wide ChunkSumSquares
- V009: V008 + **one** change: wide-path resident gamma/bias FP32 when `colsPad*(sizeof(T)+4)*2 ≤ 80KiB`

## V009 only delta vs V008

`ProcessWide` loads gamma/bias FP32 once (`LoadGammaBiasResident`) and indexes `gammaF32[colBegin]` in pass2. Skips per-chunk gamma/bias DMA/queue/Cast when resident. Falls back to V008 chunked params when D is huge (e.g. 32768). Hot path unchanged.

## Template (npu_kernel_dev / B001)

`src/add_rms_norm_bias_kernel.cpp` → `kernel.asc`. dtype 0=FP32, 1=FP16, 2=BF16.

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-12.log`.
