# R-W4-4 V060 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `0bff5757051627eb1952c34b02f1dc82b6494480aa8a5d68adc663d97ceaf370`.
- Single change: `kSmallFp32BatchMaxWidth`, `6272 -> 6400`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 6392 | PASS, `rc=0` | FAIL, `rc=3` | 817296 | 4.77885 | not run |
| 6400 | PASS, `rc=0` | FAIL, `rc=3` | 818213 | 4.79085 | not run |
| 6408 | PASS, `rc=0` | PASS, `rc=0` | 0 | 4.52995e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and pre/post device/process snapshots are retained beside `v060-correctness.log`. C15 was excluded.

## Local

Fixed device 4; `128x6408` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w6408-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of all six pair speedups; no sample or pair was removed.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 19.607500 | 12.250800 | 0.863150 / 0.574700 | 1.600507721945x | +7.356700 | NO |
| 2 | 12.257000 | 13.094350 | 0.855000 / 0.611850 | 0.936052572293x | -0.837350 | YES |
| 3 | 12.327800 | 12.639350 | 0.720000 / 0.781900 | 0.975350789400x | -0.311550 | YES |
| 4 | 11.599200 | 12.284550 | 0.880750 / 0.991700 | 0.944210410638x | -0.685350 | YES |
| 5 | 11.951700 | 12.139850 | 0.718000 / 0.453900 | 0.984501455949x | -0.188150 | YES |
| 6 | 12.367500 | 12.824650 | 0.666050 / 0.751250 | 0.964353803028x | -0.457150 | YES |

- Partial one-shape score at `128x6408`: `1.067496125542x`, delta `+6.749612554%`; Candidate faster in `1/6` pairs.
- Pair 1's delta is outside combined MAD; the other five are within. Pooled raw CV: Parent `24.746434%`, Candidate `8.293485%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All numeric samples are retained.
- Device 4 AICore was 62% before and 0% after Local; HBM was 59192 MB before and 59195 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- Width 6408 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- `CURRENT_LOCAL_BEST=NONE`; V060 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V060/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
