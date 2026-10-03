# HBH-09 V002 Measurement Recovery — Window D Result

```text
WINDOW_START_UTC=2026-10-03T07:13:56Z
WINDOW_END_UTC=2026-10-03T07:14:12Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=4
SHAPE=BF16_D6144_ROWS80
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
BUILD=PASS
CORRECTNESS=PASS_12_OF_12
LOCAL_FORMAL_LEASE=ACQUIRED_AND_RELEASED
```

## Same-binary qualification

Exact Parent qualification passed with 45 warmups and 2 blocks of 31
device-event samples: median `10.540 us`, MAD/median `3.7951%`, block drift
`0.3795%`. All 62 raw samples are retained.

## Paired Parent/Candidate measurement

Paired P/C ran immediately on d4 under the same lease with 45 warmups per
side, 31 samples per side per block, and 8 alternating blocks. All 496 raw
samples are retained without trimming.

| Block | Parent median us | Candidate median us | Parent MAD/median | Candidate MAD/median | Paired sample-delta median % |
|---:|---:|---:|---:|---:|---:|
| 0 | 19.180 | 19.560 | 33.06% | 31.49% | -7.772 |
| 1 | 19.100 | 15.420 | 27.75% | 25.68% | -14.346 |
| 2 | 16.520 | 16.800 | 21.31% | 27.62% | 0.000 |
| 3 | 11.080 | 11.140 | 14.80% | 15.26% | -0.810 |
| 4 | 9.800 | 9.760 | 4.29% | 2.87% | -1.633 |
| 5 | 11.040 | 10.140 | 13.04% | 6.11% | -1.961 |
| 6 | 11.880 | 11.300 | 20.20% | 15.75% | +1.025 |
| 7 | 10.260 | 15.040 | 8.19% | 32.85% | +26.991 |

Parent paired MAD/median exceeds 10% in 6/8 blocks. The paired delta medians
range from `-14.346%` to `+26.991%`, including a zero block and clear
direction reversal. The P/C distribution is not stable relative to the
same-binary noise estimate; no valid Local score or score-based verdict is
claimed.

```text
VALID_LOCAL_SCORE=NO
LOCAL_SCORE=NONE
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
OFFICIAL_CHAMPION=R31B_V011_45.16
ONLINE_READY=NO
DIRECT_ONLINE_SUBMISSION=0
NO_NEXT_PERFORMANCE_REVISION=YES
NEXT=NEW_TEMPORAL_WINDOW_RESOURCE_SCAN; SAME_SOURCE_AND_RUNNER
```

The d4 pre/post HBM snapshots were `61977/61979 MB` used out of `65536 MB`;
AICore was `0%` in the d4 usage samples. Host load, AICore, resident processes,
and the shared lease check are retained in the support logs. Existing jobs
were not modified. The final generic wrapper status line is ambiguous, but
the preceding `LOCAL_FORMAL_LEASE=ACQUIRED` and explicit release line establish
that this comparison held and released its d4 lease.
