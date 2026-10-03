# HBH-09 V002 Qualification Recovery — Window A

```text
WINDOW_START_UTC=2026-10-03T06:16:55Z
WINDOW_END_UTC=2026-10-03T06:17:04Z
HOST=hwnput3
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
PROTOCOL=45_WARMUPS;31_DEVICE_EVENT_SAMPLES_PER_BLOCK;2_BLOCKS;FULL_RAW
```

The measurement resource scan is retained in
`support/qualification-recovery-20261003/resource-scan-window-a.log`. At scan
time all eight cards had more than 100 MB free HBM, no active lease was present
in the shared lease log, host load average was `33.52/52.21/56.72`, and 34 users
were logged in. Existing VLLM/Python workloads were left untouched. A device-6
lease appeared between scan and admission; d6 was correctly skipped without
launching a comparison.

| Device | Primary probe | HBM usage at admission | Qualification | Median us | MAD/median | Block drift | Evidence |
|---:|---|---:|---|---:|---:|---:|---|
| 0 | FP32 D6144 rows=40 | 97% | PASS | 8.820000 | 0.061225 | 0.002268 | `support/qualification-recovery-20261003/window-a-d0-fp32-r40/` |
| 1 | BF16 D6144 rows=40 | 93% | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE | 14.960000 | 0.296791 | 0.030749 | `support/qualification-recovery-20261003/window-a-d1-bf16-r40/` |
| 2 | FP32 D6144 rows=80 | 94% | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE | 17.279999 | 0.351852 | 0.000000 | `support/qualification-recovery-20261003/window-a-d2-fp32-r80/` |
| 3 | BF16 D6144 rows=80 | 91% | PASS | 10.800000 | 0.020370 | 0.001852 | `support/qualification-recovery-20261003/window-a-d3-bf16-r80/` |
| 4 | FP32 D6144 rows=40 | 94% | NEEDS_VALIDATION | 9.360000 | 0.170940 | 0.170940 | `support/qualification-recovery-20261003/window-a-d4-fp32-r40/` |
| 5 | BF16 D6144 rows=40 | 97% | PASS | 8.490000 | 0.029446 | 0.030624 | `support/qualification-recovery-20261003/window-a-d5-bf16-r40/` |
| 6 | FP32 D6144 rows=80 | 91% | NOT_RUN — SHARED_LEASE_CONFLICT | — | — | — | `support/qualification-recovery-20261003/window-a-d6-fp32-r80/qualification.log` |
| 7 | BF16 D6144 rows=80 | 37% | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE | 17.039999 | 0.410798 | 0.458920 | `support/qualification-recovery-20261003/window-a-d7-bf16-r80/` |

All seven admitted comparisons used the unchanged exact Parent/Candidate source
identity and one comparison per device. The three qualified primary
shape/device pairs (d0 FP32 rows=40, d3 BF16 rows=80, d5 BF16 rows=40) proceed
immediately to paired Parent/Candidate timing on those same devices. No result
from an unqualified shape is eligible for pairing or score calculation.
