# R31A — Track-B Next Hypotheses v3 (batch path)

Date: 2026-09-29
ROUTE: R31A (Champion Conservative Evolution)
Lineage base: **V026** (LOCAL_BEST, source SHA `7f2029e478d7675e02c2e017b0db3eaf33ae5dc0fa478479b340f441ab46bb18`)
Context class: `HISTORICAL_EXPLOIT`
Scope: 2–4 non-duplicate single-mechanism hypotheses, per MAIN-1 focused on the **batch path** (`ProcessWideFp32Batched`, rows=2 D<32768). Read-only research. No revision is declared here; wait for Main to select.

**V026 state:** CachedRows pass-1 staging liveness (H4) + pass-2 parameter staging liveness (H5) + V024's invRms row-scale hoist. Combined ≈ −8.3% vs V016. The CachedRows affine/dataflow space is worked out (H4/H5/H6 done or exhausted). This round targets the batch path, the other live code region.

---

## Batch-path code map (V026 `ProcessWideFp32Batched` + `ComputeWideFp32InvRmsPipelined`)

```text
per batch (kWideFp32BatchRows = 8 rows; rows=2 → batchRows=2):
  RMS phase (per row, ComputeWideFp32InvRmsPipelined):
    double-buffered tile loop: Add(value,x,res) → Mul(square) → ReduceSum(partial)
      → SetFlag<V_MTE2>(release)            [NO PipeBarrier before SetFlag — see H9 evidence]
    cross-tile ReduceSum → finalize (Duplicate + Sqrt on xLocal, 2 V/S round trips, scalar 1.0f/)
  affine phase (per tile, shared gamma/bias across rows):
    Load gamma/bias (once per tile) + Load first row's x/res
    per batchRow:  Add(value,x,res) → PipeBarrier → SetFlag<V_MTE2>(inputRelease)
                   [refill next row's x/res]
                   Muls(value,invRms[row]) → PipeBarrier
                   Mul(value,gamma) → PipeBarrier
                   Add(value,bias) → PipeBarrier → SetFlag<V_MTE3>(outputReady)
                   WaitFlag<V_MTE3> → Store → SetFlag<MTE3_V>(outputRelease)
    SyncVToMTE2()   [release gamma/bias for next tile]
```

UB budget at D=24576 batch: 8 buffers × kWideFp32BatchTileElems(6112) × 4 B = 195584 B, limit 196608 B, **free ≈ 1 KB**. A second gamma/bias staging pair needs 48896 B — no room. Every hypothesis below must fit in existing UB.

**Key structural fact:** the batch path reads x/res **twice** (once in the RMS phase, once in the affine phase). This re-read is the known inefficiency versus CachedRows' single-read value cache, but removing it would require caching the full row (the CachedRows architecture) — not a clean batch-path OFAT.

---

## H9 — Remove the PipeBarrier before SetFlag in the batch-path affine loop

- **Research space:** local data flow / sync (barrier reduction in the batch affine chain).
- **Where:** `ProcessWideFp32Batched` affine tile loop, two sites: `Add(value,x,res) → PipeBarrier → SetFlag<V_MTE2>(inputRelease)` and `Add(value,bias) → PipeBarrier → SetFlag<V_MTE3>(outputReady)`. One conceptual mechanism: the `PipeBarrier` immediately before a `SetFlag<HardEvent::V_*>` is redundant because the V pipe's in-order execution already completes the preceding V op before the SetFlag is processed.
- **MECHANISM (with in-source evidence):** The same function's own RMS tile loop does `ReduceSum(...) → SetFlag<V_MTE2>(releaseEvent)` **with no PipeBarrier** (submission.asc:2235-2236). That pattern shows a `SetFlag<HardEvent::V_*>` can follow a V op directly and still correctly signal "V done with the staging". The affine loop's two `PipeBarrier` immediately before `SetFlag` are inconsistent with this and are therefore claimed redundant. Removing them lets the SetFlag (and the following MTE2 refill / MTE3 Store) issue earlier. The remaining three `PipeBarrier`s in the affine (Muls→Mul, Mul→Add, and none needed after Add-bias once the 4th is gone) stay — those are V→V dependencies on `valueLocal`.
- **BOTTLENECK:** per-tile-per-row barrier overhead in the affine chain. At D=24576 rows=2 the affine runs 5 tiles × 2 rows = 10 iterations, each with 2 candidate barriers → up to **20 barriers** removed.
- **EXPECTED_SHAPES:** rows=2 D=24576 (the batch path; this is the only probe shape exercising `ProcessWideFp32Batched`). Also any rows=2 D<32768.
- **WHY_IT_MAY_HELP:** V024 (H2) removed 5 per-tile barriers in CachedRows and gained −3.66%. If each barrier costs ~0.05–0.1 µs, 20 removals could be ~1–2 µs of a ~14.5 µs kernel = 7–14%. Even if barriers are cheap, the removal is on the critical path of the affine chain.
- **WHY_IT_MAY_FAIL:** if Ascend C's `SetFlag` does **not** implicitly wait for prior V ops (i.e. it is on a control pipe that can run ahead), the barrier is load-bearing and correctness fails (507035-class or a wrong-value). The RMS loop's `ReduceSum → SetFlag` pattern is the evidence against this, but it is not proof. Correctness-first is mandatory.
- **ASCEND_FEASIBILITY:** `SetFlag`/`WaitFlag` on `HardEvent::V_MTE2`/`V_MTE3` are the existing vocabulary in this function. Removing a `PipeBarrier` is a one-line deletion per site.
- **UB/CORE/DMA_IMPACT:** no UB change. No DMA change. Fewer V-pipe drain points.
- **SYNC_IMPACT:** removes 2 `PipeBarrier<PIPE_V>` per tile per row (the ones immediately before `SetFlag<V_MTE2>` and `SetFlag<V_MTE3>`). The `SetFlag`/`WaitFlag` pairs themselves are unchanged. The V→V barriers (Muls→Mul, Mul→Add) are unchanged.
- **PRECISION_RISK:** none — no arithmetic change.
- **WHY_NOT_DUPLICATE:**
  - H6/V027 was the **pass-1/pass-2 boundary** staging-release timing in CachedRows (a different site and a different mechanism: buffer-role + release placement). H9 is a **barrier-count** reduction inside the batch affine loop.
  - H4/V025 (pass-1 staging liveness) and H5/V026 (pass-2 staging liveness) are CachedRows prefetch/release mechanisms; the batch path already double-buffers and does not use them.
  - V024 (H2) hoisted the invRms Muls in CachedRows; the batch path's Muls is per-tile-per-row (tile-sized value, cannot hoist) — different mechanism.
  - V021 (pass-2 MTE3 store-drain wait), R31B H1 (store drain), STORE-EPILOGUE-X H1 (blocked writeback), EPILOGUE-ARITH (Mul/Add fusion) all untouched.
- **MINIMAL_OFAT_DIFF:** in the `ProcessWideFp32Batched` affine tile loop only: delete the `AscendC::PipeBarrier<PIPE_V>();` immediately before each `SetFlag<AscendC::HardEvent::V_MTE2>(inputRelease)` and `SetFlag<AscendC::HardEvent::V_MTE3>(outputReady)`. No arithmetic, store, dispatch, or tile-constant change. (A fuller variant deletes both; the minimal first step may delete only the `V_MTE3` site and leave the `V_MTE2` site for a follow-up.)
- **EXPECTED_LOCAL_PROBES:** rows=2 D=24576, same-binary + interleaved P/C vs V026 on d6 (gap=5s). No-regress control: rows=2 D=32768 (CachedRows, batch edit not exercised — same-code noise band).

---

## H10 — Batch RMS finalize onto the reduce buffer (H6 cross-path analog)

- **Research space:** local data flow / sync (cross-path application of H6's mechanism).
- **Where:** `ComputeWideFp32InvRmsPipelined`'s finalize (submission.asc:2254-2266). One conceptual mechanism: move the finalize's 1-element `Duplicate + Sqrt` from `xLocal[0]` (`xBuf_`) onto `reduceLocal[0]` (`reduceFp32Buf_`, free after the cross-tile ReduceSum), so the x/res staging can be released before the finalize and the affine phase's first x/res load can overlap the finalize's V/S round trips.
- **MECHANISM:** same as H6/V027 but in the batch path's RMS function. The finalize arithmetic is unchanged (same `Duplicate + Sqrt + 1.0f/`); only the buffer holding the 1-element intermediates changes and the staging release moves earlier.
- **BOTTLENECK:** the RMS finalize's 2 V/S round trips sit on the critical path before the affine phase (per row).
- **EXPECTED_SHAPES:** rows=2 D=24576 (the batch path).
- **WHY_IT_MAY_HELP:** hides the affine phase's first x/res load behind the finalize's V/S tail. But this is **one overlap per batch** (not per tile), so the effect is small.
- **WHY_IT_MAY_FAIL:** H6/V027 showed this exact mechanism is within noise in CachedRows (−0.70% median, band ±1%). The batch path's boundary is once per batch (fewer opportunities than CachedRows' once per row). Expectation is low.
- **ASCEND_FEASIBILITY:** `reduceFp32Buf_` is 8 floats and free after the cross-tile ReduceSum; the 1-element ops fit. The staging release is a plain sync-placement change.
- **UB/CORE/DMA_IMPACT:** no UB change. One earlier MTE2 issue per batch.
- **SYNC_IMPACT:** the `SyncVToMTE2` moves earlier (right after the `squareSum` pull instead of after `invRms`).
- **PRECISION_RISK:** none — same finalize value, different buffer.
- **WHY_NOT_DUPLICATE:**
  - H6/V027 is the **CachedRows** finalize (different function). This is the **batch path's** finalize. MAIN-1 permits a cross-path mechanism applied once per path ("每路径只动一处"). The mechanism is the same; the site is different.
  - V023 restructured the finalize's **arithmetic** (vector-pipe meanSquare + Rsqrt); H10 keeps the arithmetic unchanged. Different mechanism.
  - H9 is a barrier-count change in the affine loop; H10 is a buffer-role + sync-placement change in the RMS finalize. Different sites and mechanisms.
- **MINIMAL_OFAT_DIFF:** in `ComputeWideFp32InvRmsPipelined` only: point `Duplicate`/`Sqrt` at `reduceLocal[0]` instead of `xLocal[0]`, and move the `SyncVToMTE2` to right after the `squareSum` `GetValue`. No arithmetic change.
- **EXPECTED_LOCAL_PROBES:** rows=2 D=24576, same-binary + interleaved P/C vs V026. Low expectation (H6 was within noise).

---

## H11 — Fold invRms into gamma per row in the batch affine (V024 analog) — UB-blocked / trade-off

- **Research space:** affine (gamma/bias application), batch-path analog of V024's Muls hoist.
- **Where:** `ProcessWideFp32Batched` affine tile loop, the `Muls(value, invRms[row]) → Mul(value, gamma)` pair. One conceptual mechanism: precompute `gammaScaled = gamma * invRms[row]` and apply `Mul(value, gammaScaled) + Add(value, bias)`, removing one V op and one barrier from the **value** path per tile per row (the `Muls` moves onto the gamma buffer and can overlap the `Add(value,x,res)`).
- **MECHANISM:** `Add(value,x,res)` and `Muls(gammaScaled, gamma, invRms[row])` are independent (different buffers) and can run in parallel; then `Mul(value, gammaScaled) → Add(value, bias)`. The value chain drops from 4 ops / 4 barriers to 3 ops / 3 barriers; the `Muls` is off the value critical path.
- **UB BUDGET (computed):** `gammaScaled` is tile-sized (6112 floats = 24448 B). The batch path's free UB is ≈ 1 KB. **No room** for a dedicated `gammaScaled` buffer.
- **Trade-off variant:** overwrite `gammaBuf_` with `gammaScaled` in place. But gamma is **shared across the batch rows**; overwriting it with row-0's scaled version forces a gamma reload per row, destroying the parameter reuse (the batch path's core strength). At rows=2 that is +5 tile-sized gamma loads (≈ 120 KB extra MTE2) against a saving of ~10 V ops + ~10 barriers — the MTE2 cost likely exceeds the V saving.
- **VERDICT:** `INFEASIBLE` as a clean OFAT at the current tile width and UB budget (needs either a new buffer, which is UB-blocked, or a parameter-reuse sacrifice, which is a net loss). Documented so the idea is not silently retried. It reopens only if a tile-policy change frees UB (out of scope) or the batch size drops to 1 row (then there is no parameter reuse to lose — but rows=2 is the probe shape).
- **WHY_NOT_DUPLICATE:** V024 (H2) hoisted the Muls in **CachedRows** where the full-row value buffer allowed a one-shot full-row scale. The batch path's value is tile-sized so the same hoist needs a per-row `gammaScaled` — a different constraint. BATCH-RESIDENT-X owns gamma/bias residency on the A001 FastKernel path; this is scoped to R31A's batch path and concludes UB-blocked there.
- **EXPECTED_LOCAL_PROBES:** none while infeasible.

---

## Cross-hypothesis uniqueness

| # | space | function | mechanism in one line | vs H6/V027 | vs H4/H5 (V025/V026) | vs V024 (H2) | feasible? |
|---|---|---|---|---|---|---|---|
| H9 | sync / data flow | batch affine loop | delete the `PipeBarrier` before `SetFlag` (2 sites) | different (barrier count vs boundary release) | different (batch, no prefetch) | different (barrier vs Muls hoist) | yes, with correctness risk |
| H10 | sync / data flow | batch RMS finalize | finalize 1-element ops onto the reduce buffer (H6 analog) | same mechanism, different path (cross-path, once per path) | different | different | yes, low expectation |
| H11 | affine | batch affine loop | fold invRms into gamma per row | different | different | same goal, different constraint (tile-sized value) | **INFEASIBLE** (UB) |

H9 and H10 are the two viable entries; H11 is documented `INFEASIBLE` so it is not retried.

---

## Recommended order for the next performance revision

1. **H9** first — the potentially significant lever (up to 20 barrier removals at D=24576 rows=2), backed by the RMS loop's own `ReduceSum → SetFlag` pattern. Correctness-first: if the barrier turns out to be load-bearing, correctness fails and that is the answer.
2. **H10** only if H9's result leaves room — it is the H6 analog with a low expectation (H6 was within noise) and is a cross-path application rather than a fresh lever.
3. **H11** is not to be attempted without a tile-policy change.

Every revision is OFAT from **V026**, declared with the standard block before any code change, and the paired harness must be rebuilt with the parent module from the declared DIRECT_PARENT (V026) for the OFAT comparison.

---

## If no hypothesis is pursued

If Main judges H9 too risky and H10 too weak, the honest reading is that the batch path (like CachedRows) is near its clean-OFAT ceiling at the current tile width and UB budget. In that case flag **LANE_NEEDS_PLANNING_REVIEW** with: Local Best chain V016→V024→V025→V026 (≈ −8.3%) established; V026 is the Online candidate (WORTHY, package ready); the assigned research space (CachedRows affine/dataflow, and now the batch path's clean OFAT) is exhausted; and the batch path's remaining idea (H11) is UB-blocked. Planning then decides: push Online, or reassign/close. **Do not self-PARK.**
