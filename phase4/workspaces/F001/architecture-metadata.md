# F001 Architecture Metadata

## candidate
F001

## architecture hypothesis
Persistent row ownership is the simplest deterministic mapping for an RMS reduction: one Vector Core owns each complete row and performs both passes locally. The first pass computes the row sum of squares in a fixed FP32 accumulation order. The second pass rereads the row and fuses normalization, gamma, and bias. The mapping removes cross-core reduction traffic and leaves no inter-core dependency.

## core mapping
Use the runtime Vector Core count queried by the host. Launch `blockDim = min(vector_core_count, max(rows, 1))`; Core `c` handles rows `c, c + blockDim, ...`.

## blockDim strategy
`blockDim` is selected from `ACL_DEV_ATTR_VECTOR_CORE_NUM` and the row count. No fixed core count is assumed. Kernel-side bounds check returns for an unused block.

## row ownership
Exclusive whole-row ownership. A row is never written by more than one Core.

## D ownership
The owning Core owns all `D` elements of its row. Gamma and bias are indexed by the row-local column.

## UB allocation estimate
0 bytes of UB are required by the scalar fallback-style implementation. Inputs are read from GM, and the row accumulator, inverse RMS, and current values reside in scalar registers. This is below the 184 KiB candidate Vector workspace limit.

## TQue allocation
None. There are no TQue slots or double buffers.

## parameter residency lifetime
Gamma and bias are read once per output element during the second pass. Their lifetime is one row pass; no cross-row residency is assumed.

## reduction topology
Single-Core sequential FP32 sum of squares in increasing column order. No partial sums or cross-Core reduction are used.

## cross-core synchronization strategy
None. Core ownership is disjoint and output stores are independent.

## cross-core synchronization count
0

## expected x rereads
1 reread per element: one read in the sum-of-squares pass and one read in the output pass.

## expected residual rereads
1 reread per element, matching x.

## expected gamma reload groups
One scalar gamma load per output element; no grouped reload is assumed.

## expected bias reload groups
One scalar bias load per output element; no grouped reload is assumed.

## workspace usage
Kernel workspace: 0 bytes. Host allocation is limited to input, parameter, and output tensors; no reduction workspace is passed to the kernel.

## hot-path domain
Rows with `rows > 0`, `64 <= D <= 32768`, and `D % 16 == 0`, for FP16, BF16, or FP32. This covers the aligned common domain while keeping the row ownership and deterministic accumulation order.

## hot-path dispatch condition
Host passes the actual dtype, `rows`, `D`, epsilon, inverse-D, and selected blockDim. The kernel's row-stride path is valid when `rows > 0`, `D >= 64`, `D <= 32768`, and `D % 16 == 0`.

## fallback domain
Positive row counts and all legal `D` values, including non-16-aligned D, tail rows, and all three supported dtypes. The same scalar implementation naturally handles NaN, Inf, and arbitrary epsilon values through IEEE arithmetic.

## fallback dispatch condition
Any legal input outside the aligned hot-path predicate, especially `D % 16 != 0`, uses the bounds-checked scalar row loop. The kernel performs no out-of-range vector access.

## compile evidence
Compiled on `cann-server3` (`Ascend910B3`, `dav-2201`, CANN `8.5.0.alpha002`) with the local `op_kernel/f001_add_rms_norm_bias_kernel.asc` object target. Build log: `/tmp/codex-f001/build-final.log`.
