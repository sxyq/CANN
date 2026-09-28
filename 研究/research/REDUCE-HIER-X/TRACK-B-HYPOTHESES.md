# TRACK-B HYPOTHESES — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/REDUCE-HIER-X
BRANCH=exp/main2-r2-reduce-hier
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16 (OFFICIAL_ANCHOR)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
FROZEN_SEED=phase4/workspaces/REDUCE-HIER-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
MODE=TRACK-B research only — no kernel edit until Main approves exactly one hypothesis

## Scope of this document

Study ONLY the RMS square-sum reduction topology on the frozen R31B-V011
baseline. Every hypothesis below is a single reduction-topology variable on one
of the allowed axes:

- FP32 partial accumulation
- hierarchical reduction
- vector reduction organization
- partial-sum lifetime
- reduction workspace organization

Target: lower RMS reduction latency and UB traffic with identical mathematical
semantics (same FP32 intermediate semantics, same result tolerance).

Out of scope by route policy (do not appear as variables here): dtype
specialization, row scheduling, multi-row DMA, wide specialization, sync
removal, gamma/bias caching, epilogue arithmetic, mode selection / rows-per-block.

## Reduction sites in the frozen seed

Concrete sites each hypothesis may touch (study only; no edit this turn):

| Site | Location in frozen seed | Current shape |
|---|---|---|
| S1 generic multi-tile first pass | `Process()` tile loop, ~lines 299–357 | per-tile `ReduceSum(reduceFp32[tileIndex], xFp32, residualFp32, valid)` then one collapse `ReduceSum(scalarLocal, reduceFp32, xFp32, tileCount)` + `GetValue` |
| S2 wide FP32 full-Y first pass | `ProcessWideFp32FullCacheRows`, ~lines 2113–2155 | per-tile `ReduceSum(reduceLocal[batchRow * kWideFullYReduceStride + tile], …)` then per-row collapse `ReduceSum(…, tileCount)` + `GetValue` |
| S3 full-row pipelined | `ProcessFp32FullRowOutputPipelined`, ~lines 916–946 and 1035–1064 | same two-stage: per-tile partial at `reduceLocal[col / kTileElems]`, then `ReduceSum(residualFp32, reduceLocal, xFp32, tileCount)` |
| S4 small FP32 batched / contiguous | `ProcessSmallFp32Batched`, `ProcessSmallFp32ContiguousBatched`, ~lines 1394–1641 | one `ReduceSum` per row into `reduceLocal[batchRow * kSmallFp32ScalarStride]`; no cross-tile collapse |
| S5 mid single-tile | `ProcessNarrowMidOverlap`, ~lines 533–573 | one `ReduceSum(reduceLocal, xFp32, residualFp32, valid)` + `GetValue`; no multi-tile partial bank |

Cross-tile partial bank + end-of-row collapse exists only where
`tileCount = ceil(rowWidth / kTileElems) >= 2` with `kTileElems = 4096`
(D > 4096 on S1/S2/S3). Single-tile D bands (S4/S5) have no collapse stage and
are controls, not win shapes, for H1–H3.

Buffer facts used by the hypotheses:

- `reduceFp32Buf_` is `InitBuffer`'d at `kTileElems * sizeof(float)` = 16 KB on
  the narrow/generic paths (`~line 149`) and at
  `wideFullYRows_ * kWideFullYReduceStride * sizeof(float)` (≤ 8×16×4 B) on
  wide (`~line 79`). Live partial slots on multi-tile paths are only
  `tileCount` (≤ 8 for legal D ≤ 32768) per row, or
  `rows * kSmallFp32ScalarStride` / `rows * kWideFullYReduceStride` when batched.
- `ReduceSum(dst, src, work, count)` always uses `xFp32Buf_` or
  `residualFp32Buf_` as its `work` argument; `reduceFp32Buf_` is only the
  partial bank / destination. Each `ReduceSum` call pays its internal V/S
  handoff (comment at `~line 331`).
- Scalar handoff of the RMS denominator is one `GetValue(0)` after the collapse
  (S1 `~line 359`, S2 `~line 2147`). Batched paths already group `GetValue`
  under one `SyncVToS` (S4). Number of scalar reads is not a variable in this
  document (sync-removal is out of scope).

---

## HYPOTHESIS-1 — Eager running-fold of tile partials (partial-sum lifetime)

- **MECHANISM**
  Change only WHEN the per-tile FP32 square-sum partial is merged into the row
  square-sum. Today every tile writes its partial into the
  `reduceFp32Buf_` bank (`reduceFp32[tileIndex]` / `reduceLocal[… + tile]`) and
  one end-of-row `ReduceSum(…, tileCount)` collapses the bank. After H1 each
  tile still runs the same per-tile `ReduceSum` into a 1-element temp slot, but
  that partial is immediately folded into a single FP32 accumulator slot with a
  1-element `Adds(acc, acc, tmp, 1)`. The partial bank is never filled. The
  end-of-row collapse `ReduceSum` disappears; the existing single `GetValue`
  reads the accumulator. Per-tile `ReduceSum` width, square `Mul`, mean/epsilon
  tail, and output pass are untouched.

- **EXPECTED_BOTTLENECK**
  The end-of-row collapse call: one `ReduceSum` over `tileCount` elements
  (2–8) that still pays the API’s fixed internal V/S handoff for almost no
  vector work. Second-order: partial-bank write traffic and keeping `tileCount`
  slots live until the collapse.

- **FILES/FUNCTIONS TO TOUCH**
  - `Process()` first-pass loop and its following collapse (`~lines 299–366`)
  - `ProcessWideFp32FullCacheRows` per-row tile loop and per-row collapse
    (`~lines 2113–2155`)
  - `ProcessFp32FullRowOutputPipelined` tile-partial + collapse
    (`~lines 916–946`, `~lines 1035–1064`)
  - `InitBuffer(reduceFp32Buf_, …)` may shrink to `tileCount + 1` slots
    (`~line 149`) — same variable (partial lifetime / live set), not a second
    mechanism. Do not touch S4/S5 (no collapse stage there).
  Frozen-seed path only; no host / CMake / runner.

- **WHY_ORTHOGONAL_TO_MAIN1**
  MAIN-1 worktrees under `/Users/sunyiyang/Desktop/Project/cann-sixlane/` are
  read/write forbidden and are not a parent or donor. This change is confined
  to RMS square-sum merge timing inside one Vector-Core kernel on the R31B-V011
  parent. It does not touch row scheduling, core ownership, multi-row DMA,
  wide specialization, dtype splits, gamma/bias residency, epilogue math, or
  any MAIN-1 lane mechanism named in the route brief’s forbidden list.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - `REDUCE-INVSCALE-X` H3 widens the per-tile `ReduceSum` span (fewer, larger
    partials) and keeps a deferred collapse; H1 keeps per-tile width and removes
    the deferred collapse by eager fold. H4 of that route collapses to a single
    whole-row `ReduceSum` for UB-fitting rows (still one API call over all
    squares); H1 never holds all squares and never issues a whole-row call.
    That route’s H1/H2 are reciprocal-tail / output-load scheduling and are
    outside this route’s reduction-topology scope.
  - `ASYNC-TRIPLE-X` owns MTE3 / triple overlap; `ALIGN-TAIL-X` owns
    DataCopy/DataCopyPad bulk+tail; `SCHED-ROWGROUP-X` owns core row ownership;
    `BATCH-RESIDENT-X` owns multi-row DMA; `UB-LIVENESS-X` owns buffer
    aliasing; `EPILOGUE-FUSE-X` owns epilogue fusion. None change when tile
    partials die.
  - Idea-pool R006 “hierarchical UB tree” is a multi-level merge shape (see
    H3), not a streaming fold. R011 is a manual vector tree (see H2).

- **EXPECTED_WIN_SHAPES**
  Multi-tile FP32 rows where the collapse is paid and `tileCount` is small
  (fold cost `tileCount-1` cheap Adds is less than one short `ReduceSum`):
  - D=8192 (`tileCount=2`) — strongest expected share
  - D=6144 / 5120 / 4097–8191 generic (`tileCount=2`)
  - D=12288 (`tileCount=3`)
  Wide full-Y D=8192+ and full-row D=8192 paths exercise S2/S3.
  Expected direction: lower reduction-tail latency; UB partial-bank traffic
  down to one accumulator slot.

- **EXPECTED_RISK_SHAPES**
  - Single-tile bands (S4/S5, D≤4096): no collapse today → no signal; must not
    be used as primary evidence (same lesson as REDUCE-INVSCALE probe shape).
  - Large `tileCount` (D=32768, 8 tiles): 7 sequential 1-element Adds may cost
    more than one `ReduceSum` over 8 floats; win may reverse to neutral/negative
    at the widest legal D.
  - Shapes where the second pass already dominates: reduction-tail share small
    → overall delta inside noise.

- **CORRECTNESS_RISK**
  Low. Arithmetic stays FP32. Only the association of the same summands changes
  (sequential left fold vs the collapse call’s internal tree). Golden is a
  double-precision reference under the route’s FP32 tolerance; per-tile
  `ReduceSum` results are unchanged. No change to
  `meanSquare = squareSum * invRowWidth + epsilon` or to `invRms`. Residual
  risk: forgetting the accumulator’s initial zeroing, or aliasing `tmp`/`acc`
  with the `ReduceSum` work buffer (`xFp32`/`residualFp32`).

- **MEASUREMENT_PLAN**
  1. Declaration block before any edit (ROUTE / REVISION / DIRECT_PARENT /
     PARENT_SOURCE_SHA / PARENT_SCORE=45.16 / SINGLE_HYPOTHESIS /
     CONTEXT_CLASS / WHY_NOT_DUPLICATE).
  2. Build + link on server3 with exact source; NPU correctness on the route’s
     existing shape matrix (FP32/FP16/BF16, rows 1 and multi, D including
     4096 / 6144 / 8192 / 16384 / 32768, aligned and unaligned D).
  3. Same-binary qualification of the Direct Parent at each probe shape under
     `local-timing-protocol.md` (device events primary, warmup ≥ 45, ≥ 11–21
     in-process samples, MAD/median ≤ 0.10 and block drift ≤ 0.10). Shaped
     probes: FP32 rows=4 D=8192, rows=2 D=16384, rows=1 D=32768, plus a
     single-tile control rows=4 D=4096.
  4. Interleaved P/C pairs (≥ 4) on a leased device from
     `server3-device-leases.tsv`, one Route per device, raw samples retained.
  5. Primary stats only: per-block median of `DEVICE_EVENT_US`, MAD/median,
     p10/p90, paired block delta. Single-variable audit: only merge timing and
     the matching partial-slot footprint change.

---

## HYPOTHESIS-2 — Manual in-tile vector tree replacing per-tile ReduceSum (R011)

- **MECHANISM**
  Change only HOW each tile’s square vector becomes one FP32 partial. Keep the
  per-tile `Mul` square and the end-of-row collapse. Replace the per-tile
  `AscendC::ReduceSum(reduceFp32[tileIndex], xFp32, residualFp32, valid)` with
  an explicit UB vector reduction tree over the `valid` squares (pairwise
  `Adds` halving, or a fixed fan-in tree) that writes the same 1-element FP32
  partial and performs 0 intermediate `GetValue`. The collapse stage still runs
  one `ReduceSum` over the partial bank (unchanged topology). One conceptual
  variable: the in-tile reduction primitive / tree, not the number of stages.

- **EXPECTED_BOTTLENECK**
  Per-tile `ReduceSum` internal V/S handoff, paid `tileCount` times per row on
  multi-tile rows and once per row on single-tile rows. For D=4096–8192 the
  handoff may be a large share of the reduction pass; for very wide tiles the
  vector work itself dominates and the handoff share shrinks.

- **FILES/FUNCTIONS TO TOUCH**
  - All sites that issue a long per-tile `ReduceSum`: S1 `~line 334`,
    S2 `~line 2132`, S3 `~lines 932 / 1050 / 1177`, S5 `~line 560`
  - Optionally S4 per-row `ReduceSum` (`~lines 1414 / 1612`) only if the same
    helper is shared; prefer to land S1 first and keep S4 as control.
  - A small local helper for the tree + its work slots in `reduceFp32Buf_` /
    a dedicated work area. No host / CMake / runner.

- **WHY_ORTHOGONAL_TO_MAIN1**
  Pure in-kernel vector reduction organization on the R31B-V011 parent. No
  scheduling, DMA layout, dtype, param residency, or epilogue change. MAIN-1
  `cann-sixlane` stays untouched and is not a parent.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - Distinct from H1: H1 changes when partials merge (lifetime); H2 keeps the
    deferred partial bank and changes only the long per-tile reduction
    primitive.
  - Distinct from H3: H3 reshapes the short end-of-row merge tree; H2 does not
    touch that merge.
  - Distinct from `REDUCE-INVSCALE-X` H3: that hypothesis keeps the `ReduceSum`
    API and widens its span; H2 replaces the API call with a manual tree at the
    same span.
  - Distinct from `REDUCE-INVSCALE-X` H4: that uses one whole-row `ReduceSum`
    for fitting rows; H2 never changes stage count.
  - Donor R011 (“manual vector reduce”, full vector tree with 0–1 GetValue) is
    idea-pool owner REDUCE-X and still unexplored on this parent; historical
    R011 attempts used GetValue-heavy trees and correctness-failed — this
    variant is 0-intermediate-GetValue and is a new attempt, not a restore of
    old sources.

- **EXPECTED_WIN_SHAPES**
  Multi-tile and single-tile rows where per-tile `ReduceSum` handoff is exposed:
  - D=4096 / 5120 / 6144 FP32 (one or two long reduces per row)
  - D=8192 (two reduces of 4096)
  Small D with many rows (S4 batched) only if the helper is shared later;
  first landing should show the multi-tile/single-tile long-reduce band.

- **EXPECTED_RISK_SHAPES**
  - Very wide D (32768) where the 4096-element vector work dominates: handoff
    share small → delta inside noise.
  - Shapes whose correctness depends on `ReduceSum`’s exact work-buffer
    contract; a manual tree must not assume `residualFp32` is free at that
    point.
  - Any shape where the tree indexing mishandles `valid` not a power of two
    (unaligned D tails).

- **CORRECTNESS_RISK**
  Moderate–high. Idea-pool record: prior R011-style manual trees
  correctness-failed. Specific hazards: odd or non-power-of-two `valid` tails,
  tree level indexing, UB work-area aliasing with `xFp32`/`residualFp32`, and
  FP32 association drift versus `ReduceSum`’s internal tree (tolerance should
  still pass if the golden is double-based, but must be measured, not assumed).
  Mitigation: implement a single helper, prove it on one shape before widening
  the landing, and keep `ReduceSum` as a fallback path in the same revision only
  if that does not add a second performance variable (it may not — see OFAT
  note below). Cleanest OFAT: helper replaces the call at one site family only.

- **MEASUREMENT_PLAN**
  Same protocol as H1 steps 1–5. Probes must include at least one single-tile
  long-reduce shape (D=4096 or 6144) and one multi-tile shape (D=8192 or
  16384) so the per-tile handoff share is visible. Correctness gate first and
  stricter than H1 (full 16-shape matrix before any timing). Falsification: if
  in-tile manual tree is not faster than `ReduceSum` at D=4096 on the same
  binary pair, the handoff hypothesis is wrong for this toolchain and H2 is
  parked.

---

## HYPOTHESIS-3 — Hierarchical two-level tile-partial merge (R006 hierarchical UB tree)

- **MECHANISM**
  Change only the SHAPE of the end-of-row merge. Keep per-tile `ReduceSum`
  partials in the bank (unchanged lifetime and per-tile width). Replace the
  single flat `ReduceSum(…, tileCount)` collapse with an explicit two-level
  hierarchical tree in UB: partition the `tileCount` partials into
  `ceil(tileCount / G)` groups of fan-in G (G=2 or 4), merge each group to a
  level-1 partial with a short `ReduceSum` or pairwise `Adds`, then one final
  merge over the level-1 partials. For `tileCount ≤ G` this degenerates to the
  parent’s flat merge (variable does not fire). One conceptual variable: merge
  tree depth / fan-in.

- **EXPECTED_BOTTLENECK**
  The flat collapse’s shape when `tileCount` is large enough that a single
  short `ReduceSum` is a poor fit for the partial vector (its internal tree and
  V/S handoff versus a pure-vector pairwise level). Secondary: partial-bank
  read locality when level-1 slots are packed next to their group.

- **FILES/FUNCTIONS TO TOUCH**
  - Collapse block in `Process()` (`~lines 354–359`)
  - Collapse block in `ProcessWideFp32FullCacheRows` (`~lines 2139–2155`)
  - Collapse block in `ProcessFp32FullRowOutputPipelined`
    (`~lines 943–946`, `~lines 1061–1064`, `~lines 1190–1193`)
  - `reduceFp32Buf_` layout for level-1 slots (same variable). No host / CMake.

- **WHY_ORTHOGONAL_TO_MAIN1**
  Merge-tree shape only, inside this route’s kernel on R31B-V011. No MAIN-1
  worktree, no scheduling/DMA/dtype/epilogue coupling.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - Distinct from H1: H1 removes the deferred merge by eager fold; H3 keeps the
    deferred partial bank and only reshapes its merge.
  - Distinct from H2: H2 replaces the long per-tile `ReduceSum`; H3 leaves
    per-tile calls intact and changes only the short merge topology.
  - Distinct from `REDUCE-INVSCALE-X` H3/H4: those change per-call span or
    remove a stage; H3 adds an explicit intermediate merge level with fan-in G.
  - Donor R006 idea-pool entry “hierarchical UB tree” is still listed
    unexplored (historical evidence is H001 wide chunks, a different knob).
    This hypothesis is that unexplored tree, on the R31B parent.

- **EXPECTED_WIN_SHAPES**
  Only multi-tile rows with `tileCount > G`:
  - G=2: D=16384 (4 tiles) and D=32768 (8 tiles)
  - G=4: D=32768 (8 tiles)
  At D=8192 (`tileCount=2`) the hierarchy is a no-op versus parent.

- **EXPECTED_RISK_SHAPES**
  - D=8192 and below: no fire → no signal.
  - D=32768 with G=2: more short calls (4 group merges + 1 final) than the
    parent’s single flat call; extra V/S handoffs may make this slower. The
    hypothesis is that a pure-vector pairwise level avoids those handoffs; if
    the implementation still uses `ReduceSum` per group it is likely a loss.
  - Any shape where level-1 slots overwrite live partials (indexing).

- **CORRECTNESS_RISK**
  Low–moderate. Same summands and FP32; association changes with tree shape
  (within tolerance if golden is double-based). Concrete hazards: odd
  `tileCount` (D not a multiple of 4096), group boundaries, and level-1 slot
  aliasing with the per-tile partial bank.

- **MEASUREMENT_PLAN**
  Same protocol as H1 steps 1–5, but probes must include D=16384 and D=32768
  (where the variable fires) and a D=8192 negative control (variable does not
  fire; expect delta ≈ 0). Falsification: if hierarchical merge is not faster
  than flat merge at D=32768 on the paired device-event runs, park H3 and keep
  the flat collapse. Implementation note for OFAT: prefer a pure-vector
  pairwise level (no `ReduceSum` per group) so the only variable is tree shape,
  not primitive mix; if `ReduceSum` must be used per group, record that the
  primitive did not change and only fan-in/depth did.

---

## HYPOTHESIS-4 — Right-sized dense partial workspace (reduction workspace organization)

- **MECHANISM**
  Change only the reduction workspace footprint and layout. Today
  `reduceFp32Buf_` is `InitBuffer`'d at `kTileElems * sizeof(float)` = 16 KB on
  narrow/generic paths (`~line 149`) while live partial slots are only
  `tileCount` (≤ 8) per row on multi-tile paths, or
  `rows * kSmallFp32ScalarStride` / `rows * kWideFullYReduceStride` on batched
  paths. H4 right-sizes that buffer to the live slot count plus one 1-element
  accumulator/temp (e.g. `tileCount + 1` or a small constant 16), packs
  partials densely (drop unused stride holes where the collapse source is
  already a contiguous run of `tileCount`), and keeps the `ReduceSum` work
  argument on `xFp32Buf_` / `residualFp32Buf_` as today. No change to number
  of `ReduceSum` calls, merge timing, or tree shape. The freed UB is not reused
  by any second mechanism in this revision.

- **EXPECTED_BOTTLENECK**
  UB footprint and partial-bank traffic: a 16 KB allocation for ≤ 32 B of live
  partials, and stride padding (`kSmallFp32ScalarStride = 8`,
  `kWideFullYReduceStride = 16`) that separates rows when the collapse reads
  only `tileCount` contiguous elements per row. Possible bank/locality effects
  on the collapse read and on overall UB pressure next to `valueFp32Buf_`
  (`kCacheElems * 4` B) and the x/residual tiles.

- **FILES/FUNCTIONS TO TOUCH**
  - `InitBuffer(reduceFp32Buf_, …)` in `Init` (`~lines 79–80`, `~line 149`)
  - Partial-bank indexing in S1 (`reduceFp32[tileIndex]`), S2
    (`reduceLocal[batchRow * kWideFullYReduceStride + tile]`), S3
    (`reduceLocal[col / kTileElems]`), S4
    (`reduceLocal[batchRow * kSmallFp32ScalarStride]`)
  - Collapse source length already `tileCount` / per-row span — unchanged.
  No host / CMake / runner. Wide `ChooseWideFullYRows` budget math may need a
  matching reduce-bytes update (`~lines 1297–1322`) — still the same variable
  (workspace bytes).

- **WHY_ORTHOGONAL_TO_MAIN1**
  Buffer sizing and layout inside this kernel only. No MAIN-1 worktree, no
  scheduling/DMA/dtype/epilogue mechanism.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - Distinct from H1–H3: those change merge timing, primitive, or tree shape;
    H4 keeps the two-stage `ReduceSum` topology bit-identical and only changes
    allocation size and slot indexing.
  - Distinct from `UB-LIVENESS-X`: that route’s scope is buffer aliasing /
    liveness across phases (reuse of the same bytes for different roles). H4
    does not alias buffers into new roles; it only shrinks and densifies the
    partial bank that already exists for reduction.
  - Distinct from `REDUCE-INVSCALE-X` H3: that grows the reduce span and its
    buffers; H4 shrinks the partial bank without changing span.

- **EXPECTED_WIN_SHAPES**
  Uncertain by construction (organization-only). Candidates where UB pressure
  and partial layout are most visible:
  - Wide FP32 full-Y (D > 8192, several rows per core) — large
    `valueFp32Buf_` plus reduce bank compete for UB
  - Generic multi-tile D=8192–32768 where the 16 KB reduce bank sits next to
    `valueFp32Buf_` (8192 floats = 32 KB) on the same pipe
  Honest prior: many shapes may show delta ≈ 0; this is a layout/footprint
  hypothesis whose value is partly as a precondition for later UB-using ideas
  (not combined here).

- **EXPECTED_RISK_SHAPES**
  - Any path that silently relies on `reduceFp32Buf_` having spare slots beyond
    the packed partials (audit all `reduceFp32Buf_.Get` / `reduceLocal[…]`
    uses before editing).
  - Batched paths with stride-based scalar slots (S4): densifying must not
    change the `GetValue` offsets without updating every reader.
  - Wide `ChooseWideFullYRows` row-count math: reducing reduce-bytes could
    change how many rows fit, which would look like a row-occupancy change —
    that would break single-variable scope and must not happen in this
    revision (keep `wideFullYRows_` decision inputs unchanged, or defer the
    budget-line touch to a later revision if it would move row count).

- **CORRECTNESS_RISK**
  Low if only sizes/indices change and every reader is updated together.
  Highest risk is an index mismatch on batched stride slots (`kSmallFp32ScalarStride`
  / `kWideFullYReduceStride`) used both as partial dest and as the later
  `GetValue` / `Duplicate` / `Sqrt` slot. Must re-run the full correctness
  matrix; a shrunk buffer that overruns is an immediate functional failure.

- **MEASUREMENT_PLAN**
  Same protocol as H1 steps 1–5. Expectations are weak, so the bar is: (a) full
  correctness PASS, (b) paired delta on at least one wide FP32 multi-row shape
  and one generic multi-tile shape, (c) if |paired delta| ≤ same-binary noise
  floor on every probe, record `NEEDS_ONE_MORE_LOCAL` / not a win and keep the
  hypothesis as layout cleanup only. Do not promote on footprint change alone.

---

## Cross-hypothesis OFAT notes

- Each hypothesis is one conceptual variable. H1 may also shrink
  `reduceFp32Buf_` because the live set shrinks with the lifetime change; if
  the Main review treats footprint as a second mechanism, split H1 into
  (1a) eager fold with buffer size unchanged and (1b) size follow-up, and run
  1a first.
- H2 landing at one site family (S1 only) is a valid single change; widening
  to S2/S3/S5 is a later revision or an explicitly declared combination with
  `WHY_THIS_COMBINATION_IS_NEW`.
- H3’s variable fires only for `tileCount > G`; do not use D=8192 as its
  primary evidence.
- H4 must not change `wideFullYRows_` selection inputs in the same revision as
  the packing change, or `SINGLE_CHANGE_AUDIT` risks FAIL.

## Recommended first OFAT revision

**H1 — Eager running-fold of tile partials (partial-sum lifetime).**

Why first: smallest diff (one `Adds` fold per tile, delete one collapse
`ReduceSum`, one `GetValue` unchanged); lowest correctness risk (per-tile
`ReduceSum` results and the mean/epsilon/invRms tail are untouched); cleanest
single variable; measurable on multi-tile shapes that already exercise the
collapse (S1/S2/S3); expected win concentrated at `tileCount = 2–3`
(D≈6144–12288) where the removed call is a short `ReduceSum` with a full V/S
handoff. H2 is the highest-upside but highest-risk follow-on (R011 history);
H3 only fires at large `tileCount`; H4 is likely inside noise and is best kept
as a later layout revision.

---

## REQUEST_MAIN_APPROVAL

Requesting Main approval for exactly one first revision:

**HYPOTHESIS-1 — Eager running-fold of tile partials (partial-sum lifetime)**

- ROUTE: REDUCE-HIER-X
- REVISION: to be assigned at edit time (next after frozen R31B-V011 seed)
- DIRECT_PARENT: FROZEN_R31B_V011
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- PARENT_SCORE: 45.16 (OFFICIAL_ANCHOR)
- SINGLE_HYPOTHESIS: merge per-tile FP32 square-sum partials eagerly into one
  running accumulator; remove the end-of-row collapse `ReduceSum`; one
  `GetValue` of the accumulator; per-tile `ReduceSum` width, square, mean,
  invRms, and output pass unchanged.
- CONTEXT_CLASS: FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
- WHY_NOT_DUPLICATE: see H1 section (distinct from REDUCE-INVSCALE-X H3/H4
  span/stage changes and from H2/H3 primitive/tree variables)

No kernel, host, CMake, runner, shared-control, or CANNJudge action is taken
until this approval is issued.
