# HBH-09 V002 — Measurement Recovery Window F Protocol

```text
PREDECLARED_UTC=2026-10-03T07:34Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=5
PRIMARY_SHAPE=BF16_D6144_ROWS40
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
```

Window F retries a real primary combination on d5 in a temporally distinct
window. Window A on d5 previously qualified this shape; Window C later failed
the same-binary gate. Neither result is treated as dispositive for the current
window. Re-scan all devices, HBM, AICore/processes, host load, and shared/local
leases immediately before admission. Keep the Candidate, Parent, and runner
unchanged.

Under the d5 formal `flock`, run the exact-shape Parent same-binary probe first
(45 warmups, 31 device-event samples in each of 2 blocks). Only an exact
`SAME_BINARY_QUAL=PASS` permits immediate paired timing on the same device and
lease (45 warmups, 31 samples per side in each of 8 alternating blocks). Save
all raw data and pre/post snapshots. If qualification does not PASS, do not run
Parent/Candidate timing. No source, binary, runner, shape, dtype, or filtering
changes are authorized.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
