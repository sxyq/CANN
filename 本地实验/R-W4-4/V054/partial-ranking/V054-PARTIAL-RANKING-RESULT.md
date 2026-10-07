# R-W4-4 V054 Partial Ranking

- Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `8160756d4f1767383af7137f473448e4c9ed5fa5967ced92e73c530be8da1247`.
- Single change: `kSmallFp32BatchMaxWidth`, `5504 -> 5632`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 5624 | PASS, `rc=0` | FAIL, `rc=3` | 718534 | 4.86651 | not run |
| 5632 | PASS, `rc=0` | FAIL, `rc=3` | 719678 | 4.78816 | not run |
| 5640 | PASS, `rc=0` | PASS, `rc=0` | 0 | 4.76837e-06 | six interleaved pairs |

All Parent/Candidate correctness stdout, stderr, raw event TSVs, stats, and pre/post snapshots are retained beside `v054-correctness.log`. Device snapshots are committed. Raw process snapshots remain local but are excluded from Git because they contain unrelated `code-server --connection-token` arguments. C15 was excluded.

## Local

Fixed device 4; `128x5640` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w5640-fp32-raw.tsv`; the run index and stdout/stderr are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 19.298900 | 18.227000 | 1.821550 / 1.135600 | 1.058808361222x | +1.071900 | YES |
| 2 | 19.099500 | 19.617800 | 1.373600 / 1.571250 | 0.973580116017x | -0.518300 | YES |
| 3 | 18.688900 | 19.678600 | 1.759850 / 1.228450 | 0.949706788085x | -0.989700 | YES |
| 4 | 19.084100 | 18.990300 | 1.641400 / 1.470500 | 1.004939363780x | +0.093800 | YES |
| 5 | 19.323250 | 19.875900 | 1.205950 / 1.779700 | 0.972194969788x | -0.552650 | YES |
| 6 | 18.240150 | 18.746900 | 1.102350 / 1.076850 | 0.972968864186x | -0.506750 | YES |

- Partial one-shape score at `128x5640`: `0.988699743846x`, delta `-1.130025615%`; Candidate faster in `2/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `12.158454%`, Candidate `12.235389%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 5640 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 59% before and 62% after Local; HBM was 59192 MB before and 59194 MB after. Existing processes were left untouched.
- `CURRENT_LOCAL_BEST=NONE`; V054 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V054/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
