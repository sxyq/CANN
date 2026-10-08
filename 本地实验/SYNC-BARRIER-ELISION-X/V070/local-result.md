# V070 Local Result

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V070`
- Direct parent: exact `R31B-V011`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `d48ef8b0cfe9633e06fca4abb36839add0ff00b4b7b5a489df6da53a0babb590`
- Shape / dtype: `128x256 / bf16`
- Device: `3`
- Method: device-event timing, 31 interleaved parent/candidate pairs, warmup before samples
- Raw evidence: `logs/local.log`

| Metric | Parent | Candidate |
|---|---:|---:|
| median device latency (us) | 20.1600 | 18.7400 |
| mean device latency (us) | 24.1484 | 21.6006 |
| median throughput (Gelem/s) | 1.625397 | 1.748559 |
| p10-p90 device latency (us) | 8.6600-26.8200 | 8.6000-27.7200 |
| MAD device latency (us) | 4.5400 | 4.4200 |
| CV device latency | 1.09203 | 0.82267 |

- Paired median delta, candidate minus parent: `+1.6400 us`
- `LOCAL_SCORE = -8.134918%`
- `LOCAL_DELTA = -8.134918%` relative to the exact parent local measurement
- Throughput delta by median: `+0.123162 Gelem/s`
- Quality: `POOR/NOISY`; raw samples include large outliers and paired/pooled direction is contradictory
- Load context: device 3; pre HBM used `9132 MB / 65536 MB` (about `56404 MB` free); AICore `4%`; existing Python process retained; no process was disturbed
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; Local Best remains exact `R31B-V011`
