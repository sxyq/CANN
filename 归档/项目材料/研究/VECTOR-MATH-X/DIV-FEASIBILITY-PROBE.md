# SEQ-FUSE-2 DIV_FEASIBILITY_PROBE — result

Date: 2026-09-27 (C2C overnight; probe only, no performance revision)
Route: VECTOR-MATH-X
Decision: MAIN-ROUTE-RECORD-SEQ-FUSE-2-PROBE.md
Spec: SEQ-FUSE-2-SPEC.md (DIV_FEASIBILITY_PROBE section)
Host: cann-server3, device 4, one-shot per form (isolated process per
form so hazard forms does not take down the others)
Probe binary: `vmx_div_probe` SHA
17be3095f5e11c078dad4b2bb29822bd8977ff3206d6231152c52fcdd4e94bd1

## VERDICT: PASS — safe form identified

**Safe buffer form (record for V003):**

```text
Div(dstSlot, onesSlot, denomSlot, count)
- dstSlot, onesSlot, denomSlot: three DISTINCT UB slots
- all three 32B-aligned bases (no 4B-offset sub-slices)
- count = 8 (or 1) — both exact
- onesSlot initialized once per core: Duplicate(onesSlot, 1.0f, 8)
```

`Div` on dav-c220 is **exact IEEE single-precision division** —
correctly rounded, 0 ulp vs the double-precision reciprocal on every
sweep value. It is NOT fast-approx like `Rsqrt` (which measured 0.007
max_abs and was rejected in V001).

## Test matrix

| # | form | expected | observed | verdict |
|---|---|---|---|---|
| P1 | `Div(out, ones, denom, 8)`, distinct + 32B-aligned | PASS, ulp ≤ 1 | RC=0, **max_ulp=0.000** on all 8 sweep values | PASS |
| P2 | `Div(denom, ones, denom, 8)` (dst == src1) | 507035 | RC=0 but **silent corruption** — garbage results (ulp 8e5–5.8e10), no abort | hazard CONFIRMED (different failure mode than predicted: data corruption, not sync abort) |
| P3 | `Div(out, ones[1], denom, 8)` (ones at +4B offset) | 507035 | **ACL_FAIL sync=507035**, process exit 13 | hazard CONFIRMED exactly as predicted |
| P4 | `Div(out, ones, denom, 1)`, distinct + aligned | PASS, ulp ≤ 1 | RC=0, element 0 exact (0.800000012 vs 0.8, ulp=0); elements 1–7 undefined (count=1 writes only out[0] — probe artifact, not a failure) | PASS |
| P5 | P1 over denominator sweep | ulp ≤ 1 | RC=0, **max_ulp=0.000** across normal / 1e-6 / 1e6 / sqrt(eps)-region / 8 / 0.03125 / 3 | PASS |

Denominator sweep used (P5): 1.25, 0.875, 1e-6, 1e6, 1.1920929e-07
(sqrt(eps) region), 8.0, 0.03125, 3.0.

## Findings

1. **Precision**: 0 ulp on every value — the reciprocal via `Div` is
   bit-identical to the correctly-rounded IEEE result. SEQ-FUSE-2's
   correctness risk on the reciprocal itself is closed.
2. **507035 root cause = 4B-offset operand**, confirmed (P3). The
   workaround is exactly what the spec prescribed: dedicated 32B-aligned
   slots, never a sub-slice at a 4B offset. This matches V001's earlier
   1-element-op alignment finding on this target.
3. **dst == src1 is unsafe in a second way**: it does not abort (so the
   prior "507035 when dst==src1" attribution conflated two hazards) —
   it silently produces wrong values because the divisor is overwritten
   mid-operation. V003 must keep dst distinct from both sources. The
   spec's P1 form already does.
4. Prior V002 note "V-side Div tested and reverted (ACL_FAIL
   sync=507035)" is now explained: the tested form used `onesSlot` at an
   unaligned offset and/or dst==src1. Both are avoidable.

## Consequence for SEQ-FUSE-2

The Div feasibility blocker is cleared. V003 may proceed to Main
decision with the P1 form as the only legal Div shape:
`Div(invRmsSlot, onesSlot, meanSqSlot, 1-or-8)`, all slots distinct and
32B-aligned. Form B (single pull + scalar divide) is no longer needed
as a fallback — the pure-V chain is fully feasible.

Not done (out of probe scope): no kernel edit, no V003, no timing, no
Online.
