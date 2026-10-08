# V094 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V094`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1016`; Candidate SHA256 `cfa9be0899be047b47c92186188529e2c69a8eb476d898aa07b73005398c78d8`.
- `COMPILE=PASS`; Parent/Candidate correctness probe build `PASS`.
- `CORRECTNESS=PASS` on FP32 `128x1008/1016/1024`; all six Parent/Candidate checks returned `rc=0,bad=0`. Maximum absolute errors were `1.43051e-6`, `1.19209e-6`, and `1.66893e-6` by width.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded exact-Parent C15 FP32 `1x32768`. C15 was not rerun; this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

## Local Result

Device 2; FP32; six interleaved Parent/Candidate pairs per shape in alternating P,C / C,P order; 45 warmups; 31 device-event samples per block; two blocks; `batch_n=64`; no samples discarded. All 36 invocations returned `rc=0,bad=0`; all 36 raw TSVs contain 62 samples (2,232 total, 1,116 per side).

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Shape score is the arithmetic mean of its six pair speedups. Route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. The combined-MAD direction check is `abs(Parent median - Candidate median) <= Parent MAD + Candidate MAD`; calculations use unrounded raw TSV values.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1008 | 0.960750931, 0.942226399, 1.075318149, 0.990171979, 1.079732975, 1.059624463 | 1.017970815882x | +1.797081588% | 3/6 | 2/6 |
| 128x1016 | 0.953800167, 0.991695680, 0.947470698, 0.985828757, 1.005511938, 0.996804078 | 0.980185219534x | -1.981478047% | 1/6 | 4/6 |
| 128x1024 | 1.054555212, 0.952211863, 1.010018684, 0.929374480, 0.996582740, 0.982780340 | 0.987587219634x | -1.241278037% | 2/6 | 4/6 |
| Equal-shape geomean | - | **0.995114327762x** | **-0.488567224%** | **6/18** | **10/18** |

Pooled latency/jitter uses all 372 device-event samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Device events are primary; wall samples and per-invocation summaries are retained in the raw/stat files.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) |
|---|---|---:|---:|---:|---:|---:|---:|
| 128x1008 | Parent | 8.339103 / 7.866715 / 1.775758 | 21.294% | 0.314680 | 7.434279-8.855654 | 7.135940-21.811600 | 15.472168 |
| 128x1008 | Candidate | 7.816099 / 7.755155 / 0.371186 | 4.749% | 0.239690 | 7.435902-8.221808 | 7.214370-9.685310 | 16.507466 |
| 128x1016 | Parent | 7.614467 / 7.531715 / 0.300770 | 3.950% | 0.161245 | 7.310817-8.011812 | 7.081560-9.001250 | 17.079069 |
| 128x1016 | Candidate | 8.140120 / 7.723595 / 1.845041 | 22.666% | 0.197965 | 7.444123-8.274880 | 7.184380-29.329100 | 15.976177 |
| 128x1024 | Parent | 7.736329 / 7.627965 / 0.676792 | 8.748% | 0.212810 | 7.322904-8.136683 | 7.069370-16.632500 | 16.942401 |
| 128x1024 | Candidate | 7.852704 / 7.674215 / 1.126697 | 14.348% | 0.205470 | 7.422027-8.339377 | 6.905630-27.860000 | 16.691320 |

## Device and Verdict

- Fresh pre-correctness snapshot at `2026-10-08T16:32:13.105508458Z`: device 2 had 4,700/65,536 MB HBM used (conservative free 60,836 MB); AICore 1%, AIVector 5%. No MODE probe was active. The active process table listed existing PID 2055832 (`python train.py`, PromptKD), using 1,330 MB; it was not disturbed.
- Pre-Local snapshot at `2026-10-08T16:36:53.171313671Z`: HBM 4,700 MB used (about 60,836 MB free), HBM usage 7%, AICore 1%, AIVector 3%. Across 18 pair pre-snapshots, AICore ranged 0-15% (mean 3.833%), AIVector 2-8% (mean 4.556%), HBM usage stayed 7%, and HBM bandwidth was 0-2% (mean 0.389%). PID 2055832 remained present and untouched.
- Across 18 pair post-snapshots, AICore ranged 0-11% (mean 1.778%), AIVector 1-7% (mean 4.167%), HBM usage stayed 7%, and HBM bandwidth was 0-2% (mean 0.778%). Post-Local snapshot showed 4,701 MB HBM used; PID 2055832 remained present. Device 2 was released at `2026-10-08T16:43:10.358805760Z` after raw capture.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`. The external training process remained on the device; pooled CV reached 21-23% on Parent 128x1008 and Candidate 128x1016, with large outliers. Candidate was faster in 6/18 pairs and only 10/18 shifts were within combined MAD; no stable improvement is established.
- `LOCAL_SCORE=0.995114327762x` (partial route-local only); `LOCAL_DELTA=-0.488567224%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=R31B-V011`. V094 is not promoted.
- Local capture ran `2026-10-08T16:37:51.603077592Z`-`2026-10-08T16:43:10.356014607Z`.

All raw TSVs, per-invocation stats/stdout/stderr/return codes, pair pre/post device snapshots, process snapshots, probe identities, build logs, and the explicit release record are retained in this directory.
