# CANN 8.5 API Evidence

Environment: server3 CANN `8.5.0.alpha002`, SoC `Ascend910B3`, `--npu-arch=dav-2201`. The install does not expose the Ascend C Markdown API set to this account; signatures and restrictions below were checked against the installed 8.5 headers and the local Ascend C API references.

## Changed FP32 path

- `DataCopy(LocalTensor<T> dst, LocalTensor<T> src, uint32_t count)` is declared as the Level 2 vector-data overload in `aarch64-linux/tikcpp/tikcfw/interface/kernel_operator_data_copy_intf.h:258-266`. `T` is the same type on both operands; V001 instantiates only `T=float`. `count` is an element count. The call is guarded by `count*sizeof(float) % 32 == 0` and both `GetPhyAddr() % 32 == 0`. The local API reference `ascendc-api-best-practices/references/api-datacopy.md` documents the 32-byte alignment requirement and warns that misaligned DataCopy can corrupt data. Every other case keeps Adds.
- `Adds(LocalTensor<T> dst, LocalTensor<T> src, U scalarValue, int32_t count)` is declared in `aarch64-linux/tikcpp/tikcfw/interface/kernel_operator_vec_binary_scalar_intf.h:93`. Its generic tensor and scalar types permit `float` / `float`; it remains the fallback for unaligned counts or addresses.
- `LocalTensor::GetPhyAddr()` returns `uint64_t` in `aarch64-linux/tikcpp/tikcfw/interface/kernel_tensor.h:140-141`; the modulo test is on the physical UB address used by DataCopy.

## Existing conversion paths retained for other dtypes

- `Cast(dst, src, RoundMode, count)` remains unchanged for non-FP32 instantiations. `phase4`'s local `api-precision.md` reference specifies `CAST_NONE` for half-to-float widening and `CAST_ROUND` for float-to-half/BF16 narrowing. The candidate retains those exact modes. The CANN 8.5 conversion declarations/implementations are in `tikcpp/tikcfw/interface` and `tikcpp/tikcfw/impl`; the `dav-2201` build instantiates the existing half/BF16 paths.
- No arithmetic API other than the unchanged FP32 `Adds(..., 0.0f, count)` fallback is introduced. Both ToFloat and FromFloat use the same-type FP32 local-copy mechanism; no reduction, arithmetic order, scheduling, buffer, or dtype dispatch is changed.

API review references: `/Users/sunyiyang/Desktop/Project/cann/.agents/skills/ascendc-api-best-practices/references/api-datacopy.md`, `api-arithmetic.md`, `api-precision.md`, and server3's installed CANN 8.5 headers named above.
