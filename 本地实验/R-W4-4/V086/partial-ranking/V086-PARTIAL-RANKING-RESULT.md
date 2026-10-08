# V086 Partial Ranking Result

- `REVISION=V086`
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1152`
- `PARENT=R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`)
- `CANDIDATE_SHA256=3f8963bcc3d11fd19a3b1975c37a2fba836032316861b54f1c494ba3b15ecbba`
- `C15=EXCLUDED_KNOWN_PARENT_FAILURE`; not attributed to Candidate
- `PARTIAL_CORRECTNESS=YES`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`

## Gates

- Compile: PASS. Route build and Parent/Candidate reference probe builds returned `rc=0`; logs and probe identities are retained.
- Correctness: Parent and Candidate both passed all three assigned FP32 shapes, each `rc=0`, `bad=0`. For 128x1144 and 128x1152, both max_abs values were `1.19209e-6`; for 128x1160, both were `1.43051e-6`.
- The invalid `npu-smi info -t proc -i 4` attempt and its CLI usage/error are retained as `v086-device4-pre-invalid-proc-query-error.log`. A fresh `ps` inventory confirmed PID 2999855 and no active MODE probe before device use.

## Local Method and Score

Device-event samples used one fixed device (4), 45 warmups, 31 samples per block, 2 blocks, batch_n=64, and 6 interleaved P/C pairs per shape. Pair order was P,C; C,P; P,C; C,P; P,C; C,P. All 36 invocations returned `rc=0`, `bad=0`; each raw TSV retains 62 device-event and host-wall observations. All 2,232 values per metric are retained without filtering.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Each shape score is the arithmetic mean of its six pair speedups; the route-local score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. Pooled summaries below use all 372 samples per side and shape; throughput is `128 * width / mean(device_us)`.

| Shape | Six pair speedups | Shape score | Delta | Candidate-faster pairs | Median shifts within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1144 | 1.023208, 0.990636, 0.952417, 1.008206, 0.995105, 0.987064 | 0.992772736963x | -0.722726304% | 2/6 | 6/6 |
| 128x1152 | 0.894204, 0.960439, 1.025228, 0.990430, 0.922603, 1.011153 | 0.967342804139x | -3.265719586% | 2/6 | 6/6 |
| 128x1160 | 1.031190, 0.946179, 0.975639, 0.992822, 0.966150, 0.938396 | 0.975062659749x | -2.493734025% | 1/6 | 6/6 |
| Equal-shape geomean | - | **0.978335000636x** | **-2.166499936%** | 5/18 | 18/18 |

Pooled latency and jitter are P/C. CV uses population standard deviation divided by mean; MAD and quantiles are recomputed from raw TSV values. Throughput is in Gelem/s.

| Shape | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | min-max (us) | Throughput (P/C Gelem/s) | Wall median (P/C us) |
|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1144 | 10.492428 / 10.286100 / 1.515000; 10.553676 / 10.394400 / 1.350438 | 14.439% / 12.796% | 0.892325 / 0.860180 | 8.794250-12.331880 / 8.982156-12.240480 | 7.576560-19.160900 / 7.886250-17.824100 | 13.955969 / 13.874976 | 12.379850 / 12.543650 |
| 128x1152 | 10.266084 / 9.741720 / 5.352595; 11.287377 / 9.950155 / 6.214034 | 52.139% / 55.053% | 1.016385 / 1.024840 | 8.083685-12.233670 / 8.433154-14.079580 | 6.625940-101.362000 / 6.928750-77.972800 | 14.363412 / 13.063796 | 11.891200 / 12.249300 |
| 128x1160 | 9.688433 / 9.554220 / 1.383907; 10.346375 / 9.722030 / 4.321090 | 14.284% / 41.764% | 0.878780 / 0.953420 | 8.175404-11.516100 / 8.139281-11.933900 | 6.855000-18.748100 / 6.298440-61.764400 | 15.325493 / 14.350920 | 11.736200 / 11.884650 |

All 18 pair median shifts are within the sum of the paired Parent/Candidate MADs, and Candidate-faster directions are only 5/18. The 128x1152 pooled device CV is 52.139%/55.053%, with retained maxima of 101.362/77.9728 us; 128x1160 Candidate CV is 41.764% with a 61.7644 us maximum. These support `LOAD_QUALITY=NOISY` and `MEASUREMENT_QUALITY=NOISY`; no samples were dropped. The negative score is numeric route-local evidence, not a stable performance conclusion.

## Verdict and Device

- `LOCAL_SCORE=0.978335000636x` (partial route-local only)
- `PARTIAL_ROUTE_LOCAL_SCORE=0.978335000636x`
- `LOCAL_DELTA=-2.166499936%`
- `LOAD_QUALITY=NOISY`
- `MEASUREMENT_QUALITY=NOISY`
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`
- `CURRENT_LOCAL_BEST=NONE`; V086 is not promoted
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`

Before Local, device 4 showed 59,192/65,536 MB HBM used (6,344 MB conservative free), AICore 62% in the full snapshot (63% usage query), AIVector 34%, and PID 2999855 using 55,666 MB. The process was not disturbed. Post-capture HBM remained 59,192 MB used, AICore was 61%, and AIVector was 39%; the same PID remained. Raw capture ended at `2026-10-08T05:28:03.577427318Z`; device 4 was explicitly released at `2026-10-08T05:29:01.116700762Z`. Snapshot, process, and release records are retained in this directory.
