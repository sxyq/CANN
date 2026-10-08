# V089 Partial Ranking Result

- `REVISION=V089`; `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1040`
- `PARENT=R31B-V011`; exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `CANDIDATE_SHA256=0a55efcbc284756d88098c12d4ae6e66e3697827b5a70889433f08d1d624a501`
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact Parent C15 FP32 1x32768. C15 remains excluded; this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`

## Gates

- Route Compile: PASS (`CMAKE_CONFIGURE_RC=0`, `ROUTE_BUILD_RC=0`).
- Reference probe build: PASS for exact Parent and Candidate targets (`PROBE_CONFIGURE_RC=0`, `PROBE_BUILD_RC=0`).
- Correctness: Parent and Candidate both passed FP32 128x1032, 128x1040, and 128x1048; every invocation `rc=0`, `bad=0`, `max_abs=1.43051e-6`.
- Local: all three jointly-correct shapes were captured on device 4. The 36 invocations all returned `rc=0`, `bad=0`; all 36 raw TSVs, stats, stdout and stderr files are retained (372 device-event and 372 wall samples per side/shape).

## Local Method and Score

Device 4; FP32; 45 warmups; 31 samples/block; 2 blocks; `batch_n=64`; six interleaved pairs per shape in P,C; C,P; P,C; C,P; P,C; C,P order. No samples were discarded. For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Each shape score is the arithmetic mean of its six pair speedups; route-local score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`.

| Shape | Six pair speedups | Shape score | Delta | Candidate-faster | Within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1032 | 0.991350, 0.963182, 0.993810, 0.993519, 1.057947, 0.999107 | 0.999818836106x | -0.018116% | 1/6 | 6/6 |
| 128x1040 | 0.958783, 1.042481, 1.008399, 0.951971, 1.055344, 0.999832 | 1.002801540657x | +0.280154% | 3/6 | 6/6 |
| 128x1048 | 1.022426, 0.920948, 0.977352, 1.041497, 0.981042, 1.034131 | 0.996232731112x | -0.376727% | 3/6 | 6/6 |
| Equal-shape geomean | - | **0.999614094624x** | **-0.038591%** | 7/18 | 18/18 |

Pooled device latency/jitter use all 372 samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Wall median is diagnostic, in microseconds.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) | Wall median (us) |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| 128x1032 | Parent | 8.450408 / 8.457810 / 1.109796 | 13.133% | 0.658595 | 7.049033-9.676813 | 5.376250-13.196600 | 15.631908 | 10.312700 |
| 128x1032 | Candidate | 8.706198 / 8.464220 / 2.286268 | 26.260% | 0.612500 | 7.303375-9.716406 | 5.480310-32.683400 | 15.172639 | 10.451800 |
| 128x1040 | Parent | 8.852556 / 8.781250 / 1.024888 | 11.577% | 0.667505 | 7.564281-10.218460 | 5.660310-11.805600 | 15.037465 | 10.827050 |
| 128x1040 | Candidate | 8.728558 / 8.807810 / 1.037459 | 11.886% | 0.697815 | 7.306786-10.063940 | 5.678750-11.456600 | 15.251087 | 11.042600 |
| 128x1048 | Parent | 10.836878 / 9.347190 / 9.223894 | 85.116% | 0.878605 | 8.030377-12.122260 | 6.368440-113.527000 | 12.378473 | 11.559450 |
| 128x1048 | Candidate | 9.617060 / 9.335470 / 1.496206 | 15.558% | 0.782335 | 8.111808-11.423120 | 6.848750-23.478100 | 13.948546 | 11.478350 |

All 18 pair median shifts are within the corresponding combined MAD; direction is mixed (7/18 Candidate-faster). The Candidate reaches 32.6834 us at 128x1032 and Parent reaches 113.527 us at 128x1048. CV reaches 26.260% for Candidate at 1032 and 85.116% for Parent at 1048. These data support `LOAD_QUALITY=NOISY` and `MEASUREMENT_QUALITY=NOISY`; the numeric score is retained but is not a stable performance conclusion.

## Device and Verdict

- Pre-Local snapshot `2026-10-08T08:39:35.208951888Z`: device 4 HBM 59,192/65,536 MB used; conservative free HBM 6,344 MB; AICore 49%, AIVector 37%, HBM bandwidth 60%. PID 2999855 (`VLLMEngineCor`, 55,666 MB) was present and left untouched.
- Correctness pre-use snapshot was `2026-10-08T08:13:02.574400145Z`; first correctness started `2026-10-08T08:35:25.437435846Z`, 22m22.863s later. The snapshot showed 6,344 MB free and no active probe. This freshness gap is retained as a procedural limitation; the later pre-Local snapshot again showed 6,344 MB free and no probe.
- Post-Local snapshot `2026-10-08T08:51:15.428426870Z`: HBM 59,192/65,536 MB used; AICore 60%, AIVector 35%, HBM bandwidth 61%; the same existing PID remained. No MODE reference probe remained active.
- Raw capture ended `2026-10-08T08:48:19.563953244Z`; device 4 was explicitly released `2026-10-08T08:51:17.642477853Z`.
- `LOCAL_SCORE=0.999614094624x` (partial route-local only); `LOCAL_DELTA=-0.038591%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=NONE`; V089 is not promoted. Continue the approved cutoff OFAT from exact R31B-V011.
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

All correctness and Local raw files, build logs, runtime setup, pre/post device/process snapshots, release record, and failed diagnostic/parse attempts are retained alongside this note. Initial probe `--help` was a loader-only check that returned 127 before NPU initialization because `libgraph.so` was not on the environment path; correctness and Local ran after sourcing the toolkit environment and adding `/usr/lib/aarch64-linux-gnu` to `LD_LIBRARY_PATH`.
