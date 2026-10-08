# SYNC-BARRIER-ELISION-X V060 Result

- Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate `submission.asc` SHA256: `c866c465c0a3e5cb0726b7384602eecc4bf7f181cfb43391e823111745f57786`.
- Single change: delete only the generic `Process()` `SyncSToV()` after scalar `meanSquare` and immediately before `Duplicate(xFp32, meanSquare, 1)`; see `diff.patch`.
- Compile: PASS for `sync_barrier_elision_v060` and `sync_barrier_elision_correctness`, CANN `8.5.0.alpha002`; `logs/compile-v060.log`.
- Correctness: PASS_BITWISE, 7/7 FP16 cases, zero Parent/Candidate bit mismatches on device 3; `logs/correctness-v060-device3.log`.
- Local: FP16 8x128 on device 3; 31-pair same-binary qualification per binary and 62 interleaved Parent/Candidate event pairs in two blocks. Raw event, wall, throughput, qualification, and load data are retained in `logs/`.

## Raw-Derived Score

Primary estimator: `-100 * median(candidate_device_us - parent_device_us) / median(parent_device_us)` over all 62 paired device-event samples. The pooled paired median delta is `-1.2100 us`, Parent median is `14.4500 us`, and `LOCAL_SCORE=+8.373702%`.

| Block | P median / mean (us) | C median / mean (us) | Median paired delta (us) | Paired-median score |
|---|---:|---:|---:|---:|
| 1 (31 pairs) | 12.30 / 13.4013 | 6.28 / 11.9342 | -1.72 | +13.983740% |
| 2 (31 pairs) | 16.12 / 14.0103 | 13.26 / 13.5129 | -0.82 | +5.086849% |
| Pooled (62 pairs) | 14.45 / 13.7058 | 8.01 / 12.7235 | -1.21 | +8.373702% |

Pooled paired-delta mean/stdev/range is `-0.9823/9.5410 us`, `-24.26 to +30.50 us`. Device-event CV is `51.58%` Parent and `65.36%` Candidate. Pooled wall median/mean is `67.8300/70.1245 us` Parent and `68.4000/69.8860 us` Candidate. Throughput median/mean is `0.070875/0.098831 GElement/s` Parent and `0.128114/0.119691 GElement/s` Candidate.

The pooled ratio-of-independent-medians estimator is `+80.399501%`, materially different from the primary paired estimator. No samples were filtered; the difference and high event variance are retained as noise evidence.

## Qualification, Load, And Verdict

- Same-binary qualification: Parent first/second event medians `7.34/7.14 us`, median drift `2.7624%`; Candidate `9.80/5.66 us`, median drift `53.5576%`. Both qualification logs retain all 31 samples per binary.
- Device 3 snapshots showed `61,597 MB` free of `65,536 MB` HBM. A Python process (PID `2975184`, 563 MB) was present on device 3; it was left untouched. AICore readings at pre-qualification, pre-Local, between-block, and post-capture snapshots were `1/0/2/1%`.
- Host load averages (1/5/15 minutes) were `59.60/72.14/74.26`, `51.75/66.34/71.99`, `57.48/65.73/71.49`, and `62.92/65.58/70.69` at the four retained snapshots.
- Device 3 was released after capture at `2026-10-08T11:34:20Z`; see `DEVICE_RELEASE_RECEIPT.md` and `logs/device3-postcapture-release.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. The two block estimates vary, pooled event CVs are high, paired deltas span both large gains and regressions, and Candidate qualification drift is 53.5576%. Exact `R31B-V011` remains Local Best. This single-shape Local is not comparable to Official; no Online or shared-record writes were made.
