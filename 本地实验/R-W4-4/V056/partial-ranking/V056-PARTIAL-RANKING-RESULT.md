# R-W4-4 V056 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `0e7bcb75487d0c1d106ec428262c0c457f15fa2b203e006bacb4b7f5f2611957`.
- Single change: `kSmallFp32BatchMaxWidth`, `5760 -> 5888`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 5880 | PASS, `rc=0` | FAIL, `rc=3` | 751444 | 4.51594 | not run |
| 5888 | PASS, `rc=0` | FAIL, `rc=3` | 752618 | 4.77329 | not run |
| 5896 | PASS, `rc=0` | PASS, `rc=0` | 0 | 4.52995e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and pre/post device/process snapshots are retained beside `v056-correctness.log`. C15 was excluded.

## Local

Fixed device 4; `128x5896` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w5896-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 19.474850 | 19.664500 | 1.131400 / 1.426100 | 0.990355717155x | -0.189650 | YES |
| 2 | 19.053750 | 19.175650 | 1.390650 / 1.087950 | 0.993642979508x | -0.121900 | YES |
| 3 | 18.966250 | 19.217800 | 0.914400 / 1.016550 | 0.986910572490x | -0.251550 | YES |
| 4 | 19.186550 | 18.948250 | 1.108450 / 1.726700 | 1.012576359294x | +0.238300 | YES |
| 5 | 12.225950 | 12.179800 | 0.589800 / 0.703250 | 1.003789060576x | +0.046150 | YES |
| 6 | 12.263400 | 12.028450 | 0.716200 / 0.789200 | 1.019532857517x | +0.234950 | YES |

- Partial one-shape score at `128x5896`: `1.001134591090x`, delta `+0.113459109%`; Candidate faster in `3/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `23.552282%`, Candidate `28.277744%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 5896 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 65% before and 0% after Local; HBM was 59192 MB before and 59195 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- `CURRENT_LOCAL_BEST=NONE`; V056 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V056/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
