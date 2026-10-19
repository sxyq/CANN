# MAIN-2 Round-2 — SCHED-ROWGROUP-X V001

Received: 2026-09-24T05:36:40Z
SHA: 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c (unchanged)

## Baseline gates (parent, shape 17×256)
- d4: n=10 CV=0.2764 range/med=1.0311 median=142.162 unstable=True
- d5: n=8 CV=0.3535 range/med=1.2411 median=119.933 unstable=True
- d6: n=8 CV=0.1343 range/med=0.4399 median=144.103 unstable=False
- stable_device: 6
- all_unstable: False

## Pairs
| pair | shape | order | parent_us | v001_us | delta% | bad |
|---|---|---|---:|---:|---:|---:|
| 01 | rows=7 width=65 dtype=0 | PC | 146.067 | 114.426 | -21.662 | 0 |
| 02 | rows=33 width=100 dtype=0 | PC | 166.338 | 132.527 | -20.327 | 0 |
| 03 | rows=17 width=257 dtype=0 | CP | 175.859 | 146.708 | -16.576 | 0 |
| 04 | rows=17 width=256 dtype=0 | CP | 132.256 | 111.766 | -15.493 | 0 |

PARENT_TIMES: [146.067, 166.338, 175.859, 132.256]
CANDIDATE_TIMES: [114.426, 132.527, 146.708, 111.766]
LOAD_QUALITY: LOAD_CONTAMINATED
MEDIAN_DELTA: -18.452
WORST_DELTA: -15.493
DIRECTIONAL_CONSISTENCY: 1.0
NOISE_MARGIN: 3.085
aligned: -15.493
CORRECTNESS: PASS

## Decision: **NEEDS_ONE_MORE_LOCAL**

Stable baseline d6 but pair parent jitter LOAD_CONTAMINATED (max sofm 0.558). MEASUREMENT_BLOCKED for candidate judgment. Raw deltas [-21.662, -20.327, -16.576, -15.493] median -18.452. Correctness PASS.

Devices released. support/results-round2/ written. No V002. No CANNJudge.
