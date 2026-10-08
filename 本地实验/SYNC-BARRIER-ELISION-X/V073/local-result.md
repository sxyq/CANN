# V073 Local Result

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V073`
- Direct parent: exact `R31B-V011`; V071 and V072 were not promoted
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `0286cdcd8ff7bdc7e321aef1f720785345835623d436d4d5eafb6a8098901d5d`
- Shape / dtype: `128x256 / bf16`
- Device / method: device 3; device-event timing; 60 warmups; 31 interleaved parent/candidate pairs
- Raw evidence: `logs/local.log`

| Metric | Parent | Candidate | Candidate - Parent |
|---|---:|---:|---:|
| median device latency (us) | 14.7600 | 14.5200 | -0.2400 |
| paired-median device delta (us) | - | - | +0.5000 |
| median throughput (Gelem/s) | 2.220054 | 2.256749 | +0.036695 |
| mean device latency (us) | 13.6819 | 13.7826 | +0.1006 |
| device latency CV | 0.38265 | 0.39294 | - |

- Runner `LOCAL_SCORE = -3.387541%`; the route scorer is `-100 * paired_median_delta / parent_median` using the runner's full-precision values.
- `LOCAL_DELTA = -3.387541%` relative to the exact parent measurement under the route's paired-delta scorer.
- Unpaired median latency diagnostic: `-0.2400 us`; it is not the paired decision metric.
- Quality: `POOR/NOISY`; paired deltas span `-13.4800` to `+14.8000 us`.
- Repeatability: not independently reproduced; one complete raw run is retained.
- Load context: pre HBM used `7223/65536 MB` (`58313 MB` free), AICore `1%`; post HBM used `7225/65536 MB` (`58311 MB` free), AICore `4%`; Python process `pid=3456615` remained present with `3848 MB`; no process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
