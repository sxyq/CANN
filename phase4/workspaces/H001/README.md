# H001 AddRmsNormBias

This candidate targets small `D` and high `R` on Ascend910B3/DAV_2201 Vector Core.

## V002 online-compile fixes (npu_kernel_dev)

Submitted file is `src/add_rms_norm_bias_kernel.cpp` (platform entry `kernel.asc`):

- First line `#include <cmath>`, second `#include "kernel_operator.h"`, last line `}`
- No `TensorInfo` / `TensorGroupInfo` redefinition; no `<type_traits>` / `std::is_same`
- BF16 via vector `Cast` only (`H001Convert`); no scalar `static_cast` bf16 forms
- `DataCopyExtParams` / `DataCopyPadExtParams` default-constructed then field-assigned (no brace init)
- `run_kernel` ABI: x/x_info … output/output_info, availableCoreNum, stream, epsilon
- Entry: `extern "C" __global__ __vector__ void add_rms_norm_bias`
- `PipeBarrier<PIPE_*>` unqualified; `AscendC::GetBlockIdx`; `__builtin_sqrtf`

## Architecture

- `D <= 1024`: adaptive multi-row tile (up to 64), resident FP32 gamma/bias, batch scalar RMS, vector Muls/Mul/Add
- `D > 1024`: one-row chunked fallback
- dtype buckets: 0=FP32, 1=FP16, 2=BF16

## Build

`build_server3.sh` (cann-server3) runs device + submission (mock judge.asc include) + full link. Log: `logs/compile-05.log`.
Local-only: `src/compile_adapter.hpp`, `src/mock_judge_local.cpp`.
