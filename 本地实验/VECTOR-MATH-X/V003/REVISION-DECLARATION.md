# REVISION DECLARATION — VECTOR-MATH-X V003

ROUTE=VECTOR-MATH-X
REVISION=V003
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS_ID=SEQ-FUSE-2 (VMX-N1)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
DATE=2026-09-29

## SINGLE_HYPOTHESIS

Inline-reciprocal denominator: the per-row denominator tail runs entirely in
the V pipe (mean-square Muls+Adds, Sqrt, Div reciprocal) with no V/S
round-trip and no scalar divide; invRms is applied to the value tiles from UB
(broadcast apply).

ONE CONCEPTUAL VARIABLE = denominator-tail placement
(scalar handoff chain → pure-V chain + broadcast apply).

## PARENT_CHOICE

FROZEN_R31B_V011, NOT V002. Rationale:
(a) V002 is MEASUREMENT_BLOCKED and never became a validated parent;
(b) V002's broadcast-mul apply is a sub-part of this fused tail, so stacking
    would bake an unvalidated delta into the parent line while adding no
    independent variable;
(c) frozen parent keeps the Official calibration base at 45.16 and the single
    variable attributable end-to-end.
V001 (dbe776f9) and V002 remain documented partials of the same axis.

## DIV SAFETY CONSTRAINT (from probe, mandatory)

- Form: `Div(dstSlot, onesSlot, denomSlot, 8-or-1)`
- dstSlot, onesSlot, denomSlot: three DISTINCT UB slots
- all three 32B-aligned bases (no 4B-offset sub-slices) — 4B offset triggers ACL 507035
- dst must NOT alias either source — dst==src1 is silent data corruption (probe P2)
- onesSlot initialized once per core: `Duplicate(onesSlot, 1.0f, 8)` (host constant, no pull)
- Div on dav-c220 = exact IEEE single-precision division (probe P1/P5 max_ulp=0.000)
- Rsqrt forbidden (fast-approx, max_abs 0.007) — not used

Probe evidence: 研究/VECTOR-MATH-X/DIV-FEASIBILITY-PROBE.md,
本地实验/VECTOR-MATH-X/SEQ-FUSE-2-PROBE/probe-raw-logs.txt
Probe binary SHA 17be3095f5e11c078dad4b2bb29822bd8977ff3206d6231152c52fcdd4e94bd1.

## EXPECTED_SHAPES

- medium: 4x2048 / 8x1024 / 2x4096 FP32 (Official weak cases 7/6/4/8 class) — directional only if same-binary fails
- small: 8x256 / 2x256 FP32
- large control: 1x32768 FP32 (same-binary passable, tail amortized, expect ≈ 0)
- S1 lengthened band (formal verdict shapes, ≥10–12µs row-dense): 16x2048 / 32x1024 / 16x4096 class — to be calibrated by measured kernel length

## UB IMPACT

+4 aligned 8-float UB slots: onesSlot (init-once), meanSqSlot, invRmsSlot, bcastSlot.
Reused dead-after-reduction scratch (reduceFp32Buf_ tail region) where lifetimes allow.
xFp32Buf_ NOT aliased (holds BF16 gamma in lowp paths per V002 finding).
No new full-size buffers.

## DMA IMPACT

None. No DataCopy/Load/Store site is touched.

## SYNC IMPACT

Per row: −2 V/S round-trips (4 pipe handoffs), −2 GetValue pulls, −1 push
Duplicate, −3 scalar ops (mul, add, divide). Remaining PipeBarrier only inside
the V chain. Every remaining wait keeps its parent position (no wait reordering
— orthogonal to MIX-A).

## PRECISION_RISK

Low. Div = exact IEEE single-precision (probe 0 ulp vs double reference),
bit-identical to scalar `1.0f/x` within 1 ulp. ε and invD values unchanged.
No fast-approx op. Unified FP32 intermediates. No dtype arm.

## OFAT / SINGLE_CHANGE_AUDIT

Only the denominator tail (12 live sites) changes from scalar handoff chain to
pure-V chain + broadcast apply. No reduction / store / DMA / scheduling / tiling
/ dtype change. Form A (broadcast apply) vs Form B (single-pull Muls) is the
apply step of the same fused tail, settled by the DIV probe (Form A viable).

## WHY_NOT_DUPLICATE

- VECTOR-MATH-X V001 (dbe776f9): moved mean-square into V but kept both V/S
  round-trips + scalar divide + scalar Muls apply — partial of this axis.
- VECTOR-MATH-X V002 (06095762): broadcast-Mul apply only, Div reverted —
  partial of this axis.
- SEQ-FUSE-2 subsumes both as internal stages of one fused tail.
- STORE-H2B (writeback) untouched; EPILOGUE VMLA/SCALE-FOLD (affine) untouched;
  REDUCE-HIER (reduction topology) untouched; SCHED (row ownership) untouched;
  DTYPE-SPECIAL-X (per-dtype) untouched.

## MEASUREMENT PLAN (Main-approved S1 + S3)

1. compile → link → correctness (full battery) on server3
2. same-binary per exact shape (blocks=2, warmup=45, samples=41, device-event)
3. S1 lengthened row-dense shapes (≥10–12µs) as formal LOCAL_ACCEPTED/REJECTED verdicts
4. interleaved P/C pairs ≥4 groups per verdict shape
5. S3 tail-cost micro-probe as mechanism upper-bound side evidence
6. medium 4x2048/8x1024/8x256: direction-only if same-binary cannot qualify — never LOCAL_BEST
7. 1x32768 control (no-regression)

## STATUS

SOURCE_SHA=f019d099973b8ab4df07741b494376d9807a6d6a0b1686cfc0aca63ee4245bc8
COMPILE_RC=0 (server3 vector_math_x_link)
LINK_RC=0 (link SHA 89d6944873385d38064476d6f2d707f2ef8690c80b51c52fa41970a2e82317a4)
CORRECTNESS=53/54 PASS (1 FAIL resident_wide_colsplit FP32 D=16384 = PRE_EXISTING_PARENT_FAILURE, V002 same case same max_abs 0.44175)
SAME_BINARY=PASS on all 6 S1 shapes (MAD/med 0.013-0.044, drift 0.001-0.019)
LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL
