# ADDR-H1 V002 — three-run Local Score

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=ADDR-H1 pass-1 row-base state
DIRECT_PARENT=R31B V011
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=40b1548eb0862691e92b10f242e3f98ccf4d232596a2704ddbc25eab7b3aa3f3
BUILD=PASS
CORRECTNESS=PASS on BF16-D32768 reduced-tail; exact Parent/Candidate output match
TARGET=BF16 rows=9 D=32768 reduced-tail, device 7
TIMING=device-event primary, 45 warmups, 21 interleaved pairs/run
RUN_COUNT=3, all runner RC=0
```

| Run | Parent average (us) | Candidate average (us) | Delta (us) | Delta (%) |
|---:|---:|---:|---:|---:|
| 1 | 40.160 | 46.149 | +5.989 | +14.912849 |
| 2 | 52.845 | 49.488 | -3.357 | -6.352540 |
| 3 | 35.040 | 33.630 | -1.410 | -4.023973 |

```text
PARENT_AVG_US=42.681667
CANDIDATE_AVG_US=43.089000
LOCAL_DELTA_US=+0.407333
LOCAL_DELTA_PERCENT=+0.954352
MEASUREMENT_QUALITY=POOR
LOCAL_CLASSIFICATION=LOCAL_NEGATIVE (engineering score; direction/run spread is mixed)
LOCAL_VERDICT=LOCAL_SCORE_COMPLETE_NOT_ACCEPTED
OFFICIAL_SCORE=UNCHANGED; R31B V011 remains 45.16
ONLINE=NOT_SUBMITTED
```

The measured device remained shared with an unrelated VLLM Engine and Python
process; no task was touched.  This is the mandated three-run basic score,
not a stable-performance claim or an Official score.  No extra timing window
was added.

Raw evidence and pre/post snapshots:

- `local-score-20261003/run1/`
- `local-score-20261003/run2/`
- `local-score-20261003/run3/`
