# SEQ-FUSE-2 SPEC — INLINE-RECIPROCAL DENOMINATOR

Date: 2026-09-27 (C2C overnight; research/spec only — NOT implemented)
Route: VECTOR-MATH-X (Track-B spec)
Status: approval-ready specification. Kernel edit forbidden until the
DIV_FEASIBILITY_PROBE below passes and Main approves a revision.
Parent of record: FROZEN R31B-V011 (`R31B-V011-LP-ROW-PIPELINE_kernel.asc`)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

HYPOTHESIS_ID=SEQ-FUSE-2

## MECHANISM

Run the entire denominator tail in the V pipe: per-row mean-square math,
`Sqrt`, and the reciprocal (`Div`) stay in UB; the tail's **2 V/S
round-trips and the scalar divide disappear**. Apply the UB-resident
invRms to the value tiles with a broadcast `Mul` (0 pulls; 1 pull +
`Muls` kept as the fallback form if the broadcast apply fails probe).

Frozen-parent tail today (per row):

```text
ReduceSum(reduceDest, ...)            // V
SyncVToS()                            // round trip 1
squareSum = reduceDest.GetValue(0)    // pull
meanSquare = squareSum * invRowWidth + epsilon   // scalar mul + add
SyncSToV()                            // push
Duplicate(xFp32, meanSquare, 1)       // push
Sqrt(xFp32, xFp32, 1)                 // V
PipeBarrier
SyncVToS()                            // round trip 2
invRms = 1.0f / xFp32.GetValue(0)     // pull + scalar divide
SyncSToV()                            // push
Muls(valueTile, valueTile, invRms, valid)        // scalar-source apply
```

SEQ-FUSE-2 tail:

```text
ReduceSum(reduceDest, ...)                    // V (unchanged)
Muls(meanSqSlot, reduceDest, invRowWidth, 1)  // V: meanSquare
Adds(meanSqSlot, meanSqSlot, epsilon, 1)      // V
Sqrt(meanSqSlot, meanSqSlot, 1)               // V: denominator
PipeBarrier
Div(invRmsSlot, onesSlot, meanSqSlot, 1)      // V: invRms = 1/denominator
PipeBarrier
// apply — Form A (target, 0 pulls):
Mul(valueTile, valueTile, bcastSlot, valid)   // bcastSlot = invRms broadcast
// apply — Form B (fallback, 1 pull):
//   SyncVToS(); invRms = invRmsSlot.GetValue(0); SyncSToV();
//   Muls(valueTile, valueTile, invRms, valid)
```

One conceptual variable: **denominator-tail placement** (scalar handoff
chain vs pure-V chain). No change to reduction topology, epsilon, invD
value, tile geometry, DMA, or scheduling. Form A vs Form B is the
apply step of the same fused tail and is settled by the feasibility
probe, not by a second revision.

The reciprocal slot needs a 32B-aligned, dedicated 8-float UB slot
(`onesSlot` = {1.0f×8}, `meanSqSlot`, `invRmsSlot`, `bcastSlot` — all
base-aligned; 1-element V ops at 4B offsets are known to abort with
ACL 507035 on this target, see probe).

## BOTTLENECK_EVIDENCE (frozen R31B-V011 denominator sites)

Two V/S round-trips + scalar mean-square + scalar divide per row. Live
sites (function : first round-trip : second round-trip : scalar divide):

| function | round trip 1 | round trip 2 | scalar divide |
|---|---|---|---|
| `Process` generic per-row | `:358` | `:365` | `:366` |
| `ProcessNarrowMidOverlap` | `:565` | `:572` | `:573` |
| `ProcessBf16FullTileBatchedOutputPipelined` (per batchRow) | `:685` | `:698` | `:701` |
| `ProcessFp16FullTileBatchedOutputPipelined` (per batchRow) | `:807` | `:820` | `:823` |
| `ProcessBf16FullRowOutputPipelined` | `:945` | `:952` | `:953` |
| `ProcessFp16FullRowOutputPipelined` | `:1063` | `:1070` | `:1071` |
| `ProcessFp32FullRowOutputPipelined` | `:1192` | `:1199` | `:1200` |
| `ProcessSmallFp32Batched` (per batchRow) | `:1421` | `:1434` | `:1437` |
| `ProcessSmallFp32FullTileBatched` (per batchRow) | `:1518` | `:1531` | `:1534` |
| `ProcessSmallFp32ContiguousBatched` (per batchRow) | `:1618` | `:1631` | `:1634` |
| `ProcessSmallLowPrecisionContiguousBatched` (per batchRow) | `:1730` | `:1743` | `:1746` |
| `ProcessWideFp32FullCacheRows` (per batchRow) | `:2146` | `:2153` | `:2154` |
| `ProcessWideLowPrecision` (per batchRow) | `:3233` | `:3240` | `:3240`-region |

Same-shape sites in the uncalled functions (`ProcessWideFp32CachedRows`
`:1895/:1902`, `ComputeWideFp32InvRms[Pipelined]` `:1957/:2064`-region,
`ProcessWideFp32PanelResident`, `ProcessWideFp16/Bf16CachedRows`) are
dead code in this parent (V003 collapsed the wide dispatch); they are
out of scope.

Per-row cost being removed: 4 pipe handoff barriers, 2 scalar pulls, 1
push-back duplicate, 1 scalar mul + 1 scalar add + 1 scalar divide. On
batched rows the chain repeats per `batchRow` (`:685-701` pattern).

## DIV_FEASIBILITY_PROBE (mandatory before any kernel edit)

The previous V-side `Div` attempt hit **ACL_FAIL sync=507035** when
`dst == src1` or when the `onesSlot` buffer was used. This probe tests
`Div` correctness and the 507035 avoidance form **only** — it is a
feasibility micro-probe, NOT a performance revision: no warmup/samples
protocol, no P/C pairs, no timing statistics, no submission candidate.

Probe object: a standalone tiny kernel (separate build target, e.g.
`div_probe`) with three dedicated UB slots — `onesSlot[8] = {1.0f×8}`,
`denomSlot[8]`, `outSlot[8]` — all 32B-aligned bases, plus one deliberate
4B-offset alias for the hazard case. Host compares device results to a
double-precision `1.0f / x` reference and records device RC.

Test matrix (run each on d4 or d5, one shot each):

| # | form | expectation |
|---|---|---|
| P1 | `Div(outSlot, onesSlot, denomSlot, 8)`, all distinct + aligned | must PASS (RC=0, ulp ≤ 1) |
| P2 | `Div(denomSlot, onesSlot, denomSlot, 8)` (dst == src1) | expect 507035 — confirms the hazard |
| P3 | `Div(outSlot, onesOffset, denomSlot, 8)` where `onesOffset` starts at +4B | expect 507035 — confirms the alignment root cause |
| P4 | `Div(outSlot, onesSlot, denomSlot, 1)` distinct + aligned | must PASS (count=1 form) |
| P5 | P1 repeated with denominator sweep: normal, 1e-6, 1e+6, exact zero-guarded `sqrt(eps)` region | ulp ≤ 1 vs scalar reference |

Pass criteria: **P1, P4, P5 RC=0 and max |ulp| ≤ 1** (scalar `1.0f/x` and
V `Div` are IEEE-equivalent within 1 ulp; anything wider blocks on
precision), and P2/P3 reproduce 507035 as predicted (if they instead
pass, the earlier failure form is misattributed and the probe notes it).

Fallback if P1 fails on this toolchain: the seq-fuse still works with
Form B (single pull + scalar divide removed only if `Div(count=1)`
works — otherwise SEQ-FUSE-2 stops; see STOP_CONDITION).

Probe acceptance artifact: `phase4/local/VECTOR-MATH-X/SEQ-FUSE-2-PROBE/`
with per-form RC + ulp table. Probe verdict authorizes the kernel edit; it
takes no timing lease beyond a one-shot device run.

## TARGET_SHAPES (medium band critical)

Per-row handoff region — the Official weak cases 7/6/4/8 class:

| band | shape | role |
|---|---|---|
| medium | 4x2048 FP32 | primary (V002 partial measured -14.0% 5/5 here) |
| medium | 8x1024 FP32 | primary (V002 partial -9.2% 3/3) |
| medium | 2x4096 FP32 | primary (V002 partial -6.2%) |
| small | 8x256 FP32 | secondary (V002 partial -4.2%) |
| small | 2x256 FP32 | control |
| large | 1x32768 FP32 | control — same-binary passable (MAD/med 0.02–0.05), tail amortized, expect ≈ 0 |
| large | 1x16384 FP16 | dtype control (lowp tail) |

## EXPECTED_EFFECT

Per row: −2 V/S round-trips, −2 scalar pulls, −1 push duplicate, −3
scalar ops (mul, add, divide). On batched medium rows ×B. Expected
3–8% on medium band if the tail handoff is binding there; ≈ 0 on large
(wide) rows where the tail is amortized over the row. This targets the
per-row handoff region only — orthogonal to STORE-H2B's wide-store win
(case 14 class).

## FILES/FUNCTIONS

`R31B-V011-LP-ROW-PIPELINE_kernel.asc` only (when approved):

1. `Init`: add 4 aligned 8-float UB slots (`onesSlot` init-once with
   `Duplicate(onesSlot, 1.0f, 8)` — compile-time constant, no pull;
   `meanSqSlot`, `invRmsSlot`, `bcastSlot` reuse existing dead-after-
   reduction scratch where lifetimes allow — `reduceFp32Buf_` tail or
   `xFp32Buf_` after the reduction chain, per V002's finding that
   `xFp32Buf_` holds BF16 gamma in some paths and must not be aliased).
2. Per-row denominator tails listed in the evidence table — replace the
   scalar chain with the pure-V chain and the apply form settled by the
   probe.
3. No host / CMake / runner change except the standalone probe target.

## WHY_ORTHOGONAL_TO_MAIN1

- WIDE-X-FRESH4 (wide tile-width): no tile-width or wide-architecture
  change; the wide tail sites are touched only where the same per-row
  chain exists, with no wide-path structure change.
- MODE-X-R015C (row→block mapping / segment copy): no mapping, no
  segment, no DMA change.
- MIX-A (wait placement / sync): the removed handoffs are part of the
  fused arithmetic chain — the win claim is arithmetic placement, not
  wait reordering; every remaining wait keeps its parent position.
- DTYPE-SPECIAL-X (dtype arithmetic / copy elision): the tail is unified
  FP32 for every T (the parent already promotes to FP32 here); no
  `if constexpr` dtype arm is added, no `Adds(x, 0)` copy elision.
- ALIGN-TAIL-X (copy geometry): no DataCopy/Pad selection, no bulk/tail
  split.

## WHY_NOT_DUPLICATE

- **VECTOR-MATH-X V001** (`dbe776f9…`, NEEDS_ONE_MORE_LOCAL): moved the
  mean-square math into V (`Muls`+`Adds`) but kept both V/S round-trips,
  the scalar divide, and `Muls`-with-scalar apply. SEQ-FUSE-2 removes
  the handoffs themselves — different variable (tail placement end-to-
  end vs partial chain move). If V003 is built from frozen parent it
  *subsumes* V001's step as an internal stage of one fused chain, which
  is why the single hypothesis is stated as placement, not as "move one
  more op".
- **VECTOR-MATH-X V002** (VM-H3a partial, MEASUREMENT_BLOCKED): replaced
  scalar `Muls` with `Duplicate`+vector `Mul` and **reverted** the V-side
  `Div` after 507035. SEQ-FUSE-2's apply step is a sub-part of the fused
  tail; the `Div` form is the new element and must clear the probe first.
- **STORE-H2B / STORE-H2B-GATED** (FUTURE_CANDIDATE): UB→GM writeback
  organization; SEQ-FUSE-2 touches no store site.
- **EPILOGUE VMLA / SCALE-FOLD**: post-RMS affine arithmetic, closed by
  BOTTLENECK-NOTE; SEQ-FUSE-2 stops at invRms production + normalize
  apply, no affine change.

## CORRECTNESS_RISK

- **Div precision**: `1.0f/x` scalar vs V `Div` must agree within 1 ulp
  (probe P5 sweep). The runner golden is double-precision based with a
  tolerance; the V001 finding was that `Rsqrt` (fast-approx, max_abs
  0.007) does NOT fit that tolerance — `Div` is exact-IEEE and should,
  but the probe proves it before any kernel edit.
- **507035 form**: probe P2/P3 pin the failing forms; the kernel uses
  only distinct, base-aligned slots (P1 form). Any 1-element V op on a
  4B-offset address is forbidden in the edit (V001's alignment finding).
- **Slot lifetime**: `bcastSlot`/`invRmsSlot` must not alias buffers
  live during the apply (V002 note: `xFp32Buf_` holds BF16 gamma in
  lowp paths). The edit audits each site's scratch liveness before
  choosing the slot.

## LOCAL_MEASUREMENT_PLAN

Protocol: same-binary + interleaved P/C, warmup=45, samples=41,
blocks=2, 6 pairs, device-event primary, one d6/d5 window.

**Short-kernel measurement constraint (open item for Main):**
SHORT-KERNEL-MEASUREMENT-NOTE.md established that 5–6 µs kernels cannot
pass the flat same-binary MAD/median ≤ 0.10 rule (best observed 0.104
over 3 devices, 3 calibration modes; 12 µs kernels pass at 0.02). The
primary shapes (4x2048/8x1024/2x4096) sit in that band. The plan
therefore runs the protocol as-is and records the same-binary statistic
per shape; where the flat rule cannot pass, the paired direction is
reported as **directional evidence** under the note's proposed
length-stratified threshold (≤ 0.25 for < 10 µs) — Main must either
adopt that threshold as policy or treat medium-band P/C as
direction-only. No kernel change is entailed by this choice.

Order: (1) same-binary both sides, all target shapes; (2) P/C on
medium primaries first; (3) small/large controls. Falsifier: if medium
deltas sit inside the measured noise band and large is ≈ 0, the tail
handoff is not binding — close the line.

## STOP_CONDITION

Stop before or during implementation if any of:

1. **DIV_FEASIBILITY_PROBE fails**: P1/P4/P5 not RC=0 with ulp ≤ 1 →
   SEQ-FUSE-2 infeasible on this toolchain; record probe table and
   return to Main (Form B alone does not justify a revision if the
   count=1 Div also fails — the scalar divide would remain).
2. **Precision regression**: NPU correctness (full FP16/BF16/FP32 ×
   small/medium/large matrix) shows any new mismatch vs parent.
3. **Medium band dead**: after the full protocol, all medium primaries
   show deltas inside the noise band with large ≈ 0 — the tail handoff
   is not the binding cost at these lengths; no second revision on the
   same chain.
4. **Main policy blocks measurement**: if the short-kernel threshold
   question is unresolved and the flat rule keeps medium shapes
   unmeasurable (MEASUREMENT_BLOCKED as in V002), stop and wait for the
   policy decision rather than burning more device windows.

## DRAFT REVISION-DECLARATION — VECTOR-MATH-X V003 (not implemented)

```
ROUTE=VECTOR-MATH-X
REVISION=V003
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS_ID=SEQ-FUSE-2
SINGLE_HYPOTHESIS=Inline-reciprocal denominator: the per-row denominator
  tail runs entirely in the V pipe (mean-square Muls+Adds, Sqrt, Div
  reciprocal) with no V/S round-trip and no scalar divide; invRms is
  applied to the value tiles from UB (broadcast Mul, or single-pull
  Muls if probe settles Form B).
PARENT_CHOICE=FROZEN_R31B_V011, NOT V002 — rationale: (a) V002 is
  MEASUREMENT_BLOCKED and never became a validated parent; (b) V002's
  broadcast-mul apply is a sub-part of this fused tail, so stacking
  would bake an unvalidated delta into the parent line while adding no
  independent variable; (c) frozen parent keeps the Official
  calibration base at 45.16 and the single variable (tail placement)
  attributable end-to-end. V001 (dbe776f9) and V002 remain documented
  partials of the same axis.
DIV_FEASIBILITY_PROBE=REQUIRED_BEFORE_EDIT (P1–P5 matrix; see spec)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
WHY_NOT_DUPLICATE_MAIN1=WIDE: no tile-width/wide structure change;
  MODE: no row-block/segment/DMA change; MIX-A: no wait reordering
  (removed handoffs are inside the fused arithmetic chain);
  DTYPE-SPECIAL: no dtype arm, unified FP32 tail; ALIGN-TAIL: no copy
  geometry change.
WHY_NOT_DUPLICATE_MAIN2=VECTOR-MATH V001/V002 are partials of this axis
  (superseded by the fused chain); STORE-H2B untouched (writeback);
  EPILOGUE VMLA/SCALE-FOLD closed (affine arithmetic).
SINGLE_CHANGE_AUDIT=PENDING (one variable: denominator-tail placement)
MAIN_APPROVAL=PENDING
CORRECTNESS_FIX=NONE REQUIRED (Div is IEEE-equivalent to scalar within
  1 ulp; probe proves before edit)
SOURCE_SHA=PENDING
COMPILE_RC=PENDING
LINK_RC=PENDING
CORRECTNESS=PENDING
LOCAL_VERDICT=PENDING
MEASUREMENT=PENDING
```

Awaits: DIV_FEASIBILITY_PROBE execution, Main approval, and the
short-kernel measurement-threshold policy decision. No kernel edit, no
worktree, no timing lease.
