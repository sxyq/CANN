# SYNC-BARRIER-ELISION-X V066 Result

## Revision and Correctness

- Direct Parent and Local Best: exact `R31B-V011`; Parent source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: remove only the `PIPE_V` immediately after row-wise bias `Add` in the FP16 `width > 192` fallback of `ProcessSmallLowPrecisionContiguousBatched`.
- Candidate source SHA256: `1676a5fa9125c2670797f3c68ac90762c488336bc16fb8fb554c7a14f61d0efc`; the parent/candidate diff contains exactly this one deletion.
- Compile: `PASS` for the Candidate and correctness targets. The initial host `<vector>` failures and the host-only C++ include-path fix remain in the three `compile-v066*.log` files; the kernel hypothesis was unchanged.
- Correctness: `PASS`, 8/8 FP16 Parent/Candidate bitwise cases, including the target `128x256` case on device 3. The earlier device-2 log is retained but excluded from the authoritative result because device 2 was allocated to MODE-DISPATCH V093.

## Same-Binary Qualification

Both binaries completed 31 same-binary samples on device 3. These are qualification diagnostics, not Local score samples.

| Binary | First/second median (us) | First/second MAD/median | First/second CV | Median drift | Range (us) |
|---|---:|---:|---:|---:|---:|
| Parent R31B V011 | 16.50 / 15.76 | 17.45% / 13.45% | 0.37468 / 0.45820 | 4.5877% | 6.52-46.36 |
| Candidate V066 | 27.38 / 26.94 | 43.61% / 23.39% | 0.59439 / 0.58277 | 1.6200% | 6.28-57.74 |

The Parent and Candidate qualification logs preserve every device-event latency, throughput, wall-time sample, and summary statistic. Qualification was noisy, especially for the Candidate.

## Local Measurement

- Shape/dtype: `128x256`, FP16; device-event latency is primary, wall time and throughput are retained as diagnostics.
- Device: NPU 3, Ascend 910B3. Device 2 was excluded by the MODE-DISPATCH V093 allocation. Fresh snapshots before Candidate qualification and each Local block showed device 3 with 5% HBM used (about 62.1 GB free), 0% AICore, and no running NPU process. Device 3 was explicitly released after capture at `2026-10-08T16:20:21.503298934Z`.
- Host load averages (1/5/15 min): pre-qualification `68.62/57.34/54.31`; pre-block 1 `67.83/59.07/55.06`; pre-block 2 `51.32/55.85/54.13`; post-capture `38.29/51.23/52.65`. Snapshots are retained in `logs/`.
- Method: 60 warmups per invocation, two interleaved blocks of 31 Parent/Candidate pairs, alternating pair order. All 62 pairs are retained without filtering in `logs/local-v066-block1.log` and `logs/local-v066-block2.log`.

The primary score follows the Route's established formula: `-100 * median(Candidate - Parent paired device-event delta) / median(Parent device-event latency)`. Positive means lower Candidate latency. The separate pooled median-latency ratio is `100 * (1 - median(Candidate) / median(Parent))`; the mean-latency ratio is an outlier-sensitive diagnostic.

| Block | P/C median (us) | P/C mean (us) | Median paired C-P (us) | Paired-median score | Median-latency ratio | Mean-latency ratio |
|---|---:|---:|---:|---:|---:|---:|
| 1 (31 pairs) | 10.06 / 9.28 | 11.968387 / 14.812903 | -0.08 | +0.795229% | +7.753479% | -23.766913% |
| 2 (31 pairs) | 7.82 / 7.60 | 11.078710 / 9.930968 | -0.24 | +3.069054% | +2.813299% | +10.359888% |
| Pooled (62 pairs) | 8.52 / 7.74 | 11.523548 / 12.371935 | -0.20 | +2.347418% | +9.154930% | -7.362204% |

Pooled raw-derived distributions:

| Metric | Parent | Candidate |
|---|---:|---:|
| Device latency median / mean (us) | 8.5200 / 11.523548 | 7.7400 / 12.371935 |
| Device latency stdev / CV | 5.431702 / 0.471357 | 14.231981 / 1.150344 |
| Device latency MAD (us) | 1.5800 | 0.9900 |
| Device latency min-max (us) | 6.4600-28.2200 | 6.5200-101.5800 |
| Device latency p10-p90 (us) | 6.9840-19.5660 | 6.7420-18.7840 |
| Throughput median / mean (Gelem/s) | 3.846095 / 3.393844 | 4.234299 / 3.668459 |
| Wall-time median / mean (us) | 78.5010 / 79.705371 | 78.2705 / 86.760000 |

The pooled paired C-P delta has median `-0.2000 us`, mean `+0.848387 us`, stdev `12.867078 us`, MAD `2.9900 us`, range `-13.1000..+73.3600 us`, and p10/p90 `-11.4980/+8.9980 us`. The Candidate has a `101.5800 us` device-event outlier and a `329.6640 us` wall-time outlier. Block medians shifted from `10.06/9.28 us` to `7.82/7.60 us` (Parent/Candidate), while host load also changed. The two block paired-median scores agree in sign, but high qualification jitter, the Candidate outlier, broad paired deltas, and the pooled mean-latency regression make this result noisy. The positive numeric score is retained but is not promoted.

## Verdict

- `LOCAL_SCORE=+2.347418%` using the primary paired-median formula; `LOCAL_DELTA=-0.2000 us` median paired Candidate-minus-Parent device latency.
- Separate pooled median-latency ratio: `+9.154930%`; mean-latency ratio: `-7.362204%`.
- Verdict: `LOCAL_REJECTED_NOISY`. `CURRENT_LOCAL_BEST` remains exact `R31B-V011`; V066 is not the parent for a later sibling.
- This single-shape Local score is not an Official score and is not comparable to Official `45.16`. No Online action was taken.
- Raw qualification, correctness, Local, compile-fix, and resource-snapshot evidence remains in this V066 directory.
