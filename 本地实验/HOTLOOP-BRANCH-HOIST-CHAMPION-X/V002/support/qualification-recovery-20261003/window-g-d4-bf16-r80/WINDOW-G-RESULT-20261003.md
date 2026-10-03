# HBH-09 V002 — Window G qualification and paired result

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
UTC=2026-10-03T07:49Z
DEVICE=4
SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
SAME_BINARY=PASS
QUALIFICATION_SAMPLES=62
QUALIFICATION_MEDIAN_US=10.340000
QUALIFICATION_MAD_OVER_MEDIAN=4.0619%
QUALIFICATION_BLOCK_DRIFT=1.1605%
PAIRED=PASS_RUNNER_RC0
PAIRED_BLOCKS=8
PAIRED_RAW_SAMPLES=496
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
```

Paired block median deltas (Candidate minus Parent) were:

```text
block0=+0.816323%
block1=-7.649251%
block2=+7.514454%
block3=+1.491764%
block4=-17.743829%
block5=-6.951867%
block6=+23.287668%
block7=+11.446092%
```

The paired direction is mixed and the Parent/Candidate block jitter is large;
there is no stable gain or stable regression beyond the observed noise. All 496
paired samples, qualification samples, and resource snapshots are retained.
This is not a valid Local score and does not produce a Candidate verdict.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
NEXT_ACTION=NEW_PRIMARY_SHAPE_TEMPORAL_WINDOW
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
