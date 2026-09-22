# H001 AddRmsNormBias

Small-D / high-R on Ascend910B3/DAV_2201 Vector Core.

## V003 (online-compile aligned to B001 npu_kernel_dev shape)

Submitted body: `src/add_rms_norm_bias_kernel.cpp` (platform entry `kernel.asc`).

Matches the 15/15 B001 template skeleton (shape only, H001 compute kept):

- first `#include <cmath>`, second `#include "kernel_operator.h"`, last `}`
- no anonymous / nested namespace, no `<type_traits>`, no `std::is_same`, no `if constexpr`
- no `PipeBarrier`, no `__builtin_sqrtf`, no `.template Get`
- `TQue` EnQue/DeQue sync; `AscendC::Sqrt` / `Duplicate` for RMS
- field-assigned `DataCopyExtParams` / `DataCopyPadExtParams`
- three `extern "C" __global__ __vector__` entries (fp16/bf16/fp32)
- `run_kernel` B001 shape: tiling on GM via `aclrtMalloc`/`Memcpy`/`Free`, `aclrtSynchronizeStream`
- dtype map 0=FP32, 1=FP16, 2=BF16 (27 accepted as BF16)

H001 architecture: D<=1024 multi-row tile + resident FP32 gamma/bias + batch scalar RMS; D>1024 chunked fallback.

## Build

`build_server3.sh` on cann-server3: device + mock-judge submission + full link. Log: `logs/compile-06.log`.
Local-only helpers: `src/compile_adapter.hpp`, `src/mock_judge_local.cpp`.
