# TRACK-B HYPOTHESES V003 — EPILOGUE-FUSE-X (MAIN-2 R2)

ROUTE=EPILOGUE-FUSE-X
REVISION_TARGET=V003
DIRECT_PARENT=FROZEN_R31B_V011 (roll back from V002; V002 LOCAL_REJECTED)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_EPILOGUE_DATAFLOW
TRACK=Track-B read-only research; no kernel edit in this document.

---

## 0. LESSONS FROM V001 / V002

| rev | mechanism | result | lesson |
|---|---|---|---|
| V001 SCALE-FOLD | move normalize multiply to gamma scratch | ±5% noise band | relocating V passes without cutting them is invisible |
| V002 VMLA in-place + bias reload | `MulAddDst` into bias, reload per row | **+3.4% regression** (6/6 pairs) | per-row param reload + `SyncVToMTE2` tax exceeds the fused-instruction gain |

**Constraint for V003:** cut a full-tile UB writeback or V pass on the post-RMS path **without any parameter reload, without extra sync primitives, and without dtype split**.

---

## 1. WHERE THE WRITEBACKS LIVE (frozen parent)

FP32 second pass, per tile (after `invRms` is obtained):

```text
Muls(valueTile, valueTile, invRms, valid)     // UB write #1 on valueTile
PipeBarrier
Mul (valueTile, valueTile, gammaSlice, valid) // UB write #2 on valueTile
PipeBarrier
Add (valueTile, valueTile, biasSlice, valid)  // UB write #3 on valueTile
PipeBarrier
Store(outputGm_, ..., valueTile, valid)       // UB read, GM write
```

**3 full-tile UB writebacks per tile.** BF16 adds a 4th (`FromFloat`). The arithmetic `y·invRms·gamma + bias` needs at minimum 2 vector ops (multiply + add); the normalize is a 3rd. Any fusion must reduce the op count on the value tile without adding a preload writeback elsewhere.

---

## 2. DISPATCH REALITY CHECK

`cacheParams = cacheRow && localRows > 1` (L245). This splits the second-pass sites:

| condition | params | bias lifetime | vmla in-place safe? |
|---|---|---|---|
| `localRows == 1` (most shapes) | loaded per tile | single-use per tile | **YES — free** |
| `localRows > 1` (64×8192 etc.) | cached across rows | reused across rows | NO — needs reload (V002's failure) |

`ProcessFp32FullRowOutputPipelined` only runs when `localRows > 1` (needs `rowCount > coreCount ≈ 40`). The **generic tiled second pass** (`Process()` L427–461) handles all `localRows == 1` shapes and is the wider-reaching target.

---

## 3. HYPOTHESES

---

### HYPOTHESIS-V003-1 — VMLA-INPLACE-UNCACHED: vmla into per-tile-loaded bias

**MECHANISM**
Replace `Mul(valueTile, gammaSlice)` + `Add(valueTile, biasSlice)` with a single
`MulAddDst(biasSlice, valueTile, gammaSlice)` (`vmla: bias += y_norm·gamma`),
using the **per-tile-loaded bias buffer as the accumulator**. The normalize
`Muls(valueTile, valueTile, invRms)` is unchanged. Result: 2 V passes per tile
instead of 3, and 1 UB writeback on the value tile instead of 3 (the vmla
writes to biasLocal, which is reloaded next tile anyway).

```text
// before (per tile):
Muls(valueTile, valueTile, invRms); Mul(valueTile, valueTile, gamma); Add(valueTile, valueTile, bias);
// after (per tile):
Muls(valueTile, valueTile, invRms);
MulAddDst(biasLocal, valueTile, gammaLocal);   // bias = bias + y_norm·gamma
Store(..., biasLocal, valid);                   // store from accumulator
```

Arithmetic identity: `bias + (y·invRms)·gamma` — identical to the original
`(y·invRms)·gamma + bias`. FP32 intermediates throughout. Uniform for every `T`
where `MulAddDst` accepts the dtype pair (`float,float` / `half,half`).

**Why no reload tax:** this applies ONLY to the `cacheParams == false` arm
(`localRows == 1`), where `Load(biasLocal, biasGm_, col, valid)` runs every
tile. Destroying the accumulator is free — the next tile reloads it.

**EXPECTED_BOTTLENECK**
Three serialized PIPE_V passes with three full-tile UB writebacks on the value
tile per tile. The `Mul` + `Add` pair is two dependent passes that can be fused
into one `vmla` when the accumulator is single-use.

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` (frozen parent), `cacheParams == false`
arms only:
- `Process()` tiled second pass L434–438 (FP32 uncached) and L457–460 (BF16 uncached
  uses `xFp32`/`residualFp32` as gamma/bias FP32 — vmla into `residualFp32`).
- `ProcessNarrowMidOverlap` L581–584 (FP32 non-resident) and L598–604 (BF16).
- `ProcessSmallLowPrecisionContiguousBatched` non-broadcast per-row arm (L1765–1773).
- Half-domain uncached arm (L446–450): `MulAddDst(biasLocal, outputLocal, gammaLocal)`
  in `Tuple<half,half>`.
No change to cached-params arms (`cacheParams == true`), broadcast helpers,
wide paths, or host code.

**WHY_ORTHOGONAL_TO_MAIN1 (DTYPE-SPECIAL-X)**
Pure FP32/FP32 (or half/half) instruction fusion on the existing accumulator.
No `if constexpr` dtype split is added — the `MulAddDst` call uses the same-type
pair the site already uses. No conversion helper is touched. DTYPE-SPECIAL-X
owns per-dtype arithmetic/copy specialization; this changes the **affine issue
form** uniformly. The bias buffer is an existing per-tile load target, not a
copy-elision pattern.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- VECTOR-MATH-X / REDUCE-HIER-X: invRms derivation untouched.
- COEFF-LOCALITY-X / BATCH: no parameter DMA, residency, or stripe change.
- UB-CHAMPION-X: no buffer lifetime change; biasLocal is an existing buffer.
- ASYNC-TRIPLE-X: store loop and MTE3 events byte-identical.
- V002 VMLA: that was in-place into **cached** bias with reload; this is
  in-place into **per-tile-loaded** bias with no reload. Different lifetime.

**EXPECTED_WIN_SHAPES**
- All `localRows == 1` shapes hitting the generic tiled second pass:
  2×8192, 4×8192, 8×8192 FP32 (the shapes the Main listed).
- `ProcessNarrowMidOverlap` non-resident arm: 1×256, 2×257.
- Est. 8–15% epilogue V-pass reduction (3→2 per tile), translating to
  ~5–10% kernel-level on epilogue-bound short kernels.

**EXPECTED_RISK_SHAPES**
- `localRows > 1` (64×8192): not affected (cached-params arm unchanged).
  Use as zero-delta control.
- FP16 uncached: `MulAddDst` in `Tuple<half,half>` — half-domain accumulation.
  The result is `bias_half + y_norm_half·gamma_half`, which is the same
  rounding as the original `Mul`+`Add` in half. No precision change.

**CORRECTNESS_RISK**
Low. The vmla computes `bias + y_norm·gamma` in one rounding vs the original's
`y_norm·gamma` then `+ bias` (two roundings). FP32 difference is ≤1 ulp. Half
difference is ≤1 half-ulp. Must pass the full 54-case battery.

**MEASUREMENT_PLAN**
1. Same-binary Parent + V003 on 2×8192 FP32 and 4×8192 FP32 (VMLA-active shapes)
   and 64×8192 FP32 (zero-delta control), warmup=45, device events.
2. One revision: only `Mul`+`Add` → `MulAddDst` in the uncached arms.
   `SINGLE_CHANGE_AUDIT` must show no cached-arm or wide-path edit.
3. ≥6 interleaved P/C pairs on 2×8192 and 4×8192; control must be ~0.
4. Falsifier: if the uncached shapes show no drop while the V-pass count
   visibly fell, the epilogue is not the bottleneck — stop.

---

### HYPOTHESIS-V003-2 — PRELOAD-VMLA-ALTERNATING: accumulator copy + vmla for cached params

**MECHANISM**
For the `cacheParams == true` arm (multi-row cores), replace `Mul`+`Add` with
`Adds(outTile, biasSlice, 0.0f)` + `MulAddDst(outTile, valueTile, gammaSlice)`,
where `outTile` alternates between two dead FP32 scratch buffers
(`reduceFp32Buf_`, `residualFp32Buf_`) matching the double-buffered store.
No bias destruction, no reload. The preload `Adds` is a bias **copy**, not a
destroy-and-reload.

```text
// per tile (2-tile row):
Muls(valueTile, valueTile, invRms);          // 1 V, valueTile
Adds(outTile, biasLocal[col], 0.0f);          // 1 V, outTile = bias
MulAddDst(outTile, valueTile, gammaLocal[col]); // 1 V, outTile = bias + y_norm·gamma
Store(..., outTile);
```

3 V passes per tile (same count as frozen), but the `Adds`+`vmla` pair writes
`outTile` twice instead of `valueTile` three times — **one fewer full-tile
writeback on the hot value tile**. The `Adds` can also issue while the previous
tile's MTE3 store drains (different buffer), giving partial overlap.

**EXPECTED_BOTTLENECK**
On multi-row cores, the affine still takes 3 V passes per tile but concentrates
2 of them on a cold scratch, leaving `valueTile` with only the normalize
writeback.

**FILES/FUNCTIONS TO TOUCH**
`ProcessFp32FullRowOutputPipelined` pass 2 (L1210–1214), `ProcessBf16FullRowOutputPipelined`
(L972–976), `ProcessFp16FullRowOutputPipelined` (L1090–1096) cached arms.
Accumulators: `reduceFp32Buf_.Get<float>()` and `residualFp32Buf_.Get<float>()`
(alternating by `useFirst`), both dead after pass 1.

**WHY_ORTHOGONAL_TO_MAIN1**
Same vmla instruction, same FP32 intermediates, no dtype split. The accumulator
is an existing dead scratch, not a new buffer or a copy-elision helper.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
No param DMA change (unlike V002). No UB lifetime model change. Store events
identical. Same non-duplication as V003-1.

**EXPECTED_WIN_SHAPES**
- 64×8192 FP32 (the VMLA path). Est. 3–8% from one fewer value-tile writeback
  plus partial store overlap.

**EXPECTED_RISK_SHAPES**
- Single-tile rows: no double-buffer alternation, one accumulator — same count.
- If `reduceFp32Buf_`/`residualFp32Buf_` have a residual race with pass 1's
  `Duplicate`/`Sqrt`/`GetValue`, correctness will fail (the V002 diagnostic
  suspected this; the fix is to use only `reduceFp32Buf_` + `residualFp32Buf_`
  and avoid `xFp32Buf_`).

**CORRECTNESS_RISK**
Medium. Must verify the scratch buffers are truly dead after `SyncSToV()`.
The V002 diagnostic failed on out-tile management; this hypothesis uses
different scratch choices and no bias reload to isolate the issue.

**MEASUREMENT_PLAN**
1. Correctness first (54-case battery), with explicit 64×8192 max-abs check.
2. Same-binary + P/C on 64×8192 only (the VMLA-active shape).
3. Falsifier: if 64×8192 still regresses, the vmla pattern itself is
   unprofitable on this hardware — drop the entire vmla direction.

---

### HYPOTHESIS-V003-3 — TILE-WIDE VALUE PASS: one value-tile writeback for normalize+affine

**MECHANISM**
Keep `Mul`+`Add` (no vmla) but fuse the normalize into the gamma multiply by
building `scale = gamma·invRms` in a dead scratch, then applying
`Mul(valueTile, valueTile, scaleSlice)` + `Add(valueTile, valueTile, biasSlice)`.
The value tile is written **twice** (Mul + Add) instead of three times
(Muls + Mul + Add). The scale build writes the scratch once per tile (or once
per row if hoisted).

```text
// per tile:
Muls(scaleTile, gammaSlice, invRms, valid);   // 1 V, scaleTile (cold scratch)
PipeBarrier
Mul(valueTile, valueTile, scaleTile, valid);   // 1 V, valueTile
PipeBarrier
Add(valueTile, valueTile, biasSlice, valid);   // 1 V, valueTile
PipeBarrier
Store(..., valueTile, valid);
```

3 V passes per tile (same count) but **2 value-tile writebacks instead of 3** —
the normalize moves off the hot tile. This is V001's fold expressed as a
writeback-reduction claim rather than a critical-path claim.

**EXPECTED_BOTTLENECK**
Three full-tile UB writebacks on the value tile; the normalize is the only one
that can move to a different operand.

**FILES/FUNCTIONS TO TOUCH**
All non-wide second-pass sites where `Muls(valueTile, invRms)` precedes
`Mul(valueTile, gammaSlice)`: `Process()` L427–437, `ProcessNarrowMidOverlap`
L579–584, `ProcessFp32FullRowOutputPipelined` L1210–1214, batched per-row arms.
Scale scratch: `reduceFp32Buf_` (dead in pass 2).

**WHY_ORTHOGONAL_TO_MAIN1**
Arithmetic identity `(y·a)·b → y·(a·b)`, FP32 intermediates, uniform across T.
No dtype split. Same orthogonality as V001.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
Same as V003-1. V001 was this mechanism measured as a critical-path change
(noise band); V003-3 reclaims it as a writeback-count change with the
scratch choice justified by pass-1 liveness.

**EXPECTED_WIN_SHAPES**
- 2×8192, 4×8192, 8×8192 FP32 (generic tiled): 1 fewer value-tile writeback.
- Est. 3–8% if UB write bandwidth is the limiter.

**EXPECTED_RISK_SHAPES**
- If UB write bandwidth is not the bottleneck (V001's result), this will also
  land in the noise band.

**CORRECTNESS_RISK**
Low (same reassociation as V001, measured max_abs 1e-7..1e-6).

**MEASUREMENT_PLAN**
1. Same-binary + P/C on 2×8192 and 4×8192 FP32.
2. One revision: only the normalize fold. Scale scratch must be `reduceFp32Buf_`
   (not `xFp32Buf_`).
3. Falsifier: if no signal beyond noise, UB writebacks are not the limiter.

---

## 4. SUMMARY TABLE

| ID | Single variable | est. win | Risk | Readiness |
|---|---|---|---|---|
| **V003-1 VMLA-INPLACE-UNCACHED** | affine issue form (Mul+Add → vmla) in the per-tile-loaded bias arm | **8–15%** epilogue, 5–10% kernel on `localRows==1` shapes | low (no reload, no sync) | **READY_FOR_MAIN_REVIEW** |
| V003-2 PRELOAD-VMLA-ALTERNATING | accumulator copy + vmla for cached params, 1 fewer value-tile writeback | 3–8% on 64×8192 | med (scratch liveness) | NEEDS_MORE_EVIDENCE |
| V003-3 TILE-WIDE VALUE PASS | normalize moves off the value tile (writeback count 3→2) | 3–8% if UB bandwidth-bound | low (V001 re-measured) | NEEDS_MORE_EVIDENCE |

---

## 5. RECOMMENDATION

**V003 = HYPOTHESIS-V003-1 (VMLA-INPLACE-UNCACHED).**

Reasons:
1. Cuts a full V pass **and** two full-tile value-tile writebacks per tile
   (3→1 value-tile writeback) — the strongest writeback reduction available
   without FMA hardware.
2. Zero parameter reload, zero extra sync — the exact tax that killed V002.
3. Applies to the widest shape set (`localRows == 1` generic tiled second pass:
   2×8192, 4×8192, 8×8192 — the shapes the Main listed).
4. Same `MulAddDst` primitive as V002 but in the **single-use accumulator**
   regime where it is free.
5. H2 needs scratch-liveness validation first; H3 is V001 re-measured with a
   different claim.

Minimal OFAT diff (conceptual, uncached FP32 arm):

```text
// before:
Muls(valueTile, valueTile, invRms, valid);
Mul(valueTile, valueTile, gammaLocal, valid);
Add(valueTile, valueTile, biasLocal, valid);
// after:
Muls(valueTile, valueTile, invRms, valid);
MulAddDst(biasLocal, valueTile, gammaLocal, valid);
// Store changes source: valueTile → biasLocal
```

---

## 6. REQUEST_MAIN_ROUTE_RECORD

REQUEST_MAIN_ROUTE_RECORD: **HYPOTHESIS-V003-1 — VMLA-INPLACE-UNCACHED.**

Requested disposition: record V003-1 as the single hypothesis for performance
revision V003 of EPILOGUE-FUSE-X, parent FROZEN_R31B_V011 (roll back from V002;
PARENT_SOURCE_SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`).
Scope: `cacheParams == false` second-pass arms only (per-tile-loaded bias).
kernel edit follows the selected hypothesis in the current specification. V003-2 and V003-3
stay in backlog with their stated prerequisites.
