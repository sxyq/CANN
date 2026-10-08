# V071 Local Result

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V071`
- Direct parent: exact `R31B-V011`; V070 was not promoted
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `56d551bb768f32bf289338e8d74c6405710c385a6a6d3996043384dd3a5f59a1`
- Shape / dtype: `128x256 / bf16`
- Device / method: device 3; device-event timing; 60 warmups; 31 interleaved parent/candidate pairs
- Raw evidence: `logs/local.log`

| Metric | Parent | Candidate | Candidate - Parent |
|---|---:|---:|---:|
| median device latency (us) | 17.5800 | 19.3400 | +1.7600 |
| paired-median device delta (us) | - | - | +0.2200 |
| median throughput (Gelem/s) | 1.863936 | 1.694312 | -0.169624 |
| mean device latency (us) | 17.3316 | 18.5716 | +1.2400 |
| device latency CV | 0.38518 | 0.45166 | - |

- Runner `LOCAL_SCORE = -1.251426%`; this is `-100 * paired_median_delta / parent_median`.
- `LOCAL_DELTA = -1.251426%` relative to the exact parent measurement under the route's paired-delta scorer.
- Unpaired median latency diagnostic: `+1.7600 us`, or `-10.0114%` candidate-vs-parent.
- Quality: `POOR/NOISY`; candidate has a `49.7800 us` device outlier and paired deltas span `-16.7200` to `+41.8200 us`.
- Repeatability: not independently reproduced; one complete raw run is retained.
- Load context: pre HBM used `4705/65536 MB` (`60831 MB` free), AICore `0%`; post HBM used `4708/65536 MB` (`60828 MB` free), AICore `4%`; Python process on device 3 remained present; no process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
