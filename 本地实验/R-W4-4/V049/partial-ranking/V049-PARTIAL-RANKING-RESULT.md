# R-W4-4 V049 Partial Ranking

- Parent: exact R31B-V011, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `574ef10c053f111ead3104cf1105bc6ae0cf7e8e5e050928a7f200c9e17fee23`.
- Single change: `kSmallFp32BatchMaxWidth`, `4864 -> 4992`.
- Compile: PASS for route `device`/`submission` and both isolated correctness probes.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent failure; it was not repeated and is not a Candidate regression. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Correctness

Host `hwnput3`, device 4, FP32; Parent passed all tested widths.

| Width | Candidate | bad | max_abs | Local |
|---:|---|---:|---:|---|
| 4984 | FAIL, `rc=3` | 636148 | 4.50150 | not run |
| 4992 | FAIL, `rc=3` | 637629 | 4.79881 | not run |
| 5000 | PASS, `rc=0` | 0 | 3.57628e-06 | six interleaved pairs |

## Local

Fixed device 4; warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0; odd pairs Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each side retained 372 device-event samples. Pair speedup is `median(Parent device_us)/median(Candidate device_us)`; the sole eligible shape score is the arithmetic mean of its six pair speedups.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 18.3498 | 17.7391 | 2.00688 / 1.50891 | 1.034426774752x | +0.6107 | YES |
| 2 | 18.4167 | 17.9598 | 1.53703 / 1.51547 | 1.025440149668x | +0.4569 | YES |
| 3 | 18.0723 | 18.8916 | 1.71500 / 2.12938 | 0.956631518770x | -0.8193 | YES |
| 4 | 18.5686 | 19.0198 | 2.19031 / 2.19531 | 0.976277353074x | -0.4512 | YES |
| 5 | 17.4811 | 17.6350 | 1.30172 / 1.67203 | 0.991273036575x | -0.1539 | YES |
| 6 | 17.8281 | 17.8830 | 1.97203 / 2.21266 | 0.996930045294x | -0.0549 | YES |

- Partial one-shape score at `128x5000`: `0.996829813022x`, delta `-0.317018698%`; Candidate faster in `2/6` pairs.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. All 6/6 pair deltas are within combined MAD. Pooled raw CV: Parent `15.921885%`, Candidate `34.017057%`; Candidate maximum `71.4156 us` is retained.
- Width 5000 exceeds the changed cutoff, so this is an unchanged-path control, not evidence of threshold-path benefit. No full-route aggregate is claimed.
- Device-4 AICore was 58% before/after correctness and 59% before/after Local; HBM was 59192 MB before and 59195 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `CURRENT_LOCAL_BEST=NONE`; V049 is not promoted or an Official candidate.

All raw correctness and Local files, stdout/stderr, compile/correctness/local logs, run index, and device/process snapshots are retained here. Generated build directories are excluded from the evidence commit.
