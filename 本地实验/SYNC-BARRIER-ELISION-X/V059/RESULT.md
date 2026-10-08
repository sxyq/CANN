# SYNC-BARRIER-ELISION-X V059 Result

- Direct Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `submission.asc`, SHA256 `f605ee1765eef7818485101d3d6621ed2d4ff35ae3758cf033bee8d7dcfe1030`.
- Single change: delete only the common `PIPE_V` barrier after the dtype-specific input-combine branch and before the batched square `Mul` in `ProcessSmallLowPrecisionContiguousBatched`; see `diff.patch`.
- Compile: PASS, target `sync_barrier_elision_v059`; CANN `8.5.0.alpha002`; log `logs/compile-v059.log`. Candidate executable SHA256 `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`.
- Correctness: PASS_BITWISE, 7/7 FP16 Parent/Candidate cases with zero mismatches on device 3; `logs/correctness-v059-device3.log`.
- Local: FP16 128x128 on device 3; two 31-pair interleaved blocks, 62 total. Same-binary qualification completed for Parent and Candidate (31 pairs each). All event, wall, throughput, and qualification samples remain in the logs.

## Raw-Derived Score

Primary estimator is `-100 * median(candidate_device_us - parent_device_us) / median(parent_device_us)` over all 62 paired device-event samples. The pooled median paired delta is `-0.2800 us`, Parent median is `15.6200 us`, and `LOCAL_SCORE=+1.792574%`.

| Block | P median / mean (us) | C median / mean (us) | Median paired delta (us) | Paired-median score |
|---|---:|---:|---:|---:|
| 1 (31 pairs) | 14.78 / 12.6523 | 14.16 / 13.2626 | -0.16 | +1.082544% |
| 2 (31 pairs) | 16.00 / 14.1897 | 12.40 / 12.2942 | -0.80 | +5.000000% |
| Pooled (62 pairs) | 15.62 / 13.4210 | 13.36 / 12.7784 | -0.28 | +1.792574% |

Sensitivity estimators are materially different: pooled ratio-of-independent-medians `+16.916168%`, ratio-of-means `+5.028652%`, and geometric mean of paired Parent/Candidate speed ratios `+7.502902%`. Block geometric-mean scores are `-3.011824%` and `+19.157557%`. These are not interchangeable estimators; no samples were removed.

Pooled device-event standard deviation/CV/range: Parent `5.6229 us / 41.90% / 5.94-26.06 us`; Candidate `6.3747 us / 49.89% / 5.84-32.26 us`. Paired delta mean/stdev/range is `-0.6426/8.1677 us`, `-17.02 to +14.06 us`. Pooled wall-latency median/mean: Parent `62.4610/66.6664 us`; Candidate `63.1355/64.1769 us`. Throughput median/mean: Parent `1.048913/1.526759 GElement/s`; Candidate `1.230761/1.653640 GElement/s`.

## Qualification, Load, And Verdict

- Same-binary Parent: 31 pairs, first/second event medians `13.48/16.76 us`, CV `52.52%/53.26%`, median drift `21.6931%`.
- Same-binary Candidate: 31 pairs, first/second event medians `15.40/15.48 us`, CV `58.65%/48.64%`, median drift `0.5181%`.
- Device 3 snapshots consistently showed HBM `3,428/65,536 MB` used (`62,108 MB` free), AICore `0%`, and no NPU-3 process. Host load averages were high: pre-qualification `61.88/62.60/61.26`, pre-Local `58.74/63.49/61.84`, between blocks `53.83/60.89/61.10`, and post-capture `49.67/56.33/59.32`. Other visible host/device processes were left untouched.
- Device 3 was explicitly released after raw result capture at `2026-10-08T07:55:01Z`; see `DEVICE_RELEASE_RECEIPT.md` and `logs/device3-postcapture-release.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. The paired estimator is mildly positive in both blocks, but Parent qualification drift, pooled CVs, broad paired-delta range, and cross-estimator/block spread make the result non-promotable. Exact `R31B-V011` remains Local Best. This local score is not an Official score. No Online or shared-record writes were made.
