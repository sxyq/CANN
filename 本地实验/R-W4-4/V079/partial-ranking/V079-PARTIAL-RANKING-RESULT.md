# R-W4-4 V079 Partial Ranking Result

- `REVISION=V079`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `d3a471ccd4e82bb1244ecb0b991f693d52e5e674cd2def36a19f4b62a34d8592`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 8192`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`). Correctness used the established C++ include/runtime paths and fixed device 4.

| FP32 shape | Exact Parent | V079 Candidate | Local |
|---|---|---|---|
| `128x8184` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3`, `bad=1046897`, `max_abs=4.4924` | not run |
| `128x8192` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0`, `max_abs=2.86102e-06` | yes |
| `128x8200` | FAIL, `rc=3`, `bad=525055`, `max_abs=1.15452` | FAIL, `rc=3`, `bad=469905`, `max_abs=0.859855` | not run |

At `128x8200`, the exact Parent also fails, so this is outside the jointly correctness-valid region and is not classified as a Candidate regression.

## Partial Local

Host `hwnput3`, fixed device 4, `128x8192` FP32. This is the only jointly passing tested shape and is on the V079 threshold-selected path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; all samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v079-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.993741277695x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.6258722305%`.
- Candidate was faster in 2/6 pairs and slower in 4/6. Every pair's median difference is within the sum of its Parent and Candidate MADs; the direction is mixed and the pooled Candidate includes a retained `36.2538 us` maximum.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean | `21.007642` | `21.275390` |
  | Median | `20.998750` | `21.038900` |
  | MAD | `1.012350` | `1.059200` |
  | P10 | `19.048130` | `19.090550` |
  | P90 | `23.135530` | `23.397350` |
  | Min / max | `14.382500 / 24.982800` | `16.024700 / 36.253800` |
  | CV | `7.61347946%` | `9.50523510%` |

- The accelerator remained busy; device 4 AICore was `63% -> 63%`, HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched. `LOAD_QUALITY=NOISY`; all raw samples and maxima are retained. `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES` because the small mixed-direction differences are within paired noise.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T02:09:33Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V079 probe command remained.
- `CONTROL_ONLY=NO`; this Local measures the jointly-correct threshold-selected shape `128x8192`.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: jointly valid at `128x8192`; Candidate-only failure at `128x8184`; both Parent and Candidate fail at `128x8200`.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.993741277695x` (partial, noisy; `-0.6258722305%`).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V079 is not promoted. Continue the threshold OFAT from exact `R31B-V011`; C15 remains excluded.
