# R-W4-4 V053 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `138f425c48bfb37904a6d6ca3143fb4f2b85b2bad6afe346ba9c4f937bdc6201`.
- Single change: `kSmallFp32BatchMaxWidth`, `5376 -> 5504`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 5496 | FAIL, `rc=3` | 702391 | 4.69776 | not run |
| 5504 | FAIL, `rc=3` | 703705 | 4.84821 | not run |
| 5512 | PASS, `rc=0` | 0 | 4.29153e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 19.3009 | 18.1734 | 1.70719 / 1.79109 | 1.062041225087x | +1.1275 | YES |
| 2 | 19.2516 | 18.2570 | 1.48109 / 1.71781 | 1.054477734568x | +0.9946 | YES |
| 3 | 18.7366 | 18.9422 | 1.37438 / 1.66828 | 0.989145928139x | -0.2056 | YES |
| 4 | 18.4722 | 18.9702 | 1.23578 / 1.78844 | 0.973748299965x | -0.4980 | YES |
| 5 | 18.9045 | 19.2122 | 0.917813 / 1.37188 | 0.983984135081x | -0.3077 | YES |
| 6 | 17.8531 | 19.3688 | 1.25641 / 1.43250 | 0.921745281071x | -1.5157 | YES |

- Partial one-shape score at `128x5512`: `0.997523767319x`, delta `-0.247623268%`; Candidate faster in `2/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `11.297686%`, Candidate `13.440554%`; all samples retained.
- Width 5512 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 64% before and 59% after correctness, and 60% before and 66% after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V053 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
