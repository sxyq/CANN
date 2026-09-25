# REDUCE-INVSCALE-X — Next-Hypothesis Research (TRACK-B)

Route agent: REDUCE-INVSCALE-X (MAIN-2). Worktree `cann-next6/REDUCE-INVSCALE-X`.
Mode: Long-Horizon Parallel Exploration (execution-contract §R). This file is append-target for
per-route research; no kernel edits, no `.asc`/`.cpp` changes, no device runs this turn.

---

## CURRENT_CANDIDATE

- Route **REDUCE-INVSCALE-X**, revision **V002**, `SOURCE_SHA256 = bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26`.
  Unchanged this turn (TRACK-A frozen: no kernel edits, no new revisions, no device runs).
- Direct parent **V001** (`f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023`, correctness FAIL D>6144);
  grandparent **FULL-R006-V001-REDUCTION-ARCH** (`94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266`).
- Composition: **R006 reduction architecture + R019 invscale normalization + one correctness repair**
  (extra `SyncVectorToMte2()` after gamma/bias read). `SINGLE_CHANGE_AUDIT = PASS`.
- Correctness: **PASS all 16 shapes**, D in 64..32768, FP32/FP16/BF16, rows 1 and 3, aligned and unaligned D.
  Compile/link PASS on server3 (Ascend910B3 / dav-2201 / CANN 8.5.0.alpha002). Decision = `NEEDS_ONE_MORE_LOCAL`.
- Kernel structure (as delivered, for research reference only):
  - Per tile `kReduceTileElems = 6144`: LoadNative x, LoadNative r → `Add(y)` → `Mul` square →
    `ReduceSum` into a packed partial (`kPartialCapacity = 64`) → `SyncVectorToMte2`.
  - One `CollapsePartials` `ReduceSum` over collected partials (or a 1-element `Adds` copy when count==1).
  - Normalization tail (once per row): `Muls(invRowWidth)` → `Adds(epsilon)` → `Sqrt` →
    `VectorScalarRead` (V→S) → scalar `return 1.0f / rmsValue` → `ScalarToVector` (S→V).
  - `WriteNormalizedRow` per tile: LoadNative x/r → `Add(y)` → `Muls(y, invRms)` → gamma/bias
    reload tile-by-tile (deliberately no parameter residency / no input-reread avoidance).
- Probe shape in all local runs so far: **FP32 rows=1 D=6144** (single-tile band, D ≤ 6144).

## CURRENT_BLOCKER

- **Local performance measurement is BLOCKED for this route.** Evidence (consolidation-20260924, local-timing-protocol):
  - Window qualification `WINDOW_UNQUALIFIED` 2/2 on d6+d4; same-binary noise floor **not yet run**
    for this route's Direct Parent at its exact probe shape.
  - Parent at D=6144 measured `UNSTABLE` on d4/d5/d6 (CV 0.26–0.37, range/median 0.74–1.10) under legacy
    wall-clock method; a later R2 reference run was `MEASUREMENT_BLOCKED` (parent UNSTABLE, candidate0).
  - Timing budget for the day exhausted; legacy wall-clock samples tagged `LEGACY_TIMING_METHOD`, not mixable.
- Consequence: **no P/C, no V003, no kernel edit** is permitted while candidates stay unjudged under
  `MEASUREMENT_BLOCKED`. All output below is research only; every hypothesis needs the same-binary parent
  run to PASS (unified device-event protocol, warmup≥10, ≥21 samples, in-process) for its exact shape
  before it can be measured.
- This is a measurement-infrastructure blocker, not a Route failure. No Main decision is required to
  continue research; a Main decision IS required to open a device window for same-binary qualification.

## BOTTLENECK_MODEL

Reasoned from source (timing blocked, so no measured attribution; shapes marked as expectation):

1. **Serialized per-tile reduction loop.** Each tile runs load → `Add` → square-`Mul` → `ReduceSum`
   → `SyncVectorToMte2` back-to-back with no overlap to the next tile's load in the reduction pass.
   Cost scales with ceil(D/6144); dominates multi-tile rows (D > 6144).
2. **Per-row normalization tail on the critical path.** `Muls → Adds → Sqrt → VectorScalarRead →
   scalar 1.0f/x → S→V` is ~5 serial dependent steps, once per row, executed *before* the output pass
   starts. The output Muls needs the scalar; the output MTE2 loads do not. Fixed per-row cost.
3. **Two-stage reduction.** Per-tile `ReduceSum` (6144→1 partial) + `CollapsePartials` (≤64→1) +
   a 1-element `Adds` copy when a row yields a single partial. The collapse re-reads UB partials
   (memory round-trip) rather than keeping a running sum.
4. **Output pass reloads x/residual and gamma/bias tile-by-tile by design** — this isolates the
   reduction architecture but leaves param-residency / input-reread wins on the table; those are
   R014/R002/R31 territory owned by other routes and are out of scope here (see DUPLICATE_CHECK).

D-band behavior:
- **D ≤ 6144 (single tile):** reduction = one `ReduceSum` + 1-element copy; the fixed tail (item 2) is a
  large share of total. The probe shape D=6144 lives here, so multi-tile topology changes (items 1/3)
  will show **no** signal at the current probe shape.
- **D > 6144 (multi-tile):** collector + collapse exercised; this is where V002's repair applies and
  where items 1/3 matter. Hidden testcase shape is UNKNOWN — do not infer it from score deltas.

FP32 accumulation: already FP32 end-to-end for FP32 input (`Add`/square/`ReduceSum` on `float`);
the golden uses a double-precision reference. Any low-precision accumulator variant is blocked by
G001 negative evidence on large-D and R004 historical BLOCKED status (see OPTIONAL-HYPOTHESIS-5 note).

---

## HYPOTHESIS-1 — Shorten the sqrt/reciprocal dependency chain (reciprocal-sqrt)

- **MECHANISM:** In `ComputeRowRms`, replace the two-step tail `Sqrt(rowSum)` then scalar
  `1.0f / rmsValue` (separated by a `VectorScalarRead` V→S and a `ScalarToVector` S→V) with a single
  reciprocal-sqrt evaluation of `1/sqrt(mean + eps)`. Only this tail sequence changes; tile size,
  collector, pipeline, scheduling and gamma/bias policy untouched.
- **BOTTLENECK:** Serial per-row normalization-tail latency — two dependent inverse-ish operations
  split by a cross-pipe read — sitting on the critical path before the first output Muls (item 2 above).
- **EXPECTED_SHAPES:** Largest relative effect where the fixed per-row tail is a big share of total:
  narrow/short rows and rows>1 (small and medium D). Near-zero at wide multi-tile D (tile work dominates).
  At the current single-tile probe D=6144 the tail share is moderate, so signal may be small.
- **WHY_IT_MAY_HELP:** Removes one dependent op and, if fully vector-side, one V→S + S→V hop per row;
  fewer serial steps before the output pass can issue its first Muls.
- **WHY_IT_MAY_FAIL:** The operand is a single float — a 1-element `Duplicate(1.0)+Div` to replace the
  scalar division may be no faster than the scalar division itself; the `VectorScalarRead` may remain the
  dominant latency regardless; a dedicated reciprocal-sqrt primitive may not exist or may not be faster on
  this toolchain; effect likely below the currently-blocked noise floor.
- **ASCEND_FEASIBILITY:** `AscendC::Sqrt` is proven working (V002 device PASS). A reciprocal-sqrt
  intrinsic (e.g. `AscendC::Rsqrt`) availability on dav-2201 / CANN 8.5.0.alpha002 is **UNCONFIRMED** —
  verify against the Ascend C ops API on server3 before implementing. Fallback that always exists:
  vector `Duplicate(1.0)+Div` then a single read, which still removes the scalar division (not the read).
- **UB/CORE/DMA_IMPACT:** UB none (1-element `rowSum` slot unchanged). Core: marginally fewer scalar/vector
  ops per row. DMA none.
- **SYNC_IMPACT:** Low. Must keep the S→V ordering so the first output Muls sees a valid `invRms`.
- **PRECISION_RISK:** Low–moderate. A fused reciprocal-sqrt can differ in ULP from sqrt-then-divide;
  golden is double-precision with FP32 atol/rtol 1e-4 — verify against ops-precision-standard before timing.
- **DUPLICATE_CHECK:** R020 ("Sqrt normalization — rsqrt primitive alternative") is listed
  `still_unexplored` with owner REDUCE — this hypothesis **is** that idea, not yet attempted. Distinct
  from R019 (chose Duplicate+Div → scalar recip + Muls; already in V002) and R017 (FP32 intermediates,
  covered). No other NEXT6 route (SCHED/ALIGN/BATCH/ASYNC/UB) touches the rms reciprocal chain.
- **MINIMAL_OFAT_DIFF:** Swap only the final tail sequence in `ComputeRowRms`; nothing else.
- **EXPECTED_LOCAL_PROBES:** device-event P/C at D=6144 FP32 (existing probe) and a multi-tile shape
  (D=8192 / 16384), rows=1 and rows=3; isolate per-row tail by timing many rows. Blocked until same-binary
  parent PASS at the exact shape.
- **CLASSIFICATION:** `NEEDS_MORE_EVIDENCE` (reciprocal-sqrt primitive availability unconfirmed;
  effect size unknown; timing blocked).

---

## HYPOTHESIS-2 — Post-reduction normalization scheduling (overlap tail with first output loads)

- **MECHANISM:** Today `ComputeRowRms` runs to completion (through Sqrt → read → scalar reciprocal)
  *before* `WriteNormalizedRow` issues its first x/residual MTE2 loads. Those loads (and the tile-0 `Add`)
  do **not** depend on `invRms`; only the first `Muls` does. Reorder so the first output tile's MTE2 loads
  are issued while the rms tail is still executing, and only the first `Muls` waits for `invRms`.
  Scheduling change only — no change to reduction math, reciprocal, tile size, or collector.
- **BOTTLENECK:** Serialization between reduction-completion and output-pass start (item 2 above): the
  first tile's MTE2 load latency and the rms tail sit idle back-to-back instead of overlapping.
- **EXPECTED_SHAPES:** Helps where both the tail and the first load are non-trivial vs total: multi-tile
  D>6144 and rows>1. Small at the single-tile probe D=6144 (only one tile → small overlap window), so the
  current probe may understate it.
- **WHY_IT_MAY_HELP:** Hides the first load latency behind the scalar/vector tail; removes idle cycles
  between the two passes, per row.
- **WHY_IT_MAY_FAIL:** Buffer aliasing — `value_`/`fp32A` are shared by the reduction tail (`rowSum` work)
  and the output `Add`, so only the MTE2 *load* (not the `Add`) can move earlier; the win may be a single
  load latency (small); the reduction pass's trailing `PipeBarrier`s already bound the handoff; and this
  route has a demonstrated ordering-hazard history (V002 was exactly a missing `SyncVectorToMte2`), so a
  reordering risks reintroducing an ordering defect on D>6144.
- **ASCEND_FEASIBILITY:** Feasible in one stream by moving the first `LoadNative` x/r (and arming MTE2)
  ahead of the tail while keeping vector work ordered. No new API.
- **UB/CORE/DMA_IMPACT:** UB none (same buffers). Core: fewer idle cycles. DMA: one earlier MTE2 issue
  for tile 0 per row.
- **SYNC_IMPACT:** **HIGH** — must preserve `SyncMte2ToVector` / `SyncVectorToMte2` ordering so a tile-0
  load cannot overwrite buffers the tail still reads (`rowSum`/`partials_` live in UB; x/r staging is
  `inputX_`/`inputR_`). Manageable, but this is the primary risk and correctness must be re-proven first.
- **PRECISION_RISK:** None — pure reordering, identical arithmetic.
- **DUPLICATE_CHECK:** Distinct from **ASYNC-TRIPLE-X** (adds MTE3 store overlap / triple
  MTE2/V/MTE3 pipeline across tiles — a DMA-side idea). Distinct from **R013** double-buffer (buffer-depth
  change for loads). Distinct from R006/R019 (neither overlaps the rms tail with output loads). This is
  intra-row normalization-tail vs first-output-load ordering only.
- **MINIMAL_OFAT_DIFF:** Move only the first output-tile MTE2 load earlier in `WriteNormalizedRow`
  relative to `ComputeRowRms` return; no arithmetic, tile, collector, or reciprocal change.
- **EXPECTED_LOCAL_PROBES:** Full 16-shape correctness first (sync-sensitive), then device-event P/C at
  D=6144 and D=8192/32768, rows=1 and 3. Blocked on both correctness revalidation and timing qualification.
- **CLASSIFICATION:** `READY_FOR_MAIN_REVIEW` — clean single-variable reorder, no arithmetic change,
  no duplicate overlap with active routes. Lead scheduling candidate; implementation must be preceded by
  correctness revalidation and an open device window.

---

## HYPOTHESIS-3 — Partial reduction width (wide-chunk ReduceSum) + vector-reduction utilization

- **DONOR RECORD**
  - `SOURCE_ROUTE`: **H001**, revision **V007 → V008** (`V007 "ReduceSum hot sum(u*u)" 23.29`,
    `V008 "wide ChunkSumSquares ReduceSum" 29.04`; a +5.75 online jump historically — results.tsv).
  - `MECHANISM`: combine a *wider* span of elements into each `ReduceSum` (wide chunk square-and-reduce)
    instead of emitting a narrow partial per load-tile and collapsing afterwards.
  - `OLD_CONTEXT`: H001 row-batched route; wide-D slices; ReduceSum already proven hot there.
  - `CURRENT_CONTEXT`: REDUCE-INVSCALE-X R006 branch — per-tile (6144) `ReduceSum` into a packed
    partial, then a separate `CollapsePartials`. The knob = per-partial `ReduceSum` width.
  - `WHY_ORTHOGONAL`: changes only the partial-reduction width; keeps the two-stage collector/collapse
    topology, the R019 invscale normalization, the tile pipeline, and row scheduling unchanged.
  - `WHY_NOT_DUPLICATE`: H001's chunking lived inside a row-batched multi-row architecture; here the same
    width knob is applied to the reduction-only R006 branch and measured against the R006 parent. It is
    **not** R005 large-tile (that varied load-tile size for copy efficiency, I001 used 2048) and **not**
    MID/WIDE tile sweeps (different branch, different objective).
- **MECHANISM:** widen the per-tile `ReduceSum` so fewer, larger partials are produced per row — i.e.
  increase the span each `ReduceSum` covers (`kReduceTileElems`, or decouple reduce-width from load-width),
  reducing the number of `ReduceSum` calls and the collapse re-read per row.
- **BOTTLENECK:** items 1/3 in the bottleneck model — per-tile `ReduceSum` serialization and the second
  `CollapsePartials` re-read; wide chunks raise vector-reduction utilization per call.
- **EXPECTED_SHAPES:** only affects multi-tile rows (D > 6144); at the single-tile probe D=6144 there is
  exactly one `ReduceSum` already, so **no signal** at the current probe shape. Best at wide multi-tile D.
- **WHY_IT_MAY_HELP:** fewer `ReduceSum` invocations and fewer partial round-trips per row; better
  vector-unit utilization per reduction call (the H001 V008 pattern).
- **WHY_IT_MAY_FAIL:** **UB ceiling is the gating risk.** The seven `kReduceTileElems`-sized buffers
  (`inputX_`, `inputR_`, `output_`, `fp32A_`, `fp32B_`, `value_`, `reduceWork_`) already sum to
  ~7 × 6144 × 4 ≈ **168 KB at FP32**; widening the tile grows all seven. Whether dav-2201 UB has enough
  headroom is UNCONFIRMED. Also: in the legal D ≤ 32768 band a row yields only ceil(D/6144) ≤ 6 partials,
  far below `kPartialCapacity = 64`, so the collapse is already cheap — the win may be marginal even
  where feasible.
- **ASCEND_FEASIBILITY:** conditioned on UB headroom (confirm exact UB size and `InitBuffer` totals on server3).
  May be feasible for FP16/BF16 (input buffers 2-byte → lower total) but not FP32 at 6144.
- **UB/CORE/DMA_IMPACT:** UB **high** (grows with width; the constraint). Core: fewer reduction calls.
  DMA: unchanged load volume (same elements, fewer reduce passes).
- **SYNC_IMPACT:** low — same barrier structure per tile, fewer tiles.
- **PRECISION_RISK:** low — still FP32 accumulation; wider chunks change summation order only (fewer
  partial merges), within FP32 tolerance.
- **DUPLICATE_CHECK:** see donor record; not covered by R005/MID/WIDE (different branch/knob), not by
  R017/R019. H001 is historical/retired, so no active-route conflict.
- **MINIMAL_OFAT_DIFF:** change **only** `kReduceTileElems` (or the reduce-span); leave topology,
  normalization, scheduling untouched. If buffer-count must also drop to fit UB, that is a second variable
  and this hypothesis must be split/re-scoped first.
- **EXPECTED_LOCAL_PROBES:** needs a **multi-tile** shape (D=8192/16384/32768), device-event P/C, after
  UB confirmation and same-binary parent PASS at that shape. Not measurable at D=6144.
- **CLASSIFICATION:** `NEEDS_MORE_EVIDENCE` — UB headroom and the ≤6-partial reality must be settled
  before this is worth a device window; likely narrow but not ruled out.

---

## OPTIONAL-HYPOTHESIS-4 — Reduction topology for the fitting D-band (single-stage whole-row reduce)

- **DONOR / IDEA RECORD**
  - `SOURCE_ROUTE`: **R006** idea-pool note ("hierarchical UB tree", `still_unexplored` for REDUCE);
    R011 manual vector tree (idea-pool item 1, owner REDUCE, prior REDUCE-X attempts all correctness-FAIL).
  - `MECHANISM`: collapse the two-stage (per-tile partial → `CollapsePartials`) reduction into one stage
    — a single `ReduceSum` over the whole row's squares held contiguously — for rows whose squares fit UB.
  - `WHY_ORTHOGONAL`: changes reduction stage count only; normalization, scheduling, gamma/bias untouched.
  - `WHY_NOT_DUPLICATE`: distinct from HYP-3 (HYP-3 keeps two stages but widens each; this removes a stage
    and fixes width = whole row). R011 attempts that failed used GetValue-based trees; a contiguous
    single `ReduceSum` is a different mechanism (no GetValue golden path).
- **MECHANISM:** for the D-band where the row's squares fit in UB (D ≤ ~one UB buffer), replace the
  per-tile partial emission + separate collapse with one `ReduceSum` over the full row.
- **BOTTLENECK:** item 3 — the `CollapsePartials` re-read/merge stage.
- **EXPECTED_SHAPES:** only multi-tile rows that still fit UB; at the single-tile probe D=6144 the current
  code is *already* effectively single-stage (one `ReduceSum` + a 1-element copy), so **no win there**.
- **WHY_IT_MAY_HELP:** removes one reduction stage and its UB round-trip for medium multi-tile rows.
- **WHY_IT_MAY_FAIL:** holding a whole row's squares needs D×4 bytes of UB in addition to the I/O buffers;
  for D=32768 that is 128 KB of squares alone → **UB-infeasible for FP32** across most of the multi-tile
  band. For single-tile rows there is no stage to remove. Net: the only band where a stage exists (multi-tile)
  is exactly the band where the single-stage buffer does not fit → structurally squeezed.
- **ASCEND_FEASIBILITY:** feasible only for a narrow medium-D FP16/BF16 band, if at all; confirm UB first.
- **UB/CORE/DMA_IMPACT:** UB **very high** (full-row square buffer). Core: one `ReduceSum` vs several.
  DMA: may require loading the row in fewer, larger chunks (layout change risk).
- **SYNC_IMPACT:** moderate — restructures the load/compute ordering of the reduction pass.
- **PRECISION_RISK:** low–moderate (FP32, different summation grouping).
- **DUPLICATE_CHECK:** not covered by R006 (which *introduced* two-stage chunking), not by R011 (different
  mechanism). No active NEXT6 route owns whole-row reduction.
- **MINIMAL_OFAT_DIFF:** replace the two-stage reduce with one stage for the fitting band only; nothing else.
- **EXPECTED_LOCAL_PROBES:** multi-tile shapes across the UB-fit boundary; blocked on UB confirmation and
  timing qualification.
- **CLASSIFICATION:** `INFEASIBLE` for FP32 across most of the D>6144 band (UB); `NEEDS_MORE_EVIDENCE`
  only if exact UB headroom shows a usable medium-D window, most plausibly FP16/BF16. Low expected value.

---

## OPTIONAL-HYPOTHESIS-5 — FP32 accumulation screening + external reciprocal-sqrt idea

- **FP32 ACCUMULATION (theme) — screened, no hypothesis raised:** accumulation is already FP32 end-to-end
  for FP32 input (`Add`, square-`Mul`, `ReduceSum` on `float`) with a double-precision golden; R017
  "FP32 full intermediate" is `covered`. Widening further is impossible (FP32 is already the top of this
  dtype), and *narrowing* (low-precision accumulator) is blocked by G001 large-D negative evidence and
  R004 historical BLOCKED. **Conclusion: no FP32-accumulation change is viable → theme screened out.**
- **EXTERNAL IDEA (feeds HYPOTHESIS-1):**
  - `EXTERNAL_IDEA`: fuse the RMSNorm reciprocal-sqrt into a single device primitive / single pass so the
    `1/sqrt(mean+eps)` inverse is produced without a separate divide, and (in some public kernels) without
    a scalar round-trip.
  - `SOURCE`: public RMSNorm/LayerNorm CUDA/Triton kernel literature — fused `rsqrt`-based normalization
    (e.g. `__frsqrt_rn`-style fast reciprocal-sqrt used to fold mean/variance inverse into one step).
  - `MECHANISM`: one reciprocal-sqrt evaluation replaces `Sqrt` + separate `1/x`.
  - `WHY_DIFFERENT_FROM_EXISTING_ROUTES`: no NEXT6 route (SCHED/ALIGN/BATCH/ASYNC/UB) and no phase4 route
    has applied a fused reciprocal-sqrt to this branch's per-row tail; R020 lists it `still_unexplored`.
  - `EXPECTED_BOTTLENECK`: the serial per-row sqrt→scalar-reciprocal tail (bottleneck item 2).
  - `APPLICABLE_ROUTE`: REDUCE-INVSCALE-X (this route).
  - `PROVENANCE_CLASS`: `PUBLIC_KNOWN_CONCEPT` — general public technique, **no code copied**, no URL
    fetched this turn (public fetch unavailable); must be re-grounded against Ascend C API before use.
  - This external idea **is** HYPOTHESIS-1; recorded here to satisfy the external-idea provenance field.
- **CLASSIFICATION:** `DUPLICATE` (for the low-precision-accumulator branch, it duplicates blocked R004/G001);
  the fused reciprocal-sqrt external idea is already captured as HYPOTHESIS-1 (`NEEDS_MORE_EVIDENCE`).

---

## RECOMMENDED_NEXT

1. **Do not edit V002** (TRACK-A). It stays the correctness-accepted candidate at
   `bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26`; no V003 while measurement is blocked.
2. **Unblock measurement first (Main decision needed):** run the unified same-binary (parent-only) noise
   floor for this route's **Direct Parent** at its **exact probe shape** under the device-event protocol
   (warmup≥10, ≥21 samples, in-process). Until that PASSes, no P/C for any hypothesis can run.
3. **Lead hypothesis when a window opens: HYPOTHESIS-2** (overlap normalization tail with first output
   load) — clean single-variable reorder, no arithmetic change, no new buffer (UB-safe), no duplicate with
   active routes. Must re-run the full 16-shape correctness first because it is sync-sensitive.
4. **Second: HYPOTHESIS-1** (reciprocal-sqrt tail) — confirm `Rsqrt`-equivalent availability in Ascend C
   on server3, then measure; low UB/DMA impact, covers the scalar-reciprocal + dependency-chain themes.
5. **Hold HYPOTHESIS-3 and OPTIONAL-HYPOTHESIS-4** until exact dav-2201 UB headroom is confirmed; both are UB-conditional and only
   affect multi-tile shapes (invisible at the current D=6144 probe). Prefer FP16/BF16 probes if pursued.
6. **Themes screened out this cycle:** FP32 accumulation (already maxed; narrowing blocked), input-reread /
   parameter residency (R002/R014/R31 territory — other routes), reduction-loop double-buffering
   (R013/ASYNC-TRIPLE territory and UB-tight).

### Handoff summary
- Screened hypotheses written: **3 main** (HYPOTHESIS-1..3, all fields) **+ 2 optional**
  (OPTIONAL-HYPOTHESIS-4 all fields, OPTIONAL-HYPOTHESIS-5 screening/external-idea) → **5 total**,
  each single-variable (OFAT), each with a classification.
- Classifications: HYP-1 `NEEDS_MORE_EVIDENCE`, HYP-2 `READY_FOR_MAIN_REVIEW`, HYP-3 `NEEDS_MORE_EVIDENCE`,
  OPT-4 `INFEASIBLE`/`NEEDS_MORE_EVIDENCE`, OPT-5 `DUPLICATE` (low-precision) with external idea folded into HYP-1.
- Sources inspected: execution-contract §R, local-timing-protocol, idea-pool-29-routes,
  architecture-evidence-map, consolidation-20260924, next-round-plan, results.tsv,
  V001/V002 source-meta + diff + MAIN-REVIEW + local-result, ROUND2/L005 reports, historical
  R006/R017/R019 archives, V002 kernel `submission.asc`.
- Blockers requiring Main: (a) open a device window for same-binary parent qualification at the probe shape;
  (b) confirm dav-2201 UB headroom to decide HYP-3 / OPT-4 viability.

