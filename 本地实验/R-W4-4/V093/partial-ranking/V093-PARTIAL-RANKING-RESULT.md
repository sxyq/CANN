# V093 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V093`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1020`; Candidate SHA256 `9fbe6deda3c5dbfdcf8b95f0b0433e2161e1c78af6578668a97f48b25cd6369e`.
- `COMPILE=PASS`; Parent/Candidate correctness probe build `PASS`.
- `CORRECTNESS=PASS` on FP32 `128x1012/1020/1028`, Parent and Candidate, all six checks `rc=0,bad=0`. Maximum absolute errors by width: `1.90735e-6`, `1.43051e-6`, `1.66893e-6`.
- `PARTIAL_CORRECTNESS=YES`; the exact Parent has a known failure at excluded C15 FP32 `1x32768`. C15 was not rerun and this is not a Candidate regression.
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

## Local Method and Score

Device 2; FP32; six interleaved Parent/Candidate pairs per shape; order alternated P,C then C,P; 45 warmups; 31 device-event samples in each of two blocks per invocation; `batch_n=64`; no samples discarded. All 36 invocations returned `rc=0,bad=0`; all 36 raw TSVs contain 62 timed samples (2,232 total, 1,116 per side).

For each pair, `pair_speedup = median(Parent raw device_us) / median(Candidate raw device_us)`. Shape score is the arithmetic mean of its six pair speedups. Route score is the equal-weight geometric mean of the three shape scores. Delta is `(score - 1) * 100%`. Calculations use unrounded TSV values.

| Shape | Six pair speedups | Shape score | Delta | Candidate faster | Shift within combined MAD |
|---|---|---:|---:|---:|---:|
| 128x1012 | 0.979844166, 0.986026838, 0.997496570, 0.962438319, 0.984612536, 0.971277238 | 0.980282611378x | -1.971738862% | 0/6 | 2/6 |
| 128x1020 | 1.010999872, 0.980380013, 0.995277548, 0.980713885, 1.013906454, 1.054574106 | 1.005975312856x | +0.597531286% | 3/6 | 5/6 |
| 128x1028 | 1.002535187, 0.971515886, 0.976237258, 0.980261082, 0.945593951, 0.963681788 | 0.973304192057x | -2.669580794% | 1/6 | 3/6 |
| Equal-shape geomean | - | **0.986245511524x** | **-1.375448848%** | **4/18** | **10/18** |

Pooled latency/jitter uses all 372 device-event samples per side and shape. CV is population standard deviation divided by mean. Throughput is `128 * width / mean(device_us) / 1000` in Gelem/s. Device events are primary; wall median is retained in the per-invocation stats files.

| Shape | Side | Mean / median / stdev (us) | CV | MAD (us) | p10-p90 (us) | Min-max (us) | Throughput (Gelem/s) |
|---|---|---:|---:|---:|---:|---:|---:|
| 128x1012 | Parent | 7.630117 / 7.589220 / 0.259709 | 3.404% | 0.093750 | 7.434536-7.853810 | 7.305620-10.908700 | 16.976936 |
| 128x1012 | Candidate | 7.752810 / 7.719065 / 0.204492 | 2.638% | 0.120315 | 7.538534-8.000596 | 7.260310-8.690000 | 16.708264 |
| 128x1020 | Parent | 7.795894 / 7.612500 / 1.067094 | 13.688% | 0.179375 | 7.391841-8.226315 | 7.212500-25.742800 | 16.747277 |
| 128x1020 | Candidate | 7.746059 / 7.608595 / 0.905402 | 11.689% | 0.142655 | 7.427721-8.050627 | 7.013120-23.809100 | 16.855022 |
| 128x1028 | Parent | 7.570317 / 7.467185 / 0.988772 | 13.061% | 0.121245 | 7.296873-7.798752 | 6.226250-25.842500 | 17.381571 |
| 128x1028 | Candidate | 7.844759 / 7.661410 / 0.780539 | 9.950% | 0.142655 | 7.443214-8.227219 | 7.207190-14.730900 | 16.773491 |

## Device and Verdict

- Fresh pre-correctness snapshot on device 2: 3,422/65,536 MB HBM used (62,114 MB free), HBM usage 5%, AICore 0%, no running NPU process on device 2.
- Across 36 pair pre/post snapshots, device 2 had no running NPU process; mean AICore was 0%, and HBM usage was 5% in every snapshot. Post-Local snapshot showed 3,423 MB HBM used (62,113 MB free); existing processes on other devices were untouched.
- `LOAD_QUALITY=LOW_LOAD`; `MEASUREMENT_QUALITY=NOISY`. Candidate was faster in 4/18 pairs; only 10/18 shifts were within combined MAD. Widths 1020 and 1028 show high CV and large timing outliers; no stable improvement is established.
- `LOCAL_SCORE=0.986245511524492x` (partial route-local only); `LOCAL_DELTA=-1.375448847551%`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=R31B-V011`. V093 is not promoted.
- Local command capture began `2026-10-08T15:54:58.266977398Z`; final invocation ended `2026-10-08T16:00:14.438931544Z`; post-Local capture/release completed at `2026-10-08T16:00:18.397476895Z`.
- The wrapper emitted a shell syntax error after writing all Local outputs, post-run snapshots, and the device release receipt. Exact error is preserved in `v093-post-capture-shell-error.txt`. Read-only audit verified 6/6 correctness checks and 36/36 Local runs completed; no correctness or Local rerun was made.

All raw TSVs, per-invocation stats/logs/return codes, per-pair device snapshots, gate logs, and the explicit device release receipt are retained alongside this result.
