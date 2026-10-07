# R-W4-4 V051 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `794e8ae72f1d914e144043eb5e359ad605f804ac35a6362dbcd80000d36afe01`.
- Single change: `kSmallFp32BatchMaxWidth`, `5120 -> 5248`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 5240 | FAIL, `rc=3` | 669301 | 4.87180 | not run |
| 5248 | FAIL, `rc=3` | 670289 | 4.80086 | not run |
| 5256 | PASS, `rc=0` | 0 | 3.8147e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 18.8698 | 18.7947 | 1.79313 / 2.06625 | 1.003995807329x | +0.0751 | YES |
| 2 | 18.7661 | 18.9027 | 1.84172 / 2.30891 | 0.992773519127x | -0.1366 | YES |
| 3 | 19.1347 | 19.2445 | 2.02469 / 2.09844 | 0.994294473746x | -0.1098 | YES |
| 4 | 18.4575 | 17.8239 | 1.97031 / 1.41969 | 1.035547775739x | +0.6336 | YES |
| 5 | 19.7448 | 17.5544 | 1.61094 / 1.16375 | 1.124777833478x | +2.1904 | YES |
| 6 | 19.3436 | 18.0155 | 2.05891 / 2.19297 | 1.073719852349x | +1.3281 | YES |

- Partial one-shape score at `128x5256`: `1.037518210295x`, delta `+3.751821030%`; Candidate faster in `4/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `48.167352%`, Candidate `43.687910%`; Parent maximum `194.709 us` and Candidate maximum `100.508 us` are retained.
- Width 5256 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 58% before and 56% after correctness, 62% before and 58% after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V051 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
