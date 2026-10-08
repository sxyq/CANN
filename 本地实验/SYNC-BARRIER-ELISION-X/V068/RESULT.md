# SYNC-BARRIER-ELISION-X V068 Result

## Revision and Correctness

- Direct Parent and Local Best: exact `R31B-V011`; parent source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: delete only the `PIPE_V` after BF16 row-wise gamma `Mul` and before bias `Add` in the width-greater-than-192 fallback of `ProcessSmallLowPrecisionContiguousBatched`.
- Candidate source SHA256: `ca52cc2f9d8a5df5e1be99aaf0470c627bc6b80f4d16e5e502e43f8f7d6a2c9b`; the Parent/Candidate diff is exactly that one barrier deletion (`diff.patch`).
- Compile: `PASS` for Candidate and BF16-aware runner. Build directory `/tmp/sync-v068-build.fFQAMj`; see `logs/compile-v068.log`.
- Correctness: `PASS`, 9/9 bitwise Parent/Candidate cases on device 5: eight FP16 controls plus target BF16 `128x256`. See `logs/correctness-v068-device5.log`.
- Candidate executable SHA256: `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`.
- Correctness/Local runner SHA256: `61058ed987f28989287631af1aee23ba791e78c6b36e06450d3a61c21effed23`.

## Same-Binary Qualification

Parent and Candidate each completed 31 same-binary qualification pairs on device 5. These are stability diagnostics, not part of the 62 Local pairs.

| Binary | First/second median (us) | First/second MAD/median | First/second CV | Median drift | Range (us) |
|---|---:|---:|---:|---:|---:|
| Parent R31B V011 | 10.28 / 9.74 | 16.34% / 12.12% | 1.56955 / 1.29463 | 5.3946% | 8.00-147.12; second max 112.58 |
| Candidate V068 | 12.84 / 10.32 | 29.75% / 14.34% | 1.44825 / 1.75081 | 21.7617% | 8.36-193.88; second max 159.82 |

All qualification samples retain device-event latency, throughput, and wall time in `logs/qualification-v068-parent-device5-20261008T171021Z.log` and `logs/qualification-v068-candidate-device5-20261008T171047Z.log`. Both binaries show severe jitter and outliers.

## Local Measurement

- Shape/dtype: `128x256`, BF16. Primary metric is ACL device-event latency; throughput and wall time are retained for every sample.
- Method: 60 warmups per invocation; two interleaved blocks of 31 Parent/Candidate pairs, alternating pair order; 62 raw pairs total. No samples were removed. Raw logs: `logs/local-v068-block1-device5-20261008T171202Z.log` and `logs/local-v068-block2-device5-20261008T171236Z.log`.
- Device 5 was selected after fresh snapshots: device 2 remained excluded for MODE-DISPATCH V093; device 3 had an active Python NPU process. Device 5 had about 5.3 GB free HBM, but a resident `VLLMWorker_TP` process (PID 89602, 56,700 MB process memory) and about 45% AICore use throughout. The process was not disturbed. This loaded context limits measurement quality.
- Pre-Local snapshots: `logs/npu-snapshot-v068-pre-local-block1-device5-20261008T171142Z.log` and `logs/npu-snapshot-v068-pre-local-block2-device5-20261008T171218Z.log`. Post-capture snapshot and route timing release: `logs/npu-snapshot-v068-post-capture-release-device5-20261008T171301Z.log` at `2026-10-08T17:13:01Z`. No V068 device command ran after release.
- Additional pre-qualification snapshot `logs/npu-snapshot-v068-pre-qualification-device5-20261008T170957Z.log` has an in-file `SNAPSHOT_UTC=2026-10-08T17:09:57Z`; the filename was corrected without changing file contents.
- Host load averages were not captured contemporaneously with the Local blocks. NPU HBM/AICore/process snapshots are retained; HBM bandwidth was not available in those snapshots.

The primary paired-median score is `-100 * median(Candidate - Parent paired device-event delta) / median(Parent device-event latency)`; positive means lower Candidate latency. The pooled median-latency ratio is separately `100 * (1 - median(Candidate) / median(Parent))`; mean-latency ratio is an outlier-sensitive diagnostic.

| Block | P/C median (us) | P/C mean (us) | Median paired C-P (us) | Paired-median score | Median-latency ratio | Mean-latency ratio |
|---|---:|---:|---:|---:|---:|---:|
| 1 (31 pairs) | 10.04 / 9.28 | 18.113548 / 18.494194 | +0.02 | -0.199203% | +7.569721% | -2.101439% |
| 2 (31 pairs) | 9.18 / 10.98 | 10.707097 / 12.782581 | +1.82 | -19.825708% | -19.607843% | -19.384189% |
| Pooled (62 pairs) | 9.39 / 10.17 | 14.410323 / 15.638387 | +0.54 | -5.750799% | -8.306709% | -8.522117% |

Pooled raw-derived distributions:

| Metric | Parent | Candidate |
|---|---:|---:|
| Device latency median / mean (us) | 9.3900 / 14.410323 | 10.1700 / 15.638387 |
| Device latency stdev / CV | 16.406262 / 1.138508 | 19.492060 / 1.246424 |
| Device latency MAD (us) | 1.0900 | 1.3600 |
| Device latency min-max (us) | 8.0200-102.0400 | 8.0800-119.3200 |
| Device latency p10-p90 (us) | 8.3900-15.3800 | 8.6860-15.0760 |
| Throughput median / mean (Gelem/s) | 3.489674 / 3.084114 | 3.222029 / 2.969721 |
| Wall-time median / mean (us) | 153.2110 / 165.242371 | 141.9260 / 159.023952 |

The pooled paired C-P delta has median `+0.5400 us`, mean `+1.228065 us`, stdev `26.175369 us`, MAD `3.1300 us`, range `-87.5800..+105.9200 us`, and p10/p90 `-6.2140/+6.2620 us`. Values above 50 us occur in 3 Parent and 3 Candidate samples, all in block 1. Block 1's median-latency ratio is positive while its paired-median score is nearly flat; block 2 regresses on all three score definitions. This block disagreement, qualification jitter, and resident VLLM load make the pooled numeric result noisy and unsuitable for promotion.

## Verdict

- `LOCAL_SCORE=-5.750799%` by the primary paired-median formula; `LOCAL_DELTA=+0.5400 us` median paired Candidate-minus-Parent device latency.
- Separate pooled median-latency ratio: `-8.306709%`; mean-latency ratio: `-8.522117%`.
- Verdict: `LOCAL_REJECTED_NOISY`. No promotion; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
- This single-shape Local result is not an Official score and is not comparable to Official `45.16`. No Online action was taken.
