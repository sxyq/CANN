# I001 V002 Architecture Metadata

candidate: I001
revision: V002
main_change: self-contained judge-facing kernel.asc (no acl.h, no local type defs, template-conformant)

architecture hypothesis: Wide-D is hurt by tiny D-tiles and full GM rereads. Retained-y when the FP32 row fits UB, large-tile two-pass streaming otherwise.

core mapping: row-parallel, contiguous row ranges per Vector Core.

blockDim strategy: min(availableCoreNum, 40, rows).

row ownership: each core owns a contiguous row range.
D ownership: full D per row.

UB allocation estimate:
- x/r/scratch T tiles: 3 * tile * sizeof(T)
- fA/fB float tiles: 2 * tile * 4
- reduce tmp: 128 B
- retained u row: D * 4 only when mode=retained
- budget target <= 150 KiB working set for retain decision

TQue allocation: 3 x TQue VECIN depth 1 (x, residual, scratch). TBuf for fA/fB/uRow/tmp.

parameter residency lifetime: gamma/bias loaded per D-tile per row.

reduction topology: intra-core ReduceSum over sum(u*u). No cross-core sync in V002.

cross-core synchronization strategy: none.
cross-core synchronization count: 0.

expected x rereads: 1 (retained) or 2 (two-pass).
expected residual rereads: 1 (retained) or 2 (two-pass).
expected gamma reload groups: 1 per D-tile per row.
expected bias reload groups: same as gamma.

workspace usage: none (V002 dropped D-stripe/aclrtMalloc to fix online CE).

hot-path domain:
- retained: retainNeed <= 150 KiB
- two-pass row: retainNeed > 150 KiB

hot-path dispatch condition: host-side in run_kernel from shape/dtype.

fallback domain: FP32/FP16/BF16, rank 2/3/4 via R=prod(leading), D=shape[-1], D 64..32768 including unaligned D via DataCopyPad + exact counts.

fallback dispatch condition: any legal input falls into retained or two-pass.

online compile notes:
- first line #include <cmath>
- last line }
- extern "C" void run_kernel
- __global__ __vector__ entry with __gm__ uint8_t* params
- DataCopyPadExtParams via constructor (no aggregate = {)
- no acl/acl.h
- no TensorInfo/TensorGroupInfo redefinition
