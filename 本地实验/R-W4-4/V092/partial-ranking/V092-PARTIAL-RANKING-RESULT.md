# V092 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V092`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1024`; Candidate SHA256 `f4b3623b4a8cb86a43328f5dbc15fe6826ab8f13cbcc88745d0e23aa9da2b49f`.
- `COMPILE=PASS`; exact Parent/Candidate correctness probe targets built successfully.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded C15 FP32 1x32768. C15 was not rerun; this is not a Candidate regression.

## Correctness

Exact Parent and Candidate both passed on device 4 for FP32 128x1016, 128x1024, and 128x1032. Every invocation returned `rc=0`, `bad=0`; `max_abs` was respectively `1.19209e-6`, `1.66893e-6`, and `1.43051e-6`.

The first probe build succeeded with a stale harness copy whose Candidate SHA still matched Parent; no correctness run was made with that binary. The harness Candidate was refreshed to the V092 source and the Candidate probe rebuilt (`rc=0`). The first correctness attempt then failed before runtime with `rc=127` because `libgraph.so` was not on the loader path; all logs and rc files are preserved. The retry sourced `set_env.sh` and added `/usr/lib/aarch64-linux-gnu` to `LD_LIBRARY_PATH`; all six Parent/Candidate checks passed. No Candidate/source change was made to address either harness/environment issue.

## Local Method and Score

Device 4; FP32; six interleaved Parent/Candidate pairs per shape in P,C; C,P alternating order; 45 warmups; 31 device-event samples in each of two blocks per invocation; `batch_n=64`; no samples discarded. All 36 invocations returned `rc=0`, `bad=0`. The 36 raw TSVs contain 62 timed samples each (2,232 total: 1,116 per side across the three shapes). Raw device and wall samples, invocation stats/logs, and per-pair pre/post device/process snapshots are retained.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Each shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. All calculations use unrounded raw TSV values.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Shift within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1016 | 0.990670093, 0.983499817, 1.006662445, 0.964043752, 1.035694835, 0.933457484 | 0.985671404285x | -1.432859572% | 2/6 | 6/6 |
| 128x1024 | 1.037935806, 0.994419229, 1.033169988, 0.981520205, 0.946224489, 1.010568138 | 1.000639642398x | +0.063964240% | 3/6 | 6/6 |
| 128x1032 | 1.030499904, 0.963416351, 0.936722307, 0.993748756, 1.027862798, 0.964143629 | 0.986065624221x | -1.393437578% | 2/6 | 6/6 |
| Equal-shape geomean | - | **0.990767822684x** | **-0.923217732%** | **7/18** | **18/18** |

Pooled latency/jitter uses all 372 device-event samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Wall median is diagnostic; device events are primary.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) | Wall median (us) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1016 | Parent | 9.479846 / 9.341565 / 1.181307 | 12.461% | 0.728600 | 8.130129-10.990430 | 6.631870-15.371200 | 13.718366 | 11.555200 |
| 128x1016 | Candidate | 9.680065 / 9.489535 / 1.361456 | 14.065% | 0.751550 | 8.049652-11.315420 | 7.040620-16.897200 | 13.434621 | 11.724750 |
| 128x1024 | Parent | 9.597354 / 9.367340 / 1.712755 | 17.846% | 0.780775 | 7.798217-11.751140 | 6.384690-17.913100 | 13.657097 | 11.682050 |
| 128x1024 | Candidate | 9.528203 / 9.323440 / 1.550642 | 16.274% | 0.858460 | 7.865250-11.206600 | 6.595310-22.195000 | 13.756213 | 11.516350 |
| 128x1032 | Parent | 9.667409 / 9.437345 / 1.576621 | 16.309% | 0.914530 | 7.845091-11.686520 | 5.948120-16.247800 | 13.664054 | 11.678400 |
| 128x1032 | Candidate | 9.814233 / 9.469375 / 1.577962 | 16.078% | 0.843425 | 8.171313-11.883630 | 6.460630-16.597500 | 13.459636 | 11.484200 |

All 18 paired median shifts are within combined MAD and direction is mixed (Candidate faster in 7/18). These samples do not establish a stable improvement.

## Load, Device, and Verdict

Across the 18 pair snapshots, HBM usage was 90% in every pre/post snapshot. Pre-Local AICore averaged 55.33% (0-66%), AIVector 27.72% (0-36%), and HBM bandwidth 55.83% (0-67%); post-Local values averaged 56.56% (0-66%), 31.78% (0-69%), and 54.33% (0-67%), respectively. The pre-Local full snapshot showed device 4 using 59,193/65,536 MB HBM (6,343 MB free); the post-Local snapshot showed 59,192 MB used (6,344 MB free). Existing PID 2999855 (`VLLMEngineCor`, 55,666 MB) was left untouched.

- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`.
- `LOCAL_SCORE=0.990767822684x` (partial route-local only); `LOCAL_DELTA=-0.923217732%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=R31B-V011`. V092 is not promoted.
- Local capture ended `2026-10-08T15:27:05.213063104Z`; post-Local snapshot `2026-10-08T15:28:01.369045241Z`.
- This numeric local result is not an Official Score and is not comparable to Official.
- Device 4 was explicitly released at `2026-10-08T15:32:32.762388823Z` after raw/numeric result capture; see `v092-device4-release.txt`. Existing processes were untouched.
- Next bounded cutoff sibling remains rooted at exact `R31B-V011`; no new revision is included in this evidence.
