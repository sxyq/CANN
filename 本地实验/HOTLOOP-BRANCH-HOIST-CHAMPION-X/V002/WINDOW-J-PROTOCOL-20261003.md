# HBH-09 V002 — Measurement Recovery Window J Protocol

```text
PREDECLARED_UTC=2026-10-03T08:27Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=7
PRIMARY_SHAPE=BF16_D6144_ROWS40
REPEAT_BATCH_N=1
RUNNER_SOURCE_SHA256=86d6ad6df82ec7b2c6b414c2759a72a48567cf8f11a382a7fdcd003f45a955c0
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
```

Window J uses the restored original N=1 runner on the BF16 rows=40 primary
shape. Prior attempts on rows=40 were d1 and d5, but their qualification
windows failed; this is a new temporal window on d7. Refresh HBM, processes,
host load, shared lease, and local lock state immediately before admission.

Run Parent same-binary first (45 warmups, 31 samples x 2 blocks). Only exact
qualification PASS permits immediate 8-block paired P/C under the same lock.
Keep all raw data and snapshots. No source, binary, runner, shape, dtype, or
filtering changes are permitted.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
