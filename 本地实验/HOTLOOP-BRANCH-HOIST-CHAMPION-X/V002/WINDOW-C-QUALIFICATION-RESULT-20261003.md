# HBH-09 V002 Measurement Recovery — Window C Qualification Result

```text
WINDOW_START_UTC=2026-10-03T07:04:51Z
WINDOW_END_UTC=2026-10-03T07:05:00Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=5
SHAPE=BF16_D6144_ROWS40
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
LOCAL_FORMAL_LEASE=ACQUIRED
```

The exact Parent same-binary runner completed 45 warmups and 62 device-event
samples. Qualification failed for this shape/window: pooled median
`12.700 us`, MAD/median `37.1654%`, and block drift `57.1654%` (block medians
`15.180 us` and `7.920 us`). The raw file retains all 62 samples. Per protocol,
Parent/Candidate timing did not run.

The admission snapshot recorded d5 HBM `64057/65536 MB` (about 1479 MB free),
AICore 5%, host load `52.29/59.41/60.91`, and resident VLLM/Python processes.
There was no active shared d5 lease. Existing processes were not stopped,
paused, or migrated. Exact Parent, Candidate, and runner hashes matched the
predeclared values.

## Session log interpretation correction

`formal-session.log` is preserved byte-for-byte as generated. It begins with
`LOCAL_FORMAL_LEASE=ACQUIRED` and records `QUALIFICATION_RC=1`; the qualification
log and 62-row raw sample file confirm the runner did execute. Its final line
`LOCAL_FORMAL_LEASE=CONFLICT` is an outer-wrapper misclassification: `flock`
propagated the child runner's exit code 1, and the wrapper incorrectly treated
that same code as lock contention. It is not evidence of a lease conflict.
The lock command exited and released its descriptor after qualification
finished. Future invocations must use a distinct sentinel for lock-acquisition
failure and preserve the child's qualification result separately.

```text
SAME_BINARY_QUALIFICATION=MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE
PARENT_CANDIDATE_TIMING=NOT_RUN
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
NEXT=FRESH_RESOURCE_AND_LEASE_SCAN_IN_A_LATER_WINDOW; TRY_ANOTHER_PRIMARY
```

This is a shape/window noise failure, not a Candidate correctness or
performance verdict and not a system-wide measurement blocker. All evidence
is under `support/qualification-recovery-20261003/window-c-d5-bf16-r40/`.
