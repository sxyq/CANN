# HBH-09 V002 — Measurement Recovery Window G Protocol

```text
PREDECLARED_UTC=2026-10-03T07:47Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=4
PRIMARY_SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
```

Window G retries the BF16 rows=80 primary shape on d4 in a fresh temporal
window. The prior d4 window qualified but its paired Parent blocks were too
noisy; this retry keeps the same runner and exact source identities and does not
reinterpret that earlier result as a score. Re-scan HBM, AICore/processes,
host load, shared lease state, and local formal lock immediately before entry.

Run Parent same-binary qualification first with 45 warmups and 31 device-event
samples in each of 2 blocks. Only exact `SAME_BINARY_QUAL=PASS` permits
immediate 8-block, 31-sample-per-side interleaved P/C timing under the same d4
formal lock. Preserve every raw sample and snapshot; no source, binary, runner,
shape, dtype, or filtering changes.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
