# HBH-09 V002 Measurement Recovery — Window B Result

```text
WINDOW_START_UTC=2026-10-03T06:54:43Z
WINDOW_END_UTC=2026-10-03T06:54:59Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=4
SHAPE=FP32_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
BUILD=PASS
CORRECTNESS=PASS_12_OF_12
LOCAL_FORMAL_LEASE=ACQUIRED_AND_RELEASED
```

Attempt 001 at 06:45 failed before entering the runner because its process
environment could not resolve `libruntime.so`; it produced no NPU timing. The
per-command CANN loader-path correction is recorded separately. Attempt 002
used the same parent, Candidate, runner binary, shape, and dtype. No Kernel or
runner source was changed.

## Same-binary qualification

The exact Parent completed 45 warmups and 2 blocks of 31 device-event samples.
Qualification passed: median `10.070 us`, all-sample MAD/median `4.0715%`,
block drift `0.1986%`. All 62 raw samples are retained.

## Paired Parent/Candidate measurement

Immediately after qualification, the exact Parent and Candidate ran on d4
under the same lease: 45 warmups per side, 31 samples per side per block, and
8 alternating blocks. All 496 raw paired samples are retained; none was
trimmed or selected by direction.

| Block | Parent median us | Candidate median us | Parent MAD/median | Candidate MAD/median | Paired sample-delta median % |
|---:|---:|---:|---:|---:|---:|
| 0 | 9.560 | 10.240 | 5.86% | 14.06% | +1.570 |
| 1 | 10.640 | 9.760 | 13.16% | 8.61% | -0.868 |
| 2 | 12.420 | 12.220 | 26.57% | 23.24% | -7.937 |
| 3 | 12.520 | 13.700 | 22.84% | 21.31% | +7.175 |
| 4 | 17.800 | 12.880 | 28.09% | 19.88% | -27.885 |
| 5 | 14.140 | 12.200 | 25.88% | 16.39% | -16.000 |
| 6 | 17.720 | 14.200 | 31.72% | 24.37% | -11.074 |
| 7 | 12.880 | 13.740 | 19.88% | 25.33% | +16.840 |

Paired Parent MAD/median exceeds 10% in 7/8 blocks, and the paired block
directions vary materially (five faster, three slower). This distribution is
not a stable effect relative to the same-binary noise estimate; no valid
Local score or `LOCAL_ACCEPTED`/`LOCAL_REJECTED` verdict is claimed.

```text
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
OFFICIAL_CHAMPION=R31B_V011_45.16
ONLINE_READY=NO
DIRECT_ONLINE_SUBMISSION=0
NEXT=NEW_TEMPORAL_WINDOW_RESOURCE_SCAN; NO_KERNEL_CHANGE; NO_NEXT_PERFORMANCE_REVISION
```

Pre/post resource snapshots and raw runner logs are under
`support/qualification-recovery-20261003/window-b-d4-fp32-r80/attempt-002/`.
The shared server lease table was read-only; no existing process was stopped,
paused, or migrated.
