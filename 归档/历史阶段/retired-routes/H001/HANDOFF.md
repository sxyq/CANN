# H001 Final Handoff

ROUTE: H001
STATUS: PARKED
BEST_REVISION: V008
BEST_SCORE: 29.04
BEST_PASS_COUNT: 15/15

## ARCHITECTURE
Fresh Small-D / High-R specialist. Multi-row tile (up to 64–128 rows) for D≤1024 with resident FP32 gamma/bias; chunked two-pass full-row wide fallback for D>1024 with uint64 GM offsets. Vector `ReduceSum` for sum(u*u) on hot and wide paths with dedicated 8KiB tmp. TQue depth 1, Free-before-next-Alloc. B001-shaped judge template (3 dtype `__global__ __vector__` entries + `run_kernel` GM tiling).

## WHAT_WORKED
- Judge template matching B001 skeleton (V003): first online compile/run after CE.
- Two-pass full-row wide RMS + uint64 row*cols offsets (V004): first 15/15 (12.54).
- Vector `ReduceSum` for sum(u*u) (V007 hot 12.55→23.29; V008 wide 23.29→29.04) — dominant speed lever.
- One optimization per online shot after 15/15 baseline.

## WHAT_FAILED
- Online CE until template/ABI matched (V001–V002).
- Wide per-chunk RMS with `1/D_full` (V003): ~99.9% WA on D>1024.
- Depth-2 TQue Alloc-before-Free (V005): 0/15 TLE deadlock.
- Fused ApplyRow alone (V006): flat vs V004.

## UNFINISHED
- V009 wide resident gamma/bias (compiled, not online).
- T05 still r≈7, T14 r≈6.1, T11 r≈3.2 at park vs best times.

## REUSABLE_IDEAS
- `ReduceSum<float,true>(partial, src, reduceTmp, count)` + 8KiB disjoint tmp is the correct vector reduce on dav-2201.
- Chunked fallback must accumulate sum(u*u) over the full row before invRms.
- `uint64_t row * cols` for GM offsets on large R*D.
- bisheng rejects `static_cast` bf16↔float; use `AscendC::ToFloat`/`ToBfloat16` or vector `Cast`.
- Never Alloc depth-1 TQue before Free of held tensor.
- B001 15/15 skeleton: cmath + kernel_operator.h, file-scope prefixed types, TQue EnQue/DeQue, field-assigned DataCopy*ExtParams, N dtype entries, run_kernel with aclrtMalloc tiling.

## DO_NOT_REPEAT
- Depth-2 queue without matching Free lifecycle.
- Per-chunk RMS on wide rows.
- Multiple optimizations in one online shot after a green baseline.
- Online submit of non-template exotic constructs (anonymous namespace, PipeBarrier, __builtin_sqrtf, .template Get, if constexpr) even if local mock accepts them.

## LAST_AGENT
general-5 (H001 small-D high-R).

## LAST_COMMIT
`bfc26a1` phase4(H001): add V009 wide resident params
