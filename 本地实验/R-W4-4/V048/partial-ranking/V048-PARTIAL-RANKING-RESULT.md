# R-W4-4 V048 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `bd900346df18fd4956a2b6ea2e14ae8bc97950d8b3dff99315bff30cd983dc5f`.
- Single change: `kSmallFp32BatchMaxWidth`, `4736 -> 4864`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 4856 | FAIL, `rc=3` | 620136 | 4.78134 | not run |
| 4864 | FAIL, `rc=3` | 621294 | 4.72886 | not run |
| 4872 | PASS, `rc=0` | 0 | 3.8147e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 16.1005 | 16.6266 | 1.55219 / 1.53937 | 0.968357932470x | -0.5261 | YES |
| 2 | 17.2436 | 17.3123 | 2.74656 / 1.28719 | 0.996031723110x | -0.0687 | YES |
| 3 | 16.9783 | 17.3717 | 1.65297 / 1.91859 | 0.977353972265x | -0.3934 | YES |
| 4 | 17.2353 | 17.6588 | 2.78500 / 2.04391 | 0.976017622942x | -0.4235 | YES |
| 5 | 16.9369 | 17.0352 | 2.22156 / 2.05094 | 0.994229595191x | -0.0983 | YES |
| 6 | 15.7770 | 17.3822 | 3.55078 / 1.47875 | 0.907652656166x | -1.6052 | YES |

- Partial one-shape score at `128x4872`: `0.969940583691x`, delta `-3.005941631%`; Candidate faster in `0/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `90.148861%`, Candidate `103.191410%`; Parent maximum `183.711 us` and Candidate maximum `191.611 us` are retained.
- Width 4872 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 53% before and 61% after correctness, 75% before and 63% after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V048 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
