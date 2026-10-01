# HOTLOOP-BRANCH-HOIST-CHAMPION-X — Track-B handoff

## Scope and frozen parent

- ROUTE: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- TRACK: `B` / research-only; `FOCUS_AXIS=BRANCH_KIND`
- MAIN_SELECTED: `NONE`
- DIRECT_PARENT: `R31B V011`
- PARENT_OFFICIAL: `45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent artifact audited: `线上结果/R31B/V011/submission.asc` (3554 lines, SHA matches the parent SHA above).
- Fixed algorithm boundary: retain V011 donor/path selection, arithmetic order, DMA ownership, tile width, synchronization protocol, and buffers. This note proposes only removal/hoisting of repeated internal control decisions; it does not propose a selective fastpath or a new dispatch domain.
- `origin` freshness: `FETCH_OK`; one bounded `timeout 20s git fetch --no-tags origin` completed successfully. Worktree was clean before this handoff.

V011's generic `Process()` loop has a runtime coefficient-residency mode (`cacheParams`; for BF16, `cacheParamFp32`) tested in the output tile body, plus a final-row predicate around its next-row prefetch. Its active low-precision wide path (`ProcessWideLowPrecision`) recomputes the full-tile/tail length for each flattened row/tile unit and alternates two DMA slots with `useA`. Device dtype selection uses `if constexpr`; host dtype dispatch occurs once per launch, not inside the hot loop. The panel/alignment fallback in `ProcessWideFp32PanelResident()` has no caller (`ProcessWideFp32()` calls `ProcessWideFp32FullCacheRows()` instead). These are excluded from the surviving pool.

## Evidence context reviewed

- The canonical champion record and V011 `result.json` both say `R31B V011`, `15/15`, Official `45.16`, submission `6ab2c10c0304f72a56a0c5cb`; champion source/sidecar SHA is the parent SHA above. V011 is the required frozen direct parent even though later R31B experiments exist.
- Relevant historical route catalog: `技术路线/技术路线总表.md` summarizes all R001–R029 and FULL-R001–FULL-R029 terminal evidence. Branch-hoist boundary cases include R005/FULL-R005 (tile iteration count), R009/R010 (tail copy semantics), R011/R028 (vector/scalar reduction/synchronization), R022/FULL-R022 (codegen record), and R029/FULL-R029 (two differently named records: five-mode dispatch idea versus wide cached-row implementation; FULL-R029 ended 4/15 Runtime Error). These are not all the same mechanism.
- R022 nuance: the canonical route table records `FULL-R022-AUTO-CODEGEN-V001` at 15/15, 24.11, while the historical idea-pool specifically describes dtype×D-bucket codegen as “PLANNED / not used”. Therefore this handoff treats broad code generation as historical, but does not claim that the exact dtype×D specialization proposal was implemented or tested. None of HBH-01..03 generates code or creates new dtype/D buckets.
- R029 nuance: `LEGACY-R029-TILING-FIVE-MODES` is an outer dispatch taxonomy (`SPLIT_D/SINGLE_N/MERGE_N/MULTI_N/NORMAL`); `FULL-R029-WIDE-CACHED-ROW-V001` is the old implementation with Runtime Error. Neither is an internal repeated branch in V011's already-selected loop. V011's dispatch domain remains fixed.
- R028/FULL-R028 targets scalar synchronization/GetValue reduction and records 15/15, 25.06. MAIN-2 Wave-1 scalar/S3 evidence treats handoff/sync changes as a separate cost axis. No hypothesis here changes a scalar operation, reduction, or sync.
- R31 lineage evidence reviewed: R31A V016 Official `45.00`; R31B V006 changed MTE3 queue depth (`43.91`), V009 changed wide FP32 MTE2 depth (`43.81`), V010 added narrow/mid input/parameter overlap, V011 won `45.16` with low-precision wide-row pipeline, and later V012/V013 remained below it. R31B V017's strong local FP16/BF16-wide signal still returned Official `44.68`; R31B V015's extra FP32 mid predicate was reviewed as selecting an already-existing operation. R31A V028 also remained below its anchor (`44.07` vs `45.00`); MIX-A V003 Official is `44.69` and V007's sync edit did not establish a clean local result. Their multimode/dispatch and MTE3-wait or sync changes are adjacent but not this branch-only mechanism.
- MAIN-2 Wave-1 records (`研究/主代理/MAIN-2/`): M2-3 was COEFF-LOCALITY-X with V001–V004, zero accepted variants and `LANE_NEEDS_PLANNING_REVIEW`; its Official V001 remains 44.16. Wave-1 also records five later local-positive/Official-negative cases and a candidate-side measurement bias; local percentage is not evidence of Official improvement. The current task explicitly scopes this new handoff to the named research route; no Wave-1/shared status was edited.
- The Official 15-case shape/dtype map remains unavailable in the MAIN-2 records. Shape targets below are kernel-path probes, not claims about specific Judge cases.

## Surviving hypothesis pool

The three entries below are distinct branch-hoist mechanisms. Each keeps the same selected V011 path and operations; only the location/form of an invariant mode, row-tail, or per-row tile-tail control decision changes.

### HBH-01 — Generic coefficient-residency mode guard

- ID: `HBH-01`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- FOCUS_AXIS: `BRANCH_KIND`
- Mechanism: resolve the existing generic `cacheParams` mode (and its BF16-only `cacheParamFp32` subcase) once before the output tile loop, then enter an equivalent cached-parameter or streamed-parameter body. Keep `cacheRow` behavior and every operation in the selected mode unchanged; remove only repeated coefficient-mode tests/tensor selection from each tile.
- Bottleneck: scalar/control issue and conditional gamma/bias source selection in the generic two-pass output loop (source lines 389, 416–451), repeated for each tile.
- Targets: generic widths `4097 <= D < 8192` (e.g. `D=6144`) for FP32/FP16/BF16, with both existing `localRows=1` and `localRows>1` modes. `D=8192` is a control only when the existing dispatch reaches generic (`localRows=1`); do not route around the existing multi-row specializations.
- Why it may help: the predicate is constant for all tiles in a row/process. A single outer mode decision could remove repeated conditional coefficient loads and cached-vs-streamed tensor indexing without changing coefficient residency.
- Why it may fail: the compiler may already fold/hoist the flag; the hot loop may instead be limited by vector work or synchronization. Duplicating the two bodies may raise code size/register pressure. COEFF history also shows parameter preloading itself is not a reliable gain, so this must demonstrate branch removal rather than claim a residency win.
- UB/DMA/sync/precision impact: no UB allocation or buffer count change; same DataCopyPad spans and same DMA ordering; no new events and no removed barriers; no arithmetic, cast, rounding, or reduction change. The only allowed difference is control placement.
- OLD_MECHANISM: R014/FULL-R014 and COEFF-LOCALITY-X V001–V004 change coefficient residency, preload timing, or UB/tile budgeting; R022/FULL-R022 is the broad dtype×D codegen axis; R029 is outer mode dispatch. COEFF V004 specifically widened preload eligibility and retained a small residual confounded by control bias.
- NEW_DIFFERENCE: the existing V011 cached/streamed coefficient choice remains identical; only the loop-invariant branch is moved outside its per-tile body. No generated dtype×D variants or new V011 path selection.
- Duplicate analysis: distinct from HBH-02's row-prefetch terminal guard and HBH-03's tile-tail validity. Not `TILECOUNT-STATIC-UNROLL` if the tile loop remains runtime-counted and offsets remain dynamic. Any fixed tile-count specialization, loop-branch removal, or static offset propagation is `DUPLICATE_REJECTED` and belongs to `TILECOUNT-STATIC-UNROLL`.
- Related old routes: R014/FULL-R014, COEFF-LOCALITY-X V001–V004, R022/FULL-R022, R029/FULL-R029, R028/FULL-R028, and R31B V015 (an added predicate with no distinct operation was already judged a no-op).
- Minimal experiment: one OFAT source change in the generic output loop, hoisting only `cacheParams`/`cacheParamFp32` selection. Use `D=6144` FP32 with `localRows=1` and `localRows>1` plus a BF16 `localRows>1` probe. The diff must preserve each existing call, event, buffer, conversion, and math operation.
- Expected evidence: compiler/control-flow output proves repeated mode tests disappeared (otherwise stop as no-op); exact-source identity; correctness across both mode states; then same-binary-qualified, interleaved Parent/Candidate device-event samples and code-size/register/UB report. `FOCUS_AXIS=BRANCH_KIND`.
- Maturity: `NEEDS_MORE_EVIDENCE`.

### HBH-02 — Generic next-row prefetch terminal guard

- ID: `HBH-02`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- FOCUS_AXIS: `BRANCH_KIND`
- Mechanism: in the generic cached-row path, separate the existing non-terminal rows from the final owned row so the `row + 1 < beginRow + localRows` test around next-row x/residual prefetch is not repeated in the row body. Keep the prefetch at its current position after pass one and before scalar normalization; leave the tile loop and `hasPrefetchedFirstTile` logic untouched.
- Bottleneck: one loop-tail comparison and conditional next-row prefetch decision per owned row in the generic cached path (source line 342).
- Targets: generic `D=6144`, FP32/FP16/BF16, `localRows>=2`; use `localRows=1` as a control. For `D=8192`, include only cases that already enter the generic function under V011 dispatch.
- Why it may help: most rows in a multi-row core have a next row, while only the final row does not. Moving the terminal distinction to the row-loop boundary could remove the in-body tail decision without changing the prefetch itself.
- Why it may fail: this saves at most one branch per row, duplicates a large row body or adds an equivalent loop-bound check, and may increase code size with no reduction in DMA latency. If the moved prefetch changes overlap with the reduction tail, the proposal is invalid.
- UB/DMA/sync/precision impact: same x/residual buffers, same one-row lookahead, same `SyncMTE2ToV`/`SyncVToMTE2` sequence, and same valid lengths. No precision or arithmetic effect is intended; no new DMA or synchronization mechanism is allowed.
- OLD_MECHANISM: ASYNC-OVERLAP-CHAMPION-X V002 reorders next-row issue and changes MTE3 wait placement; V011 already has its row prefetch. R015/FULL-R015 and BATCH-RESIDENT change multi-row DMA/ownership.
- NEW_DIFFERENCE: relocate only the final-row control boundary; preserve V011's exact next-row loads and their position relative to reduction, with no sync/dataflow change.
- Duplicate analysis: distinct from HBH-01 coefficient-source selection and HBH-03 per-tile tail validity. This is a dynamic row-count tail, not fixed tile-count specialization. The `col==0` first-tile load omission is deliberately not included here because peeling it would propagate a static tile offset and is `DUPLICATE_REJECTED` for `TILECOUNT-STATIC-UNROLL`.
- Related old routes: R002/FULL-R002, R013/FULL-R013, R015/FULL-R015, BATCH-RESIDENT-X, ASYNC-OVERLAP-CHAMPION-X V002, R31B V003/V006/V009/V011.
- Minimal experiment: one OFAT generic-loop split with `D=6144` and `localRows=2+`, plus a `localRows=1` control. Preserve the exact per-row prefetch issue point and exact tile body; diff the emitted event/load order before considering correctness or timing.
- Expected evidence: exact source/assembly identity and unchanged ordered DMA/event trace; correctness for terminal and non-terminal rows; only then same-binary-qualified interleaved timing plus code-size cost. `FOCUS_AXIS=BRANCH_KIND`.
- Maturity: `NEEDS_MORE_EVIDENCE`.

### HBH-03 — Per-row runtime tail-length hoist (no loop split)

- ID: `HBH-03`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- FOCUS_AXIS: `BRANCH_KIND`
- Mechanism: in the active `ProcessWideLowPrecision()` flattened row/tile stream, compute the runtime final-tile valid length once from unchanged `rowWidth`/`tileWidth`, then derive each unit's existing `valid` value from that length and the already-decoded `tile`. Keep the flattened loop, unit order/count, `tileCount`, ping-pong parity, and per-unit operation sequence intact; do not split it into full-tile and tail loops.
- Bottleneck: repeated per-unit tail-length min/subtraction in both V011 wide low-precision passes (`ProcessWideLowPrecision`), where most tiles are full and only the final tile may be short. This narrowed candidate does not claim to remove or relocate the per-tile tail predicate.
- Targets: FP16/BF16 wide rows `D=8192, 16384, 32768` as full-tile controls plus a diagnostic `[2,12289]` FP16/BF16 tail case when the unchanged V011 `tileWidth` makes `D % tileWidth != 0`. If 12289 is not in the accepted test domain, choose another existing-domain width `D=n*tileWidth+r`, `0<r<tileWidth`; do not change `tileWidth` to manufacture a hit.
- Why it may help: a precomputed row-tail length may avoid repeated min/subtraction while preserving the existing per-tile branch and byte counts.
- Why it may fail: the compiler may already hoist or simplify the calculation; retaining the tail predicate may leave little scalar work to save, and the wide-row DMA/V pipeline may dominate. If generated code does not remove repeated length arithmetic, classify this as a no-op.
- UB/DMA/sync/precision impact: same valid byte count, same DataCopyPad, same `rd0/rd1` and `rel0/rel1` event protocol, same FP16/BF16 conversion and reduction order. No UB or buffer change; tail must remain masked/padded exactly as V011.
- OLD_MECHANISM: V011 already computes per-unit `valid` in the active row/tile pipeline; FULL-R009/PURE-R009 and ALIGN-TAIL-X are tail-copy/copy-semantics families. WIDE-X-FRESH4 changes tile width and is explicitly not this proposal. ADDR-H1..H3 preserve V011's valid lengths while changing (or, for H3, proposing to change) address/index expressions.
- NEW_DIFFERENCE: hoist only the runtime final-tile valid-length calculation within the existing V011 flattened loop. Preserve the decoded `u`, `batchRow`, `tile`, `rowOffset`, `col`, every GM address expression, and the existing per-unit tail predicate/loop structure; no generated D bucket, static offset propagation, or changed dispatch domain.
- Duplicate analysis: the valid-length code surface is adjacent to ADDR-H1..H3's explicitly preserved invariant, but those candidates do not optimize valid/tail length. Keep this candidate limited to computing the same runtime final-tile length once and deriving exactly the old per-unit values from the existing `tile`; do not alter address/index expressions or loop structure. Static unrolling, removing the tile counter/loop branch because trip count is fixed, or propagating constant tile offsets is `DUPLICATE_REJECTED` and owned by `TILECOUNT-STATIC-UNROLL`. A dtype-specific dispatch or new tail fastpath is also out of scope.
- Related old routes: R009/R010 and ALIGN-TAIL-X address copy/tail semantics; WIDE-X-FRESH4 changes tile width; R022 addresses generated dtype×D buckets; R029 addresses dispatch modes. Those are not evidence that this narrowed calculation-hoist has been tested.
- Minimal experiment: one FP16 wide V011-equivalent source change with an unchanged `tileWidth`, a full-tile control (`D=8192` or `16384`), and one tail-bearing width satisfying the predicate above. Compare per-unit valid values, address arguments, and ordered DMA/event operations; reject if the flattened loop or per-tile predicate changes or if codegen shows no repeated length arithmetic removed.
- Expected evidence: source/assembly comparison proves unchanged loop/address/control structure and exact valid lengths, then tail/full correctness including max error, followed only then by shape-qualified interleaved timing. Report any code-size or register/UB change even if performance is neutral. `FOCUS_AXIS=BRANCH_KIND`.
- Maturity: `NEEDS_MORE_EVIDENCE`.

### HBH-04 — First-tile load predicate peel (rejected)

- ID: `HBH-04`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- FOCUS_AXIS: `BRANCH_KIND` (proposed)
- Mechanism: peel the first generic tile so `col != 0 || !hasPrefetchedFirstTile` is not evaluated in the tile loop.
- Bottleneck / target: first-tile load control for cached multi-row rows (`D=6144/8192`, FP32/FP16/BF16, `localRows>1`).
- Why it may help / fail: could eliminate a per-tile load decision; but the optimization relies on `col=0` and first-tile offset propagation, duplicates the prologue, and risks the existing prefetch invariant.
- UB/DMA/sync/precision impacts: no UB change intended; a peel changes DMA issue structure and has a stale-buffer risk. Sync/event order must remain byte-for-byte equivalent, and arithmetic/rounding order must remain unchanged; no benefit is admissible if any load/event ordering or precision behavior changes.
- OLD_MECHANISM: V011's `hasPrefetchedFirstTile` conditional load.
- NEW_DIFFERENCE: none after ownership check; the proposed gain is specifically static first-tile offset propagation.
- Duplicate analysis: `DUPLICATE_REJECTED` — static offset propagation/loop peeling is explicitly owned by `TILECOUNT-STATIC-UNROLL`; do not implement under this route.
- Related old routes: R005/FULL-R005 and WIDE-X-FRESH4 tile-iteration changes; R013/R31B V011 pipeline state.
- Minimal experiment: none under this route; refer to `TILECOUNT-STATIC-UNROLL`.
- Expected evidence: no route experiment. `FOCUS_AXIS=BRANCH_KIND`; rejection itself is the expected evidence.

### HBH-05 — Fixed tile-count/parity branch specialization (rejected)

- ID: `HBH-05`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- FOCUS_AXIS: `BRANCH_KIND` (proposed)
- Mechanism: specialize `tileIndex + 1 < tileCount`, `u + 1 < units`, or `useA/nextA` into fixed-trip unrolled bodies in the wide pipeline.
- Bottleneck / target: repeated counter/parity control in FP16/BF16 wide rows `D=8192–32768`.
- Why it may help / fail: could reduce counter/parity branches, but changes exactly the static-trip/offset/pipeline-control axis owned elsewhere; code growth and changed issue ordering may erase any gain.
- UB/DMA/sync/precision impacts: any useful version would risk changing buffer-slot selection, DMA issue order, and event release/wait placement; arithmetic and rounding must also remain unchanged. It is not feasible under this route's freeze.
- OLD_MECHANISM: V011's dynamic tile counter and alternating 2-deep DMA/event protocol.
- NEW_DIFFERENCE: none within this ownership boundary.
- Duplicate analysis: `DUPLICATE_REJECTED` — fixed-trip unrolling, removal of tile counter/loop branch, and static offset propagation are owned by `TILECOUNT-STATIC-UNROLL`.
- Related old routes: R005/FULL-R005, WIDE-X-FRESH4 (tile width), R013/FULL-R013 and R31B V006/V009/V011 (pipeline depth/schedule).
- Minimal experiment: none under this route; point to `TILECOUNT-STATIC-UNROLL`.
- Expected evidence: no route experiment. `FOCUS_AXIS=BRANCH_KIND`; rejection itself is the expected evidence.

## Cross-route duplicate audit — HOTLOOP-ADDR-HOIST-CHAMPION-X

Compared with the committed ADDR handoff at `3c65f7e053f79bf8b4d87a1b2d8de43fba4d7889` (`研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md`): ADDR-H1 hoists a row base, ADDR-H2 uses a rolling input-address cursor, and ADDR-H3 replaces flattened index decoding with stateful counters. All three explicitly preserve per-tile valid lengths and DMA/event order.

| Axis | Cross-route result |
|---|---|
| (a) Address/index expressions | `OVERLAP=NO`. HBH-01 selects the existing coefficient mode; HBH-02 concerns the generic row-prefetch terminal guard. HBH-03 is narrowed to keep `u`, `batchRow`, `tile`, `rowOffset`, `col`, all GM offsets, and flattened-loop structure unchanged. Thus no duplicate of ADDR-H1/H2/H3. |
| (b) Row-tail/prefetch branch | `OVERLAP=NO`. HBH-02 does address the generic `row + 1 < end` prefetch guard, but ADDR-H1..H3 do not change this guard or prefetch schedule. |
| (c) Per-tile valid/tail length | `CODE-SURFACE OVERLAP=YES; MECHANISM DUPLICATE=NO`. HBH-03 touches this code surface, while ADDR-H1..H3 freeze valid lengths rather than optimize them. HBH-03 is bounded to hoisting the runtime tail-length calculation and deriving identical per-unit `valid` values from the existing decoded tile; it must not change the address/index expressions, tail predicate, loop shape, DMA bytes, or operation order. |

OLD_MECHANISM versus NEW_DIFFERENCE: ADDR-H1..H3 propose row-base, input-cursor, and index-decode transforms respectively; the surviving HBH candidates target coefficient-mode placement, a row-prefetch terminal guard, and (HBH-03 only) repeated tail-length arithmetic. The shared V011 parent and preserved valid-length invariant do not make these mechanisms duplicates. `MAIN_SELECTED=NONE` remains unchanged.

## Explicit duplicate/rejection audit

| Candidate class | Outcome | Reason |
|---|---|---|
| Fixed-trip tile-count specialization, static unroll, removing the tile counter/loop branch, or static offset propagation | `DUPLICATE_REJECTED` | HBH-04/HBH-05; owned by `TILECOUNT-STATIC-UNROLL`; explicitly excluded from this route. |
| Dtype-specific generated bodies or dtype×D code generation | `DUPLICATE_REJECTED` | R022/FULL-R022 is the old codegen axis; V011's dtype choice is `if constexpr`, not a repeated runtime branch. |
| New path selection, dispatch-domain expansion, R029 five-mode taxonomy, selective fastpath, or donor replacement | `DUPLICATE_REJECTED` | Violates the frozen V011 algorithm/donor-selection boundary; R029 is an outer dispatch axis, not internal branch hoisting. |
| New DMA, tile-width, buffer, sync, reduction, or math change disguised as branch cleanup | `INFEASIBLE_FOR_THIS_ROUTE` | Violates the task's Track-B and frozen-parent constraints. |
| V011 `ProcessWideFp32PanelResident()` full-panel/alignment fallback | `NOT_A_HOTLOOP_CANDIDATE` | The helper has no caller; active FP32 wide dispatch goes through `ProcessWideFp32FullCacheRows()`. |
| Parity-based replacement of V011's `useA`/`nextA` event-slot control by alternating hand-written bodies | `DUPLICATE_REJECTED` | Fixed alternating bodies are loop unrolling/control removal; refer to `TILECOUNT-STATIC-UNROLL`. |

Thus the pool has three independent surviving hypotheses (`HBH-01`..`HBH-03`); `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO`. No candidate is selected: `MAIN_SELECTED=NONE`.

## Handoff decision

This is a research handoff only. No Revision, build, correctness, timing, profiling, server3 work, Online action, shared schedule edit, or kernel/source/build-file edit was performed. Planning/Review must choose at most one surviving hypothesis before any Track-A work.
