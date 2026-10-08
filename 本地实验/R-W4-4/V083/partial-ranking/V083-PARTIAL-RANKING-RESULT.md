# R-W4-4 V083 Partial Ranking Result

- `REVISION=V083`; direct comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `f3639b16113de579efe221aa8aacd01448ea3d3dd7f471cb7811dde8b0517fcd`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `7680 -> 7936`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure and was excluded; it was not run here.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and Parent/Candidate reference probe builds passed. Correctness ran on host `hwnput3`, fixed device 4, FP32, using the exact V011 Parent.

| Shape | Parent | V083 Candidate | Candidate bad / max abs | Local |
|---|---|---|---:|---|
| `128x7928` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1014080 / 4.75908` | not run |
| `128x7936` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1015150 / 4.7839` | not run |
| `128x7944` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0 / 2.38419e-06` | control-only Local below |

The two shapes selecting the changed threshold path fail Candidate correctness. The only jointly correct shape is above the cutoff and uses the unchanged control path.

## Partial Local

Local was limited to the jointly correct `128x7944` FP32 control shape, on fixed device 4. Six interleaved P/C pairs used 45 warmups, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each invocation returned `rc=0,bad=0`; all 744 device-event samples were retained (372/side). Raw files are `v083-local-pair*-raw.tsv`; pair medians/MADs are in `v083-local-paired-summary.tsv`.

For each pair, `pair_speedup = Parent_median / Candidate_median` over its 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of six pair speedups; delta is `(score - 1) * 100`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.979917539337x`; `PARTIAL_ROUTE_LOCAL_DELTA=-2.008246066%`.
- Candidate was faster in 3/6 pairs. All six pair median differences were within the combined Parent+Candidate MAD; the data do not resolve a direction.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean (us) | `22.084934140` | `22.379190054` |
  | Median (us) | `22.054050000` | `22.389850000` |
  | MAD (us) | `1.448150000` | `1.664500000` |
  | P10 / P90 (us) | `19.584440 / 25.058760` | `19.775500 / 25.640390` |
  | Min / max (us) | `11.428100 / 27.855600` | `11.612200 / 28.251200` |
  | CV | `11.000246911%` | `12.891447660%` |
  | Effective throughput (Gelem/s) | `46.106361417` | `45.414864325` |

- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. The score is numeric and retained despite noisy load; it is control-only and not evidence of a threshold-path performance effect.
- Pre-Local snapshot at `2026-10-08T03:00:55.430081712Z`: AICore `59%`, HBM `59192/65536 MB` (6344 MB free). Post-Local snapshot at `2026-10-08T03:05:06.973369060Z`: AICore `65%`, HBM `59195/65536 MB` (6341 MB free). Existing PID `2999855` (`VLLM::EngineCore`) was present in both process snapshots and left untouched.
- Local raw and post-Local load/process snapshots completed at `2026-10-08T03:05:06.973369060Z`. `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T03:06:25.591326873Z`, after confirming no active MODE probe process.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed the two changed-path widths and passed only the unchanged control.
- `PARTIAL_CORRECTNESS=YES`; `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded C15.
- `LOCAL_SCORE=0.979917539337x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V083 is not promoted. The next threshold experiment must start from exact `R31B-V011` and exclude C15.
