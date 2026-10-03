# HBH-09 V002 — Window E qualification result

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
UTC=2026-10-03T07:23Z
DEVICE=7
SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
SAME_BINARY=NEEDS_VALIDATION
SAMPLES=62 (2 blocks x 31)
MEDIAN_US=20.370000
MAD_OVER_MEDIAN=21.2568%
BLOCK_DRIFT=9.5238%
QUALIFICATION_RC=1
PARENT_CANDIDATE_TIMING=NOT_RUN
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
```

The pre/post snapshots show d7 HBM at 24,790–24,792 / 65,536 MB (about
40.7 GB free), AICore at 0%, and AIVector at 1–2%. A resident `python3`
process (PID 439848, 21,260 MB) was present and left untouched. The shared
lease scan reported no active d7 lease. Host load was 50.10/57.17/59.46 before
and 47.65/56.42/59.20 after; available host memory was about 761 GiB. The
device had ample free HBM and no lease conflict, but the same-binary noise gate
did not pass. Therefore no Parent/Candidate timing ran, and this is not a
Candidate performance failure or a valid Local score.

All 62 raw event samples, qualification output, session output, and pre/post
resource snapshots are retained beside this note. No source, binary, runner,
shape, dtype, or measurement parameters were changed.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
NEXT_ACTION=NEW_RESOURCE_SCAN_AND_SAME_BINARY_QUALIFICATION_ON_A_DIFFERENT_PRIMARY_DEVICE_OR_TEMPORAL_WINDOW
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
