# INTEGRATION-X V001 — timing summary (2026-09-27)

Device: server3 d4. Lease R2-INTEGRATION-V001-TIMING.
Protocol: warmup=45, samples=41, same-binary blocks=2, 6 interleaved P/C pairs
(odd pair P->C, even C->P). Primary metric: ALL_DEVICE median_us.
Clean pair: |delta| <= 50%. Same-binary gate: MAD/median <= 0.15.

PAIRED_DELTA = (C - P) / P; negative favors candidate.

| shape | sb P med/MAD | sb C med/MAD | sb gate | PAIRED_DELTA median | clean n | favor C/P | reading |
|---|---|---|---|---|---|---|---|
| 33x100 FP32 | 9.24 / 0.44 | 7.08 / 0.32 | PASS | **-3.35%** | 6 | 4/2 | win (SCHED preserved) |
| 32x256 FP32 | 7.04 / 0.37 | 6.76 / 0.41 | PASS | +2.30% | 4 | 2/2 | no signal |
| 8x256 FP32 | 15.55 / 11.63 | 5.53 / 1.00 | FAIL | +2.29% | 4 | 2/2 | blocked (sb) |
| 17x257 FP16 | 17.45 / 11.58 | 6.37 / 0.97 | FAIL | +0.30% | 5 | 2/3 | ~0, no collapse |
| 8x8192 FP32 | 6.46 / 0.37 | 7.59 / 0.73 | PASS | -0.80% | 6 | 3/3 | neutral (wide unchanged) |

Per-pair deltas (%):

- 33x100: -15.88, +8.74, +0.51, -2.74, -3.96, -11.06
- 32x256: -8.55, [+76.70 out], +10.09, [-60.66 out], +12.50, -5.49
- 8x256: [-60.16 out], +19.93, +5.71, -1.12, [-58.59 out], -4.79
- 17x257: -9.52, +9.19, [+161.97 out], +46.04, -17.19, +0.30
- 8x8192: +1.82, +10.12, -4.57, -3.41, -15.40, +8.33

## Reading against the approval's expectations

| expectation | result |
|---|---|
| 33x100 ~= SCHED alone (-4%) | met (-3.35%, 4/6 favor C, sb PASS) |
| batched 8x256/32x256 ~= VECTOR alone (-7%) | **not met** (32x256 +2.3% 2/2 split; 8x256 sb-FAIL) |
| 17x257 ~= 0 (gated ownership) | met (+0.30%, no V001-style collapse) |
| wide 8x8192 unchanged | met (-0.80%, 3/3 split) |

The VECTOR-MATH batched win does not reproduce in the stack. Its own V001
verdict was NEEDS_ONE_MORE_LOCAL (3 clean pairs of 6, "marginally above
noise"), and 32x256 here measures as no-signal under a PASS same-binary
window. Merge fidelity is not the cause: the merged file matches the
VECTOR source at every denominator site (verified by diff against both
single-mechanism sources).

## Verdict

- LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL (one shape class wins, two shapes
  blocked, batched class neutral)
- ONLINE_WORTHY = NO under the stated criterion ("wins on multiple shape
  classes without regression") — only the 33x100 class wins.
- No regression on any clean shape; the stack is submission-safe and
  functionally equivalent to SCHED V002 alone if Main wants the
  calibration point anyway.
