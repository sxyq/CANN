# V078 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V078`
- Direct Parent / Local Best: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `43117b270750daa290ae3f266889c361039d683e36bcc76e452639b18e10a450`
- Compile: PASS, target `sync_barrier_elision_v078`, `/tmp/sync-v078-build.141554`.
- Correctness: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.

## Measurement

- Shape / dtype: `128x256 / bf16`.
- Device: NPU 3, device-event timing with wall-clock diagnostics.
- Method: 60 warmups; 31 interleaved Parent/Candidate pairs per run; three independent runs; no outlier filtering.
- Parent: exact `R31B-V011`.

| Run | Parent median us | Candidate median us | Paired median delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Throughput delta | Local score |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 17.2600 | 14.5600 | -1.4800 | 1.898494 | 2.250549 | +0.352055 | +8.574740% |
| 2 | 9.2200 | 9.5400 | -0.2200 | 3.554013 | 3.434801 | -0.119212 | +2.386115% |
| 3 | 9.2000 | 10.7400 | -0.2600 | 3.561739 | 3.051024 | -0.510715 | +2.826088% |

Primary route result: `LOCAL_SCORE = +2.826088%`, `LOCAL_DELTA = +2.826088%`, using the latest complete run's paired-median scorer. The three run scores are retained without direction-based filtering.

Run statistics:

- Run 1 device CV: Parent `0.43346`, Candidate `0.41017`; paired delta range `-22.1200` to `+21.4200 us`.
- Run 2 device CV: Parent `0.41244`, Candidate `0.43471`; paired delta range `-12.6600` to `+19.8800 us`.
- Run 3 device CV: Parent `0.52686`, Candidate `0.47233`; paired delta range `-26.7800` to `+21.9800 us`.
- Wall medians by run (Parent/Candidate us): `69.201/67.930`, `61.781/61.440`, `68.141/68.441`.

All raw device-event samples, wall timings, throughput, and statistics are retained under `logs/local-run1.log`, `logs/local-run2.log`, and `logs/local-run3.log`. Device context is retained in `logs/device-context-pre-final.log` and `logs/device-context-post-final.log`.

- HBM on device 3: `7225/65536 MB` used, approximately `58311 MB` free.
- AICore on device 3: `1%` pre-run and `9%` post-run.
- Other process context: Python PID `3456615` present at `3850 MB`; other processes were not disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- `LOAD_QUALITY = LOW`
- `REPEATABILITY = FAIL / noisy direction across independent runs`
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Preserved non-final attempts

- `logs/local-run1-pre-whitespace-fix.log`, `logs/local-run2-pre-whitespace-fix.log`, and `logs/local-run3-pre-whitespace-fix.log` are preliminary measurements from a candidate with unintended indentation and are excluded from the result.
- `logs/compile-fail-1.log`, `logs/compile-fail-2.log`, `logs/correctness-fail-1.log`, and `logs/correctness-fail-2.log` are preserved setup/failure evidence and are not rewritten.

## Continuity timestamps

- `RULE_REFRESH_TIMESTAMP = 2026-10-08T19:37:49Z`
- `EDIT_TIMESTAMP = 2026-10-08T19:38:15Z` (first edit; final candidate whitespace fix completed before final Compile)
- `COMPILE_START_TIMESTAMP = 2026-10-08T19:49:26Z`
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T19:50:08Z`
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T19:50:38Z`
- `CORRECTNESS_PASS_TIMESTAMP = 2026-10-08T19:50:44Z`
- `LOCAL_START_TIMESTAMP = 2026-10-08T19:51:34Z`
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T19:52:40Z`
- `LOCAL_RESULT_TO_NEXT_EDIT_TIMESTAMP = recorded when V079 edit begins`
- `COMPILE_PASS_TO_CORRECTNESS_START_SECONDS = 30`
- `CORRECTNESS_PASS_TO_LOCAL_START_SECONDS = 50`
- `ONLINE = FORBIDDEN`
