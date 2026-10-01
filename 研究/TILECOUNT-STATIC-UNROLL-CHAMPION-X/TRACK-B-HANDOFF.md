# TRACK-B Handoff — TILECOUNT-STATIC-UNROLL-CHAMPION-X

## State and scope

```text
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
ASSIGNMENT=MAIN-2 research-only M2-4 (per task assignment)
TRACK=TRACK-B
MAIN_SELECTED=NONE
IMPLEMENTATION=NOT_STARTED
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO (3 source-level candidates; codegen-gated)
REMOTE_FRESHNESS=UNKNOWN
```

This is a read-only route handoff. No kernel/source/build/runner edits, Revision, build, correctness run, timing, profiling, server work, or Online action was performed. Only this route research file is intended to change.

Assignment metadata caveat: the checked-in `研究/主代理/MAIN-2/CAMPAIGN-STATUS.md` currently labels M2-4 as `ASYNC-OVERLAP-CHAMPION-X`. This handoff follows the explicit task assignment; it does not update shared scheduling or decide route ownership/lifecycle.

One bounded fetch was attempted before this handoff; the remote route ref was unavailable. No retry was made. Findings below use the current local evidence only. The worktree began clean on `w2/m2/tilecount-unroll` at `ed860e392d7604694ac6664da60aff1fc1f4c04f`.

## Baseline and exact qualifying path

| Field | Evidence |
|---|---|
| `DIRECT_PARENT` | R31B V011 |
| `PARENT_OFFICIAL` | 15/15, 45.16; current Overall Champion in `技术路线/路线成绩表.tsv` |
| `PARENT_SOURCE_SHA` | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| Source identity | SHA-256 of `线上结果/R31B/V011/submission.asc` matches the parent SHA and `source-meta.json` |
| Geometry to preserve | `kTileElems=4096`, `kCacheElems=8192`; no tile-width, cache-width, UB-allocation, or tiling-policy changes |

In `线上结果/R31B/V011/submission.asc`, FP32 `Process()` dispatches to `ProcessFp32FullRowOutputPipelined` only when `rowWidth == kCacheElems` and `localRows > 1` (around lines 178–180). For that exact path, `kCacheElems / kTileElems = 8192 / 4096 = N=2`. Three separate inner loops have a compile-time-invariant two-iteration bound:

1. gamma/bias parameter preload (`col=0; col<8192; col+=4096`, around line 1129);
2. first-pass x/residual load, add/square, and partial reduction (around line 1164);
3. output arithmetic and store (around line 1203).

The row loop and the launch-dependent `localRows` are not compile-time fixed and are explicitly out of scope. No N=3 or N=4 path was proven; all candidates below use only N=2.

Concrete target instance: **shape `[128,8192]`, dtype FP32, `availableCoreNum=32`**. The wrapper clamps requested blocks to row count, so this gives `blockCount=32`, `localRows=4`, and selects the path on every block. More generally, `[128,8192]` selects it for blocks with `localRows>1`; the exact launch count must be recorded. Official case-to-shape mapping is missing in the records, so this is a qualified probe target, not a claim about any particular Official case.

Important qualification limit: source constants prove the source-level trip count, **not** what the existing compiler emitted. There is no parent disassembly/codegen artifact in the reviewed evidence. If the parent already emits straight-line two-tile code for a loop, a source-level unroll there has no distinct mechanism and that candidate must be rejected rather than implemented as a no-op.

## Candidate hypotheses

Each hypothesis is a separate one-loop experiment and a separate possible Revision. Each has `FOCUS_AXIS=TILE_COUNT`, specializes only `N=2`, and preserves `kTileElems=4096`, `kCacheElems=8192`, all UB geometry, and the existing path dispatch. These are unranked research candidates; none is selected.

### TCSU-H1 — parameter-preload N=2 unroll

- `FOCUS_AXIS=TILE_COUNT`
- **Mechanism:** In `ProcessFp32FullRowOutputPipelined` only, explicitly expand the two iterations that load gamma and bias at offsets 0 and 4096. Do not change parameter lifetime, residency, prefetching, or buffer allocation.
- **Bottleneck:** Per-block setup loop control and tile-offset calculation before rows are processed.
- **DIRECT_PARENT / SOURCE_SHA:** R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Exact domain / target:** FP32 `[128,8192]`, `availableCoreNum=32` (`blockCount=32`, `localRows=4`); the source dispatch condition is `rowWidth=8192 && localRows>1`.
- **Why it may help:** Removing a two-trip loop backedge and making the two offsets immediate may simplify this setup sequence if codegen currently retains its counter/branch.
- **Why it may fail:** The loop runs once per block, while four parameter DMAs dominate its tiny control cost; the compiler may already unroll it. Expected ceiling is low.
- **UB / core / DMA / sync / precision:** UB sizes and tensor lifetimes unchanged; block/core assignment and `localRows` unchanged. Same two gamma and two bias tile loads, same order; no added or removed synchronization/event operations. Arithmetic and reduction are untouched, so precision order is unchanged.
- **`OLD_MECHANISM` / `NEW_DIFFERENCE`:** R014 / FULL-R030 / COEFF-LOCALITY-X concern parameter residency, reuse, or MTE2 prefetch. This candidate introduces none of those; it only removes loop control for the existing fixed N=2 preload. It is distinct only if parent codegen still has the loop.
- **Duplicate check / related routes:** R014, FULL-R030, COEFF-LOCALITY-X. No reuse/prefetch overlap; do not increase cache/parameter tile size. Conditional `PASS` subject to parent-codegen inspection.
- **Minimum experiment after explicit Main selection:** Compare exact-parent and one-loop-only candidate codegen using identical compiler options. First prove the parent has a loop backedge and candidate emits the two fixed loads; otherwise reject. Then verify unchanged UB allocation and load count/order, run targeted and project correctness, qualify same-binary, and interleave parent/candidate timing only on the exact target shape under the project protocol.
- **Expected evidence:** source SHA and one-loop diff; compiler/version/options and parent/candidate assembly; backedge/address-instruction comparison; unchanged UB bytes and four parameter-load operations; correctness report; same-binary and raw paired timing with noise/load qualification. No Official claim from a single probe.

### TCSU-H2 — first-pass N=2 tile-loop unroll

- `FOCUS_AXIS=TILE_COUNT`
- **Mechanism:** In the first-pass loop only, explicitly expand the two 4096-element tiles. Preserve the `hasPrefetchedFirstTile` decision, next-row prefetch, both per-tile `SyncMTE2ToV` / `SyncVToMTE2` boundaries, and the existing two partial reductions.
- **Bottleneck:** Repeated loop counter/branch and tile-offset/index calculation in the row’s input-and-square-reduction pass.
- **DIRECT_PARENT / SOURCE_SHA:** R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Exact domain / target:** FP32 `[128,8192]`, `availableCoreNum=32` (`blockCount=32`, `localRows=4`); `rowWidth=8192 && localRows>1` selects the function containing this loop.
- **Why it may help:** A retained backedge and dynamic `col / kTileElems` or offset work could be replaced by fixed tile-0/tile-1 operations. The second tile’s offset/reduction destination become constants.
- **Why it may fail:** Each iteration performs DMA, vector operations, barriers, and reduction; their cost likely dwarfs two loop-control steps. The compiler may already fold/unroll the loop. More straight-line code may increase register live ranges or instruction footprint.
- **UB / core / DMA / sync / precision:** No UB allocation or tile layout change; block/core assignment and `localRows` unchanged. Preserve the same first-tile conditional load and second-tile load, same x/residual DMA count/order, and next-row prefetch. Keep the original per-tile synchronization and reduction ordering exactly; do not pipeline, remove fences, or fold reductions. No arithmetic reassociation, so FP32 precision order remains unchanged.
- **`OLD_MECHANISM` / `NEW_DIFFERENCE`:** R005 changes tile geometry to reduce iteration count; R013/ASYNC routes change buffering or overlap; REDUCE-HIER/R011 change reduction topology. This hypothesis changes none of those. It specializes the existing, proven N=2 loop only. Any tile width, number of tiles, DMA schedule, or synchronization change is outside this route and `DUPLICATE_REJECTED`.
- **Duplicate check / related routes:** R005, R013, ASYNC-OVERLAP-CHAMPION-X, REDUCE-HIER-X / R011, and SHAPE-TILING-CHAMPION-X. No geometry or pipeline changes; conditional `PASS` only if parent codegen still has loop control.
- **Minimum experiment after explicit Main selection:** Inspect parent codegen first; if already straight-line N=2, reject. Otherwise create one revision expanding only this loop, retain its dynamic row prefetch condition and exact sync/reduction sequence, then compare assembly, test correctness, and run same-binary plus interleaved parent/candidate timing on `[128,8192]` FP32. Do not use any other N or shape to imply benefit.
- **Expected evidence:** parent/candidate assembly demonstrating removal of only this loop backedge; unchanged tile offsets `{0,4096}`, loads, barriers/events, `ReduceSum` count/order, UB allocation, and register/spill report; exact-source correctness and qualified raw P/C samples. A timing gain without a codegen delta is not attributable to this hypothesis.

### TCSU-H3 — output-pass N=2 tile-loop unroll

- `FOCUS_AXIS=TILE_COUNT`
- **Mechanism:** In the output loop only, explicitly expand the two tiles at offsets 0 and 4096. Keep tile-specific `outputReadyFirst/Second` and `outputReleaseFirst/Second` event selection, waits, stores, and drain order in their original positions.
- **Bottleneck:** Per-row output loop control/addressing and the `useFirst`-dependent selection of event IDs for two fixed tiles.
- **DIRECT_PARENT / SOURCE_SHA:** R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Exact domain / target:** FP32 `[128,8192]`, `availableCoreNum=32` (`blockCount=32`, `localRows=4`); only the existing `rowWidth=8192 && localRows>1` path is in scope.
- **Why it may help:** Constant tile identity can fold `col==0`, the event-ID selection, and loop-index arithmetic, if these remain in generated code. The output loop executes once per row, so a residual loop-control cost repeats for all four local rows.
- **Why it may fail:** Three vector operations plus each store/event sequence dominate the control overhead; the compiler may already specialize both iterations. Straight-line code can increase code size/register pressure. Moving waits/stores to chase overlap would be a different mechanism and is forbidden here.
- **UB / core / DMA / sync / precision:** UB layout and value-buffer ownership unchanged; block/core assignment and `localRows` unchanged. Exactly two output stores per row at the original offsets; no store merge or change to store granularity. Preserve all original V↔MTE3 flag allocation, set/wait/release counts and order. Keep `Muls` → `Mul` → `Add` order exactly; no precision or rounding change intended.
- **`OLD_MECHANISM` / `NEW_DIFFERENCE`:** STORE-EPILOGUE-X merges writeback runs; R31A V021 moves a store wait; EPILOGUE routes change arithmetic/dataflow. This candidate does none of these: it only specializes loop control for the exact existing two output tiles.
- **Duplicate check / related routes:** STORE-EPILOGUE-X, R31A V021, EPILOGUE-FUSE-X / EPILOGUE-ARITH-CHAMPION-X, R005, SHAPE-TILING-CHAMPION-X. No writeback merge, wait movement, arithmetic fusion, or geometry change; conditional `PASS` on codegen evidence.
- **Minimum experiment after explicit Main selection:** Prove parent output loop is not already unrolled. Make a one-loop-only N=2 expansion, compare codegen and event/store order, verify unchanged geometry, then correctness and protocol-qualified same-binary/interleaved timing on `[128,8192]` FP32.
- **Expected evidence:** source SHA/diff; assembly showing constant event selection and removal of only the output-loop backedge; unchanged UB, two stores per row, and identical synchronization/event order; precision/correctness report; raw timing and noise qualification. Reject if generated code is already equivalent or if a benefit requires changing store/wait behavior.

## Mandatory duplicate audit summary

| Prior work | `OLD_MECHANISM` | `NEW_DIFFERENCE` and disposition |
|---|---|---|
| R005 / FULL-R005 / later larger-tile experiments | Change tile width/geometry to reduce tile iterations; affects UB usage and number of tiles. | Keep `kTileElems=4096`, `kCacheElems=8192`, UB plan, and N=2 fixed. Remove only a source loop’s control overhead. Any geometry/tile-width change is `DUPLICATE_REJECTED`. |
| R029 / FULL-R029 and SHAPE-TILING-CHAMPION-X | Runtime mode taxonomy or shape-conditioned tile-width/UB tradeoff; SHAPE-TILING is a separate MAIN-1 research route. | No new mode, shape bucket, tile size, or dispatch rule. Specialize one existing exact FP32 path’s fixed N=2 loop. Do not inspect or modify another route’s writable tree. |
| R022 / FULL-R022-AUTO-CODEGEN | Generator selects tile/vector/build parameters and emits source. Its generated kernel still contains group/row loops and tile loops whose bound is runtime `rowWidth`. | No generator or schedule parameter. The only proposed specialization is at an existing path where the source constants prove N=2. R022’s completed Official result does not establish this loop’s codegen or benefit. |
| Historical unroll/codegen references | Generic compiler-IR discussion; CUDA/GPU `#pragma unroll`; upstream `simd_vf_story/reduce` `baseline/pragma/unroll/binary` sample variants. These are not a demonstrated AddRmsNormBias Ascend C experiment on this parent/path. | Narrow candidate is an exact two-tile loop in R31B V011’s Ascend C FP32 full-row output path. Generic unrolling itself is not claimed as novel. If parent assembly already has equivalent straight-line code, the affected hypothesis is a no-op/duplicate and must not proceed. |
| HOTLOOP-BRANCH boundary | Common-loop branch hoisting belongs to HOTLOOP-BRANCH. | This route requires a proven fixed trip count and specializes one N=2 loop only. No common branch hoisting, guard restructuring, or hoisting across rows. |

## Related evidence and interpretation

- The R001–R029 / FULL-R catalog is completed historical breadth evidence; R005 owns tile-size change, R022 owns generated schedule/source, and R029 owns mode-selection taxonomy. None supplies proof that this V011 N=2 loop remains branched in generated code.
- MAIN-2 Wave-1 records report several local-positive mechanisms that did not transfer to Official (including ASYNC V001, STORE V003, and epilogue arithmetic), plus five reduction variants without a credible large-D win. Treat any anticipated unroll gain as a small, unproven instruction-control hypothesis—not an Official score forecast.
- Calibration also records candidate-side timing bias and a missing Official case-to-shape map. A single local shape result cannot establish Official relevance.
- R31A/R31B/MIX evidence remains reference-only here. No implementation, cross-route change, or route lifecycle decision is authorized by this handoff.

## Review gate

`MAIN_SELECTED=NONE`. A Main/Planning selection is required before any Revision or implementation. First evidence gate for each selected hypothesis is parent disassembly: if the compiler already unrolls that loop, stop that hypothesis. Any later experiment must be single-variable, use N=2 only, and leave tile width, UB geometry, DMA topology/count, synchronization ordering, and arithmetic/precision order unchanged.
