# SYNC-BARRIER-ELISION-X V056 Result

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `submission.asc`, SHA256 `fd3c4c4929c163dab28a4c24cbd28f85202bd32bda26f2030b298c995bb6bdc8`.
- Change: delete only the FP16 batched-path `PIPE_V` barrier after in-place Add and before ToFloat.
- Compile: PASS, `logs/compile-v056-20261008.log`.
- Correctness: PASS, 7/7 FP16 parent/candidate bitwise cases, zero mismatches, `logs/correctness-v056-device3-run1-20261008.log`.
- Local: device-event timings, FP16 128x128, 60 warmups per binary per block, 31 interleaved Parent/Candidate pairs per block, two blocks (62 pairs total). Pair order alternated P,C / C,P. Full raw samples and wall latency/throughput are in `logs/local-interleaved-v056-device3-20261008.log` and `logs/local-interleaved-v056-block2-device3-20261008.log`.

## Score Reconciliation

Score is the raw-derived pooled ratio-of-medians latency reduction:

`100 * (Parent median device_us - Candidate median device_us) / Parent median device_us`

Positive means lower Candidate latency. The 62 pooled device-event samples give Parent median/mean `13.7700/14.7206 us` and Candidate median/mean `9.4200/12.3884 us`, so Local score is `+31.590414%`. Pooled mean paired delta (Candidate minus Parent) is `-2.3323 us`; pooled median paired delta is `-1.4000 us`.

| Block | P median / mean (us) | C median / mean (us) | Ratio-of-medians score | Median paired delta (C-P, us) |
|---|---:|---:|---:|---:|
| 1 (31 pairs) | 12.1000 / 14.0077 | 8.0400 / 10.9232 | +33.553719% | -1.5200 |
| 2 (31 pairs) | 15.1400 / 15.4335 | 15.2400 / 13.8535 | -0.660502% | -0.7600 |
| Pooled (62 pairs) | 13.7700 / 14.7206 | 9.4200 / 12.3884 | +31.590414% | -1.4000 |

The runner also reports `-100 * median(paired C-P delta) / Parent median` as `+10.167030%` pooled. This is a different estimator: the median of paired differences is not the difference between the independent Parent and Candidate medians. The pooled ratio-of-medians above is the stated Local score; both are retained to make the discrepancy explicit. Block 1 and block 2 ratio-of-medians scores have opposite signs.

## Quality And Load

- Pooled device latency: Parent stdev `6.9545 us`, CV `47.24%`, MAD `5.7200 us`, range `5.88-32.04 us`; Candidate stdev `6.1133 us`, CV `49.35%`, MAD `3.0600 us`, range `5.94-33.76 us`.
- Pooled wall latency median/mean: Parent `64.8810/69.0406 us`; Candidate `63.6260/64.8344 us`.
- Pooled throughput median/mean: Parent `1.190889/1.392112 GElement/s`; Candidate `1.742420/1.632567 GElement/s`.
- Same-binary qualification used 31 pairs per binary. Parent first/second MAD-over-median was `0.252955/0.210784`, median drift `0.036101`; Candidate was `0.190361/0.307514`, median drift `0.041298`. Full samples and summaries are in the two `qualification-*-v056-device3-20261008.log` files.
- Device 3 snapshots before use, before Local, between blocks, and after capture are retained under `logs/device3-*-v056-*.log`. HBM stayed at `9,123/65,536 MB` used (`56,413 MB` free); the listed device-3 process PID 2532307 (Python, 5,750 MB) was not disturbed. AICore was 0% before the run and 1% between/after. Other device load is present in the raw snapshots.
- Device 3 Local assignment was explicitly released at `2026-10-08T04:21:58Z` after the post-capture snapshot `logs/device3-postcapture-snapshot-v056-20261008T0420Z.log`.

## Verdict

`LOCAL_REJECTED_NOISY`. The numeric pooled score is positive, but qualification MAD was high, pooled latency CV was about 47-49%, and the two Local blocks disagree in score direction. Do not promote V056; exact R31B-V011 remains Local Best. This single-shape Local result is not comparable to Official `45.16`. No Online or shared-record writes were made.
