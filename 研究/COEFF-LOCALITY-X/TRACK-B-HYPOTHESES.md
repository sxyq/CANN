# TRACK-B HYPOTHESES — COEFF-LOCALITY-X

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/COEFF-LOCALITY-X
BRANCH=exp/main2-r2-coeff-locality
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_COEFF_LOCALITY
MODE=TRACK-B research only — kernel edit follows the selected hypothesis in the current specification

## 0. Why this route (post-reduction-pivot)

Three reduction-topology variants on frozen R31B-V011 produced no usable win:
V001 eager fold -3.8% (8x8192 only, NEEDS_ONE_MORE_LOCAL), V002 pairwise tree
+6.6% regression (LOCAL_REJECTED), V003 short-span ReduceSum -0.5% neutral
(NEEDS_ONE_MORE_LOCAL). Reduction V/S is therefore not the lever. DMA + output
pass are. This route studies gamma/bias coefficient load locality only.

## 1. Load map of the frozen seed (every gammaGm_/biasGm_ site)

| # | Function | When gamma/bias load happens | Granularity | Verdict |
|---|---|---|---|---|
| 1 | `Process()` generic, `cacheParams=true` (D≤8192 ∧ localRows>1) | once before row loop (`:251-280`) | per core | optimal |
| 2 | `Process()` generic, `cacheParams=false` (localRows==1) | **per tile in output pass** (`:390-391`) | per tile | redundant when tileCount>1 |
| 3 | `ProcessNarrowMidOverlap` (128<D≤4096) | once if localRows>1, else per row (`:515`, `:539`) | per core / per row | per row = per core here (1 row) |
| 4 | `ProcessFp32/16/Bf16FullRowOutputPipelined` | once before row loop (`:1127-1131`, `:887-896`, `:1008-1012`) | per core | optimal |
| 5 | `ProcessSmallFp32*Batched` / `ProcessSmallLowPrecision*` | once before batch loop (`:1382`, `:1467`, `:1582`, `:1679`) | per core | optimal |
| 6 | `ProcessBf16/16FullTileBatchedOutputPipelined` | once before batch loop (`:633-634`, `:761-762`) | per core | optimal |
| 7 | **`ProcessWideFp32FullCacheRows`** (D>8192 FP32) | **per tile per batch** (`:2191-2192`) | per tile × per batch | **redundant across batches** |
| 8 | `ProcessWideLowPrecision` (D>8192 FP16/BF16) | per tile with **2-deep param MTE2 prefetch** (`:3251-3296`) | per tile, prefetched | best in-kernel pattern |

Buffer lifetime note: on the FP32 wide path, `gammaLocal`/`biasLocal` alias
`xBuf_`/`residualBuf_` (`:2179-2180`). Those staging buffers are consumed by
pass-1 x/residual DMA, so param residency does not outlive a batch without a
dedicated slot. The generic path has real `gammaBuf_`/`biasBuf_` (`:121-122`)
sized at `kCacheElems` (8192) — full-row residency already exists there.

UB budget on wide path: `kWideFullYBudgetBytes = 176 KiB` (`:1281`).
`ChooseWideFullYRows` (`:1294-1328`) already shrinks `tileElems` to fit
`yBytesPerRow*rows + ioBytes + workBytes + reduceBytes`. For D=32768 FP32,
y row alone is 128 KiB, so `wideFullYRows_=1` and `tileElems` drops (8 tiles at
4096 or more at 2048). Full gamma/bias residency for D=32768 is 256 KiB —
**does not** fit alongside y. Stripe/chunk residency is the only in-UB option.

## 2. Where the redundant traffic actually is

- **Wide FP32 (D>8192), multiple batches per core**: each batch reloads every
  gamma/bias tile (`:2191-2192`). With `batchLimit=1` (D=32768) and
  `localRows>1`, this is `localRows ×` the needed param DMA. For 8×32768 on
  4 cores (localRows=2) that is 2× the needed 256 KiB/core of param reads.
- **Generic single-row, multi-tile (D=6144/8192)**: `cacheParams=false` forces
  per-tile loads instead of one full-row load. Byte count is the same, but
  descriptor count is `2×tileCount` instead of `2`, and the load is not
  hoisted out of the output-pass critical path.
- **FP32 wide output pass has no param prefetch** (contrast site 8). Every
  tile pays `Load + SyncMTE2ToV` serialized before its apply. For tileCount=8
  (D=32768) that is 16 exposed MTE2 round-trips per batch.

Official case 14 is 4.4× off the best — far too large for descriptor-count
savings alone. Exposed param MTE2 latency in the wide output pass (site 7 vs
site 8) and cross-batch reload are the two mechanisms large enough to matter.

---

## HYPOTHESIS-1 — Param double-buffer prefetch in FP32 wide output pass (V011 pattern)

- **MECHANISM**
  In `ProcessWideFp32FullCacheRows` pass 2, replace the serialized
  `Load(gamma/bias) → SyncMTE2ToV → apply` sequence with a 2-deep param MTE2
  pipeline: while tile *t* is applied to the batch’s y rows, tile *t+1*’s
  gamma/bias are already in flight into a second staging pair. Exactly the
  pattern already implemented for `ProcessWideLowPrecision` (`:3251-3296`,
  `prd0/prd1`, `gBase`/`gBase[tileWidth]`). One gamma/bias tile still serves
  the whole batch (unchanged reuse). Change is load *timing* and staging
  residency only.

- **EXPECTED_BOTTLENECK**
  Exposed `SyncMTE2ToV` on gamma/bias in the wide FP32 output pass — 2 param
  MTE2 round-trips per tile, serialized against apply. For D=32768 (8 tiles at
  4096) that is 16 exposed round-trips per batch. This is the only in-kernel
  param pattern that the low-precision sibling already optimized and the FP32
  sibling did not.

- **FILES/FUNCTIONS TO TOUCH**
  `ProcessWideFp32FullCacheRows` pass 2 only (`submission.asc` ~`:2170-2235`).
  Staging: either widen `xBuf_`/`residualBuf_` to 2 tiles each (budget already
  reserves `ioTiles=2` in `ChooseWideFullYRows`, `:1301-1303`) or add a
  dedicated 2-slot param pair and shrink `tileElems` accordingly. No host,
  CMake, runner, or other process function.

- **WHY_ORTHOGONAL_TO_MAIN1**
  Single-row/single-tile in-kernel MTE2 timing for gamma/bias. No multi-row
  batch DMA, no rows/block, no wide specialization, no dtype split, no
  scheduling, no sync-removal as the variable (the existing `SyncMTE2ToV`
  stays; only its operand is prefetched). MAIN-1 `cann-sixlane` untouched.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  REDUCE-HIER-X owns reduction topology (parked). ALIGN-TAIL-X owns copy
  geometry. ASYNC-TRIPLE-X owns MTE3/triple overlap. SCHED-ROWGROUP-X owns
  core row ownership. BATCH-RESIDENT-X owns multi-row input DMA.
  UB-LIVENESS-X owns buffer aliasing. EPILOGUE-FUSE-X owns epilogue fusion.
  None owns gamma/bias MTE2 residency in the FP32 wide output pass. The donor
  is the in-kernel `ProcessWideLowPrecision` prefetch, not another route.

- **EXPECTED_WIN_SHAPES**
  D>8192 FP32: 1×32768, 1×16384, 8×32768, 8×16384. Est. −8% to −25% (removes
  ~tileCount×2 exposed MTE2 round-trips per batch; largest where tileCount is
  large and apply is short). Weak/zero on D≤8192 and on single-tile shapes.

- **EXPECTED_RISK_SHAPES**
  D≤4096 (tileCount=1, nothing to prefetch). Shapes where apply already hides
  the load (small batchRows). UB-pressure shapes where widening io tiles forces
  `tileElems` down and increases tileCount.

- **CORRECTNESS_RISK**
  Low. Pure reordering of an already-legal load; arithmetic unchanged. Main
  hazard is the same store-drain / staging-alias dance the V011 code already
  solves (`:2183-2190` waits). Must keep `SyncMTE2ToV` before each apply.

- **MEASUREMENT_PLAN**
  1. Declaration block (ROUTE / REVISION / DIRECT_PARENT / PARENT_SOURCE_SHA /
     PARENT_SCORE=45.16 / SINGLE_HYPOTHESIS / CONTEXT_CLASS / WHY_NOT_DUPLICATE).
  2. Build + link on server3; NPU correctness FP32/FP16/BF16 including
     D=8192/16384/32768 multi-tile and D=256/4096 single-tile.
  3. Same-binary per shape under `local-timing-protocol.md` (device events,
     warmup ≥ 45, samples ≥ 21, MAD/med ≤ 0.10, drift ≤ 0.10). Prefer d4 (clean
     on 1×32768: drift 0.002–0.011 in REDUCE-HIER-X V003 runs).
  4. Interleaved P/C ≥ 4 pairs. Primary probes: 1×32768 FP32, 1×16384 FP32,
     8×32768 FP32. Control: 1×4096 FP32 (expect ≈ 0).
  5. Falsification: if 1×32768 clean delta is inside noise (|Δ| ≤ ~2%), exposed
    param MTE2 is not the bottleneck; pivot to H2/H3.

---

## HYPOTHESIS-2 — Cross-batch gamma/bias stripe residency (R014, D > UB)

- **MECHANISM**
  Give gamma/bias a dedicated UB stripe (K tiles) that survives the batch loop
  instead of aliasing `xBuf_`/`residualBuf_`. When a batch’s output pass
  revisits tiles inside the resident stripe, skip the GM load and reuse the
  UB copy. Full residency is impossible for D=32768 (256 KiB params vs 176 KiB
  budget), so the variable is the **stripe width K** and the skip-if-resident
  test. One factor: stripe residency. Not a new multi-row batch DMA mode.

- **EXPECTED_BOTTLENECK**
  Cross-batch param reload on the FP32 wide path (`:2191-2192`) when
  `localRows > batchLimit` (e.g. 8×32768 with 4 cores → localRows=2,
  batchLimit=1 → 2× param DMA). Each batch reloads all tiles’ gamma/bias.

- **FILES/FUNCTIONS TO TOUCH**
  `Init` wide FP32 branch (`:75-77`) — add a `gammaStripeBuf_`/`biasStripeBuf_`
  sized `K * tileElems` and adjust `ChooseWideFullYRows` budget input.
  `ProcessWideFp32FullCacheRows` pass 2 (`:2170-2235`) — load-or-reuse per tile.
  No host / CMake / runner.

- **WHY_ORTHOGONAL_TO_MAIN1**
  In-kernel UB residency of coefficients. No multi-row DMA, no rows/block,
  no wide-path redesign (batch/y architecture untouched), no dtype/scheduling
  change. Explicitly in the allowed list (“stripe / chunk residency of
  gamma/bias for D > UB”).

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  UB-LIVENESS-X studies aliasing, not coefficient residency across batches.
  REDUCE-HIER-X is reduction. The R014 donor (“stripe-resident params for
  D>UB”) is idea-pool, owner WIDE-X historically, but this is the
  COEFF-LOCALITY-X lane and R014 is its named donor. No active MAIN-2 route
  owns gamma/bias residency.

- **EXPECTED_WIN_SHAPES** (vs parent)
  Multi-batch wide: 8×32768, 8×16384, 16×32768 (localRows≥2). Win scales with
  `(localRows-1)/localRows × K/tileCount`. With K=2 and tileCount=8:
  ~12% param-DMA reduction; with K=4: ~25%. Est. −3% to −12% kernel time
  (param DMA is one component of the output pass). Zero on localRows=1.

- **EXPECTED_RISK_SHAPES**
  1×32768 / 1×16384 (single batch — nothing to reuse across). Shapes where UB
  pressure forces `tileElems` down enough to raise tileCount and erase the win.

- **CORRECTNESS_RISK**
  Low. Coefficients are read-only and row-invariant. Hazards: UB budget
  accounting (does not evict y rows), and correct skip-if-resident test so a
  tile outside the stripe is still loaded.

- **MEASUREMENT_PLAN**
  Same protocol as H1. Primary probes: 8×32768 FP32, 8×16384 FP32. Control:
  1×32768 (expect ≈ 0 — single batch). Falsification: if 8×32768 clean delta
  is ≈ 0 while 1×32768 is also ≈ 0, cross-batch reload is not material and
  only the prefetch (H1) direction remains.

---

## HYPOTHESIS-3 — One-shot full-row gamma/bias load for single-row multi-tile generic cores

- **MECHANISM**
  In `Process()`, `cacheParams = cacheRow && localRows > 1` (`:245`) excludes
  single-row cores even when `tileCount > 1`. Extend the condition to
  `cacheRow && (localRows > 1 || tileCount > 1)` so a single-row core with
  D=6144/8192 preloads the full gamma/bias once (2 MTE2 descriptors) instead
  of `2 × tileCount` per-tile loads in the output pass (`:390-391`). Same
  bytes, fewer descriptors, and the load moves off the output-pass critical
  path (it already happens before the row loop for the multi-row case).

- **EXPECTED_BOTTLENECK**
  Per-tile gamma/bias loads in the generic output pass when `cacheParams=false`
  but `tileCount>1`. Descriptor count and critical-path placement, not bytes.

- **FILES/FUNCTIONS TO TOUCH**
  `Process()` condition at `:245` and the `cacheParams` consumer at `:390`.
  Possibly hoist the preload block (`:249-282`) to also fire for
  `localRows==1 ∧ tileCount>1`. No host / CMake / runner.

- **WHY_ORTHOGONAL_TO_MAIN1**
  Single-core, single-row param load placement. No multi-row DMA, no
  rows/block, no wide/dtype/scheduling change.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  No other route owns `cacheParams` policy in the generic path. Not reduction,
  not copy geometry, not MTE3, not core ownership, not buffer aliasing.

- **EXPECTED_WIN_SHAPES** (vs parent)
  D=6144, D=8192 with rows ≤ cores (so localRows=1). Est. −1% to −4%
  (descriptor reduction + slight critical-path relief). Zero where
  localRows>1 (already cached) or tileCount=1 (D≤4096).

- **EXPECTED_RISK_SHAPES**
  D≤4096 (no change). Multi-row cores (already optimal). UB pressure is nil
  (`gammaBuf_`/`biasBuf_` already sized `kCacheElems`).

- **CORRECTNESS_RISK**
  Very low. Pure condition change; the preload block already exists and is
  correct for the multi-row case. Only hazard: the output-pass `gammaLocal[col]`
  indexing must match the preloaded layout (`:431-433` already branches on
  `cacheParams`).

- **MEASUREMENT_PLAN**
  Same protocol. Primary probes: 1×8192 FP32, 1×6144 FP32 (single-row,
  multi-tile). Control: 1×4096 FP32, 2×8192 FP32 (expect ≈ 0 — already
  cached). Falsification: if 1×8192 clean delta ≈ 0, descriptor count is not
  material and H3 is a no-op.

---

## Recommendation

**HYPOTHESIS-1 — Param double-buffer prefetch in FP32 wide output pass.**

Why first: it is the only mechanism in the load map that (a) is large enough
to plausibly explain a 4.4× Official gap, (b) already has a proven in-kernel
donor (`ProcessWideLowPrecision` V011 prefetch), and (c) is a small
single-variable change confined to one function’s output pass. H2 is the
bigger structural idea but is UB-budget-bound and only fires when
`localRows > batchLimit`. H3 is clean but its ceiling is a few percent.

---

## REQUEST_MAIN_ROUTE_RECORD

Requesting current route record for exactly one first revision:

**HYPOTHESIS-1 — Param double-buffer prefetch in FP32 wide output pass (V011 pattern)**

- ROUTE: COEFF-LOCALITY-X
- REVISION: V001
- DIRECT_PARENT: FROZEN_R31B_V011
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- PARENT_SCORE: 45.16 (OFFICIAL_ANCHOR)
- SINGLE_HYPOTHESIS: 2-deep gamma/bias param MTE2 prefetch in
  `ProcessWideFp32FullCacheRows` pass 2 (port of the existing
  `ProcessWideLowPrecision` V011 pattern). Staging lifetime and load timing
  only; one gamma/bias tile still serves the whole batch; arithmetic, row
  batching, dtype dispatch, and DMA counts for x/residual unchanged.
- CONTEXT_CLASS: FROZEN_STRONG_BASELINE_COEFF_LOCALITY
- WHY_NOT_DUPLICATE: see H1 section (distinct from all active MAIN-2 lanes;
  donor is the in-kernel low-precision sibling, not another route)

Kernel, host, CMake, runner, shared-control, and CANNJudge actions follow
the current specification is recorded.
