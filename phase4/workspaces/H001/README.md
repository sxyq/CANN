# H001 AddRmsNormBias

This candidate targets small `D` and high `R` on Ascend910B3/DAV_2201 Vector Core.

## Architecture

- `D <= 1024`: hot path stages many rows per tile (adaptive, up to 64). Gamma and bias stay resident in FP32 for the whole invocation. 32-byte-aligned tiles use one multi-block `DataCopyPad` for many rows. RMS uses batch scalar accumulation on FP32 (`u = x + residual`) without ReduceSum/queue setup. Normalize uses vector `Muls`/`Mul`/`Add` (vector repeat).
- `D > 1024`: one-row chunked fallback covers every legal width without assuming row alignment.
- FP16, BF16, FP32 share one Vector entry `add_rms_norm_bias` with dtype buckets (`0=FP32, 1=FP16, 2=BF16`).
- BF16 scalar conversion uses `AscendC::ToFloat` / `AscendC::ToBfloat16` (never unsupported backend cast forms). Bulk conversion uses vector `Cast`.

## Submission ABI

`src/add_rms_norm_bias_kernel.cpp` is the submitted source. It must not redefine `TensorInfo` / `TensorGroupInfo`. Local compile uses `src/compile_adapter.hpp` then includes the submission source.

`run_kernel` argument order: x, x_info, residual, residual_info, gamma, gamma_info, bias, bias_info, output, output_info, availableCoreNum, stream, epsilon.

Judge dtype encoding at entry: `0=FP32`, `1=FP16`, `2=BF16`.

## Build

`build_server3.sh` on `cann-server3` (CANN 8.5.0.alpha002) runs device compile, submission compile, and full link. Log: `logs/compile-04.log`.
