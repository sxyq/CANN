# V088 Partial Ranking Result

- `REVISION=V088`
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1056`
- `PARENT=R31B-V011`, exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `CANDIDATE_SHA256=9d893a4d4a9de282a1648cc927323d1772d48cee736de62ead0d68587a2fa595`
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact Parent C15 FP32 1x32768. C15 is excluded; this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`

## Gates

- Route Compile: PASS (`CMAKE_CONFIGURE_RC=0`, `ROUTE_BUILD_RC=0`).
- Reference probe build: PASS for exact Parent and Candidate targets (`PROBE_CONFIGURE_RC=0`, `PROBE_BUILD_RC=0`).
- Correctness: both Parent and Candidate passed FP32 128x1048/1056/1064, all `rc=0`, `bad=0`. Max_abs was `1.43051e-6` at 1048 and 1056, and `1.19209e-6` at 1064.
- The first launch attempt is retained: all six invocations returned `rc=127` before NPU initialization because the runtime environment did not resolve `libgraph.so`. After sourcing the toolkit environment and prepending `/usr/lib/aarch64-linux-gnu` to `LD_LIBRARY_PATH`, both binaries resolved `libgraph.so` and host GCC 11 `libstdc++`; the six correctness retries passed. No Candidate/source change was made for this runtime setup issue.

## Local Method and Score

Device 4; FP32; 45 warmups; 31 samples per block; 2 blocks; `batch_n=64`; six interleaved P/C pairs per shape, in P,C; C,P; P,C; C,P; P,C; C,P order. All 36 invocations returned `rc=0`, `bad=0`. Each raw TSV contains 62 device-event and host-wall samples. All 2,232 values per metric are retained: 372 per side per shape and 1,116 per side across these three shapes.

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Each shape score is the arithmetic mean of its six pair speedups; route-local score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`.

| Shape | Six pair speedups | Shape score | Delta | Candidate-faster | Within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1048 | 1.017933, 1.040383, 1.008330, 1.042393, 1.020357, 0.944546 | 1.012323706309x | +1.232371% | 5/6 | 6/6 |
| 128x1056 | 1.110114, 1.044183, 1.032423, 0.850593, 0.988848, 0.997121 | 1.003880434364x | +0.388043% | 3/6 | 6/6 |
| 128x1064 | 1.072658, 0.922442, 0.975244, 1.021226, 1.018541, 0.969295 | 0.996567682002x | -0.343232% | 3/6 | 6/6 |
| Equal-shape geomean | - | **1.004236646183x** | **+0.423665%** | 11/18 | 18/18 |

Pooled device latency and jitter use all 372 samples per side/shape; CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Wall median is diagnostic, in microseconds.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) | Wall median (us) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1048 | Parent | 9.692255 / 9.505625 / 1.452414 | 14.985% | 0.991090 | 7.980094-11.613730 | 6.612190-15.700000 | 13.840329 | 11.666250 |
| 128x1048 | Candidate | 9.618662 / 9.432965 / 1.442719 | 14.999% | 0.805150 | 8.036871-11.665620 | 5.732810-16.355000 | 13.946223 | 11.430200 |
| 128x1056 | Parent | 10.116746 / 9.423590 / 6.568811 | 64.930% | 1.043900 | 7.831440-12.015180 | 6.046870-129.180000 | 13.360818 | 11.551200 |
| 128x1056 | Candidate | 9.599200 / 9.377345 / 1.710951 | 17.824% | 0.855785 | 7.948406-11.537360 | 6.768750-26.169100 | 14.081173 | 11.499750 |
| 128x1064 | Parent | 10.679412 / 9.562345 / 6.688460 | 62.629% | 1.003280 | 7.941935-12.895020 | 6.766560-105.735000 | 12.752762 | 11.634000 |
| 128x1064 | Candidate | 9.995246 / 9.564690 / 1.807120 | 18.080% | 0.940300 | 8.044594-12.746550 | 6.795630-16.620300 | 13.625677 | 11.513650 |

All 18 pair median shifts are within the corresponding Parent/Candidate combined MAD; direction is mixed (11/18 Candidate-faster). Retained Parent outliers reach 129.180 us at 1056 and 105.735 us at 1064, with pooled Parent CV above 62% on both shapes. These results support `LOAD_QUALITY=NOISY` and `MEASUREMENT_QUALITY=NOISY`; no samples were discarded. The positive aggregate is numeric partial route-local evidence, not a stable performance conclusion.

## Device and Verdict

- Pre-Local snapshot `2026-10-08T07:25:17.116207781Z`: device 4 HBM 59,192/65,536 MB used; conservative free HBM 6,344 MB; AICore 57%, AIVector 34%. PID 2999855 (`VLLMEngineCor`, 55,666 MB) was present and left untouched.
- Post-Local snapshot `2026-10-08T07:32:25.767947249Z`: HBM 59,192 MB used; AICore 61%, AIVector 40%. No MODE reference probe remained active.
- Raw Local capture ended at `2026-10-08T07:31:05.072544726Z`; device 4 was released at `2026-10-08T07:32:28.031956273Z`.
- `LOCAL_SCORE=1.004236646183x` (partial route-local only); `LOCAL_DELTA=+0.423665%`.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`.
- `CURRENT_LOCAL_BEST=NONE`; V088 is not promoted. Continue from exact R31B-V011 under the authorized cutoff policy.
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

Raw stdout/stderr, stats, device-event TSVs, correctness evidence, environment retry evidence, pre/post snapshots, and release records are retained alongside this note.
