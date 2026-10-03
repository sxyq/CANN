# HBH-09 V002 — Measurement Recovery Window H Protocol

```text
PREDECLARED_UTC=2026-10-03T07:52Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=3
PRIMARY_SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
```

Window H repeats the real BF16 rows=80 primary on d3 in a new temporal window.
The earlier d3 paired evidence is retained but not reused as a score. Refresh
all resource, process, host-load, shared-lease, and local-lock facts before
admission. Use the exact existing Parent, Candidate, and runner.

Run 45-warmup, 31-sample x 2-block Parent same-binary qualification first. If
and only if it returns exact `SAME_BINARY_QUAL=PASS`, immediately run 8 paired
blocks with 31 samples per side and alternating P/C order under the same lock.
Keep all raw samples and snapshots; make no source, binary, runner, shape,
dtype, or filtering changes.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
