# R31A — Track-B Next Hypotheses (OFAT, R31A self-lineage)

Date: 2026-09-28
ROUTE: R31A (Champion Conservative Evolution)
Lineage base: **V016** (OFFICIAL_BEST 45.00, source SHA `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`)
Context class: `HISTORICAL_EXPLOIT`
Scope of this file: 3–5 non-duplicate single-mechanism hypotheses from R31A's own lineage (affine application / Rsqrt path / path selection / local data flow). Read-only research. No revision is declared or implemented here.

**Parent rule for the next performance revision:** V021 is `NEEDS_ONE_MORE_LOCAL`, not `LOCAL_ACCEPTED`, so it cannot be a parent. The next performance revision must parent from **V016**.

---

## Research-space boundaries (what this file must NOT overlap)

| Owner route | Its mechanism | How this file stays out |
|---|---|---|
| STORE-EPILOGUE-X | store / writeback merging | No hypothesis merges, reorders, or coalesces the `Store` to GM. |
| EPILOGUE-ARITH | the epilogue arithmetic chain (Mul/Add fusion into FMA etc.) | No hypothesis fuses `Mul+Add` into one instruction or rebuilds the arithmetic chain. |
| SHAPE-TILING | tile policy (tile widths, tile counts) | No hypothesis changes `kWideFp32CachedTileElems` / `kWideFp32BatchTileElems` / any tile constant. V019 already tried 7680→8192 and correctness-failed. |
| R31B | aggressive multimode / low-precision wide pipeline | Every change is a minimal single-site edit inside R31A's existing `ProcessWideFp32*` functions; no R31B buffer/pipeline transplant. |
| DTYPE-SPECIAL-X | FP32 aligned local-copy helper (`Adds(x,0)` replacement) | No dtype path / copy-helper change. |
| MIX-A | A001 FastKernel path swap | No FastKernel / mode-dispatch transplant. |

Additional adjacency noted (not in the hard no-overlap list, but declared):
- **VECTOR-MATH-X** (MAIN-2, parent `FROZEN_R31B_V011`) owns the RMS denominator pipeline broadly (`invD / mean / epsilon / sqrt / rsqrt / reciprocal / scalar-to-vector`). H1 below is a *minimal primitive substitution* confined to R31A's CachedRows finalize — not a denominator pipeline redesign — and stays on the R31A V016 parent.
- **BATCH-RESIDENT-X** owns parameter staging / gamma-bias residency / row-batch parameter reuse. H2 below is about *where the row-scale is applied*, not about caching gamma/bias across rows.
- **UB-LIVENESS-X** owns buffer lifetime / aliasing / peak-UB budgeting on its own parent code. H4 below is a same-footprint role-reassignment inside one function, not a live-set or aliasing redesign.

---

## Key dispatch fact that scopes every hypothesis

The paired probe runs `rows=2, blocks=1`, so `localRows = 2`. V016's dispatch sends FP32 wide launches as:

```text
wideFp32BatchPath_    = localRows >= 2 && rowWidth < 32768
wideFp32CachedRowPath_ = (not batch) && rowWidth <= 32768
```

| shape | path actually executed | does it exercise V021? |
|---|---|---|
| rows=2 D=24576 blocks=1 | `ProcessWideFp32Batched` (batch) | **NO** |
| rows=2 D=32768 blocks=1 | `ProcessWideFp32CachedRows` | **YES** (only exercising shape) |

Consequence: V021's `SyncMTE3ToV` placement change lives in `ProcessWideFp32CachedRows` and is only reachable at D=32768 under the probe. All five historical D=24576 paired attempts tested the batch path, not V021. This is recorded in `本地实验/R31A/V021/local-result.json` → `TIMING_RUN_20260928.PATH_DISPATCH_FINDING`.

---

## H1 — CachedRows RMS finalize via `Rsqrt`

- **Research space:** Rsqrt path
- **Where:** `ProcessWideFp32CachedRows`, the per-row RMS finalize block (submission.asc:2026-2035). One conceptual mechanism: replace the two-step `Sqrt` + scalar reciprocal with a single `Rsqrt`.
- **MECHANISM:** Currently:
  ```text
  SyncVToS(); squareSum = GetValue(0);
  meanSquare = squareSum * invRowWidth + epsilon;
  SyncSToV(); Duplicate(xLocal, meanSquare, 1); Sqrt(xLocal, xLocal, 1);
  SyncVToS(); invRms = 1.0f / xLocal.GetValue(0); SyncSToV();
  ```
  Replace `Duplicate + Sqrt + scalar 1.0f/x` with `Duplicate + Rsqrt` (or `Rsqrt` directly on the one-element tensor), then one `GetValue` for `invRms`. Removes one scalar division and shortens the V/S tail.
- **BOTTLENECK:** Per-row scalar tail: 2 V→S + 2 S→V round-trips and a scalar divide on every row. At D=32768 the vector work dominates, but the tail still holds up the pass-1 → pass-2 transition.
- **EXPECTED_SHAPES:** rows=2 D=32768 (CachedRows, the exercising shape). Any CachedRows shape.
- **WHY_IT_MAY_HELP:** One fewer scalar op per row and one less V/S boundary; `Rsqrt` is a single hardware vector op.
- **WHY_IT_MAY_FAIL:** `Rsqrt` numerics differ slightly from `Sqrt` then divide; correctness must re-pass the 1e-4 FP32 tolerance. The tail may already be hidden under MTE2, so the measured effect could be ~0 (as V021's MTE3-wait deferral was).
- **ASCEND_FEASIBILITY:** `AscendC::Rsqrt` is a standard vector API on the 910B3 V pipe. One-element tensors are already used in this block.
- **UB/CORE/DMA_IMPACT:** No UB change. No DMA change. V-pipe only.
- **SYNC_IMPACT:** May drop one `SyncSToV`/`SyncVToS` pair (if `Rsqrt` result is used directly).
- **PRECISION_RISK:** Low–medium. `rsqrt` vs `1/sqrt` can differ in the last ulp; the FP32 budget is 1e-4 abs and rel, so should fit, but must be re-verified on D=32768 and D=24576.
- **WHY_NOT_DUPLICATE:**
  - R31A V001–V021: V006 folded `mean-eps-sqrt` on the *narrow-row* path (different function); no R31A revision touched the CachedRows RMS finalize primitive. V020 = pass-1 prefetch; V021 = pass-2 MTE3 wait placement. Neither is the denominator primitive.
  - R31B: its low-precision wide pipeline is a different code region; this is one site in R31A's FP32 CachedRows.
  - DTYPE-SPECIAL-X: FP32 aligned copy helper — orthogonal.
  - MIX-A: FastKernel path swap — orthogonal.
- **MINIMAL_OFAT_DIFF:** In `ProcessWideFp32CachedRows` only: replace `Duplicate+Sqrt+GetValue+1.0f/` with `Rsqrt+GetValue`. All other paths (batch, generic, FP16/BF16, narrow) unchanged.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768 blocks=1, same-binary + interleaved P/C vs V016 on a device where D=32768 same-binary qualifies (d4 qualified 2026-09-28). Secondary: D=24576 as a no-regress control (note it exercises the batch path, so it is a *no-regress* probe, not a hypothesis probe).

---

## H2 — Hoist the invRms row-scale out of the CachedRows affine loop

- **Research space:** affine (gamma/bias) application method
- **Where:** `ProcessWideFp32CachedRows` pass 2 (submission.asc:2038-2055). One conceptual mechanism: relocate where the `invRms` scale is applied.
- **MECHANISM:** Currently the per-tile affine is `Muls(value, invRms) → Mul(value, gamma) → Add(value, bias)` (3 V ops + 3 `PipeBarrier` per tile). Change to:
  1. Before the tile loop: one full-row `Muls(valueLocal, valueLocal, invRms, rowWidth)` (1 V op + 1 barrier).
  2. Per tile: only `Mul(value, gamma) → Add(value, bias)` (2 V ops + 2 barriers).
  The tile loop's first `Load(gamma/bias)` can be issued while the full-row scale is still in flight.
- **BOTTLENECK:** Per-tile V ops and barriers on the hot cached-row tensor; the `Muls` is paid once per tile when it only depends on the row scalar `invRms`.
- **EXPECTED_SHAPES:** rows=2 D=32768 (CachedRows). Also any single-row CachedRows shape.
- **WHY_IT_MAY_HELP:** Removes one full-tile V pass and one `PipeBarrier` per tile from the value path (5 tiles at D=32768 → 5 fewer barriers); the row-scale becomes one contiguous op; first param MTE2 overlaps with the scale.
- **WHY_IT_MAY_FAIL:** The total element count is unchanged (the `Muls` work merely moves), so if the kernel is DMA-bound the effect is ~0. Hoisting changes float association order only if the scale was previously applied per-tile and rounding differed — it does not (same `invRms` scalar), so numerics are preserved exactly.
- **ASCEND_FEASIBILITY:** `Muls` over `rowWidth` elements is legal; the value buffer is `rowWidth` floats.
- **UB/CORE/DMA_IMPACT:** No UB change. No DMA change. V-pipe op count per tile drops.
- **SYNC_IMPACT:** One fewer `PipeBarrier` per tile in the affine loop.
- **PRECISION_RISK:** None (identical arithmetic, same scalar).
- **WHY_NOT_DUPLICATE:**
  - R31A V001–V021: no revision relocated the `invRms` scale. V013/V015 changed pipeline *depths* on the batched/low-precision output path (different function); V021 moved an `MTE3` wait (sync placement, not arithmetic placement).
  - EPILOGUE-ARITH owns *fusing* the `Mul+Add` chain (e.g. into FMA). This hypothesis does **not** fuse the chain — it relocates one already-separate `Muls` outside the loop. The chain inside the loop remains `Mul → Add`.
  - BATCH-RESIDENT-X owns gamma/bias *residency* (loading them once across rows). This hypothesis does not change any gamma/bias load; it changes where the row scalar is multiplied in.
  - R31B / DTYPE-SPECIAL-X / MIX-A: no such scale-hoist exists in those lines.
- **MINIMAL_OFAT_DIFF:** In `ProcessWideFp32CachedRows` pass 2 only: add one full-row `Muls` before the tile loop; delete the per-tile `Muls` and its `PipeBarrier`. Gamma/bias loads, `Mul`, `Add`, `Store`, and the (V021) deferred `SyncMTE3ToV` placement are unchanged.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768 blocks=1, same-binary + interleaved P/C vs V016. Secondary D=24576 no-regress control.

---

## H3 — Route 2-rows-per-core FP32 wide to CachedRows instead of batch

- **Research space:** path selection / dispatch conditions (full-y CachedRows vs batch vs generic)
- **Where:** `AddRmsNormBiasKernel::Init`, the `wideFp32BatchPath_` predicate (submission.asc:75-76). One conceptual mechanism: change the row-count threshold of the batch entry.
- **MECHANISM:** Currently `wideFp32BatchPath_ = localRows >= 2 && rowWidth < 32768`. Change the row threshold so `localRows == 2` prefers the full-y CachedRows path (single input read, full row in UB) over the batch path (gamma/bias tile reused across the row batch, but x/res re-read). E.g. `localRows >= 3` for batch entry, so `localRows == 2` falls through to `wideFp32CachedRowPath_`.
- **BOTTLENECK:** At `localRows == 2` the batch path's parameter-reuse benefit is small (only 2 rows share one gamma/bias tile) while it still pays the input re-read. CachedRows reads x/res once and keeps the full row in UB.
- **EXPECTED_SHAPES:** rows=2 blocks=1 (localRows=2) at D in (8192, 32768) — including the D=24576 probe shape, which would then exercise CachedRows. Also rows=4 blocks=2, rows=6 blocks=3, etc.
- **WHY_IT_MAY_HELP:** Eliminates the batch path's second x/res GM read for the 2-row case; D=24576 becomes a CachedRows shape so V021-class and H1/H2-class changes become measurable on it.
- **WHY_IT_MAY_FAIL:** For D=24576 the value cache is 96 KB (plus 60 KB staging ≈ 156 KB, fits UB), but the batch path was chosen because "one input read beats the batch path's re-read at that width" was V016's comment *for D=32768*. At D=24576 with only 2 rows the trade-off could go either way; it must be measured.
- **ASCEND_FEASIBILITY:** Pure predicate change; CachedRows already supports `localRows >= 1`.
- **UB/CORE/DMA_IMPACT:** CachedRows at D=24576 needs `valueFp32Buf_ = 24576*4 = 98304` B + 2×`7680*4 = 61440` B + reduce ≈ 157 KB < 192 KB UB. Fits. DMA pattern changes (one input read instead of two).
- **SYNC_IMPACT:** Cache-Rows sync pattern instead of the batch path's event-flag double-buffering.
- **PRECISION_RISK:** None (same arithmetic, different data-flow).
- **WHY_NOT_DUPLICATE:**
  - R31A V017 changed the CachedRows dispatch **D-width** boundary (`D24576..32768`) and was Official-REJECTED (44.45) with a FALSE_POSITIVE local. This hypothesis changes the **row-count** threshold of the *batch* entry — a different predicate, a different axis. V016 changed the D=32768 special case to send multi-row D=32768 to CachedRows; it did not touch the `localRows >= 2` batch threshold.
  - SHAPE-TILING owns tile widths/counts — this changes neither.
  - R31B / MIX-A / DTYPE-SPECIAL-X: no equivalent batch-entry row-threshold change.
- **MINIMAL_OFAT_DIFF:** One boolean constant in the `wideFp32BatchPath_` predicate (`>= 2` → `>= 3`). No kernel body change.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=24576 blocks=1 (would now be CachedRows) and rows=2 D=32768 blocks=1 (still CachedRows), same-binary + interleaved P/C vs V016. This is the one hypothesis that makes the D=24576 probe shape exercise the champion path.

---

## H4 — Pass-1 staging liveness: free the residual slot after the Add

- **Research space:** local data flow (tile-internal data reuse)
- **Where:** `ProcessWideFp32CachedRows` pass 1 (submission.asc:2005-2020). One conceptual mechanism: reassign the existing buffer roles so the residual staging is released earlier, allowing the next tile's residual MTE2 to overlap the current tile's square+reduce.
- **MECHANISM:** Currently per tile: `Add(value[col], x, res) → Mul(res, value, value) → ReduceSum(dst, src=res, ws=x, n)` then `SyncVToMTE2` — both `xBuf_` and `residualBuf_` stay live through `ReduceSum`, so the next tile's `Load(x)`/`Load(res)` wait for the whole V chain. Change to:
  1. `Add(value[col], x, res)` — both staging slots free after this.
  2. Issue `Load(res_next)` into `residualBuf_` immediately (MTE2 starts).
  3. `Mul(xBuf_, value[col], value[col])` — square into `xBuf_` (reusing the freed x slot).
  4. `ReduceSum(dst, src=xBuf_, ws=valueTail, n)` — workspace is the unwritten tail of `valueFp32Buf_` (past `col+valid`), so `residualBuf_` stays free and the `res` load keeps running.
  5. After `ReduceSum`, `xBuf_` (square) is free → issue `Load(x_next)`.
- **BOTTLENECK:** Per-tile MTE2/V serialization in the RMS first pass. The residual DMA cannot overlap the square+reduce because the residual buffer is held as the `ReduceSum` source and x as its workspace.
- **EXPECTED_SHAPES:** rows=2 D=32768 blocks=1 (5 tiles/row → 4 boundaries where the overlap pays). Also single-row CachedRows.
- **WHY_IT_MAY_HELP:** Hides part of the next residual DMA behind the current tile's `Mul+ReduceSum`. Same UB footprint as V016 (no added buffer pair).
- **WHY_IT_MAY_FAIL:** The overlap window is one `Mul+ReduceSum` of one tile; on a DMA-bound kernel this is small. The value-tail workspace must not alias the live `value[col]` region; the last tile has no unwritten tail and must fall back to the current `x`-workspace pattern.
- **ASCEND_FEASIBILITY:** `ReduceSum` accepts an explicit workspace tensor; using a disjoint slice of `valueFp32Buf_` is legal as long as it does not overlap the live `value[col..col+valid]` region or the source.
- **UB/CORE/DMA_IMPACT:** Same UB bytes (no new buffer). Enables one earlier MTE2 issue per tile.
- **SYNC_IMPACT:** The single `SyncVToMTE2` per tile is replaced by two finer releases (residual slot after `Add`, x slot after `ReduceSum`), using `SetFlag`/`WaitFlag` on `V_MTE2` events — the same event vocabulary the batch path already uses (submission.asc:2227-2242).
- **PRECISION_RISK:** None (same ops, same order on `value[col]`; the square and reduce inputs are identical).
- **WHY_NOT_DUPLICATE:**
  - R31A **V020** targeted the *same pass-1 MTE2/V overlap goal* but via a **different mechanism**: it added a second staging pair (`kWideFp32CachedStageElems = tile/2` dual slots) plus four event IDs, and it correctness-failed (507035). This hypothesis adds **no** buffer pair — it only reassigns the existing x/res roles and uses the value-tail as the reduce workspace, so the same-footprint liveness change is a distinct mechanism from V020's dual-slot prefetch. V020 is retained as a failed revision and is not mixed in.
  - R31A **V021** moved the `SyncMTE3ToV` (pass-2 output wait). This is pass-1 input-staging liveness.
  - ASYNC-TRIPLE-X owns full MTE2/V/MTE3 triple overlap; this is one boundary (residual release) in one function.
  - UB-LIVENESS-X owns buffer lifetime/aliasing as a *design axis* on its own parent code; this is a minimal same-footprint role flip in R31A's CachedRows, not a live-set/aliasing redesign.
  - R31B / DTYPE-SPECIAL-X / MIX-A: no such pass-1 role reassignment.
- **MINIMAL_OFAT_DIFF:** In `ProcessWideFp32CachedRows` pass 1 only: change which buffer receives the square and which receives the `ReduceSum` workspace, and split the `SyncVToMTE2` into a residual release after `Add` plus an x release after `ReduceSum`. Tile size, arithmetic, and pass 2 unchanged.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768 blocks=1, same-binary + interleaved P/C vs V016. Secondary D=24576 no-regress control.

---

## Cross-hypothesis uniqueness

| # | Research space | Function / pass | Mechanism in one line | Not-v021 | Not-v020 |
|---|---|---|---|---|---|
| H1 | Rsqrt | CachedRows RMS finalize | `Rsqrt` instead of `Sqrt`+divide | different site (finalize vs store-wait) | different site (finalize vs pass-1 prefetch) |
| H2 | affine | CachedRows pass 2 | hoist row-scale `Muls` out of the tile loop | different (scale placement vs store-wait) | different pass |
| H3 | path selection | `Init` dispatch predicate | batch entry `localRows>=2` → `>=3` | different (dispatch vs sync) | different (dispatch vs prefetch) |
| H4 | local data flow | CachedRows pass 1 | release residual slot after `Add`; value-tail as reduce workspace | different pass | same goal, different mechanism (no added pair) |

None of the four changes a tile constant, fuses the epilogue arithmetic chain, or merges stores.

---

## Recommended order for the next performance revision

1. **H3** first if the goal is to widen the champion path's coverage — it is a one-line predicate change, low risk, and it makes the D=24576 probe shape exercise CachedRows (so H1/H2/H4 become measurable on both shapes).
2. **H1** next — smallest, cleanest, pure Rsqrt substitution, directly on the only V021-exercising shape.
3. **H2** after H1 (they both touch CachedRows but different passes; do not combine in one revision).
4. **H4** last — most intricate sync/event change; do it only after H1/H2 are measured, and re-verify correctness carefully given V020's failure on a related mechanism.

Every revision is OFAT from **V016**, declared with the standard block before any code change.
