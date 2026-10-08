# V079 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V079`
- Direct Parent / Local Best: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `f605ee1765eef7818485101d3d6621ed2d4ff35ae3758cf033bee8d7dcfe1030`
- Compile: PASS, target `sync_barrier_elision_v079`, `/tmp/sync-v079-build.20261008`.
- Correctness: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.

## Measurement

- Shape / dtype: `128x256 / bf16`.
- Device: NPU 3; device-event timing with wall-clock diagnostics.
- Method: 60 warmups; 31 interleaved Parent/Candidate pairs; no outlier filtering.
- Parent: exact `R31B-V011`.

| Run | Parent median us | Candidate median us | Paired median delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Local score |
|---:|---:|---:|---:|---:|---:|---:|
| Initial raw run (label repair pending) | 16.7000 | 17.1000 | +0.2000 | 1.962156 | 1.916257 | -1.197606% |
| Corrected-label rerun | 8.2000 | 13.9800 | +0.3600 | 3.996098 | 2.343920 | -4.390243% |

The first run is retained in `local-pre-label-fix.log` and `local.log`; its raw timings are not discarded. The corrected-label rerun in `local-final.log` is the primary completed Local result. The latest paired delta is `+0.3600 us` and the latest candidate-minus-parent throughput delta is `-1.652178 Gelem/s` at the median summary level.

Latest-run statistics:

- Device latency CV: Parent `0.58432`, Candidate `0.59357`; paired delta range `-20.5400` to `+28.0200 us`, paired-delta CV `6.51317`.
- Wall medians: Parent `88.3130 us`, Candidate `90.0530 us`; wall CV Parent `0.14692`, Candidate `0.13963`.
- Throughput medians: Parent `3.996098 Gelem/s`, Candidate `2.343920 Gelem/s`.
- Raw device-event samples, wall timings, throughput, and snapshots are retained in `local-final.log`; the preliminary raw run remains preserved separately.
- HBM on device 3: `3480/65536 MB` pre-run and `3431/65536 MB` post-run, approximately 62 GB free.
- AICore on device 3: `0%` pre-run and `0%` post-run. Other-device processes were not disturbed.

- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- `LOAD_QUALITY = LOW`
- `REPEATABILITY = FAIL / noisy negative measurements`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Continuity timestamps

- `RULE_REFRESH_TIMESTAMP = 2026-10-08T20:00:37Z`
- `EDIT_TIMESTAMP = 2026-10-08T20:01:34Z` (candidate source mtime)
- `COMPILE_PASS_CAPTURE_TIMESTAMP = 2026-10-08T20:02:44Z`
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T20:04:45Z` (initial pass)
- `LOCAL_START_TIMESTAMP = 2026-10-08T20:05:09Z` (initial run)
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T20:05:13Z` (initial run); corrected-label rerun completed at `2026-10-08T20:09:29Z`
- `CORRECTNESS_RERUN_TIMESTAMP = 2026-10-08T20:08:50Z`
- `LOCAL_RERUN_TIMESTAMP = 2026-10-08T20:09:22Z`
- `NON_EXECUTION_GAP_COMPILE_TO_CORRECTNESS_SECONDS = 121` for the initial authoritative sequence
- `LOCAL_RESULT_TO_NEXT_EDIT_SECONDS = recorded when V080 edit begins`
- `ONLINE = FORBIDDEN`
