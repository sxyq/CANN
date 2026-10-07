# R-W4-4 V046 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `7ee85626738e5bf792731103bad358862d521b262bb0c35b2dcd096400aec285`.
- Single change: `kSmallFp32BatchMaxWidth`, `4480 -> 4608`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes. Initial nounset invocation failure and successful retry are retained in `v046-compile.log`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 4600 | FAIL, `rc=3` | 586818 | 4.85819 | not run |
| 4608 | FAIL, `rc=3` | 588033 | 4.83657 | not run |
| 4616 | PASS, `rc=0` | 0 | 3.57628e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate, even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the eligible one-shape score is the arithmetic mean of the six pair speedups.

| Pair | Parent median us | Candidate median us | Speedup | Delta us | Within P/C MAD sum |
|---:|---:|---:|---:|---:|---|
| 1 | 17.2437 | 17.4277 | 0.989442095056x | -0.1840 | YES |
| 2 | 18.2300 | 18.3783 | 0.991930700881x | -0.1483 | YES |
| 3 | 16.6662 | 18.5906 | 0.896485320538x | -1.9244 | YES |
| 4 | 18.0305 | 17.7117 | 1.017999401526x | +0.3188 | YES |
| 5 | 17.5361 | 17.9661 | 0.976066035478x | -0.4300 | YES |
| 6 | 16.9175 | 18.4122 | 0.918820130131x | -1.4947 | YES |

- Partial one-shape score at `128x4616`: `0.965123947268x`, delta `-3.487605273%`; Candidate faster in `1/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All pair deltas are within their combined MAD. Pooled raw CV: Parent `15.694914%`, Candidate `17.049630%`; Candidate max event `53.9122 us` is retained.
- Width 4616 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 0% before correctness and 44% before Local (58% after); HBM was 59192 MB before and 59194 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB, untouched.
- `CURRENT_LOCAL_BEST=NONE`; V046 is not promoted and is not an Official candidate.

All raw correctness/Local files, logs, run index, and device/process snapshots are in this directory. Generated build directories are excluded from the result commit.
