# R-W4-4 V050 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `aa2cffe3ce75d6e4d1074e5989a73cde49a59bea76c8fd64ca4d9abcab30c9d0`.
- Single change: `kSmallFp32BatchMaxWidth`, `4992 -> 5120`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 5112 | FAIL, `rc=3` | 652699 | 4.81625 | not run |
| 5120 | FAIL, `rc=3` | 654320 | 4.79948 | not run |
| 5128 | PASS, `rc=0` | 0 | 3.8147e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 17.9195 | 18.0155 | 1.20484 / 1.91797 | 0.994671255308x | -0.0960 | YES |
| 2 | 18.5498 | 18.3113 | 1.62781 / 2.04828 | 1.013024744284x | +0.2385 | YES |
| 3 | 18.2484 | 17.0252 | 1.85484 / 1.84328 | 1.071846439396x | +1.2232 | YES |
| 4 | 18.7003 | 18.7731 | 1.68547 / 1.61406 | 0.996122110893x | -0.0728 | YES |
| 5 | 18.8891 | 19.1531 | 1.63203 / 1.82188 | 0.986216330516x | -0.2640 | YES |
| 6 | 18.5977 | 19.2152 | 1.86047 / 1.89156 | 0.967863982680x | -0.6175 | YES |

- Partial one-shape score at `128x5128`: `1.004957477180x`, delta `+0.495747718%`; Candidate faster in `2/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `13.569116%`, Candidate `41.818483%`; Candidate maximum `106.955 us` is retained.
- Width 5128 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 61% before and 58% after correctness, 63% before and 60% after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V050 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
