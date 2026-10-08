# V085 Partial Ranking Result

- `REVISION=V085`
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1280`
- `PARENT=R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`)
- `CANDIDATE_SHA256=2c2885e743b6979d9015f86de1fa40fd5bdace81df86f3c937f900ac0c76c4ef`
- `C15=EXCLUDED_KNOWN_PARENT_FAILURE`; not attributed to Candidate
- `PARTIAL_CORRECTNESS=YES`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`

## Gates

- Compile: PASS. Route build and Parent/Candidate probe targets returned `rc=0`.
- Correctness: Parent and Candidate both passed all three requested FP32 shapes. Each returned `rc=0`, `bad=0`; max_abs was `1.19209e-6` for 128x1272, `1.43051e-6` for 128x1280, and `1.19209e-6` for 128x1288.
- The initial six runner launches failed before device execution (`rc=127`, missing `libgraph.so`). Those logs are preserved. Retrying with the existing CANN runtime environment succeeded; no source or probe change was made.

## Local Method and Score

Device-event samples used one fixed device (4), 45 warmups, 31 samples per block, 2 blocks, batch_n=64, and 6 interleaved P/C pairs per shape. Pair order was P,C; C,P; P,C; C,P; P,C; C,P. All 36 invocations returned `rc=0`, `bad=0`. All 2,232 raw device-event samples and wall-time samples are retained; none were filtered.

Following the established V084 route formula, each pair uses `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)` over its 62 saved raw samples. Pair medians and MADs are recomputed from the raw TSV values rather than rounded `.stats.txt` fields. Each shape score is the arithmetic mean of its six pair speedups. The route-local score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. Throughput below is separately derived from the pooled all-sample mean latency.

| Shape | Six pair speedups | Shape score | Delta | Candidate-faster pairs | Median shifts within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1272 | 0.958878, 1.019349, 0.980591, 1.083662, 1.018416, 1.068482 | 1.021563203003x | +2.156320300% | 4/6 | 6/6 |
| 128x1280 | 1.076364, 0.965499, 1.017462, 1.018859, 0.991004, 0.930541 | 0.999954998996x | -0.004500100% | 3/6 | 6/6 |
| 128x1288 | 0.968923, 1.032218, 0.999286, 0.995131, 0.965202, 1.014356 | 0.995852594724x | -0.414740528% | 2/6 | 6/6 |
| Equal-shape geomean | - | **1.005727330290x** | **+0.572733029%** | 9/18 | 18/18 |

Pooled latency, jitter, and throughput use all 372 samples per side and shape. Throughput is `128 * width / mean(device_us)`. Device columns list P/C values; ranges are inclusive min-max and p10-p90.

| Shape | P/C mean / median / stdev (us) | P/C CV | P/C MAD (us) | P/C p10-p90 (us) | P/C min-max (us) | P/C throughput (Gelem/s) | P/C wall median (us) |
|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1272 | 8.792400 / 8.767030 / 0.928848; 8.605090 / 8.611875 / 0.827890 | 10.564% / 9.621% | 0.547965 / 0.593905 | 7.696812-9.875536 / 7.508248-9.618904 | 6.931250-14.204400 / 6.364370-11.640300 | 18.517812 / 18.920894 | 10.984900 / 10.853050 |
| 128x1280 | 8.786483 / 8.526720 / 4.662522; 8.615627 / 8.559220 / 0.910653 | 53.065% / 10.570% | 0.533745 / 0.603280 | 7.402252-9.669661 / 7.570839-9.778998 | 6.199060-96.943800 / 6.322500-12.284400 | 18.646825 / 19.016608 | 10.582150 / 10.624750 |
| 128x1288 | 8.504558 / 8.424845 / 0.908804; 8.497703 / 8.422970 / 0.890062 | 10.686% / 10.474% | 0.503280 / 0.525000 | 7.466748-9.462591 / 7.489060-9.635442 | 6.405310-13.187800 / 6.099370-12.155300 | 19.385369 / 19.401008 | 10.275150 / 10.363950 |

One Parent sample at 128x1280, pair 05, measured `96.9438 us` (`98.0644 us` wall). It remains in the raw data; it drives that pooled Parent CV to 53.065%. Every pair's median shift is within the sum of its Parent/Candidate MADs; Candidate-faster directions are 4/6, 3/6, and 2/6 by shape (9/18 total). These are noisy measurements, not a stable performance conclusion.

## Verdict and Device

- `LOCAL_SCORE=1.005727330290x` (partial route-local only)
- `PARTIAL_ROUTE_LOCAL_SCORE=1.005727330290x`
- `LOCAL_DELTA=+0.572733029%`
- `LOAD_QUALITY=NOISY`
- `MEASUREMENT_QUALITY=NOISY`
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`
- `CURRENT_LOCAL_BEST=NONE`; V085 is not promoted
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`

Before Local, device 4 had 59,192/65,536 MB HBM used (6,344 MB free), AICore 0%, AIVector 0%, and existing PID 2999855 using 55,666 MB. The post-run snapshot showed the same device HBM/load; PID 2999855 remained running and untouched. Raw capture ended at `2026-10-08T04:18:55.577583967Z`; device 4 was explicitly released at `2026-10-08T04:20:49.969999759Z`. Snapshot and release records are in this directory.
