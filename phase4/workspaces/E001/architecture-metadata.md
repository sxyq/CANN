# E001 Architecture Metadata

## Candidate

`E001`

## Architecture hypothesis

The input is viewed as `R` contiguous rows of width `D`. Each Vector Core owns a contiguous row interval. A hot-path row is copied to UB, residual is added before normalization, the official DAV_2201 `RmsNorm` primitive computes the normalized row, and bias is added after normalization. The primitive tiling is built for one row because the kernel invokes it on one row-sized UB tensor at a time. A row is processed independently, so no cross-core synchronization or reduction workspace is needed.

The generic path keeps the same row ownership. It evaluates the row in fixed-size chunks, accumulates the squared sum in FP32, recomputes the row in the same order, applies gamma and bias, and writes only valid tail elements. This path is used for BF16, non-aligned `D`, unsupported official tiles, and any shape whose official temporary area does not fit the local budget.

## Core mapping

- `block_num = min(R, available_vector_cores)` for normal scheduling.
- `rows_per_block = floor(R / block_num)`.
- The final block owns the remaining rows in `rows_last_block`; no unused block enters a wait.
- A core reads and writes only its own row interval.
- Cross-core synchronization count: zero.

## Official RmsNorm API validation

Validated against CANN `8.5.0.alpha002` on `cann-server3` with `Ascend910B3` / `dav-2201` headers and libraries.

- Device API: `AscendC::RmsNorm<T>` from `adv_api/normalization/rmsnorm.h`.
- Tiling APIs: `GetRmsNormMaxMinTmpSize` and `GetRmsNormTilingInfo`.
- Supported types in the header implementation: FP16 and FP32.
- BF16 is rejected by the official interface and therefore uses the generic path.
- Tiling accepts non-32-byte `D`; the returned `hLength` and `originalHLength` preserve the original width.
- `stackBufferByteSize` must be at least the returned minimum temporary size.
- The returned maximum is shape-dependent and is not used as an unconditional reservation.

Probe command and output were produced by `op_host/e001_tiling_probe.cpp`.

| dtype | D | rows | min temporary | max temporary | result |
|---|---:|---:|---:|---:|---|
| FP32 | 64 | 1 | 288 B | 288 B | official tiling succeeds |
| FP32 | 1024 | 4 | 4128 B | 16416 B | official tiling succeeds |
| FP32 | 4096 | 4 | 16416 B | 65568 B | official tiling succeeds |
| FP32 | 32768 | 1 | 131104 B | 131104 B | official tiling succeeds |
| FP16 | 64 | 1 | 544 B | 576 B | official tiling succeeds |
| FP16 | 8192 | 1 | 65568 B | 65600 B | official tiling succeeds |
| FP16 | 32768 | 1 | 262176 B | 262208 B | exceeds 184 KiB and uses generic path |

## UB and workspace budget

The candidate budget is 184 KiB. The hot path uses one queue slot per input/output, one gamma slot, one bias slot, one row workspace, and the exact official temporary size returned by the tiling API. The host-side route must accept a hot path only when the sum of these allocations, alignment loss, and reserved framework space is within 184 KiB.

The generic path uses a fixed 2048-element chunk and FP32 accumulation. It does not depend on the official temporary area and does not allocate a cross-core reduction buffer.

## Parameter residency

Gamma and bias are copied once per core into UB. The hot path keeps them resident while rows are processed. The generic path reloads only the active chunk, which keeps the path valid for all legal `D` values.

## Hot path and generic path conditions

Hot path requires:

- dtype FP16 or FP32;
- `64 <= D <= 32768`;
- official `GetRmsNormMaxMinTmpSize` succeeds;
- official `GetRmsNormTilingInfo` succeeds for the one-row primitive shape with the selected temporary size;
- the complete allocation plan fits within 184 KiB;
- row and gamma/bias copies satisfy the selected aligned or padded transfer form.

Generic path covers BF16, non-aligned `D`, tail rows, oversized official temporary requirements, unsupported tiles, NaN, Inf, epsilon, and every legal 2D/3D/4D shape after flattening leading dimensions into `R`.

## Precision and special values

FP16 and BF16 inputs are accumulated in FP32 on the generic path. The row traversal order is deterministic. NaN and Inf are not filtered or replaced; arithmetic propagation follows the input dtype conversion and device arithmetic semantics. Epsilon is passed as a runtime float attribute.

## Source and build

- Kernel source: `phase4/workspaces/E001/op_kernel/e001_kernel.asc`
- Shared tiling record: `phase4/workspaces/E001/op_kernel/e001_tiling.h`
- Tiling probe: `phase4/workspaces/E001/op_host/e001_tiling_probe.cpp`
- Build file: `phase4/workspaces/E001/CMakeLists.txt`
- Target: `Ascend910B3`, `dav-2201`, CANN `8.5.0.alpha002`
