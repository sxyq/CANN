# V096 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V096`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1008`; Candidate SHA256 `7f44a290646218fe62f5f453d5fe018c1a2863ba61720086db23122bbc75e729`.
- `COMPILE=PASS`; root `device` and `submission` targets returned rc=0. Probe targets `clx_ref_parent_probe` and `clx_ref_candidate_probe` also built successfully.
- `CORRECTNESS=PASS` for exact Parent and Candidate on FP32 `128x1008`, `128x1012`, and `128x1016`; all six final checks returned `rc=0,bad=0`. Max absolute errors by width were `1.43051e-6`, `1.90735e-6`, and `1.19209e-6`.
- The first six correctness launches failed with `rc=1` because the Toolkit HCC `libstdc++.so.6` did not provide `GLIBCXX_3.4.29`; no kernel ran. After prioritizing the system C++ runtime, all six correctness checks passed. Preserve both attempts.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded exact-Parent C15 FP32 `1x32768`; C15 was not run and this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

## Local Score

Device 2; FP32; six interleaved Parent/Candidate pairs per shape, alternating P/C and C/P; 45 warmups, 31 device-event samples in each of two blocks per invocation, `batch_n=64`. All 36 final Local invocations returned `rc=0,bad=0`; all raw TSVs contain 62 event samples (2,232 total, 372 per side per shape). No samples were discarded. Initial 36 launch attempts returned `rc=127` because the shell library path omitted `libgraph.so`; no kernel ran. Those failures are retained separately and excluded from the numeric score.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Shape score is the arithmetic mean of six pair speedups. Route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Within combined MAD | Parent/Candidate pooled median (us) |
|---|---|---:|---:|---:|---:|---:|
| 128x1008 | 1.041570143, 1.036231974, 0.988872866, 1.049382030, 1.052356293, 0.933148886 | 1.016927032x | +1.692703180% | 4/6 | 1/6 | 7.864060 / 7.651090 |
| 128x1012 | 0.906618028, 1.000498613, 1.023167488, 0.962733712, 1.012236814, 0.999318453 | 0.984095518x | -1.590448173% | 3/6 | 5/6 | 7.808590 / 7.849065 |
| 128x1016 | 1.770011542, 0.895129788, 0.942290682, 1.046816874, 0.984501969, 0.971707380 | 1.101743039x | +10.174303918% | 2/6 | 2/6 | 7.547340 / 7.692655 |
| Equal-shape geomean | - | **1.033084361x** | **+3.308436068%** | **9/18** | **8/18** | - |

Pooled event-sample summary (372 samples per side and shape):

| Shape | Side | Mean (us) | CV | MAD (us) | Throughput (Gelem/s) |
|---|---|---:|---:|---:|---:|
| 128x1008 | Parent | 7.927344 | 6.371% | 0.177655 | 16.275817 |
| 128x1008 | Candidate | 8.205726 | 97.414% | 0.174220 | 15.723654 |
| 128x1012 | Parent | 8.221538 | 28.291% | 0.140470 | 15.755689 |
| 128x1012 | Candidate | 8.118723 | 16.198% | 0.182500 | 15.955218 |
| 128x1016 | Parent | 8.528020 | 25.929% | 0.219840 | 15.249496 |
| 128x1016 | Candidate | 7.951608 | 10.305% | 0.208905 | 16.354931 |

## Load And Verdict

- Device 2 had `FREE_HBM=59,100 MB` at Local start. Across 36 pre/post pair snapshots, HBM used was 6,436-6,439 MB of 65,536 MB; AICore ranged 0-9%. Existing PID 2055832 was present in all snapshots and was not disturbed. `npu-smi info` did not expose AIVector usage.
- `LOAD_QUALITY=PROCESS_PRESENT`; `MEASUREMENT_QUALITY=NOISY/INCONCLUSIVE`. Parent/Candidate direction was mixed (9/18 Candidate-faster pairs), only 8/18 median shifts were within combined MAD, and pooled CV ranged 6.371%-97.414%. The raw-derived positive aggregate is retained but does not establish stable improvement.
- `LOCAL_SCORE=1.033084360679x` (partial route-local only); `LOCAL_DELTA=+3.308436068%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V096 is not promoted. No Official comparison is claimed.
- Device 2 was released after capture at `2026-10-08T18:06:23.044896334Z`.

## Timestamps

- `COMPILE_PASS_TIMESTAMP=2026-10-08T17:47:52.352972167Z`.
- `CORRECTNESS_START_TIMESTAMP=2026-10-08T17:56:12.300892703Z`.
- `LOCAL_START_TIMESTAMP=2026-10-08T18:01:06.790092076Z`.
- `LOCAL_CAPTURE_END_TIMESTAMP=2026-10-08T18:05:58.508030667Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T18:10:56.647651118Z`.
- `NEXT_EDIT_TIMESTAMP=NOT_STARTED`; this timestamp is recorded when V097's one cutoff edit begins.

Raw TSVs, per-invocation stats/stdout/return codes, all pair snapshots, initial loader failures, compile/probe logs, source identities, and device release snapshot are retained in this V096 directory.
