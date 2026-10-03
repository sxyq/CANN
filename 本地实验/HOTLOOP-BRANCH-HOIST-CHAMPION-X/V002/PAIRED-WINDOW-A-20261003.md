# HBH-09 V002 Paired Recovery — Window A

```text
WINDOW_START_UTC=2026-10-03T06:22:38Z
WINDOW_END_UTC=2026-10-03T06:22:47Z
HOST=hwnput3
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
PROTOCOL=45_WARMUPS_PER_SIDE;31_DEVICE_EVENT_SAMPLES_PER_SIDE_PER_BLOCK;4_ALTERNATING_BLOCKS
```

The three shapes had passed exact Parent same-binary qualification earlier in
Window A. Each entered paired P/C immediately on its qualifying device. All
three paired runners returned RC=0 and retained all 248 raw samples, logs,
pre/post device usage, host load and process snapshots. Parent and Candidate
source identities match the hashes above.

| Device / primary shape | Parent median us | Candidate median us | Candidate−Parent us | Delta % | Same-binary noise floor | Paired block median deltas % | Assessment |
|---|---:|---:|---:|---:|---:|---|---|
| d0 FP32 D6144 rows=40 | 17.650 | 17.560 | -0.090 | -0.510% | 6.123% | -6.479, -2.599, -1.267, +4.276 | Delta is within noise; direction mixed; paired Parent pooled MAD/median 10.71% |
| d3 BF16 D6144 rows=80 | 20.290 | 19.720 | -0.570 | -2.809% | 2.037% | -7.257, +2.400, -0.916, -3.996 | Three blocks faster, one slower; paired Parent pooled MAD/median 8.43%, individual block jitter elevated |
| d5 BF16 D6144 rows=40 | 8.830 | 8.880 | +0.050 | +0.566% | 2.945% | -0.225, +3.645, -1.129, +0.226 | Delta is within noise and block directions are mixed |

The raw direction and distribution evidence do not support a stable
`LOCAL_ACCEPTED` or stable-regression `LOCAL_REJECTED` conclusion. The paired
medians are retained as diagnostic measurements, but **no valid Local score or
score-based verdict is claimed**. Current state remains
`WAITING_FOR_VALID_LOCAL_SCORE`; the same exact V002 Candidate is eligible for
another support-only measurement window. No Kernel source changed.

Raw and logs:

- `support/qualification-recovery-20261003/paired-window-a-d0-fp32-r40/`
- `support/qualification-recovery-20261003/paired-window-a-d3-bf16-r80/`
- `support/qualification-recovery-20261003/paired-window-a-d5-bf16-r40/`
