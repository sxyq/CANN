# R-W4-4 V059 Partial Ranking

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `8b9ecec2d72ea45c0bbac7391c16dc2fcdc2aac6fdd3cbf55fb36458acd63412`.
- Single change: `kSmallFp32BatchMaxWidth`, `6144 -> 6272`.
- Compile: PASS for route `device`/`submission` and isolated Parent/Candidate correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARENT_KNOWN_CORRECTNESS_FAILURE=YES`; `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32. Parent passed every tested width.

| Width | Parent | Candidate | Candidate `bad` | Candidate `max_abs` | Local |
|---:|---|---|---:|---:|---|
| 6264 | PASS, `rc=0` | FAIL, `rc=3` | 800683 | 4.51409 | not run |
| 6272 | PASS, `rc=0` | FAIL, `rc=3` | 802085 | 4.8316 | not run |
| 6280 | PASS, `rc=0` | PASS, `rc=0` | 0 | 5.00679e-06 | six interleaved pairs |

Correctness stdout, stderr, raw event TSVs, stats, and pre/post device/process snapshots are retained beside `v059-correctness.log`. C15 was excluded.

## Local

Fixed device 4; `128x6280` FP32; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; six interleaved pairs with alternating Parent/Candidate order. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Raw data is in `local-pair*-{parent,candidate}-r128-w6280-fp32-raw.tsv`; run index, stdout/stderr, device snapshots, and process-name/RSS snapshots are retained alongside them.

Pair speedup is `median(Parent device_us)/median(Candidate device_us)` computed directly from each pair's 62 raw event samples. The sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 19.180150 | 19.596550 | 0.949550 / 1.040300 | 0.978751361847x | -0.416400 | YES |
| 2 | 19.571250 | 18.943250 | 1.175300 / 0.910150 | 1.033151650324x | +0.628000 | YES |
| 3 | 19.220450 | 19.214250 | 1.024500 / 1.161600 | 1.000322677180x | +0.006200 | YES |
| 4 | 20.405950 | 19.841900 | 1.058400 / 1.113450 | 1.028427217152x | +0.564050 | YES |
| 5 | 19.852650 | 20.146100 | 1.774050 / 1.430650 | 0.985433905322x | -0.293450 | YES |
| 6 | 20.396400 | 21.011250 | 1.241750 / 1.424050 | 0.970737105122x | -0.614850 | YES |

- Partial one-shape score at `128x6280`: `0.999470652824x`, delta `-0.052934718%`; Candidate faster in `3/6` pairs.
- All six pair deltas are within combined Parent/Candidate MAD. Pooled raw CV: Parent `11.425779%`, Candidate `10.319003%` (372 samples/side). `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Numeric samples are retained, not filtered.
- Width 6280 is above the changed cutoff and uses the unchanged path; this is a control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device 4 AICore was 64% before and 57% after Local; HBM was 59192 MB before and 59195 MB after. Existing PID `2999855` remained at 55666 MB; no other process was touched.
- `CURRENT_LOCAL_BEST=NONE`; V059 is not promoted and is not an Official-comparable candidate.

`本地实验/R-W4-4/V059/partial-ranking/` is the canonical evidence directory. Generated build directories are excluded from the evidence commit.
