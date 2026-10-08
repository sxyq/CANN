# SYNC-BARRIER-ELISION-X V058 Result

- Direct Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `submission.asc`, SHA256 `2af2676bd1d076403e1bad7c4e8bcfaa7fdf5f20da1440e61d89ea5a2dadd72d`.
- Single change: delete only the `PIPE_V` barrier immediately after batched FP16 bias `Add` in `ApplyFp16GammaBiasBatch`; exact source diff is `diff.patch`.
- Compile: PASS, target `sync_barrier_elision_v058`; CMake configure/build logs are `logs/compile-v058-configure-retry-env.log` and `logs/compile-v058-build-retry-env.log`. One initial shell wrapper stopped in toolkit setup because `set -u` treated unset `LD_LIBRARY_PATH` as an error; CMake had not started. The environment-only retry without nounset passed. Candidate executable SHA256 `68d57871ed0ed9e3e41450dbba77bea9a6400a434f8e4736249a4d4a3b2de74c`.
- Correctness: PASS, 7/7 FP16 Parent/Candidate bitwise cases with zero mismatches, `logs/correctness-v058-device3-20261008T061925Z.log`. Correctness runner SHA256 `d2bc2686415cc24d45c042b3008181845846476996c4bf684faab0ee15fb3b16`.
- Local: FP16 128x128, device-event timing, 60 warmups per invocation; two complete alternating 31-pair blocks (62 total). All raw samples, throughput, wall latency, and runner summaries are retained. The runtime-reported systemError occurred after these commands completed; read-only process inspection found no active runner, and neither block was rerun.

## Score Reconciliation

The primary route-local score uses the paired interleaved sample estimator:

`-100 * median(candidate_device_us - parent_device_us) / median(parent_device_us)`

For the 62 raw pairs, median paired delta (Candidate minus Parent) is `-0.7600 us`; pooled Parent median is `16.3300 us`, giving `LOCAL_SCORE=+4.654011%`. This matches the runner's per-block estimator. Block 1 is `+5.114823%` (median paired delta `-0.9800 us`); block 2 is `+5.333333%` (`-0.4800 us`).

| Block | P median / mean (us) | C median / mean (us) | Paired-median score | Ratio-of-medians score |
|---|---:|---:|---:|---:|
| 1 (31 pairs) | 19.1600 / 18.6445 | 8.2600 / 14.9697 | +5.114823% | +56.889353% |
| 2 (31 pairs) | 9.0000 / 14.0181 | 8.6200 / 12.2942 | +5.333333% | +4.222222% |
| Pooled (62 pairs) | 16.3300 / 16.3313 | 8.5800 / 13.6319 | +4.654011% | +47.458665% |

The ratio-of-independent-medians is retained as a sensitivity value, not interpreted as a robust gain: Parent median shifts from `19.16 us` in block 1 to `9.00 us` in block 2, and Candidate's from `8.26 us` to `8.62 us`. Cross-block baseline movement makes the pooled independent medians particularly misleading. Ratio-of-means is `+16.528730%`, another estimator with a different sensitivity to outliers. No samples were removed.

Pooled device-event stdev/CV/range: Parent `9.5013 us / 58.18% / 7.38-59.00 us`; Candidate `8.4754 us / 62.17% / 6.76-51.54 us`. Paired-delta mean/stdev/range is `-2.6994/12.1397 us`, `-51.06 to +29.68 us`. Pooled wall-latency median/mean: Parent `68.5750/72.8912 us`; Candidate `68.8000/71.5834 us`. Pooled throughput median/mean: Parent `1.003416/1.314794 GElement/s`; Candidate `1.909598/1.540600 GElement/s`.

## Qualification, Load, And Verdict

- Same-binary qualification: Parent 31 pairs, first/second device-event CV `44.774%/80.930%`, median drift `48.672%`; Candidate 31 pairs, CV `47.932%/40.198%`, median drift `3.149%`. Raw logs: `logs/same-binary-parent-v058-device3-20261008T062100Z.log` and `logs/same-binary-candidate-v058-device3-20261008T062232Z.log`.
- Device 3 pre-NPU snapshot: HBM `7,284/65,536 MB` used (`58,252 MB` free), AICore 0%; listed PID `3836347` (`python`, 3,908 MB). Pre-Local and between-block snapshots retained; HBM stayed near 7,285 MB and AICore was 0%. Post-capture: HBM `7,289/65,536 MB`, AICore 4%, same process remained present and untouched. Resume snapshot also retained. Device 3 was explicitly released after capture at `2026-10-08T07:03:12Z`; see `logs/device3-release-20261008T070312Z.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. The paired estimator is modest and directionally similar across blocks, but Parent same-binary drift is 48.7%, pooled Parent/Candidate device CV is 58-62%, block medians shift substantially, and alternate pooled estimators range from +4.65% to +47.46%. Do not promote V058; exact `R31B-V011` remains Local Best. This single-shape Local score is not comparable to Official `45.16`. No Online or shared-record writes were made.
