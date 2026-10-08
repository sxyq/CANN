# V072 Local Result

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V072`
- Direct parent: exact `R31B-V011`; V071 was not promoted
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `f6c414fc69012fc138fbd67fa12a1a9dd5f34553a1404355124e562f3335da18`
- Shape / dtype: `128x256 / bf16`
- Device / method: device 3; device-event timing; 60 warmups; 31 interleaved parent/candidate pairs
- Raw evidence: `logs/local.log`

| Metric | Parent | Candidate | Candidate - Parent |
|---|---:|---:|---:|
| median device latency (us) | 12.9800 | 14.4400 | +1.4600 |
| paired-median device delta (us) | - | - | +0.1400 |
| median throughput (Gelem/s) | 2.524499 | 2.269252 | -0.255247 |
| mean device latency (us) | 13.2497 | 12.8219 | -0.4278 |
| device latency CV | 0.39585 | 0.39781 | - |

- Runner `LOCAL_SCORE = -1.078580%`; this is `-100 * paired_median_delta / parent_median`.
- `LOCAL_DELTA = -1.078580%` relative to the exact parent measurement under the route's paired-delta scorer.
- Unpaired median latency diagnostic: `+1.4600 us`, or `-11.2481%` candidate-vs-parent.
- Quality: `POOR/NOISY`; paired deltas span `-14.4400` to `+13.3600 us`.
- Repeatability: not independently reproduced; one complete raw run is retained.
- Load context: pre HBM used `4705/65536 MB` (`60831 MB` free), AICore `14%`; post HBM used `4707/65536 MB` (`60829 MB` free), AICore `4%`; Python process on device 3 remained present; no process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
