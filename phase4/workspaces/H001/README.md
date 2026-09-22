# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## Status

- V004: **15/15 online**, score 12.54 (baseline). Small-D T01–T04 are the specialist zone (r≈3–11).
- V005: hot-path speed only. Keep 15/15. Wide path unchanged (correct, slow OK).

## V005 hot-path changes

1. **Fused `ApplyRow`**: one Cast/Add of `u=x+res`, then ReduceSum(`u*u`) → invRms → Muls/Mul/Add on the same `u` (was two Cast/Add passes).
2. **ReduceSum** for RMS sum instead of scalar GetValue loop.
3. **Double-buffer** x/residual TQue depth 2: prefetch next tile while computing current.
4. **Larger tiles**: budget 140KiB / 5 buffers, max 128 rows/tile (was 64 / 3).

Wide D>1024 two-pass full-row RMS from V004 kept.

## Template (npu_kernel_dev / B001 shape)

`src/add_rms_norm_bias_kernel.cpp` → `kernel.asc`. First `#include <cmath>`, TQue EnQue/DeQue, three `__global__ __vector__` entries, GM tiling + `run_kernel` ABI. dtype 0=FP32, 1=FP16, 2=BF16.

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-08.log`.
