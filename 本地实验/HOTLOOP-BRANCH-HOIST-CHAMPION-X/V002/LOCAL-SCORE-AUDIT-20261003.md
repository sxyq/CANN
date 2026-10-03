# HBH-09 V002 simple local-score audit

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002 / HBH-09
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
TARGET=FP16 rows=80 D=6144 device=7
RUNNER=existing exact paired runner; 45 warmups; 4 blocks; 31 samples/side/block
TIMING=device-event primary
```

Build and correctness were already PASS (candidate correctness 8/8).  This
audit uses the three existing bounded simple-score runs and does not create a
new performance revision or retry window.

| Run | Parent avg (us) | Candidate avg (us) | Delta (us) | Delta (%) |
|---:|---:|---:|---:|---:|
| 1 | 14.820161 | 15.327419 | +0.507258 | +3.422757 |
| 2 | 17.828871 | 17.361936 | -0.466935 | -2.618985 |
| 3 | 17.916935 | 17.851935 | -0.065000 | -0.362785 |

The run-level values are arithmetic means of the 124 Parent and 124
Candidate device-event samples in each run.

```text
PARENT_AVG_US=16.855323
CANDIDATE_AVG_US=16.847097
LOCAL_DELTA_US=-0.008226
LOCAL_DELTA_PERCENT=-0.048802
MEASUREMENT_QUALITY=POOR
LOCAL_VERDICT=LOCAL_SCORE_RECORDED_NOT_ACCEPTED
```

The three directions are mixed and device 7 retained an unrelated Python
process throughout the snapshots.  The number is therefore an engineering
score only, not `LOCAL_ACCEPTED`, `LOCAL_BEST`, or an Official score.

Evidence:

- `support/local-score-audit-20261003/run1/`
- `support/local-score-audit-20261003/run2/`
- `support/local-score-audit-20261003/run3/`
- `local-result.json`, `source-meta.json`, and `correctness-result.json`
