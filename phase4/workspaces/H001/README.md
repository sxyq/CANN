# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## V004: fix wide-D WA (full-row RMS)

V003 was 9/15 online (T05/T09/T11/T12/T13/T15 ~99.9% error). Those are D>1024.
`ProcessWide` computed RMS from each 1024-chunk only while still dividing by full `1/D`.

V004 two-pass wide path:

1. accumulate `sum(u*u)` over **all** chunks of the row
2. `invRms = 1/sqrt(sum/D + eps)` once per row
3. second pass: `(u * invRms * gamma + bias)` per chunk and store

Also GM row offsets are `uint64_t` (`row * cols` no longer wraps on large R*D).

Hot path D<=1024 (T01–T04,T06–T08,T10,T14) unchanged.

## Template (npu_kernel_dev, B001 shape)

`src/add_rms_norm_bias_kernel.cpp` → platform `kernel.asc`.

- first `#include <cmath>`, second `"kernel_operator.h"`, last `}`
- no nested namespace / `type_traits` / `PipeBarrier` / `__builtin_sqrtf` / `.template Get`
- TQue EnQue/DeQue; `AscendC::Sqrt`+`Duplicate`; field-assigned DataCopy params
- three `__global__ __vector__` entries; `run_kernel` with GM tiling + aclrtMalloc
- dtype 0=FP32, 1=FP16, 2=BF16 (27→BF16)

## Build

`build_server3.sh` on cann-server3. Log: `logs/compile-07.log`.
Local-only: `src/compile_adapter.hpp`, `src/mock_judge_local.cpp`.
