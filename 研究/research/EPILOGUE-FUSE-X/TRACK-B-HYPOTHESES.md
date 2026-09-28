# TRACK-B HYPOTHESES — EPILOGUE-FUSE-X (MAIN-2 R2)

ROUTE=EPILOGUE-FUSE-X
BRANCH=exp/main2-r2-epilogue-fuse
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/EPILOGUE-FUSE-X
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_EPILOGUE_DATAFLOW
SEED_FILE=phase4/workspaces/EPILOGUE-FUSE-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
TRACK=Track-B (read-only research; no kernel / host / CMake / runner edit in this document)

---

## 0. SCOPE LOCK

In scope, the dataflow AFTER the RMS value is obtained:

```text
invRms (scalar, already produced)
  → normalize        (y * invRms)
  → gamma            (per-channel multiply)
  → bias             (per-channel add)
  → output conversion / store
```

Allowed mechanism classes: arithmetic fusion, UB intermediate elimination, instruction fusion, fewer UB round trips.

Out of scope (any hypothesis that needs these is rejected):
reduction topology, row scheduling, mode dispatch, dtype-specific paths,
sync/fence removal as the performance variable, wide-path redesign, multi-row DMA.

Each hypothesis below varies exactly ONE epilogue dataflow variable. No hypothesis
bundles a second performance mechanism.

---

## 1. SEED EPILOGUE INVENTORY (evidence base)

All line numbers refer to `R31B-V011-LP-ROW-PIPELINE_kernel.asc` (3554 lines).

### 1.1 Canonical second-pass chain after `invRms`

| Path | normalize | gamma | bias | convert | store |
|---|---|---|---|---|---|
| FP32 narrow/mid tiled (L427–468) | `Muls(valueTile, invRms)` | `Mul(gamma)` | `Add(bias)` | — | `Store(valueTile)` |
| half narrow/mid tiled (L439–481) | `Muls(valueTile, invRms)` | `Mul` in **half** | `Add` in **half** | `FromFloat` **before** gamma | `Store(outputLocal)` |
| BF16 narrow/mid tiled (L451–490) | `Muls` | `Mul(gammaFp32)` | `Add(biasFp32)` | `FromFloat` after bias | `Store(outputLocal)` |
| `ProcessNarrowMidOverlap` (L579–610) | `Muls(valueLocal, invRms)` | same 3-way T split | same | same | `Store` |
| `ProcessFp32FullRowOutputPipelined` (L1210–1218) | `Muls` per 4096-tile | `Mul` | `Add` | — | `Store` per tile |
| `ProcessBf16FullRowOutputPipelined` (L972–982) | `Muls` per tile | `Mul(gammaFp32)` | `Add(biasFp32)` | `FromFloat(outputTile)` | `Store` |
| `ProcessSmallFp32Batched` (L1441–1453) | per-row `Muls` | per-row `Mul` | per-row `Add` | — | one batched `Store` |
| `ProcessSmallLowPrecisionContiguousBatched` (L1750–1800) | per-row `Muls` | half: `ApplyFp16GammaBiasBatch` (repeat) or per-row `Mul`; BF16: `ApplyFp32GammaBiasBatch` or per-row FP32 `Mul` | same | half: `FromFloat` before gamma; BF16: `FromFloat` after bias | `Store` |

### 1.2 Structural facts that constrain the hypotheses

1. The value-tile affine is always **three dependent vector ops**
   (`Muls` normalize → `Mul` gamma → `Add` bias) before any conversion.
   Each op writes the full tile back to UB. This is the fusion surface.
2. `invRms` is a **per-row scalar**. `gamma` / `bias` are per-channel and are
   often already resident (`cacheParams` / `gammaFp32Buf_`).
3. `valueFp32Buf_` already holds resident `y = x + residual` on the cacheRow
   paths, so the epilogue does not re-read GM. UB intermediates that remain in
   the epilogue are: the value tile write-backs (3), the T-typed convert staging
   (`outputBuf_` or aliased `xBuf_`/`gammaBuf_`), and (BF16) the promoted
   `gammaFp32Buf_` / `biasFp32Buf_`.
4. The half path is the only path whose affine runs **outside FP32**: it
   converts `valueTile` to half first, then applies half-domain gamma/bias.
   BF16 and FP32 keep a unified FP32 affine and convert once at the end.
5. Tail columns use a shrunk `valid = TileLength(remaining)` (L332, L371).
   No software mask exists on the epilogue compute path.
6. Small-batch paths already use vector-repeat broadcast for gamma/bias
   (`ApplyFp16GammaBiasBatch` L1806, `ApplyFp32GammaBiasBatch` L1828) but the
   normalize `Muls` is still issued per row.

### 1.3 What this route deliberately does NOT touch

- `invRms` derivation (`ReduceSum` → `GetValue` → `meanSquare` → `Sqrt` → `1.0f/`).
  That sequence belongs to REDUCE-HIER-X / VECTOR-MATH-X territory.
- gamma/bias load locality, residency, DMA, striping (COEFF-LOCALITY-X / BATCH).
- store event schedule and MTE3 ring (ASYNC-TRIPLE-X).
- buffer lifetime / aliasing model (UB-CHAMPION-X / UB-LIVENESS-X).

---

## 2. HYPOTHESES

Four candidates. Each is one epilogue dataflow variable.

---

### HYPOTHESIS-1 — SCALE-FOLD: fuse normalize into a per-row scale tile

**MECHANISM**
Replace the two full-tile multiplies on the hot value tile

```text
Muls(valueTile, valueTile, invRms)      // normalize
Mul (valueTile, valueTile, gammaSlice)  // gamma
```

with a scale tile built from the already-resident gamma slice, then one multiply
on the value tile:

```text
Muls(scaleTile, gammaSlice, invRms)     // off the value-tile critical path
Mul (valueTile, valueTile, scaleTile)   // single value-tile multiply
Add (valueTile, valueTile, biasSlice)   // unchanged
```

Mathematically identical up to one reassociation rounding:
`(y * invRms) * gamma == y * (invRms * gamma)`.
The scale tile is per row (`invRms` varies per row) and is written into a
scratch that is already dead after pass one (`xFp32` / `residualFp32` slice, or
a dedicated tile-sized scratch where those are still live). On paths with
resident full-width gamma, the scale tile is built **once per row** and consumed
by every column tile of that row. On batched small-row paths the same fold
replaces the per-row normalize `Muls` loop with one scale-build over the batch
followed by the existing batched `Mul` + `Add`.

**EXPECTED_BOTTLENECK**
Three dependent PIPE_V passes over the same value tile, each a full-tile UB
write-back, inside the post-RMS critical path. The normalize multiply is the
only one of the three that can be moved onto a different operand (gamma is
already warm; `y` is not reusable after the epilogue).

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only, second-pass epilogue sites:
`Process()` tiled second pass (L427–437), `ProcessNarrowMidOverlap` (L579–584),
`ProcessFp32FullRowOutputPipelined` (L1210–1214),
`ProcessBf16FullRowOutputPipelined` (L972–976),
`ProcessSmallFp32Batched` (L1441–1448),
`ProcessSmallLowPrecisionContiguousBatched` (L1750–1796).
A small shared helper (`ApplyNormScaleFold`) is allowed if it is the single
mechanism expressed once. No host / CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
The fold is written once for every `T` and operates on FP32 intermediates
(`scaleTile` is FP32; gamma is promoted to FP32 exactly where the seed already
promotes it, and stays in its native form only where the seed already applies
native gamma). It introduces **no** `if constexpr` dtype split, no copy-elision
of `Adds(x, 0)` (DTYPE-SPECIAL-X V001), no aligned-output helper, and no
narrow-row batching. DTYPE-SPECIAL-X owns per-dtype arithmetic/conversion
specialization; this hypothesis is the opposite shape: one fused arithmetic
identity applied uniformly.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- REDUCE-HIER-X: reduction topology only; this starts after `invRms` exists.
- VECTOR-MATH-X / REDUCE-INVSCALE-X (donor): `sqrt`/`rsqrt`/`invD` derivation;
  `invRms` production is untouched and stays a scalar input here.
- COEFF-LOCALITY-X: gamma/bias **load** locality; this does not move, stripe, or
  re-reside any parameter DMA.
- UB-CHAMPION-X / UB-LIVENESS-X: no new buffer lifetime; the scale tile reuses
  a scratch that is already dead in the epilogue (`xFp32` after the uncached
  gamma promotion, L413–425).
- BATCH-RESIDENT-X (evidence only): their fused-epilogue slice also rebuilds
  pass-one and batch ownership; this hypothesis changes only the post-`invRms`
  affine on the existing R31B-V011 paths, with no batch geometry change and no
  multi-row DMA.
- ASYNC-TRIPLE-X: store issue order and MTE3 events unchanged.
- ALIGN-TAIL-X: no copy / pad / tail policy change.

**EXPECTED_WIN_SHAPES**
- Multi-tile rows (`rowWidth` = 8192 full-row paths, mid tiled widths):
  one fewer dependent full-tile pass on the value tile per tile.
- Small batched shapes (`rowWidth` ≤ 4096, `localRows` > 1): per-row normalize
  issues leave the row loop.
- Likely neutral on single-tile single-row shapes (op count is equal; only the
  critical-path operand changes).

**EXPECTED_RISK_SHAPES**
- Very short rows (D ≤ 192) where a 1-element `Muls` on gamma is not cheaper
  than a 1-element `Muls` on `y`.
- Paths where `xFp32`/`residualFp32` are still live during the epilogue (must
  pick a scratch that is genuinely dead, or the fold adds a buffer).

**CORRECTNESS_RISK**
Low–medium. Reassociation `(y*a)*b → y*(a*b)` changes FP32 rounding slightly.
Within the route's stated tolerance this should pass, but the NPU correctness
matrix must be run on all 3 dtypes before any timing. No change to reduction,
`epsilon`, or `invRms` value.

**MEASUREMENT_PLAN**
1. Same-binary qualification of the Direct Parent on the route's probe shapes
   under `local-timing-protocol.md` (device events, warmup ≥ 45, in-process).
2. One revision, one diff: only the fold. `SINGLE_CHANGE_AUDIT` must show no
   other arithmetic edit.
3. Interleaved P/C pairs on: one multi-tile full-row shape (8192), one mid tiled
   shape, one small-batched shape. Judge on paired `device_us` block medians.
4. Falsifier: if the multi-tile shape shows no direction-consistent drop beyond
   the same-binary noise floor while the batched shape moves, the mechanism is
   batch-issue reduction, not value-tile fusion — split the next revision.

---

### HYPOTHESIS-2 — CONVERT-ONCE: unified FP32 affine, single conversion at store

**MECHANISM**
Make the post-`invRms` affine identical for every `T`:

```text
Muls / Mul(gammaFp32) / Add(biasFp32)   // all FP32, value tile
FromFloat(outputLocal, valueTile)       // exactly one conversion
Store(outputGm_, outputLocal)
```

Concretely this removes the half path's early `FromFloat` and its half-domain
`Mul`/`Add` (L439–450, L588–597, L1750–1771), and replaces them with the same
FP32 affine the BF16 path already uses. gamma/bias are promoted to FP32 once
per core using the promotion the seed already performs for BF16
(`gammaFp32Buf_` / `biasFp32Buf_`, L153–156, L267–277). One code shape, no
per-dtype epilogue branch.

**EXPECTED_BOTTLENECK**
Mid-epilogue domain crossing on the half path: the value tile is converted to
half, then two dependent half-domain ops run on the T-typed output staging
buffer. That is a dtype-unique epilogue shape with its own dependent chain and
its own UB write-backs on a second buffer.

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only, the half epilogue arms and the
shared second pass:
`Process()` L439–450 and L475–481, `ProcessNarrowMidOverlap` L588–597,
`ProcessFp16FullRowOutputPipelined`, `ProcessFp16FullTileBatchedOutputPipelined`,
`ProcessWideFp16*` output pass,
`ProcessSmallLowPrecisionContiguousBatched` L1750–1771.
Init may allocate `gammaFp32Buf_`/`biasFp32Buf_` for half the same way it
already does for BF16. No host / CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1**
This hypothesis *deletes* a dtype-specific epilogue path and *creates* a single
unified FP32 intermediate chain. That is exactly the orthogonality condition:
no dtype-specific epilogue paths, unified FP32 intermediates. It does not add
per-dtype copy helpers, does not touch `Adds(x, 0)` elision, and does not
specialize narrow-row batching.
Collision note: DTYPE-SPECIAL-X's Track-B list mentions "FP32 affine/Rsqrt" as
a consideration. This is a **near-border** on wording only — their reserved axis
is dtype-specific specialization and copy elision, while this hypothesis's
declared variable is *unification of the epilogue arithmetic domain*. Main must
re-read `main1-route-selection.md` before approving this one.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- VECTOR-MATH-X: owns `invD/mean/sqrt/rsqrt` math sequence; not the affine.
- UB-CHAMPION-X: buffer lifetimes unchanged except that half's `outputBuf_`
  becomes a convert-only staging (same as BF16's today).
- BATCH / ASYNC / SCHED / ALIGN / COEFF: no batch, store, row, tail, or param
  DMA change.
- Old REDUCE-INVSCALE-X H1 (`Rsqrt`): different chain stage, not re-run here.

**EXPECTED_WIN_SHAPES**
- FP16 shapes, especially small-batched FP16 where `ApplyFp16GammaBiasBatch`
  currently runs in half domain after an early `FromFloat`.
- Any shape where the half-specific arm's extra domain crossing sits on the
  critical path of a short kernel.

**EXPECTED_RISK_SHAPES**
- FP16 shapes whose golden reference is defined on half-domain multiply/add:
  FP32 affine then one rounding is usually closer to the real value but not
  bit-identical to the current half path.
- FP32 shapes: structurally unchanged (already convert-once / no convert) →
  expect zero delta; useful as a control.

**CORRECTNESS_RISK**
Medium. Output values change by half-ULP-level re-rounding on FP16. Must pass
the full 39-case NPU correctness matrix at the route's stated tolerance before
any timing. If FP16 golden is bit-exact against the old half-domain order, this
hypothesis is rejected on correctness rather than performance.

**MEASUREMENT_PLAN**
1. Correctness matrix first (all 3 dtypes), with explicit FP16 max-abs-err
   recording against the current parent output.
2. One revision: only the domain unification. `SINGLE_CHANGE_AUDIT=PASS` requires
   no parallel change to convert helpers used by other routes' ideas.
3. Interleaved P/C on FP16 small-batched and FP16 mid-tiled shapes; FP32 shape
   as a zero-delta control (if FP32 moves, the diff is not single-variable).
4. Falsifier: if FP16 shows no gain and correctness is tolerance-only (not
   bit-exact), keep the parent; the unification alone is not a performance
   mechanism.

---

### HYPOTHESIS-3 — MASK-TAIL: software-mask full-vector epilogue on resident y

**MECHANISM**
For column tails, stop shrinking the epilogue's `valid` count. Instead, keep a
padded-resident `y` (and gamma/bias slices) at an aligned working width, zero
the padded lanes once with `Duplicate` (software mask), then run the entire
post-`invRms` chain at full vector width:

```text
Duplicate(yTailMask, 0) on lanes [D, align(D))
Muls / Mul / Add / FromFloat at full aligned width
Store only the first D elements (DataCopyPad / valid-byte store)
```

This is the R010 + R027 idea-pool donor named in the Track-B brief: manual tail
+ GPU-style fused epilogue after one `u` load, with a software mask. The mask is
FP32 zeros on the resident value tile; the chain itself stays unified FP32.

**EXPECTED_BOTTLENECK**
Partial-length vector issues on irregular widths. `TileLength(remaining)` (L332)
hands the epilogue a short `valid` for the last tile; short issues waste repeat
slots and prevent the affine from running as full-width repeats.

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only, epilogue tail handling:
the second-pass loop's `valid = TileLength(rowWidth - col)` sites
(L370–371, L503, and the wide-path tile tails), plus the store helper
`Store` (L337–338) if a valid-byte / pad store form is needed.
No reduction-site mask (that would change square-sum lanes), no row scheduling,
no host change.

**WHY_ORTHOGONAL_TO_MAIN1**
The mask is a single FP32 mechanism applied identically for every `T`; the
convert and store keep one unified form. It introduces no per-dtype copy helper
and no dtype-keyed batching. Near-border note: DTYPE-SPECIAL-X's backlog
mentions "aligned output" and "irregular narrow-row batching". Those are
copy/batch policies on their side; this hypothesis is compute-width masking on
the epilogue arithmetic only, with the store already allowed to emit exactly `D`
bytes. Main must confirm they have not started an aligned-output epilogue.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- ALIGN-TAIL-X (evidence only): owns **copy** tails (DataCopyPad / bulk+remainder
  DMA). This hypothesis does not change how x/residual/gamma/bias are copied in;
  it changes how the epilogue arithmetic treats already-resident tail lanes.
  Declared as near-border on the word "tail"; mechanisms sit in different layers.
- UB-CHAMPION-X: padding is a mask on existing resident y, not a lifetime
  redesign.
- BATCH: no multi-row DMA; the store still emits one row (or one existing batch
  block) with a valid length.
- REDUCE / VECTOR-MATH: the square-sum path is untouched; masking for the
  reduction is explicitly out of scope (it would change reduction lanes).

**EXPECTED_WIN_SHAPES**
- Irregular widths (D not a multiple of 8/16/32), e.g. the 17×257 family, and
  any shape whose last tile `valid` is far below a full repeat.
- Small D where every row ends in a short tail issue.

**EXPECTED_RISK_SHAPES**
- Already-aligned widths (D multiple of 4096 / 8192): expect zero delta.
- Very large D where one tail tile is negligible: win falls inside noise.

**CORRECTNESS_RISK**
Medium. Padded lanes must never reach the reduction (out of scope here) and must
never be stored. The store must emit exactly the original `D` elements. Any
lane that leaks into the output is a Wrong Answer. FP32 mask zeros are exact, so
the affine on the valid lanes is unchanged.

**MEASUREMENT_PLAN**
1. Correctness on irregular-D cases first (explicitly D=257-type shapes), with
   byte-count and tail-lane checks on the output buffer.
2. One revision: only the epilogue tail policy. No pad-copy change, no
   reduction mask.
3. Interleaved P/C on an irregular-width shape and an aligned-width shape.
   The aligned shape is the control (must be ~zero delta).
4. Falsifier: if irregular-D shows no gain while aligned-D is flat, partial
   `valid` issues are not the bottleneck on this target; park the donor.

---

### HYPOTHESIS-4 — ROW-WIDE-AFFINE: row-wide affine issues, then the existing tiled store

**MECHANISM**
On paths where a full row already sits contiguously in `valueFp32Buf_`
(`kCacheElems` = 8192, two 4096 tiles), run the post-`invRms` affine as
row-wide vector issues over the whole buffer, then keep the current per-tile
store sequence and its events exactly as they are:

```text
// today, per tile t in {0, 1}:
//   Muls(value[t]), Mul(value[t], gamma[t]), Add(value[t], bias[t]), Store(value[t])
// proposed:
Muls(valueLocal,      invRms,   8192)     // one issue
Mul (valueLocal,      gammaLocal, 8192)   // one issue
Add (valueLocal,      biasLocal,  8192)   // one issue
// then unchanged: ForFloat/store per 4096 tile with the same MTE3 events
```

The variable is the **width of the affine issue group** (row-wide vs per-tile),
not the store schedule, not the buffer count, and not the arithmetic identity.

**EXPECTED_BOTTLENECK**
Per-tile epilogue issue groups on full-row paths: two tiles × three dependent
ops each, with a `PipeBarrier` and a store handshake between tiles. The affine
itself does not need tile boundaries; only the store does.

**FILES/FUNCTIONS TO TOUCH**
`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only:
`ProcessFp32FullRowOutputPipelined` epilogue loop (L1203–1225),
`ProcessBf16FullRowOutputPipelined` epilogue loop (L956–989),
and the matching FP16 full-row epilogue if it shares the two-tile layout.
The store loop, event allocation (L1134–1141, L897–904), and release order
must remain byte-identical in behavior.

**WHY_ORTHOGONAL_TO_MAIN1**
Issue width is a pure dataflow/granularity choice in the unified FP32 chain;
no dtype branch is added (BF16 still converts once after the row-wide affine).
DTYPE-SPECIAL-X's copy elision and narrow-row batching are different axes.
FP32 shapes and BF16 shapes take the same structural change.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- ASYNC-TRIPLE-X: they own the MTE3 store ring and issue-order arbitration.
  This hypothesis keeps every store, every `SetFlag`/`WaitFlag`, and every
  release point in place; it only widens the compute issues before the stores.
  Declared near-border on the shared epilogue site; the variables are
  orthogonal (compute issue width vs store schedule).
- BATCH: their fused epilogue is over a B×D **batch block** with different batch
  ownership; this is per-row width on the existing full-row paths.
- UB-CHAMPION-X: no buffer lifetime change; `valueFp32Buf_` is already one
  contiguous row.
- REDUCE / VECTOR-MATH / COEFF / SCHED: untouched.

**EXPECTED_WIN_SHAPES**
- `rowWidth == 8192` FP32 and BF16 full-row paths (exactly two tiles today).
- Any full-row path where the affine is split into N tiles purely because of
  store double-buffering.

**EXPECTED_RISK_SHAPES**
- Single-tile rows (D = 4096 or less): no change.
- If the affine latency now sits fully before the first store, a short shape
  could lose the compute/store overlap the per-tile order provided. That is the
  main failure mode.

**CORRECTNESS_RISK**
Low. The arithmetic per element is unchanged (same ops, same operands, same
order within the row-wide issue). Conversion and store remain per tile. The
only risk is a wrong `calCount` on the row-wide issue (must be exactly
`kCacheElems`, not the tile length).

**MEASUREMENT_PLAN**
1. Same-binary Parent qualification on the 8192 full-row shape (the only shape
   where the variable acts) and on a 4096 shape as zero-delta control.
2. One revision: only affine issue width. Diff must not move any store or event
   line; `SINGLE_CHANGE_AUDIT` checks that literally.
3. Interleaved P/C on 8192 FP32 and 8192 BF16; 4096 control.
4. Falsifier: if 8192 regresses and 4096 is flat, the per-tile order was hiding
   store latency (ASYNC territory) — return to parent and do not re-try as a
   store change under this route.

---

## 3. ORTHOGONALITY TO MAIN-1 DTYPE-SPECIAL-X (per hypothesis)

| Hypothesis | dtype-specific epilogue path created? | Intermediates | Verdict |
|---|---|---|---|
| H1 SCALE-FOLD | No — one fold, every `T` | FP32 scale + FP32 value tile | ORTHOGONAL |
| H2 CONVERT-ONCE | No — removes the half-only arm | Unified FP32 affine, one convert | ORTHOGONAL_BY_UNIFICATION (near-border wording on "FP32 affine") |
| H3 MASK-TAIL | No — one FP32 mask, every `T` | FP32 padded y | ORTHOGONAL (near-border on "aligned output / irregular batching") |
| H4 ROW-WIDE-AFFINE | No — one issue-width change, every `T` | FP32 value tile | ORTHOGONAL |

DTYPE-SPECIAL-X reserved axis (from `main2-r2-route-registry.md`): dtype-specific
specialization, independent FP16/BF16/FP32 paths, copy elision (`Adds(x,0)` →
FP32 aligned local-copy helpers). None of H1–H4 introduces a per-dtype epilogue
path; H2 actively removes the only one present in the seed's post-RMS chain.

---

## 4. DUPLICATE SCREEN vs EXISTING MAIN-2

| Route | Owns | H1 | H2 | H3 | H4 |
|---|---|---|---|---|---|
| SCHED-CHAMPION-X | row ownership / core assignment | clear | clear | clear | clear |
| REDUCE-HIER-X | square-sum reduction topology | clear (starts after invRms) | clear | clear (no reduction mask) | clear |
| REDUCE-INVSCALE-X (donor) | invscale / invRms derivation | clear | clear | clear | clear |
| VECTOR-MATH-X (later) | invD/mean/sqrt/rsqrt sequence | clear | clear | clear | clear |
| COEFF-LOCALITY-X (deferred) | gamma/bias load locality | clear | clear | clear | clear |
| UB-CHAMPION-X (waiting on UB V003 Official) | UB lifetime / alias | clear (reuses dead scratch) | clear | clear | clear |
| BATCH-RESIDENT-X (park) | batch ownership / param residency / multi-row DMA | clear | clear | clear (no multi-row DMA) | clear |
| ASYNC-TRIPLE-X (park) | MTE3 store ring | clear | clear | clear | **near-border** (shared epilogue site; store lines must stay untouched) |
| ALIGN-TAIL-X (evidence) | copy tails / DataCopyPad | clear | clear | **near-border** (compute mask vs copy tail) | clear |

---

## 5. RECOMMENDED FIRST OFAT REVISION

**HYPOTHESIS-1 — SCALE-FOLD.**

Reasons:
1. Smallest single-variable diff: one arithmetic identity in the second pass,
   expressible as one shared helper, no Init / layout / event change required.
2. Cleanest orthogonality to DTYPE-SPECIAL-X (no dtype branch, FP32
   intermediates, no copy or batching policy).
3. Cleanest duplicate screen (every other route's axis is a different chain
   stage or a different layer).
4. Acts on the chain stage this route owns (`normalize × gamma`), which every
   path shares, so one revision produces evidence across dtype and width.
5. H2 needs a correctness-tolerance decision first, H3 needs irregular-D probes
   first, H4 is near ASYNC's store ring. H1 has neither prerequisite.

Minimal OFAT diff sketch (conceptual, not an edit):

```text
// before (every second-pass tile):
Muls(valueTile, valueTile, invRms, valid);
Mul (valueTile, valueTile, gammaSlice, valid);
Add (valueTile, valueTile, biasSlice, valid);

// after:
Muls(scaleTile, gammaSlice, invRms, valid);   // scaleTile = dead scratch
Mul (valueTile, valueTile, scaleTile, valid);
Add (valueTile, valueTile, biasSlice, valid);
```

---

## 6. REQUEST_MAIN_APPROVAL

REQUEST_MAIN_APPROVAL: **HYPOTHESIS-1 — SCALE-FOLD (fuse normalize into a
per-row scale tile; value-tile epilogue becomes Mul + Add).**

Requested disposition: approve H1 as the single hypothesis for the first
performance revision of EPILOGUE-FUSE-X on DIRECT_PARENT FROZEN_R31B_V011
(PARENT_SOURCE_SHA
a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3).
No kernel edit until Main approves exactly one hypothesis. H2 / H3 / H4 stay in
backlog with their stated prerequisites.

---

## 7. SCREENED SET SUMMARY

| ID | Single variable | Readiness | Prerequisite |
|---|---|---|---|
| H1 SCALE-FOLD | where the normalize multiply lands (folded into gamma scale) | READY_FOR_MAIN_REVIEW | none |
| H2 CONVERT-ONCE | when conversion happens (once, at store, unified FP32) | NEEDS_MORE_EVIDENCE | FP16 golden tolerance decision |
| H3 MASK-TAIL | how tail columns are computed (full-width + software mask) | NEEDS_MORE_EVIDENCE | irregular-D probe shapes qualified |
| H4 ROW-WIDE-AFFINE | what width affine issues cover (row-wide vs per-tile) | NEEDS_MORE_EVIDENCE | 8192 shape same-binary; ASYNC non-collision ack |
