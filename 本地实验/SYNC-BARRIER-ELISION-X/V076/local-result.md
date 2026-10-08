# V076 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V076`
- Direct parent and Local Best: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `dafe51fc1ea4582c3605222e282f3e68da556c8388ba3ca697fd00185e1f05f4`
- Single change: remove the post-batched-`ReduceSum` `SyncVToMTE2()` in `ProcessSmallLowPrecisionContiguousBatched` before `SyncVToS()`.
- Compile: PASS after direct build-environment fixes only. The initial Ninja-generator failure and the default-Makefiles header-path failure are retained in `compile-initial-fail.log` and `compile-default-fail.log`; final build is `/tmp/sync-v076-build.V076c2`.
- Correctness: PASS, 9/9 cases on device 3; eight FP16 cases and one BF16 case had zero parent/candidate bit mismatches. The initial runtime-library launch failure is retained in `logs/correctness-initial-fail.log`.

## Measurement

- Shape/dtype: `128x256 / bf16`; device-event timing; device 3; 60 warmups; 31 interleaved Parent/Candidate pairs per run.
- Local score formula: `-100 * paired_median(Candidate - Parent) / parent_median`; this single-shape score is not comparable to Official `45.16`.

| Run | Parent median us | Candidate median us | Paired delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Throughput delta | LOCAL_SCORE / DELTA |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 8.8600 | 9.5400 | +1.2000 | 3.698420 | 3.434801 | -0.263619 | -13.544015% |
| 2 | 17.0000 | 17.2600 | -0.0200 | 1.927529 | 1.898494 | -0.029035 | +0.117653% |
| 3 | 17.0200 | 14.4200 | -1.5200 | 1.925264 | 2.272399 | +0.347135 | +8.930672% |

- Final complete run score: `LOCAL_SCORE = +8.930672%`; `LOCAL_DELTA = +8.930672%` relative to the exact parent measurement for that run.
- Diagnostic score series: `-13.544015%`, `+0.117653%`, `+8.930672%`. The first run regressed while later runs improved; this is `POOR/NOISY`, not reliable evidence for promotion.
- Run 3 paired deltas span `-38.5600` to `+9.7000 us`; parent/candidate device-event CV is `0.56844/0.39567`. All raw samples are retained in `logs/local-run1.log`, `logs/local-run2.log`, and `logs/local.log`.
- Device context after the run: NPU 3 HBM usage rate `11%` of `65536 MB`, AICore `1%`, AIVector `3%`, temperature `41 C`, real-time power `106.9 W`; process `python` PID `3456615` used `3850 MB`. No process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; V076 is not promoted and `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Timing and continuity

- `EDIT_TIMESTAMP = 2026-10-08T19:19:17Z` (filesystem timestamp of the one-line Candidate edit)
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T19:21:32Z`
- `CORRECTNESS_FIRST_ATTEMPT_TIMESTAMP = 2026-10-08T19:21:57Z`
- `CORRECTNESS_PASS_START_TIMESTAMP = 2026-10-08T19:22:29Z`
- `LOCAL_START_TIMESTAMP = 2026-10-08T19:23:10Z`
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T19:24:17Z`
- `FIRST_COMPILE_TO_CORRECTNESS_START_SECONDS = 25`
- `COMPILE_TO_SUCCESSFUL_CORRECTNESS_START_SECONDS = 57`
- `CORRECTNESS_PASS_TO_LOCAL_START_SECONDS = 41`
- `LOCAL_RESULT_TO_NEXT_EDIT_SECONDS = recorded when V077 edit begins`
