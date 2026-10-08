# V091 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V091`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1028`; Candidate SHA256 `f41d58080592a7dfb3d9b827d8b8bebe69a2977fa85098a5d660bd334f5df952`.
- `COMPILE=PASS`; reference probe build for exact Parent and Candidate: PASS.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for C15 FP32 1x32768. C15 remains excluded; this is not a Candidate regression.

## Correctness

Parent and Candidate both passed on device 4 for FP32 128x1020, 128x1028, and 128x1036. Every invocation returned `rc=0`, `bad=0`; `max_abs` was respectively `1.43051e-6`, `1.66893e-6`, and `1.43051e-6`. Raw, stats, stdout, stderr, and rc files are retained beside the Local evidence.

## Local Method and Score

Device 4; FP32; six interleaved Parent/Candidate pairs per shape; 45 warmups; 31 device-event samples in each of two blocks per invocation; `batch_n=64`; no samples discarded. All 36 invocations returned `rc=0`, `bad=0`. The 36 raw TSVs contain 62 timed samples each (2,232 total: 1,116 per side across the three shapes). Raw event samples, wall samples, invocation stats/logs, and pre/post resource/process snapshots are retained.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. A shape score is the arithmetic mean of its six pair speedups; the route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. All calculations below use the unrounded raw TSV values.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Shift within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1020 | 0.882659437, 0.854660928, 0.973668339, 0.965766349, 0.920083289, 1.009965217 | 0.934467259931x | -6.553274007% | 1/6 | 6/6 |
| 128x1028 | 1.056114739, 1.000159422, 1.005492650, 1.017266530, 1.021598864, 1.020370906 | 1.020167185263x | +2.016718526% | 6/6 | 6/6 |
| 128x1036 | 1.076597036, 0.976541663, 1.009075011, 0.987474704, 0.999490640, 0.997443519 | 1.007770428818x | +0.777042882% | 2/6 | 6/6 |
| Equal-shape geomean | - | **0.986731554944x** | **-1.326844506%** | **9/18** | **18/18** |

Pooled latency/jitter uses all 372 device-event samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Wall median is diagnostic; device events are primary.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) | Wall median (us) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1020 | Parent | 12.596381 / 11.331750 / 8.194595 | 65.055% | 1.314350 | 9.204252-14.673480 | 7.859380-129.231000 | 10.364882 | 13.263850 |
| 128x1020 | Candidate | 12.310448 / 11.871400 / 2.658933 | 21.599% | 1.047850 | 10.210660-14.663240 | 7.521250-48.481200 | 10.605625 | 13.754050 |
| 128x1028 | Parent | 11.481729 / 11.082200 / 1.891913 | 16.478% | 1.070800 | 9.558620-14.152540 | 7.723440-22.546900 | 11.460295 | 13.012600 |
| 128x1028 | Candidate | 11.207291 / 10.858900 / 1.704426 | 15.208% | 0.893115 | 9.482598-13.527850 | 7.447810-20.146900 | 11.740928 | 12.768050 |
| 128x1036 | Parent | 11.900567 / 11.056700 / 2.578149 | 21.664% | 0.995450 | 9.504286-15.967100 | 7.830630-29.941600 | 11.142998 | 12.937800 |
| 128x1036 | Candidate | 11.880761 / 10.926600 / 3.040897 | 25.595% | 1.144075 | 9.368125-16.098980 | 7.824060-43.790900 | 11.161575 | 12.876350 |

All 18 paired median shifts are within the corresponding combined MAD; direction is mixed (Candidate faster in 9/18). Large retained tails include 129.231 us for Parent at 128x1020 and 48.4812 us for Candidate at 128x1020. This is noisy evidence, not a stable performance conclusion.

## Load, Device, and Verdict

Across the 18 per-shape pair snapshots, HBM usage was 90% in every pre/post snapshot. Pre-Local AICore averaged 59.72% (56-65%), AIVector 39.44% (29-72%), and HBM bandwidth 57.78% (22-64%); post-Local values averaged 61.78% (58-74%), 40.33% (33-71%), and 58.78% (32-63%), respectively. Existing processes were present and left untouched; the process and device snapshots are preserved.

- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`.
- `LOCAL_SCORE=0.986731554944x` (partial route-local only); `LOCAL_DELTA=-1.326844506%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=R31B-V011`. V091 is not promoted.
- Post-Local snapshot: `2026-10-08T13:15:03.561796555Z`; device 4 release: `2026-10-08T13:15:05.582658988Z`.
- The numeric local result is not an Official Score and is not comparable to Official. No new revision is started by this closeout.
- Next authorized route action: after fresh explicit device allocation, continue the bounded one-factor cutoff sweep from exact `R31B-V011`; V092 is not started here.
