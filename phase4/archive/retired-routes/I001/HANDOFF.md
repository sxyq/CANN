# I001 Final Handoff

ROUTE: I001
STATUS: PARKED
BEST_REVISION: V004
BEST_SCORE: 15.53
BEST_PASS_COUNT: 15/15
(Fastest incomplete: V005/V006 calc 25.3 at 13/15 — not a scored best)

## ARCHITECTURE
Fresh Wide-D Specialist. Two-pass row-parallel AddRmsNormBias with DataCopyPad alignment, ReduceSum + 8KiB tmp, store exact n. Later modes: retained-y when FP32 row fits ~140KiB; large-tile (2048) streaming; few-row D-split with SyncAll and workspace. Judge ABI: `GM_ADDR` + `const TensorGroupInfo&` + `int64_t availableCoreNum` + `aclrtStream`.

## WHAT_WORKED
- Exact judge ABI (V003): first online compile after CE.
- Unaligned `DataCopyPad` (`rightPadding = workN-n`) + separate ReduceSum 8KiB tmp (V004): first 15/15 at 15.53.
- Retained-y / large-tile / D-split (V005): large speedups (T05 24, T09 186, T11 259, T15 14323) when correct.

## WHAT_FAILED
- Online CE until ABI matched (V001–V002).
- T13 RE from overlapping ReduceSum tmp (fixed in V004).
- Retained-y tiny-D path: T02/T03 ~100% WA in V005/V006 (uKeep save/restore or tiny-tile keep defect) even after padding slack.
- Wide T14 remained ~118–121 ms (r≈30+) throughout.

## UNFINISHED
- V007 force D<256 to V004 two-pass (compiled, not online).
- Root cause of retained-y tiny-D WA.
- Wide T14 algorithmic gap vs best 3750 us.

## REUSABLE_IDEAS
- Unaligned D: `isPad=true`, `rightPadding=align(n)-n`, compute on workN, store n; zero pad safe for sum(u*u) if gamma/bias zero-padded too.
- `ReduceSum<float,true>(dst, src, tmp, workN)` + `dst.GetValue(0)` with dedicated 8KiB tmp.
- Retained-u buffers must be sized to padded workN, not raw cols.
- Gate retained-y above D≥256 until tiny keep-path is proven.
- D-split + 2×SyncAll for few-row wide when R≤2×cores (host workspace via aclrtMalloc or output tail).

## DO_NOT_REPEAT
- Multi-kernel / extra `__global__` entry launches that break the single-entry ABI (R31B-V008 class — same submission format risk).
- Enable retained-y on tiny D without a proven save/restore path.
- Overlap ReduceSum tmp with work buffers.
- Expect D-slice alone to fix T14 (R31A-V011 evidence).

## LAST_AGENT
general-9 (I001 wide-D v2, after general-6 stream failure).

## LAST_COMMIT
`d7cac20` phase4(A001,R31A,I001): add V018 D-split, V013 BF16 pipe, V007 tiny two-pass
