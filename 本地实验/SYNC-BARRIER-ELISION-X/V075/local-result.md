# V075 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V075`
- Direct parent and Local Best: exact `R31B-V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `06eb0799277089b4479781cac94edaec0668cd6fc74b2c7f2ccdf920058a3d6a`
- Compile: PASS; artifact and correctness runner were built under `/tmp/sync-v075-build.V075c1`.
- Correctness: PASS, 9/9 cases on device 3; eight FP16 cases and one BF16 case had zero parent/candidate bit mismatches.

## Measurement

- Shape/dtype: `128x256 / bf16`; device-event timing; device 3; 60 warmups; 31 interleaved Parent/Candidate pairs per run.
- Local score formula: `-100 * paired_median(Candidate - Parent) / parent_median`; this single-shape score is not comparable to Official `45.16`.

| Run | Parent median us | Candidate median us | Paired delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Throughput delta | LOCAL_SCORE / DELTA |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 14.2200 | 14.0800 | -0.1200 | 2.304360 | 2.327273 | +0.022913 | +0.843875% |
| 2 | 15.7800 | 15.0000 | -0.6600 | 2.076553 | 2.184533 | +0.107980 | +4.182512% |
| 3 | 11.7600 | 17.2200 | +0.2600 | 2.786395 | 1.902904 | -0.883491 | -2.210885% |

- Final complete run score: `LOCAL_SCORE = -2.210885%`; `LOCAL_DELTA = -2.210885%` relative to the exact parent measurement for that run.
- Diagnostic score series: `+0.843875%`, `+4.182512%`, `-2.210885%`; arithmetic mean `+0.938501%`. The opposite directions and large paired-delta spread make this `POOR/NOISY`, not a reliable improvement.
- Run 3 paired deltas span `-23.0400` to `+18.3000 us`; parent/candidate device-event CV is `0.46323/0.44508`. All raw samples are retained in `logs/local-run1.log`, `logs/local-run2.log`, and `logs/local.log`.
- Run 3 load context: pre HBM `7225/65536 MB` used (`58311 MB` free), AICore `0%`, post HBM `7228/65536 MB` used (`58308 MB` free), AICore `1%`; Python PID `3456615` remained present at about `3850 MB`. No process was disturbed.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- Verdict: `LOCAL_REJECTED_NOISY`; V075 is not promoted and `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Timing and continuity

- `EDIT_TIMESTAMP = 2026-10-08T19:05:24Z`
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T19:06:30Z`
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T19:06:58Z`
- `LOCAL_START_TIMESTAMP = 2026-10-08T19:07:21Z`
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T19:08:56Z`
- `NON_EXECUTION_GAP_COMPILE_TO_CORRECTNESS_SECONDS = 28`
- `LOCAL_RESULT_TO_NEXT_EDIT_SECONDS = recorded when V076 edit begins`
