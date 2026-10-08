# V095 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V095`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1012`; Candidate SHA256 `4f6273566422a96ead41a66e09104618d740ac5f2041f900bf6917833f0d81fb`.
- `COMPILE=PASS`; CMake configure and `device`/`submission` targets returned rc 0.
- `CORRECTNESS=PASS` for exact Parent and Candidate on FP32 `128x1008/1016/1024`; all six retry checks returned `rc=0,bad=0`. Maximum absolute errors by width were `1.43051e-6`, `1.19209e-6`, and `1.66893e-6`.
- The first launch attempt returned `rc=127` on all six commands because `libgraph.so` was absent from that shell's runtime library path; no kernel ran. The exact failures are preserved. After sourcing the recorded toolkit environment and applying the existing runtime library path, all correctness checks passed.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded exact-Parent C15 FP32 `1x32768`; C15 was not run and this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

## Local Method and Score

Device 2; FP32; six interleaved Parent/Candidate pairs per shape, alternating P,C / C,P; 45 warmups; 31 device-event samples in each of two blocks per invocation; `batch_n=64`; no samples discarded. All 36 Local invocations returned `rc=0,bad=0`; all 36 raw TSVs contain 62 event samples (2,232 total, 1,116 per side).

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Shape score is the arithmetic mean of its six pair speedups. Route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. The combined-MAD check is `abs(Parent median - Candidate median) <= Parent MAD + Candidate MAD`. Calculations use raw TSV values.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1008 | 1.022441394, 0.940317356, 1.031172099, 0.963955887, 0.974009710, 0.957196912 | 0.981515559849x | -1.848444015% | 2/6 | 3/6 |
| 128x1016 | 0.981263496, 0.992324042, 0.994844965, 0.966462244, 1.085582006, 1.027040090 | 1.007919473736x | +0.791947374% | 2/6 | 4/6 |
| 128x1024 | 0.932130501, 1.038537821, 1.024353901, 0.968519139, 0.976090664, 0.956981562 | 0.982768931420x | -1.723106858% | 2/6 | 1/6 |
| Equal-shape geomean | - | **0.990660425923x** | **-0.933957408%** | **6/18** | **8/18** |

Pooled latency/jitter uses all 372 device-event samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Device events are primary; host-wall samples are retained in each raw TSV.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) |
|---|---|---:|---:|---:|---:|---:|---:|
| 128x1008 | Parent | 7.655112 / 7.616565 / 0.260946 | 3.409% | 0.176410 | 7.355531-7.994286 | 6.900000-8.621560 | 16.854620 |
| 128x1008 | Candidate | 7.876506 / 7.802185 / 0.402888 | 5.115% | 0.207190 | 7.488659-8.346062 | 7.279060-11.444100 | 16.380866 |
| 128x1016 | Parent | 7.771015 / 7.726870 / 0.401011 | 5.160% | 0.333275 | 7.294342-8.306721 | 7.098120-9.170310 | 16.735009 |
| 128x1016 | Candidate | 7.736333 / 7.645315 / 0.351930 | 4.549% | 0.169535 | 7.439690-8.108159 | 7.241250-9.526870 | 16.810030 |
| 128x1024 | Parent | 7.599179 / 7.576875 / 0.278332 | 3.663% | 0.198125 | 7.289442-7.928998 | 7.033130-8.870940 | 17.248180 |
| 128x1024 | Candidate | 7.759208 / 7.661720 / 0.404053 | 5.207% | 0.146250 | 7.446471-8.156062 | 6.752190-12.325000 | 16.892445 |

## Device Context and Verdict

- Fresh pre-Local snapshot at `2026-10-08T17:17:50.604750723Z`: device 2 used 4,699/65,536 MB HBM (`FREE_HBM=60,837 MB`), HBM usage 7%, AICore 1%, AIVector 2%.
- Across 18 pair pre-snapshots, HBM used 4,699-4,702 MB (mean 4,701.6); AICore ranged 1-15% (mean 6.111%), AIVector 2-7% (mean 3.333%), and HBM bandwidth 0-2% (mean 0.667%). Across 18 post-snapshots, HBM used 4,701-4,702 MB (mean 4,701.9); AICore ranged 0-14% (mean 4.667%), AIVector 0-6% (mean 3.444%), and HBM bandwidth 0-2% (mean 0.667%).
- Existing PID 2055832 (`python train.py`, PromptKD) remained present on device 2; it was not disturbed. `npu-smi info proc` reported unsupported, and full `ps` inventories are retained.
- Device 2 was released after raw capture at `2026-10-08T17:23:53.458224975Z`.
- `LOAD_QUALITY=PROCESS_PRESENT`; `MEASUREMENT_QUALITY=NOISY/INCONCLUSIVE`. Pooled CV stayed at 3.409-5.207%, but the shape effects are mixed, Candidate was faster in only 6/18 pairs, and only 8/18 pair median shifts were within combined MAD. The aggregate is negative and does not establish a stable improvement.
- `LOCAL_SCORE=0.990660425923x` (partial route-local only); `LOCAL_DELTA=-0.933957408%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=R31B-V011`. V095 is not promoted. `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- Local capture ran `2026-10-08T17:18:35.592612449Z`-`2026-10-08T17:23:53.455756183Z`.

All raw TSVs, per-invocation stats/stdout/stderr/return codes, per-pair device snapshots, process snapshots, initial runtime-loader failures, compile/probe logs, identities, and the explicit device release receipt are retained in this directory.
