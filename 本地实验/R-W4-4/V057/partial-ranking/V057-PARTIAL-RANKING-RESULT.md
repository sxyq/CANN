# R-W4-4 V057 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `68e468935400d2645812e7de24497f4705892e0fcc7536fbea88dec3327c729e`.
- Single change: `kSmallFp32BatchMaxWidth`, `5888 -> 6016`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 6008 | PASS, `rc=0` | FAIL, `rc=3` | 767871 | 4.71163 | not run |
| 6016 | PASS, `rc=0` | FAIL, `rc=3` | 769249 | 4.80354 | not run |
| 6024 | PASS, `rc=0` | PASS, `rc=0` | 0 | 4.52995e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and pre/post device/process snapshots are retained beside `v057-correctness.log`. C15 was excluded.

## Local

Fixed device 4; `128x6024` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w6024-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 20.103750 | 19.352050 | 1.462350 / 1.280000 | 1.038843430024x | +0.751700 | YES |
| 2 | 20.095500 | 19.972350 | 1.390350 / 1.185150 | 1.006166024529x | +0.123150 | YES |
| 3 | 19.354250 | 19.623250 | 1.564200 / 1.696200 | 0.986291771241x | -0.269000 | YES |
| 4 | 19.935150 | 19.982500 | 1.357650 / 1.306400 | 0.997630426623x | -0.047350 | YES |
| 5 | 18.664700 | 18.863600 | 0.981550 / 0.891400 | 0.989455883288x | -0.198900 | YES |
| 6 | 18.965000 | 19.346400 | 0.952200 / 1.120150 | 0.980285737915x | -0.381400 | YES |

- Partial one-shape score at `128x6024`: `0.999778878937x`, delta `-0.022112106%`; Candidate faster in `2/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `11.088989%`, Candidate `11.628388%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 6024 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 66% before and 64% after Local; HBM was 59192 MB before and 59194 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- `CURRENT_LOCAL_BEST=NONE`; V057 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V057/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
