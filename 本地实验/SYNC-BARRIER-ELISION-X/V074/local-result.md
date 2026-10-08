# V074 Local Result

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V074`
- Direct parent: exact `R31B-V011`; V074 is not promoted
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `31cfc8dc07c720750f752d929445b0dc7893fa0882cecadb77adb63752606f56`
- Shape / dtype: `128x256 / bf16`
- Device / method: device 3; device-event timing; 60 warmups; 31 interleaved parent/candidate pairs
- Raw evidence: `logs/local.log`

| Metric | Parent | Candidate | Candidate - Parent |
|---|---:|---:|---:|
| median device latency (us) | 18.5600 | 19.8200 | +1.2600 |
| paired-median device delta (us) | - | - | +0.7600 |
| median throughput (Gelem/s) | 1.765517 | 1.653280 | -0.112237 |
| mean device latency (us) | 24.5529 | 21.6716 | - |
| device latency CV | 1.13506 | 0.76035 | - |

- Route scorer: `LOCAL_SCORE = -4.094819%`; it is `-100 * paired_median_delta / parent_median` using the runner's full-precision values.
- `LOCAL_DELTA = -4.094819%` relative to the exact parent measurement under the route's paired-delta scorer.
- Unpaired median latency diagnostic: candidate is `+1.2600 us`; it is not the paired decision metric.
- Throughput delta from medians: `-0.112237 Gelem/s`.
- Paired delta range: `-141.1000` to `+93.4800 us`; quality is `POOR/NOISY`.
- Repeatability: four observed attempts remained noisy and disagreed in direction; no promotion is justified. The latest complete raw run is retained in `logs/local.log`; prior numeric summaries are retained in `logs/repeat-summary.log`.
- Load context: post-run device 3 snapshot showed HBM used `7223/65536 MB` (`58313 MB` free), AICore `13%`; Python process `pid=3456615` remained present with `3848 MB`; no process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
