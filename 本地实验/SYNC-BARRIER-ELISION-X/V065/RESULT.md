# SYNC-BARRIER-ELISION-X V065 Result

- Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: remove only the `PIPE_V` between row-wise FP16 gamma `Mul` and bias `Add` in the `width > 192` fallback of `ProcessSmallLowPrecisionContiguousBatched`.
- Candidate source SHA256: `7bd83702d84f8cfbaf6aa2fb2ebf909c3d9959b06f91b94a0d89adbfe4839f34`.
- Runner source SHA256: `be83a29b2226eb8de9d58e89db50958f92a81af26bddfc698a470379eca83aef`.
- Correctness executable SHA256: `adaf87cb09749833d2162229d807f79fb38a4254d25e96fb4451fe5e6df26802`.
- Compile: PASS for Candidate and correctness targets; see `logs/compile-v065.log`.

## Correctness

The V065 runner exited 0. All eight emitted FP16 cases passed bitwise Parent/Candidate comparison with zero mismatches, including the target `128x256` fallback case. The runner's final diagnostic says `cases=7`; this is a stale summary constant, not the emitted case count. All eight per-case PASS records are preserved in `logs/correctness-v065.log`.

## Qualification

The existing same-binary runner completed 31 Parent and 31 Candidate samples; both commands exited 0. These are diagnostic checks, not Local score samples.

| Binary | First/second median (us) | MAD/median | Median drift | Observed device-event range (us) |
|---|---:|---:|---:|---:|
| Parent R31B V011 | 27.20 / 23.86 | 14.3% / 18.9% | 13.1% | 7.12-52.90 |
| Candidate V065 | 7.70 / 7.42 | 8.3% / 4.3% | 3.7% | 6.78-28.32 |

All raw qualification samples and throughput/wall-time fields are in `logs/qualification-parent-v065.log` and `logs/qualification-candidate-v065.log`. Parent qualification was notably unstable; Candidate qualification also contains large outliers.

## Local

- Shape/dtype: `128x256`, FP16; device-event latency, with wall time and throughput recorded per sample.
- Method: 60 warmups, alternating Parent/Candidate order, 31 interleaved pairs per block; 62 raw pairs total.
- Device 3 assignment: pre-run and between-block snapshots showed 65,536 MB HBM, 5% used, 0% AICore, and no process on device 3. Host load was high: pre-block-1 averages `51.46/50.06/53.35`, between-block averages `53.04/49.82/53.05`, and post-capture averages `63.21/51.66/53.35`. Snapshots and command context are preserved under `logs/device3-v065-*.log`.
- Release: device 3 was explicitly released at `2026-10-08T15:36:43.762995519Z` after the post-capture snapshot in `logs/device3-v065-post-capture-release.log`.

The primary paired-median score is `-100 * median(Candidate - Parent) / median(Parent)`; a positive number means lower Candidate latency. The separate median-latency ratio is `100 * (1 - median(Candidate) / median(Parent))`. The mean-latency ratio is also shown because it points in the opposite direction here; it is not the primary score.

| Block | P median (us) | C median (us) | Median paired delta (us) | Paired-median score | Median-latency ratio | Mean-latency ratio |
|---|---:|---:|---:|---:|---:|---:|
| 1 (31 pairs) | 8.76 | 16.28 | +0.08 | -0.913228% | -85.844749% | +9.965028% |
| 2 (31 pairs) | 23.00 | 24.92 | -0.02 | +0.086953% | -8.347826% | -2.343832% |
| Pooled (62 pairs) | 19.55 | 20.05 | +0.06 | -0.306905% | -2.557545% | +2.450124% |

Pooled raw-derived summaries:

| Metric | Parent | Candidate |
|---|---:|---:|
| Device latency median / mean (us) | 19.5500 / 20.1306 | 20.0500 / 19.6374 |
| Device latency stdev / CV | 11.7498 / 0.58368 | 8.8922 / 0.45282 |
| Device latency MAD (us) | 9.7900 | 5.5800 |
| Device latency min-max (us) | 6.2200-68.6400 | 6.5000-36.5000 |
| Device latency p10-p90 (us) | 6.9760-32.2460 | 6.9900-32.0600 |
| Throughput median / mean (Gelem/s) | 1.6761 / 2.3002 | 1.6343 / 2.2241 |
| Wall-time median / mean (us) | 78.9415 / 84.5331 | 79.7575 / 84.6514 |

Pooled paired delta was median `+0.0600 us`, mean `-0.4932 us`, stdev `11.5179 us`, MAD `5.8400 us`, range `-46.9600..+18.8800 us`, p10/p90 `-12.6920/+12.6520 us`. The block medians shifted sharply and the paired score changes sign between blocks. The median-latency and mean-latency ratios also disagree. This result is therefore `LOCAL_REJECTED_NOISY`; retain the numeric scores but do not promote V065.

Raw per-pair device latency, throughput, wall time, and order are preserved in `logs/local-block1-v065.log` and `logs/local-block2-v065.log`. No samples were removed. This single-shape Local result is not an Official score or comparable to Official `45.16`.

- Current Local Best remains exact `R31B-V011`.
- Official/Online: none; `NOT_SUBMITTED`.
