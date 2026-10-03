# HBH-09 V002 — Measurement Recovery Window C Protocol

```text
PREDECLARED_UTC=2026-10-03T07:01:04Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
DEVICE=5
PRIMARY_SHAPE=BF16_D6144_ROWS40
```

## Why this preselected target

Window A d5 same-binary qualification passed with MAD/median `2.945%` and
block drift `3.062%`. Its four paired Parent blocks had MAD/median between
`2.483%` and `4.955%`; the paired sample-delta medians were
`+0.885%, +1.942%, -0.696%, +0.670%`, all near the noise floor. This is a
primary target with the best previously observed Parent stability, while its
Candidate result remains inconclusive. Window C is temporally distinct from
Window A and increases the paired observation count without selecting samples
by Candidate direction.

## Admission snapshot

The 07:01 UTC live probe showed d5 HBM `64057/65536 MB` (approximately
1479 MB free), AICore `0%`, AIVector `2%`, and no active d5 lease in the shared
`调度/服务器设备使用.tsv` state log. Host load was `63.50/61.99/61.79` with 34
users. Existing processes are observational only and will not be stopped,
paused, or migrated. Recheck full device/process/lease state immediately
before admission; require at least 100 MB free HBM and no shared d5 lease.

## Fixed sequence

Under the existing d5 local formal flock, use the exact V002 Parent,
Candidate, and runner. Run Parent same-binary qualification first (45
warmups, 31 device-event samples per block, 2 blocks). Only an exact-shape
`SAME_BINARY_QUAL=PASS` permits immediate interleaved P/C on d5 under that
same lease. Paired P/C is predeclared at 45 warmups per side, 31 samples per
side per block, and 8 alternating blocks. Capture all raw samples and
pre/post resource snapshots. Do not alter Kernel, runner, source identity,
shape, dtype, timing boundary, or sample filtering.
