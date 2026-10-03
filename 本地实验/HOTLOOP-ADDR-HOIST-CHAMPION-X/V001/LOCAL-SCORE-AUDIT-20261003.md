# ADDR-H3 V001 simple local-score audit

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001 / ADDR-H3
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
TARGET=BF16 rows=9 D=32768 reduced-tail device=7
RUNNER=known-good R31B V016 paired runner; 45 warmups; 21 P/C pairs
TIMING=device-event primary
```

The required BF16-D40960 path remains a shared Parent/Candidate runtime
blocker (`aclrtSynchronizeStream=507035`).  The selected D32768 reduced-tail
shape is an H3-valid address-counter regime and Parent/Candidate correctness
both passed with exact equality.  The support runner change is limited to
device/shape binding and safe process cleanup; the Candidate Kernel was not
modified.

| Run | Parent avg (us) | Candidate avg (us) | Delta (us) | Delta (%) |
|---:|---:|---:|---:|---:|
| 1 | 25.750 | 27.783 | +2.033 | +7.895146 |
| 2 | 36.228 | 36.818 | +0.590 | +1.628301 |
| 3 | 121.924 | 74.809 | -47.115 | -38.636697 |

The run-level values are arithmetic means of the 21 Parent and 21 Candidate
device-event samples in each run.

```text
PARENT_AVG_US=61.300667
CANDIDATE_AVG_US=46.470000
LOCAL_DELTA_US=-14.830667
LOCAL_DELTA_PERCENT=-24.193320
MEASUREMENT_QUALITY=POOR
LOCAL_VERDICT=LOCAL_SCORE_RECORDED_NOT_ACCEPTED
D40960_CLASSIFICATION=SHARED_RUNTIME_BLOCKER
```

The third run contains large external-load tails and the qualification probe
was `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.  Per the audit control, the
three completed runs still receive a basic score, but it is not evidence of
stable improvement, `LOCAL_ACCEPTED`, or an Official score.

Evidence:

- `local-score-20261003/qualification/`
- `local-score-20261003/run1/`
- `local-score-20261003/run2/`
- `local-score-20261003/run3/`
- `CORRECTNESS-D40960-FINAL-CLASSIFICATION.md`
