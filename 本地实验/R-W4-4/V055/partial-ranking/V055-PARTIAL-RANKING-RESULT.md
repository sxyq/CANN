# R-W4-4 V055 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `76d126423789f18d9f35311999b7323397232154c65f705c188ed324aaeea814`.
- Single change: `kSmallFp32BatchMaxWidth`, `5632 -> 5760`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 5752 | PASS, `rc=0` | FAIL, `rc=3` | 735023 | 4.73877 | not run |
| 5760 | PASS, `rc=0` | FAIL, `rc=3` | 736026 | 4.53209 | not run |
| 5768 | PASS, `rc=0` | PASS, `rc=0` | 0 | 4.52995e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and device snapshots are retained beside `v055-correctness.log`. Raw correctness process snapshots remain local but are excluded from Git because their command lines include unrelated `code-server --connection-token` arguments. C15 was excluded.

## Local

Fixed device 4; `128x5768` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w5768-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 20.517950 | 20.027800 | 1.461400 / 1.790350 | 1.024473481860x | +0.490150 | YES |
| 2 | 19.518750 | 19.797500 | 1.535450 / 1.544400 | 0.985919939386x | -0.278750 | YES |
| 3 | 20.168750 | 19.400450 | 1.798150 / 1.538750 | 1.039602174176x | +0.768300 | YES |
| 4 | 20.002850 | 19.366100 | 1.732600 / 1.725800 | 1.032879619541x | +0.636750 | YES |
| 5 | 19.627200 | 18.906750 | 1.600000 / 1.937650 | 1.038105438534x | +0.720450 | YES |
| 6 | 18.513100 | 19.927200 | 0.840450 / 1.706750 | 0.929036693565x | -1.414100 | YES |

- Partial one-shape score at `128x5768`: `1.008336224510x`, delta `+0.833622451%`; Candidate faster in `4/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `13.876929%`, Candidate `15.124654%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 5768 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 59% before and 64% after Local; HBM was 59192 MB before and 59195 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- `CURRENT_LOCAL_BEST=NONE`; V055 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V055/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
