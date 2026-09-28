# R31A — Track-B Next Hypotheses v2 (from V025)

Date: 2026-09-29
ROUTE: R31A (Champion Conservative Evolution)
Lineage base: **V025** (LOCAL_BEST, source SHA `1475116735f390b426fc694d45fcdcbc676ed0758ef326d54d8c43d35e16b4fe`)
Context class: `HISTORICAL_EXPLOIT`
Scope: 3–5 non-duplicate single-mechanism hypotheses in R31A's own research space (affine / path selection / local data flow). Read-only research. No revision is declared here; wait for Main to select.

**V025 state (what is already in):** `ProcessWideFp32CachedRows` pass-1 staging liveness (residual slot freed after Add, next residual prefetched, value tail as reduce workspace when large enough) + pass-2 invRms row-scale hoisted to one full-row `Muls` before the tile loop. Combined ≈ −6.8% vs V016 (chain V016 → V024 → V025).

**Parent rule:** the next revision parents from **V025** (the new LOCAL_BEST). Per the adopted global rule, the paired harness must be rebuilt with the parent module from the declared DIRECT_PARENT for the OFAT comparison.

---

## Hard constraints for this round

- **No Rsqrt / Reciprocal** (V023 proved `Rsqrt` is an approximate ~2^-10 primitive on 910B3 and breaks the 1e-4 FP32 budget).
- **Dedup vs R31B H1** (store drain deferral) and **STORE-EPILOGUE-X H1** (blocked writeback): do not touch store drain or store merging.
- **Dedup vs R31B H3** (pass-1/pass-2 sync reduction): direction 3 below must state how its mechanism differs.
- **EPILOGUE-ARITH** owns the epilogue arithmetic chain (Mul/Add fusion into FMA). Do not fuse the affine chain.
- **SHAPE-TILING** owns tile policy. Do not change `kWideFp32CachedTileElems` / any tile constant.
- **UB budget at D=32768 (V025 CachedRows):** value cache 131072 B + x staging 30720 B + residual staging 30720 B + reduce 32 B = **192544 B**, limit 196608 B, **free ≈ 4 KB**. A second gamma/bias staging pair needs 61440 B — no room unless the value cache or staging shrinks (SHAPE-TILING).

---

## Key code map (V025 `ProcessWideFp32CachedRows`)

```text
pass 1 (per tile):  Load x, Load res (or prefetched) → Add(value[col],x,res)
                    → Mul(square into x slot) → [tail?] ReduceSum(ws=value tail)
                      else Mul(square into res slot) → ReduceSum(ws=x slot)
                    → SyncVToMTE2
cross-tile:         ReduceSum(residualLocal, reduceLocal, xLocal, tileCount)
                    → SyncVToS → GetValue(squareSum) → scalar meanSquare
                    → SyncSToV → Duplicate(xLocal,·) + Sqrt(xLocal) → SyncVToS
                    → invRms = 1.0f/GetValue → SyncSToV → SyncVToMTE2
pass 2 (V024 hoist):Muls(valueLocal, valueLocal, invRms, rowWidth) → PipeBarrier
                    per tile: Load(gamma→xBuf_, bias→resBuf_) → SyncMTE2ToV
                      → Mul(value[col],gamma) → Add(value[col],bias)
                      → SyncVToMTE2 → SyncVToMTE3 → Store → SyncMTE3ToV
```

---

## H5 — Pass-2 gamma/bias staging early release + next-tile prefetch

- **Research space:** local data flow (pass-2 parameter staging liveness) + affine (where the gamma/bias load is placed relative to the V apply).
- **Where:** `ProcessWideFp32CachedRows` pass 2 tile loop (the `Load(gamma)/Load(bias) → SyncMTE2ToV → Mul → Add` block). One conceptual mechanism: split the per-tile staging release so the next tile's gamma/bias MTE2 overlaps the current tile's V apply and Store.
- **MECHANISM:** Currently per tile `Load(gamma→xBuf_, bias→resBuf_) → SyncMTE2ToV → Mul(value[col],gammaLocal=xBuf_) → PipeBarrier → Add(value[col],biasLocal=resBuf_) → PipeBarrier → SyncVToMTE2 → SyncVToMTE3 → Store → SyncMTE3ToV`. Both staging slots stay live through `Mul+Add`, and the next `Load` waits for the whole tail. Change to:
  1. After `Mul` reads `gammaLocal` (`xBuf_`), release `xBuf_` and issue the next tile's `Load(gamma)` into it (MTE2 runs while `Add` + `Store` proceed).
  2. After `Add` reads `biasLocal` (`resBuf_`), release `resBuf_` and issue the next tile's `Load(bias)` into it (MTE2 runs while `Store` proceeds).
  Same UB footprint — no second staging pair, just earlier release of the two existing slots. This is the pass-2 analog of H4/V025's pass-1 residual release.
- **BOTTLENECK:** per-tile MTE2/V/MTE3 serialization in the output pass. The gamma/bias load cannot overlap the value apply because the staging is held through `Mul+Add`.
- **EXPECTED_SHAPES:** rows=2 D=32768 (the only probe shape exercising CachedRows; 5 tiles → 4 boundaries where the overlap can pay).
- **WHY_IT_MAY_HELP:** hides the next tile's gamma/bias DMA (a 60 KB pair) behind the current tile's `Add`+`Store`. Comparable in magnitude to H4 (pass-1 residual prefetch gave ≈ −4% on V024).
- **WHY_IT_MAY_FAIL:** the overlap window is `Add`+`Store` (shorter than H4's `Mul+ReduceSum` window). On a DMA-bound kernel the effect may be ~0. The early-release sync split adds a V→MTE2 round trip that could eat part of the gain (same trade-off H4 accepted).
- **ASCEND_FEASIBILITY:** the split-release uses `SetFlag`/`WaitFlag` on `HardEvent::V_MTE2` (the vocabulary `ProcessWideFp32Batched` already uses) or two blanket `SyncVToMTE2` placements. Both are proven in R31A.
- **UB/CORE/DMA_IMPACT:** no UB change (4 KB free, untouched). One earlier MTE2 issue per tile on the parameter path.
- **SYNC_IMPACT:** the single per-tile `SyncVToMTE2` becomes a gamma release after `Mul` plus a bias release after `Add` (two finer releases instead of one blanket). `SyncMTE2ToV` / `SyncVToMTE3` / `SyncMTE3ToV` unchanged.
- **PRECISION_RISK:** none — same ops on the same values; only the staging lifetime changes.
- **WHY_NOT_DUPLICATE:**
  - V025/H4 = **pass-1** residual staging release (different pass, different buffer).
  - V020 = pass-1 dual-slot prefetch that **added a second staging pair** + four event IDs and correctness-failed 507035. H5 adds **no** pair and is pass-2.
  - V021 = moved the pass-2 `SyncMTE3ToV` (store-drain wait). H5 does not touch the store drain; it only frees the parameter staging earlier. **R31B H1 (store drain deferral)** and **STORE-EPILOGUE-X H1 (blocked writeback)** are therefore untouched — H5 is a parameter-load placement, not a store-side change.
  - EPILOGUE-ARITH (Mul/Add fusion) untouched: `Mul` and `Add` remain separate with their barriers; the affine expression order is unchanged.
- **MINIMAL_OFAT_DIFF:** in the pass-2 tile loop only: replace the one `SyncVToMTE2` with a gamma release after `Mul` and a bias release after `Add`, and issue the next tile's `Load(gamma)`/`Load(bias)` at those release points. No arithmetic, store, dispatch, or tile-constant change.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768, same-binary + interleaved P/C vs V025 on d6/d4 (gap=5s). No-regress control rows=2 D=24576 (batch path, same-code).

---

## H6 — Release x/res staging before the RMS finalize (pass-1/pass-2 boundary)

- **Research space:** local data flow (buffer-role reassignment at the pass boundary) + sync reduction between pass 1 and pass 2.
- **Where:** the cross-tile ReduceSum → RMS finalize → `SyncVToMTE2` block between pass 1 and pass 2. One conceptual mechanism: move the finalize's 1-element vector ops off `xBuf_` onto the already-allocated `reduceFp32Buf_`, so `xBuf_`/`residualBuf_` can be released before the finalize and pass 2's first gamma/bias load overlaps the finalize's V/S round trips.
- **MECHANISM:** Currently the cross-tile `ReduceSum(residualLocal, reduceLocal, xLocal, tileCount)` uses `xBuf_` as workspace and `residualBuf_` as dst; then the finalize does `Duplicate(xLocal, meanSquare, 1) + Sqrt(xLocal, xLocal, 1)` on `xBuf_`, and only after `invRms` is pulled does `SyncVToMTE2` free the staging for pass 2. Change to: run the `Duplicate + Sqrt` on `reduceLocal[0]` (the 8-float reduce buffer already holds the partials and is free after the cross-tile reduce), and move the `SyncVToMTE2` to right after the `squareSum = residualLocal.GetValue(0)` pull. Then `xBuf_`/`residualBuf_` are free while the finalize's `meanSquare` math + `Sqrt` + `invRms` run, and pass 2's first `Load(gamma/bias)` issues into the freed staging and overlaps that finalize (which contains 2 V→S round trips).
- **BOTTLENECK:** the pass-1 → pass-2 boundary serializes a full staging release behind the RMS finalize's scalar round trips. The first parameter load cannot start until `invRms` is computed.
- **EXPECTED_SHAPES:** rows=2 D=32768 (the only probe shape exercising CachedRows).
- **WHY_IT_MAY_HELP:** hides the first tile's gamma/bias load behind the finalize's 2 V/S round trips and scalar math (a few hundred ns of stalls per row).
- **WHY_IT_MAY_FAIL:** the finalize's scalar tail may already be hidden under other work; the saving is one overlap per row (not per tile), so the effect is smaller than H5's per-tile overlap.
- **ASCEND_FEASIBILITY:** `reduceFp32Buf_` is 8 floats and already allocated; a 1-element `Duplicate`/`Sqrt` fits. The staging release is a plain `SyncVToMTE2` placement change.
- **UB/CORE/DMA_IMPACT:** no UB change. One earlier MTE2 issue per row (the first gamma/bias load).
- **SYNC_IMPACT:** the `SyncVToMTE2` moves earlier (after `squareSum` instead of after `invRms`). The V→S / S→V round trips of the finalize are unchanged.
- **PRECISION_RISK:** none — the same `Duplicate + Sqrt + 1.0f/x` arithmetic on the same value, just held in `reduceLocal[0]` instead of `xLocal[0]`.
- **WHY_NOT_DUPLICATE:**
  - V023 restructured the finalize's **arithmetic** (vector-pipe `Muls+Adds` meanSquare + `Rsqrt`, later corrected to `Sqrt`). H6 keeps V016's finalize arithmetic **unchanged** and only changes **where** the 1-element ops live and **when** the staging is released. Different mechanism (buffer role + sync placement vs arithmetic).
  - **R31B H3 (pass-1/pass-2 sync reduction)** is the adjacency MAIN-1 flagged. H6's specific mechanism is "move the 1-element finalize ops onto the reduce buffer so the x/res staging is freed before the finalize, letting the first pass-2 parameter load overlap the finalize". If R31B H3 reduces a different sync (e.g. a V↔S pair, or a barrier inside a pass), the two are distinct. State the exact sync edge each touches at declaration time; if R31B H3 turns out to be the same staging-release edge, drop H6 or re-scope it.
  - V021 = pass-2 `SyncMTE3ToV` (store side). Untouched.
- **MINIMAL_OFAT_DIFF:** in the cross-tile/finalize block only: point `Duplicate`/`Sqrt` at `reduceLocal[0]` instead of `xLocal[0]`, and move the `SyncVToMTE2` to immediately after the `squareSum` `GetValue`. No arithmetic change, no store/dispatch/tile change.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768, same-binary + interleaved P/C vs V025. No-regress control rows=2 D=24576.

---

## H7 — Pass-2 parameter double-staging via the already-stored value region

- **Research space:** local data flow (value-cache region reuse as a second parameter staging pair).
- **Where:** `ProcessWideFp32CachedRows` pass 2 tile loop. One conceptual mechanism: use the **already-stored** head of the value cache as a second gamma/bias staging pair so the next tile's parameter load fully overlaps the current tile's compute (a wider window than H5's early-release).
- **MECHANISM:** In pass 2 the value cache is read-only (`Mul`/`Add`/`Store` read `value[col]`; nothing writes it after the hoisted `Muls`). Once tile `k` is stored, the head region `value[0 … k·valid)` is free. Use it as the staging for the **next** tile's gamma/bias while `xBuf_`/`residualBuf_` hold tile `k`'s parameters — a true two-slot parameter pipeline without new UB. Per tile: apply from `xBuf_`/`resBuf_`, prefetch tile `k+1`'s gamma/bias into the stored-value head, then swap roles.
- **BOTTLENECK:** same as H5 — per-tile parameter MTE2 serialized with the V apply — but H5's window is only `Add`+`Store` per buffer, while H7 overlaps the whole `Mul`+`Add`+`Store` of the current tile with a full gamma+bias pair load.
- **EXPECTED_SHAPES:** rows=2 D=32768.
- **UB FEASIBILITY (computed):** a gamma/bias pair is 2×7680×4 = 61440 B. The stored-value head grows by one tile (30720 B) per stored tile. It only reaches 61440 B after tile 2. So the double-staging window exists for tiles 2→3 and 3→4 (2 of 4 boundaries). Tiles 0→1 and 1→2 have a short head and fall back to H5's early-release (or the original pattern). No new UB is allocated; the head is memory the value cache already owns.
- **WHY_IT_MAY_HELP:** a full parameter-pair overlap for the later tiles, on top of whatever H5 gives for the earlier ones.
- **WHY_IT_MAY_FAIL:** only 2 of 4 boundaries qualify at D=32768; the role-swap bookkeeping is more intricate than H5 and carries a higher sync-fault risk (V020's class of failure). The gain over H5 alone may be small.
- **ASCEND_FEASIBILITY:** reusing a UB region as MTE2 staging is legal as long as its live data is already consumed. The stored-value head is dead (its tiles are already written out to GM).
- **UB/CORE/DMA_IMPACT:** no UB change. One extra parameter-pair prefetch for the later tiles.
- **SYNC_IMPACT:** needs a two-slot prefetch handoff (which staging holds the current vs next parameters) — more sync bookkeeping than H5.
- **PRECISION_RISK:** none (same arithmetic).
- **WHY_NOT_DUPLICATE:**
  - H5 = early-release of the **existing** two slots (no second location). H7 = a **second staging location** from the stored-value head (a different mechanism: double-staging vs liveness release). They are alternatives in the same research space; **pick one or the other per revision, not both**.
  - V020 = a second staging pair that **added** UB (dual-slot) in **pass-1** and correctness-failed. H7 adds no UB and is pass-2.
  - STORE-EPILOGUE-X (store merge) / R31B H1 (store drain) untouched — H7 never touches the Store.
- **MINIMAL_OFAT_DIFF:** in the pass-2 tile loop: introduce a second parameter-staging view over the stored-value head and a current/next role swap; arithmetic, store, dispatch, tile constants unchanged.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=32768, same-binary + interleaved P/C vs V025. Use only after H5's result is known (H5 first; H7 only if H5's window proves insufficient).

---

## H8 — Cross-row gamma/bias tile residency in CachedRows (UB-budget analysis)

- **Research space:** affine (parameter residency across rows).
- **Where:** `ProcessWideFp32CachedRows` output pass, across the `localRows` loop. One conceptual mechanism: load each gamma/bias tile **once** and apply it to every row's corresponding tile, instead of reloading the same gamma/bias per row.
- **MECHANISM:** gamma/bias are row-invariant. At `localRows=2` (rows=2 blocks=1) the current row-outer loop reloads every gamma/bias tile twice. A tile-outer output pass (or a persistent gamma/bias tile cache) would halve the parameter MTE2.
- **UB BUDGET (computed):** keeping both rows' value tiles live while holding one gamma/bias tile needs 2×rowWidth×4 + 2×tile×4 = 262144 + 61440 = **323584 B** against a 196608 B limit — over by ~124 KB. A half-row value-cache layout still needs the x/res staging (61440 B) and totals ~254552 B — still over. The gap only closes if the tile width drops (SHAPE-TILING) or the full-row value cache is given up (which abandons CachedRows' single-input-read identity and moves toward the batch path).
- **VERDICT:** `INFEASIBLE` at the current tile width and UB budget. Documented so the idea is not silently retried. A future variant is only open if a tile-policy change (out of scope here) or a different value-cache layout is sanctioned.
- **WHY_NOT_DUPLICATE:** BATCH-RESIDENT-X owns gamma/bias residency on the A001 FastKernel path; this analysis is scoped to R31A's CachedRows and concludes UB-blocked there. R31B's low-precision wide pipeline has its own parameter handling.
- **EXPECTED_LOCAL_PROBES:** none while infeasible.

---

## Cross-hypothesis uniqueness

| # | space | pass | mechanism in one line | vs H4/V025 | vs V020 | vs V021 / R31B-H1 | vs V023 |
|---|---|---|---|---|---|---|---|
| H5 | local data flow / affine | pass 2 | early-release the two parameter slots, prefetch next gamma/bias | different pass (2 vs 1) | no added pair | no store-drain change | different (staging vs finalize arithmetic) |
| H6 | local data flow / sync | pass 1→2 boundary | move finalize's 1-element ops to the reduce buffer, release staging before the finalize | different site (boundary) | different | no store-drain change | different (placement vs arithmetic) |
| H7 | local data flow | pass 2 | second parameter-staging view over the stored-value head | different pass + different mechanism | no added UB, pass 2 | no store-drain change | different |
| H8 | affine residency | pass 2 (rows) | one gamma/bias tile per core across rows | different | — | — | INFEASIBLE (UB) |

H5 and H7 are **alternatives** in the same research space (pass-2 parameter prefetch). Declare one at a time; H5 is the safer first try (H4-proven pattern), H7 only if H5's window is too small.

---

## Recommended order for the next performance revision

1. **H5** first — the pass-2 analog of the H4 change that already delivered −4%. Cleanest, same-footprint, low precision risk.
2. **H6** next — different site (the pass boundary) and a different mechanism (sync placement + buffer role). Small but compounding with H5.
3. **H7** only if H5's per-buffer window proves insufficient — it is the stronger but riskier variant; V020's failure mode lives in this class of two-slot prefetch bookkeeping, so correctness must be watched closely.
4. **H8** is documented `INFEASIBLE` at the current tile width; do not attempt without a sanctioned tile-policy change.

Every revision is OFAT from **V025**, declared with the standard block before any code change, and the paired harness must be rebuilt with the parent module from the declared DIRECT_PARENT (V025) for the OFAT comparison.
