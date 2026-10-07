# R-W4-4 V052 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `38b38942b41a239802db056e4f909d809877ab1dabe00213fb4ca2dc46e7caf6`.
- Single change: `kSmallFp32BatchMaxWidth`, `5248 -> 5376`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 5368 | FAIL, `rc=3` | 685731 | 4.72661 | not run |
| 5376 | FAIL, `rc=3` | 687219 | 4.77354 | not run |
| 5384 | PASS, `rc=0` | 0 | 4.05312e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 11.4508 | 11.9350 | 0.64375 / 0.768438 | 0.959430247172x | -0.4842 | YES |
| 2 | 11.2733 | 11.5964 | 0.956875 / 0.803125 | 0.972137904867x | -0.3231 | YES |
| 3 | 11.4652 | 11.7536 | 0.609375 / 0.643281 | 0.975462836918x | -0.2884 | YES |
| 4 | 11.6486 | 12.2036 | 0.428594 / 0.831250 | 0.954521616572x | -0.5550 | YES |
| 5 | 11.2367 | 10.9070 | 0.789844 / 0.598438 | 1.030228293756x | +0.3297 | YES |
| 6 | 11.5970 | 11.6062 | 0.568750 / 0.487500 | 0.999207320225x | -0.0092 | YES |

- Partial one-shape score at `128x5384`: `0.981831369918x`, delta `-1.816863008%`; Candidate faster in `1/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `7.774240%`, Candidate `8.289286%`; all samples retained.
- Width 5384 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 64% before and 0% after correctness, and 0% before/after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V052 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
