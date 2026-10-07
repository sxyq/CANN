# R-W4-4 V058 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `b86de625abbe610f4c80e95d87b6aa3503b02cc7bdb8512c0a530ca1551dde44`.
- Single change: `kSmallFp32BatchMaxWidth`, `6016 -> 6144`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 6136 | PASS, `rc=0` | FAIL, `rc=3` | 784518 | 4.78309 | not run |
| 6144 | PASS, `rc=0` | FAIL, `rc=3` | 785558 | 4.78411 | not run |
| 6152 | PASS, `rc=0` | PASS, `rc=0` | 0 | 5.00679e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and pre/post device/process snapshots are retained beside `v058-correctness.log`. C15 was excluded.

## Local

Fixed device 4; `128x6152` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w6152-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 18.999050 | 19.750450 | 1.071100 / 1.152850 | 0.961955297221x | -0.751400 | YES |
| 2 | 19.732200 | 19.919850 | 0.997050 / 1.172350 | 0.990579748341x | -0.187650 | YES |
| 3 | 20.245950 | 20.042500 | 1.043450 / 1.324700 | 1.010150929275x | +0.203450 | YES |
| 4 | 20.110300 | 19.985300 | 1.745000 / 1.245950 | 1.006254597129x | +0.125000 | YES |
| 5 | 20.395150 | 20.089400 | 1.609700 / 1.504400 | 1.015219468974x | +0.305750 | YES |
| 6 | 20.617050 | 20.266100 | 1.657650 / 1.237950 | 1.017317096037x | +0.350950 | YES |

- Partial one-shape score at `128x6152`: `1.000246189496x`, delta `+0.024618950%`; Candidate faster in `4/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `13.438482%`, Candidate `28.597195%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 6152 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 64% before and 62% after Local; HBM was 59192 MB before and 59195 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- `CURRENT_LOCAL_BEST=NONE`; V058 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V058/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
