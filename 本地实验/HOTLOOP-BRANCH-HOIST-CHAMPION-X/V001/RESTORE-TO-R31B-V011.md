# HBH-10 V001 Explicit Restore Trail

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V001
FINAL_VERDICT=LOCAL_REJECTED
VERDICT_COMMIT=b0d33c09
RESTORE_TARGET=R31B V011
RESTORE_SOURCE=线上结果/R31B/V011/submission.asc
EXPECTED_RESTORED_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
REJECTED_CANDIDATE_SHA256=4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b
RESTORE_METHOD=EXPLICIT_BYTE_FOR_BYTE_SOURCE_RESTORE
GIT_RESET=NOT_USED
GIT_CLEAN=NOT_USED
```

HBH-10 V001 was rejected after two valid formal windows showed mixed
direction and no gain above the observed same-binary noise. The route source
must return to the exact official-backed parent before any sibling experiment.
The restore is intentionally recorded as a separate commit; V001 source and
all failed performance evidence remain preserved in history.
