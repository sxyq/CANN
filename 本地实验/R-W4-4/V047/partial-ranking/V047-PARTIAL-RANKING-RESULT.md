# R-W4-4 V047 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `565f9134250c6fa4822a082562c3a2ca37bb9958486bf974bdd0dbabb44a6e38`.
- Single change: `kSmallFp32BatchMaxWidth`, `4608 -> 4736`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 4728 | FAIL, `rc=3` | 603583 | 4.86008 | not run |
| 4736 | FAIL, `rc=3` | 603836 | 4.85819 | not run |
| 4744 | PASS, `rc=0` | 0 | 3.33786e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 17.8978 | 17.7458 | 1.42437 / 1.02234 | 1.008565407026x | +0.1520 | YES |
| 2 | 18.0242 | 18.3897 | 1.59328 / 1.48375 | 0.980124743742x | -0.3655 | YES |
| 3 | 18.1048 | 18.0909 | 1.41437 / 1.07891 | 1.000768342095x | +0.0139 | YES |
| 4 | 17.1502 | 17.3878 | 1.82953 / 1.63094 | 0.986335246552x | -0.2376 | YES |
| 5 | 16.8558 | 18.0709 | 1.46047 / 1.20078 | 0.932759298098x | -1.2151 | YES |
| 6 | 17.6511 | 17.6228 | 1.49141 / 1.22516 | 1.001605874208x | +0.0283 | YES |

- Partial one-shape score at `128x4744`: `0.985026485287x`, delta `-1.497351471%`; Candidate faster in `3/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `47.375721%`, Candidate `11.226632%`; Parent maximum `118.656 us` and Candidate maximum `23.3294 us` are retained.
- Width 4744 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 55% before and 61% after correctness, 57% before and 63% after Local; HBM was 59192 MB before and 59194 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V047 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
