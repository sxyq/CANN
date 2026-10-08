# V087 Partial Ranking Result

- `REVISION=V087`
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1088`
- `PARENT=R31B-V011` (route-verified exact-parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`)
- `CANDIDATE_SHA256=e98e07ee54b727b48fc12cbc8a0bcab25cbaf32e45d887695183e863c96a77c6`
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact Parent C15 FP32 1x32768; C15 remains excluded and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`

## Gates

- Compile: PASS. Route build returned `rc=0`; isolated Parent/Candidate reference probe configure and build returned `rc=0`.
- Correctness: Parent and Candidate both passed each assigned FP32 shape with `rc=0`, `bad=0`. For 128x1080, both max_abs values were `9.53674e-7`; for 128x1088, both were `1.43051e-6`; for 128x1096, both were `1.19209e-6`.
- Device 4 pre-correctness snapshot showed 59,193/65,536 MB HBM used (6,343 MB conservative free), AICore 62%, AIVector 40%. PID 2999855 was present and left untouched; no active MODE probe was found.

## Local Method and Score

Device-event samples used one fixed device (4), 45 warmups, 31 samples per block, 2 blocks, batch_n=64, and 6 interleaved P/C pairs per shape. Pair order was P,C; C,P; P,C; C,P; P,C; C,P. All 36 invocations returned `rc=0`, `bad=0`; each raw TSV retains 62 device-event and host-wall observations. All 2,232 values per metric are retained without filtering.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Each shape score is the arithmetic mean of its six pair speedups; the route-local score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. Pooled summaries use all 372 samples per side and shape; throughput is `128 * width / mean(device_us)`.

| Shape | Six pair speedups | Shape score | Delta | Candidate-faster pairs | Median shifts within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1080 | 1.041492, 0.986269, 1.021765, 0.967551, 0.939785, 0.937552 | 0.982402368207x | -1.759763179% | 2/6 | 6/6 |
| 128x1088 | 0.981564, 0.988439, 1.053196, 0.981203, 1.022817, 1.086088 | 1.018884556544x | +1.888455654% | 3/6 | 6/6 |
| 128x1096 | 0.916533, 0.986448, 1.027826, 1.028607, 0.950013, 1.001689 | 0.985185994421x | -1.481400558% | 3/6 | 6/6 |
| Equal-shape geomean | - | **0.995353932228x** | **-0.464606777%** | 8/18 | 18/18 |

Pooled latency and jitter are P/C. CV uses population standard deviation divided by mean; MAD and quantiles are recomputed from raw TSV values. Throughput is in Gelem/s.

| Shape | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | min-max (us) | Throughput (P/C Gelem/s) | Wall median (P/C us) |
|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1080 | 11.030183 / 9.636410 / 7.313504; 10.228000 / 9.795315 / 1.862761 | 66.304% / 18.212% | 0.915935 / 0.923285 | 8.150716-13.606730 / 8.320596-13.108400 | 6.676560-100.330000 / 7.422190-16.622800 | 12.532884 / 13.515839 | 11.527150 / 11.841400 |
| 128x1088 | 10.013546 / 9.374215 / 6.687467; 9.529030 / 9.347345 / 1.377724 | 66.784% / 14.458% | 0.840470 / 0.842345 | 7.978844-11.694090 / 8.068873-11.505770 | 6.790310-134.270000 / 6.831250-16.530600 | 13.907561 / 14.614709 | 11.591900 / 11.241650 |
| 128x1096 | 9.948471 / 9.838595 / 1.492550; 11.354482 / 9.991565 / 8.421396 | 15.003% / 74.168% | 0.907485 / 0.984505 | 8.302565-11.811380 / 8.536661-13.028390 | 6.790620-19.906900 / 6.525310-127.475000 | 14.101464 / 12.355297 | 11.800400 / 12.144400 |

All 18 pair median shifts are within the sum of their Parent/Candidate MADs; Candidate-faster directions are mixed (8/18). Retained outliers include Parent 100.330 us at 128x1080, Parent 134.270 us at 128x1088, and Candidate 127.475 us at 128x1096. Pooled CV reaches 66.784% for Parent at 1088 and 74.168% for Candidate at 1096. These support `LOAD_QUALITY=NOISY` and `MEASUREMENT_QUALITY=NOISY`; no sample was dropped. The aggregate score is a numeric route-local result, not a stable performance conclusion.

## Verdict and Device

- `LOCAL_SCORE=0.995353932228x` (partial route-local only)
- `PARTIAL_ROUTE_LOCAL_SCORE=0.995353932228x`
- `LOCAL_DELTA=-0.464606777%`
- `LOAD_QUALITY=NOISY`
- `MEASUREMENT_QUALITY=NOISY`
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`
- `CURRENT_LOCAL_BEST=NONE`; V087 is not promoted
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`

Before Local, device 4 showed 59,192/65,536 MB HBM used (6,344 MB conservative free), AICore 62% in the full snapshot (61% usage query), AIVector 39%, and PID 2999855 using 55,666 MB. The process was not disturbed. Post-capture HBM was 59,193 MB used, AICore 62% (58% usage query), AIVector 39%; the same PID remained. Raw capture ended at `2026-10-08T06:17:00.833707338Z`; device 4 was explicitly released at `2026-10-08T06:19:25.540597851Z`. Snapshot, process, identity, and release records are retained in this directory.
