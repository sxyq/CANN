# HBH-09 V002 — Measurement Recovery Window D Protocol

```text
PREDECLARED_UTC=2026-10-03T07:10:16Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
DEVICE=4
PRIMARY_SHAPE=BF16_D6144_ROWS80
```

Window A's d3 BF16 D6144 rows=80 Parent qualification passed with MAD/median
`2.037%` and drift `0.185%`, but its paired Parent jitter rose to `8.43%` and
the block directions were mixed. Window D tests that same declared primary
shape on d4 in a distinct time window. The 07:10 live probe showed HBM usage
`94%` (about 3.56 GB free), AICore `0%`, and no active shared d4 lease. A fresh
all-device and process snapshot is required immediately before admission;
resident processes remain observational and untouched.

Use the exact Parent, Candidate, and existing performance runner under the d4
formal flock. First run Parent same-binary qualification with 45 warmups, 31
device-event samples per block, and 2 blocks. Only exact-shape
`SAME_BINARY_QUAL=PASS` permits immediate paired timing on d4 under the same
lease. Paired P/C is fixed at 45 warmups per side, 31 samples per side per
block, and 8 alternating blocks. Retain every raw sample and pre/post load
snapshot. No source, Kernel, runner, shape, dtype, timing boundary, or sample
filtering changes are allowed.
