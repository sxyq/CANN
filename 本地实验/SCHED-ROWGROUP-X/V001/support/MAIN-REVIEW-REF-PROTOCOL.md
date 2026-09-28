# MAIN-REVIEW — SCHED under unified ref protocol (2026-09-25)

## Same-binary (Direct Parent, exact shapes, d4, warmup10, 2x31 DEVICE_EVENT)

| shape | MAD/med | block drift | verdict |
|---|---:|---:|---|
| 7x65 | 0.131 / 0.184 (r2) | 0.024 / 0.038 | NEEDS_VALIDATION → MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 7x65 batch10 | 0.060 | 0.254 | FAIL (batch worsens drift) |
| 33x100 | 0.049 | 0.062 | PASS |
| 33x100 batch10 | 0.148 | 0.111 | NEEDS (batch worse) |
| 17x257 | 0.040 | 0.138 | NEEDS_VALIDATION |
| 17x257 r2 | 0.125 | 0.103 | NEEDS_VALIDATION |
| 17x256 | 0.023 | 0.014 | PASS |

REPEAT_BATCH: not adopted (no MAD/drift improvement).

## P/C (PASS shapes only, interleaved P C C P x2, 31 samples/process)

### 33x100
| pair | P | C | delta |
|---|---:|---:|---:|
| p1 | 48.06 | 41.72 | -13.19% (C MAD high) |
| p2 | 47.64 | 71.86 | +50.84% (C MAD high) |
| p3 | 48.98 | 24.28 | -50.43% |
| p4 | 50.44 | 23.66 | -53.09% |
favor_C 3/4, median_delta -31.81% vs same-binary floor MAD/med 0.049.

### 17x256
| pair | P | C | delta |
|---|---:|---:|---:|
| p1 | 23.94 | 23.82 | -0.50% |
| p2 | 23.52 | 23.28 | -1.02% |
| p3 | 19.84 | 19.32 | -2.62% |
| p4 | 19.32 | 19.62 | +1.55% |
favor_C 3/4, median_delta -0.76% within floor 0.023.

## Decision
NEEDS_ONE_MORE_LOCAL (not ONLINE_CANDIDATE, not LOCAL_REJECTED).
- d256 delta inside same-binary noise floor.
- d100 large favorable median but one reverse pair and high C within-block MAD on p1/p2 — not two independent clean blocks.
- d65/d257 shape-blocked for Candidate.
- V001 SHA unchanged 0fae0a42…; no revision; LEGACY −18.45% remains STRONG_POSITIVE_LOCAL_SIGNAL unmerged.
