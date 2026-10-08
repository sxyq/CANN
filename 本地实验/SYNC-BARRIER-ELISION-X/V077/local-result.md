# V077 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V077`
- Direct Parent / Local Best: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `7b81ca5cbaf8665cffa7b8554d837a93f10c8be33e40973c5419c89ce30136ed`
- Compile: PASS, target `sync_barrier_elision_v077`, `/tmp/sync-v077-build.39804`.
- Correctness: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.

## Measurement

- Shape / dtype: `128x256 / bf16`.
- Device: NPU 3, device-event timing with wall-clock diagnostics.
- Method: 60 warmups; 31 interleaved Parent/Candidate pairs per run; three independent runs; no outlier filtering.
- Parent: exact `R31B-V011`.

| Run | Parent median us | Candidate median us | Paired median delta us | Parent median throughput Gelem/s | Candidate median throughput Gelem/s | Throughput delta | Local score |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 20.5200 | 18.8200 | -2.1000 | 1.596881 | 1.741126 | +0.144245 | +10.233918% |
| 2 | 9.2600 | 8.8000 | 0.0000 | 3.538661 | 3.723636 | +0.184975 | 0.000000% |
| 3 | 14.9200 | 10.4800 | +0.1000 | 2.196247 | 3.126718 | +0.930471 | -0.670241% |

Primary route result: `LOCAL_SCORE = -0.670241%`, `LOCAL_DELTA = -0.670241%` using the latest complete run's paired-median scorer.

Diagnostic series: `+10.233918%`, `0.000000%`, `-0.670241%`. All 93 raw pairs give a pooled diagnostic `+2.591284%` (parent median `16.9800 us`, candidate median `10.5800 us`); this pooled value is not promoted because the independent runs disagree in direction and have high jitter.

Run statistics:

- Run 1 device CV: Parent `0.43822`, Candidate `0.45420`; paired delta range `-39.8000` to `+12.8400 us`.
- Run 2 device CV: Parent `0.43920`, Candidate `0.77583`; paired delta range `-16.2200` to `+39.4200 us`.
- Run 3 device CV: Parent `0.46094`, Candidate `0.49796`; paired delta range `-24.4400` to `+23.0400 us`.
- Wall medians by run (Parent/Candidate us): `90.011/90.231`, `62.111/60.780`, `63.850/63.651`.
- All raw samples, wall timings, throughput, and statistics are retained under `logs/`.

Device context after measurement: NPU 3 HBM `7225/65536 MB` used (`58311 MB` free), AICore `4%`, Python process `3456615` present at `3850 MB`; other processes were not disturbed.

- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- `LOAD_QUALITY = LOW`
- `REPEATABILITY = FAIL / noisy direction across independent runs`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Continuity timestamps

- `RULE_REFRESH_TIMESTAMP = 2026-10-08T19:29:29Z`
- `EDIT_TIMESTAMP = 2026-10-08T19:30:07Z`
- `COMPILE_START_TIMESTAMP = 2026-10-08T19:30:19Z`
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T19:30:59Z`
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T19:32:25Z`
- `CORRECTNESS_PASS_TIMESTAMP = 2026-10-08T19:32:30Z`
- `LOCAL_START_TIMESTAMP = 2026-10-08T19:32:46Z`
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T19:32:58Z`
- `COMPILE_PASS_TO_CORRECTNESS_START_SECONDS = 86`
- `CORRECTNESS_PASS_TO_LOCAL_START_SECONDS = 16`
- `LOCAL_RESULT_TO_NEXT_EDIT_TIMESTAMP = recorded when V078 edit begins`
- `ONLINE = FORBIDDEN`
