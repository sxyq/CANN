# UB-LIFETIME-SAFE-CHAMPION-X Track-B Handoff

ROUTE: UB-LIFETIME-SAFE-CHAMPION-X
BRANCH: w2/m2/ub-lifetime-safe
WORKTREE: /home/data4t2/lelinfeng/cann-w2-m2-ub
TRACK: TRACK-B ONLY
BASELINE: R31B-V011
OFFICIAL_BASELINE: 45.16
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED: NONE
STATUS: RESEARCH_HANDOFF_ONLY

This handoff contains no selected implementation. Every item below is a
candidate for Planning/Review selection; at most one may become a future
Revision. The scope is only statically provable UB lifetime/reuse and UB peak
reduction. Compute order, algorithm, tiling policy, DMA order, and pipeline
structure remain fixed.

## H1

HYPOTHESIS_ID: H1-FP16-WIDE-PASS1-SCRATCH-TO-OUTPUT
MECHANISM: In the wide FP16 full-y path, reuse the pass-1 FP32 square/rsqrt
scratch backing as the half output tile after pass 1. Remove the separate
output tile allocation; do not alias the retained half y row.
BOTTLENECK: Persistent UB reservation, specifically a half output buffer that
is live only after the pass-1 FP32 scratch is dead.
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES: Wide rows D>8192, prioritizing D=16384 and D=32768, any valid row count.
TARGET_DTYPES: FP16 only.
WHY_IT_MAY_HELP: Replacing outputBuf_ (4096 half elements) with the already
allocated xFp32Buf_ view removes about 8 KiB of simultaneous UB reservation
without changing the retained y representation or arithmetic.
WHY_IT_MAY_FAIL: The typed view may violate TBuf alignment/type or compiler
capacity rules; a future path may accidentally keep an asynchronous use of
the old output slot.
UB_IMPACT: Potentially -kWideFullYTileElems*sizeof(half), about -8 KiB at the
current 4096-element tile; no new allocation.
DMA_IMPACT: Same output GM offsets, valid counts, and store order; only the
local address changes.
SYNC_IMPACT: Preserve the pass-1 scalar/vector handoff and the existing
MTE3-to-V drain at the end of each output tile. No new overlap or removed
event is allowed.
PRECISION_RISK: None from arithmetic if the half view is byte-aligned and the
same FromFloat CAST_ROUND is used; API view/alignment remains a compile gate.
DUPLICATE_CHECK: DISTINCT from UB-LIVENESS-X phase-role aliasing because the
retained y buffer is never aliased and no live value is overwritten. DISTINCT
from R31B-V011's large-row pipeline; this changes only storage lifetime.
RELATED_OLD_ROUTES: R005/FULL-R005 (tile size), R013/FULL-R013 and
ASYNC-TRIPLE-X (pipeline), R002 and UB-LIVENESS-X (y residency): all are
adjacent but not this slot transition.
MINIMAL_EXPERIMENT: Static source audit plus one future OFAT candidate on the
wide FP16 path; verify typed capacity, exact pass boundary, and no old
outputBuf_ allocation before any build/correctness decision.
BUFFER: A=xFp32Buf_ as pass-1 FP32 scratch; B=the same backing viewed as the
half output tile.
ALLOC: A at submission.asc:94; the current separate B allocation is
outputBuf_ at submission.asc:88.
FIRST_WRITE: A is first written by the pass-1 square at submission.asc:3186;
B would first be written by FromFloat at submission.asc:3322.
LAST_READ: A is last read by xFp32.GetValue(0) at submission.asc:3240;
the existing SyncSToV at submission.asc:3243 completes that phase.
REUSE_POINT: Replace outputBuf_.Get<half>() at submission.asc:3299 with a
half view of xFp32Buf_ only after the pass-1 boundary; retain the existing
MTE3-to-V wait at submission.asc:3344 before the next write.
LIFETIME_PROOF: last_read(A)=3240 < first_write(B)=3322, with the required
phase synchronization at 3243. The retained y is gammaBuf_ and is still
read separately in the half output path, so A/B does not alias live y.

## H2

HYPOTHESIS_ID: H2-FP16-WIDE-Y-TEMP-TO-REDUCE-SLOT
MECHANISM: In the same wide FP16 path, after y has been copied to the native
half yStore, reuse the FP32 value tile as the reduction scalar destination
and reduction scratch. Eliminate residualFp32Buf_ from this path only.
BOTTLENECK: A redundant FP32 temporary whose y lifetime ends before the
row-level reduction and whose allocation is retained through the output pass.
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES: Wide rows D>8192, especially D=16384 and D=32768; one or more
rows per core.
TARGET_DTYPES: FP16 only.
WHY_IT_MAY_HELP: It can remove the separate 4096-element FP32
residualFp32Buf_ while retaining the exact native half y in gammaBuf_.
WHY_IT_MAY_FAIL: ReduceSum's third operand/workspace contract may require a
distinct tensor or a larger alignment than the value tile; the API contract
must reject the idea rather than permit an unproven alias.
UB_IMPACT: Potentially -kWideFullYTileElems*sizeof(float), about -16 KiB.
DMA_IMPACT: No GM transaction changes; retained-y and parameter loads stay
unchanged.
SYNC_IMPACT: Keep the existing V-to-MTE2 completion after the pass-1 unit and
the V-to-S/S-to-V sequence around the row reduction. No barrier deletion.
PRECISION_RISK: Arithmetic order and native half y storage stay unchanged;
the only risk is ReduceSum scratch/output alias legality and stale data.
DUPLICATE_CHECK: DISTINCT from R002/UB-LIVENESS-X because the retained y is
not moved or overwritten. DISTINCT from R028 because no scalar synchronization
is removed. This is not parameter residency or multi-row DMA.
RELATED_OLD_ROUTES: R002, R006/R007/R011, R028, FULL-R011, FULL-R028,
UB-LIVENESS-X V001-V003, and R31B-V011's full-y path.
MINIMAL_EXPERIMENT: Before implementation, verify ReduceSum source/destination
requirements for the exact FP16 call sites; then inspect that valueFp32's
last pass-1 read precedes its proposed reduction write and pass-2 refill.
BUFFER: A=valueFp32Buf_ as the temporary FP32 y tile; B=the same backing as
the row-reduction output/scratch, then again as the pass-2 FP32 conversion tile.
ALLOC: A at submission.asc:93; the currently separate reduction temporary is
residualFp32Buf_ at submission.asc:95.
FIRST_WRITE: A is first written by ToFloat at submission.asc:3184 and is
last consumed by the square at submission.asc:3186 after yStore is written
at submission.asc:3182. B would first be written by the row ReduceSum
destination at submission.asc:3228.
LAST_READ: The proposed B reduction result is last read by GetValue at
submission.asc:3240; the current pass-2 value refill is at submission.asc:3318.
REUSE_POINT: Use valueFp32Buf_ as the third reduction operand at the tile
ReduceSum and as the final row-reduction destination, then refill it from
the retained half y for pass 2. Remove residualFp32Buf_ only if the API
contract permits this exact use.
LIFETIME_PROOF: last_read(A)=3186 < first_write(B)=3228; after the reduction,
last_read(B)=3240 < first_write(pass2-value)=3318. The proof is sequential
and preserves the existing event waits; it is not an unproved concurrent
alias.

## H3

HYPOTHESIS_ID: H3-BF16-MID-DEAD-FP32-CACHE-OMISSION
MECHANISM: Make BF16 parameter-conversion caches path-specific. The
gammaFp32Buf_/biasFp32Buf_ allocations are dead on the dispatcher path that
returns to ProcessNarrowMidOverlap, so do not reserve them there.
BOTTLENECK: UB peak caused by two full 8192-element FP32 parameter caches on
a path that loads native gamma/bias per row and never accesses those caches.
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES: BF16 narrow-mid rows with 128<D<=4096 after the existing
contiguous-batch dispatch; include single-row and multi-row cases that reach
ProcessNarrowMidOverlap.
TARGET_DTYPES: BF16 only.
WHY_IT_MAY_HELP: Omitting two 8192-float caches can reduce the BF16 mid-path
UB reservation by up to 64 KiB and may expose a lower-peak configuration.
WHY_IT_MAY_FAIL: InitBuffer decisions may not be expressible with the current
path predicate, or a hidden compiler/template instantiation may require the
members. If so, classify this as infeasible; do not alias the dead members.
UB_IMPACT: Potentially -2*kCacheElems*sizeof(float), about -64 KiB on the
BF16 narrow-mid path.
DMA_IMPACT: None: ProcessNarrowMidOverlap keeps its existing native gamma/bias
loads and event groups.
SYNC_IMPACT: None: preserve inputReady, paramReady, inputRelease, and all
existing V/S/MTE3 waits.
PRECISION_RISK: None if the path never reads the omitted caches; the guard
must be proven equivalent to the dispatcher condition before implementation.
DUPLICATE_CHECK: DISTINCT from R014/FULL-R014, BATCH-RESIDENT-X, and
COEFF-LOCALITY-X because it removes an unused cache and does not change
parameter residency. It is also distinct from DTYPE-SPECIAL-X arithmetic or
conversion changes.
RELATED_OLD_ROUTES: R014/FULL-R014, BATCH-RESIDENT-X, COEFF-LOCALITY-X,
DTYPE-SPECIAL-X, and R31B-V011's generic BF16 cacheParamFp32 branch.
MINIMAL_EXPERIMENT: Static call-graph audit: prove the return at
submission.asc:237-240 precedes the cache allocation consumer at 249-280,
and prove no gammaFp32Buf_/biasFp32Buf_ Get occurs in lines 499-620. Only
then assess a path-guarded InitBuffer diff.
BUFFER: A=gammaFp32Buf_ and biasFp32Buf_ on the BF16 narrow-mid path;
B=valueFp32Buf_ and the existing live path buffers.
ALLOC: A at submission.asc:153-155 under the current BF16 non-wide Init;
B at submission.asc:146 and the input/output buffers at 116-142.
FIRST_WRITE: A has no first write on this path. B first receives data in
ProcessNarrowMidOverlap at submission.asc:552-555.
LAST_READ: A has no read on this path. Its path-local last_read is NONE.
REUSE_POINT: Do not allocate A for this path; leave B's existing allocation
unchanged. If the allocator cannot omit A conditionally, reject this item
instead of creating an alias.
LIFETIME_PROOF: last_read(A)=NONE (no access after allocation because the
dispatcher returns before cacheParamFp32 use) < first_write(B)=552/555. This
is an allocation omission, not an alias; no concurrent lifetime is assumed.

## Duplicate Audit

The audit uses the committed route table, revision ledger, current task and
the referenced route research/evidence. `DUPLICATE_REJECTED` marks a prior
mechanism that must not be reintroduced as a hypothesis here.

R001: two-pass scan; not UB lifetime, distinct.
R002: full-y/input residency; adjacent only, distinct from H1/H2 because y is never aliased.
R003: direct invocation/build entry; distinct.
R004: low-precision arithmetic; distinct from allocation-only H3.
R005: large tile; `DUPLICATE_REJECTED` for tile-size changes, not for lifetime proof.
R006: reduction topology/capacity; `DUPLICATE_REJECTED` for reduction redesign, H2 keeps topology.
R007: ReduceSum primitive; `DUPLICATE_REJECTED` for changing reduction primitive, H2 only changes a proven slot.
R008: cross-core/D-slice; distinct and forbidden here.
R009: DataCopyPad tail; distinct.
R010: software tail; distinct.
R011: manual vector reduction; `DUPLICATE_REJECTED` for a new reduction tree, not H2.
R012: row-group alignment; distinct.
R013: double-buffer pipeline; `DUPLICATE_REJECTED` for new pipeline overlap, all H1-H3 preserve order.
R014: gamma/bias residency; `DUPLICATE_REJECTED` for parameter-residency changes, H3 removes a dead cache only.
R015: multi-row DMA; `DUPLICATE_REJECTED` for batched DMA, none is proposed.
R016: row/core scheduling; distinct and forbidden here.
R017: all-FP32 middle; distinct.
R018: CAST_RINT output; distinct.
R019: reciprocal/invRms math; distinct and forbidden here.
R020: sqrt/rsqrt math; distinct and forbidden here.
R021: CANN9 build chain; distinct.
R022: code generation; distinct.
R023: engineering convention; distinct.
R024: CPU validation; evidence process, not mechanism.
R025: submission integrity; distinct.
R026: performance measurement; protocol only, not a hypothesis.
R027: GPU migration reference; distinct.
R028: scalar synchronization reduction; `DUPLICATE_REJECTED` for removing V/S sync, all proposed proofs retain waits.
R029: five-mode tiling; distinct and forbidden here.
FULL-R001..FULL-R029: same audit as R001..R029; `DUPLICATE_REJECTED` for FULL-R005/R013/R014/R015/R028 mechanism copies, no duplicate lifetime item.
R31A: affine/path evolution and its delete-flag variant; `DUPLICATE_REJECTED` for arithmetic, path, or barrier changes. Lifetime-only storage remains distinct.
R31B: V011 LP row pipeline and full-y retention; `DUPLICATE_REJECTED` for another pipeline/full-y implementation. Existing x/residual-to-param reuse is also not proposed again; H1/H2 use different dead ranges.
MIX-A: mixed aligned-copy/tail/parameter composition; `DUPLICATE_REJECTED` for any mixed or DMA composition. No MIX mechanism is included.
Wave-1 UB-LIVENESS-X V001-V003: `DUPLICATE_REJECTED` for phase-role aliasing or live-y alias; H1/H2 require last-use proofs and H3 is path allocation omission.
Wave-1 BATCH-RESIDENT-X, SCHED-ROWGROUP-X, ASYNC-TRIPLE-X, ALIGN-TAIL-X, REDUCE-HIER-X, COEFF-LOCALITY-X, VECTOR-MATH-X: `DUPLICATE_REJECTED` for parameter residency, row scheduling, pipeline, tail DMA, reduction topology, parameter locality, or math. None is reused as a mechanism.
Related current routes MULTIROW-DMA-CHAMPION-X, STORE-EPILOGUE-X, EPILOGUE-FUSE-X, EPILOGUE-ARITH-CHAMPION-X, SHAPE-TILING-CHAMPION-X, and DTYPE-SPECIAL-X: `DUPLICATE_REJECTED` for their DMA/store/fusion/tiling/arithmetic changes; no lifetime hypothesis is copied.

## Route Boundary And Handoff

ROUTE_BOUNDARY: Only UB buffer lifetime/reuse that is statically safe. Every
candidate must name BUFFER, ALLOC, FIRST_WRITE, LAST_READ, REUSE_POINT and
prove last_read(A) < first_write(B). Early release, safe slot reuse, shorter
affine temporary lifetime, dtype-specific lifetime, and UB-peak-only changes
are allowed. Correctness is the priority.
ALLOWED_CHANGES: Research notes, source-level lifetime tables, call-graph and
API feasibility checks, one future selected OFAT storage-lifetime diff, and
targeted correctness/performance probe plans.
FORBIDDEN_CHANGES: Any .asc/.cpp/.h/.hpp, CMake, runner, host wrapper,
kernel, compute order, arithmetic, tiling, row/core schedule, new DMA
pattern, unproved alias, synchronization removal, Revision, build/link,
correctness, timing, profiling, server3, Online/Judge, shared ledger,
MAIN_SELECTED, or route lifecycle change.
EVIDENCE_PATHS: 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv;
技术路线/技术路线总表.md; 技术路线/技术路线图.md; 调度/当前任务.tsv;
调度/线上候选.tsv; 调度/本地线上校准.tsv;
线上结果/R31B/V011/source-meta.json; 线上结果/R31B/V011/result.json;
线上结果/R31B/V011/submission.asc; 研究/UB-LIVENESS-X/ROUTE-BRIEF.md;
研究/UB-LIVENESS-X/next-hypotheses.md;
本地实验/UB-LIVENESS-X/V003/MAIN-REVIEW.md;
本地实验/UB-LIVENESS-X/V003/local-result.json;
线上结果/UB-LIVENESS-X/V003/result.json; 研究/BATCH-RESIDENT-X/;
研究/COEFF-LOCALITY-X/; 研究/ASYNC-TRIPLE-X/;
研究/SCHED-CHAMPION-X/; 研究/STORE-EPILOGUE-X/.
PROPOSED_ONE_FACTOR_DIFF: Planning/Review selects at most one of H1-H3;
the selected diff may touch only the named InitBuffer/view/lifetime site,
with no bundled pipeline, DMA, arithmetic, or synchronization optimization.
No hypothesis is selected by this child.
EXPECTED_LOCAL_PROBES: Not run in Track-B. After external selection, use
same-binary and correctness first on FP16 D=16384/32768 for H1/H2 and BF16
D=256/1024/2048/4096 for H3, with one-row and multi-row cases that reach the
named path. Record UB allocation/peak and exact source identity; only then
use the repository's approved local timing protocol.
CHILD_RECOMMENDED_HYPOTHESIS: NONE_SELECTED. Return H1-H3 to Planning/Review
without implementation preference.
OPEN_QUESTIONS: Does the target compiler permit a larger FP32 TBuf viewed as
half? Does ReduceSum permit the exact H2 destination/third-operand reuse?
Can InitBuffer be guarded by the same runtime path predicate without changing
template layout? What is the exact UB allocator behavior for omitted buffers?
Any unresolved answer blocks that hypothesis; it does not justify an alias.

TRACK_B_STARTED: YES
429_OCCURRED: NO
KERNEL_CHANGED: NO
REVISION_CREATED: NO
BUILD_LINK_CORRECTNESS_TIMING_PROFILING: NO
SERVER_RUN: NO
ONLINE_JUDGE: NO
MAIN_SELECTED: NONE
