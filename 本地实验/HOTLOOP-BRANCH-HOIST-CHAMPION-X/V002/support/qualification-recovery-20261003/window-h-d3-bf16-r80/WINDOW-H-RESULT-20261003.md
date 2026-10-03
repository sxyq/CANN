# HBH-09 V002 — Window H qualification and paired result

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
UTC=2026-10-03T07:54Z
DEVICE=3
SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
SAME_BINARY=PASS
QUALIFICATION_SAMPLES=62
QUALIFICATION_MEDIAN_US=21.390000
QUALIFICATION_MAD_OVER_MEDIAN=8.8359%
QUALIFICATION_BLOCK_DRIFT=0.2805%
PAIRED=PASS_RUNNER_RC0
PAIRED_BLOCKS=8
PAIRED_RAW_SAMPLES=496
BLOCK_DELTAS=+4.850,+0.915,+0.096,-5.362,+0.622,+6.042,+8.279,-7.983%
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
```

Window H used the exact existing Parent, Candidate, and runner under one d3
formal lock. Qualification passed, but paired block direction was mixed and
the per-block distributions contained substantial jitter. The result does not
support either a stable gain or a stable regression. All raw samples,
qualification/session output, and resource snapshots are retained; no sample
was removed and no source or binary was changed.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
NEXT_ACTION=SUPPORT_ONLY_MEASUREMENT_CALIBRATION_REVIEW
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
