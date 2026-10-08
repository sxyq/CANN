# SYNC-BARRIER-ELISION-X V067 Result

## Revision and Correctness

- Direct Parent and Local Best: exact `R31B-V011`; Parent source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: remove only the `PIPE_V` immediately after row-wise BF16 bias `Add` in the `width > 192` fallback of `ProcessSmallLowPrecisionContiguousBatched`.
- Candidate source SHA256: `f9b114bd621d81f059c851db3e6e19c10de7b089a16a46a51d953faa3a914225`. The source diff is exactly one barrier deletion.
- Compile: `PASS` for the Candidate and BF16-aware correctness/Local runner. Build directory: `/tmp/sync-v067-build.NUSG9l`; successful log: `logs/compile-v067.log`.
- Correctness: `PASS`, 9/9 bitwise Parent/Candidate cases on device 5: eight FP16 controls and the target BF16 `128x256` case. The BF16 case exercises the row-wise fallback. Raw log: `logs/correctness-v067-device5.log`.
- Candidate executable SHA256: `b84e74349d5f84c9687f3696d1a44d1c008916e984fb21a1c8ec19588de7ef49`.
- Correctness/Local runner executable SHA256: `de86f67d77f99a49a427f31b3acccbfd89bc6ad99ba398fd2188b75150ac94c3`.

## Same-Binary Qualification

Parent and Candidate each completed 31 same-binary qualification pairs on device 5. These diagnostics are separate from the Local score samples.

| Binary | First/second median (us) | First/second MAD/median | First/second CV | Median drift | Range (us) |
|---|---:|---:|---:|---:|---:|
| Parent R31B V011 | 10.88 / 9.52 | 22.43% / 10.92% | 1.42485 / 1.74065 | 13.3333% | 7.90-197.94 |
| Candidate V067 | 11.46 / 9.02 | 20.77% / 8.20% | 1.30516 / 1.73212 | 23.8281% | 8.06-160.78 |

Every qualification sample retains device-event latency, throughput, and wall time in `logs/qualification-parent-v067-device5.log` and `logs/qualification-candidate-v067-device5.log`. Both binaries show substantial jitter and outliers.

## Local Measurement

- Shape/dtype: `128x256`, BF16. Primary metric is device-event latency; throughput and wall time are retained for each sample.
- Method: 60 warmups per invocation; two interleaved blocks of 31 Parent/Candidate pairs; alternating pair order; 62 raw pairs total. No samples were removed. Raw logs: `logs/local-v067-block1-device5.log` and `logs/local-v067-block2-device5.log`.
- Device: NPU 5, Ascend 910B3. Device 2 was excluded because MODE-DISPATCH V093 remained assigned there. Device 3 was skipped because its fresh snapshot showed an active Python NPU process. Device 5 had at least 5.3 GB free HBM but a resident VLLM process throughout the measurement (about 91% HBM use, 46% AICore, and 47% HBM bandwidth use); the process was not disturbed. This load is retained as the principal measurement-quality limitation.
- Host load averages (1/5/15 min): post-Compile/pre-Correctness `74.53/64.89/62.19`; post-Correctness/pre-qualification `63.80/64.35/62.26`; pre-block 1 `63.62/62.93/61.93`; pre-block 2 `87.41/69.15/64.08`; post-capture `63.99/66.45/63.53`. Full device/process snapshots are retained in `logs/npu-snapshot-v067-*.log`.
- Device 5 Route timing was explicitly released after the post-capture snapshot at `2026-10-08T16:50:16.097879877Z`; the VLLM process remained running and untouched.

The primary paired-median score is `-100 * median(Candidate - Parent paired device-event delta) / median(Parent device-event latency)`; positive means lower Candidate latency. The pooled median-latency ratio is separately reported as `100 * (1 - median(Candidate) / median(Parent))`. Mean-latency ratio is an outlier-sensitive diagnostic.

| Block | P/C median (us) | P/C mean (us) | Median paired C-P (us) | Paired-median score | Median-latency ratio | Mean-latency ratio |
|---|---:|---:|---:|---:|---:|---:|
| 1 (31 pairs) | 10.32 / 13.36 | 27.474194 / 38.872258 | +1.68 | -16.279070% | -29.457364% | -41.486439% |
| 2 (31 pairs) | 10.48 / 11.76 | 24.098710 / 29.693548 | +1.40 | -13.358779% | -12.213740% | -23.216341% |
| Pooled (62 pairs) | 10.35 / 13.12 | 25.786452 / 34.282903 | +1.47 | -14.202899% | -26.763285% | -32.949286% |

Pooled raw-derived distributions:

| Metric | Parent | Candidate |
|---|---:|---:|
| Device latency median / mean (us) | 10.3500 / 25.786452 | 13.1200 / 34.282903 |
| Device latency stdev / CV | 39.406684 / 1.528193 | 48.089077 / 1.402714 |
| Device latency MAD (us) | 1.6200 | 4.0900 |
| Device latency min-max (us) | 8.0000-193.5400 | 7.9600-211.9400 |
| Device latency p10-p90 (us) | 8.7200-68.4440 | 8.4860-105.3100 |
| Throughput median / mean (Gelem/s) | 3.166017 / 2.714342 | 2.497561 / 2.383814 |
| Wall-time median / mean (us) | 216.7120 / 194.931710 | 197.6965 / 189.987694 |

The pooled paired C-P delta has median `+1.4700 us`, mean `+8.496452 us`, stdev `58.796155 us`, MAD `4.1700 us`, range `-166.4600..+201.5000 us`, and p10/p90 `-41.4420/+72.4740 us`. Values above 50 us occur in 9 Parent and 14 Candidate Local samples. The Candidate has a `211.9400 us` device-event maximum and a `416.7130 us` wall-time maximum. Both blocks' paired-median scores are negative, but the extreme jitter, outliers, resident VLLM load, and high qualification drift prevent a reliable performance conclusion.

## Verdict

- `LOCAL_SCORE=-14.202899%` using the primary paired-median formula; `LOCAL_DELTA=+1.4700 us` median paired Candidate-minus-Parent device latency.
- Separate pooled median-latency ratio: `-26.763285%`; mean-latency ratio: `-32.949286%`.
- Verdict: `LOCAL_REJECTED_NOISY`. No promotion; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
- This single-shape Local result is not an Official score and is not comparable to Official `45.16`. No Online action was taken.
