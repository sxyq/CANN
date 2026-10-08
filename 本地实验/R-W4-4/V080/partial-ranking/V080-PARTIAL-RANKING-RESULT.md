# R-W4-4 V080 Partial Ranking Result

- `REVISION=V080`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `b86de625abbe610f4c80e95d87b6aa3503b02cc7bdb8512c0a530ca1551dde44`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 6144`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`). Correctness used the established C++ include/runtime paths and fixed device 4.

| FP32 shape | Exact Parent | V080 Candidate | Candidate bad / max abs | Local |
|---|---|---|---:|---|
| `128x6136` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `784280 / 4.53888` | not run |
| `128x6144` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `785370 / 4.7333` | not run |
| `128x6152` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0 / 5.00679e-06` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x6152` FP32. This width is above the V080 cutoff and uses the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; all samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v080-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.982952181578x`; `PARTIAL_ROUTE_LOCAL_DELTA=-1.7047818422%` (one unchanged control shape only).
- Candidate was faster in 2/6 pairs and slower in 4/6. Every pair's median difference is within the sum of its Parent and Candidate MADs.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean | `18.963084` | `19.135321` |
  | Median | `19.050950` | `19.278600` |
  | MAD | `1.020650` | `1.044400` |
  | P10 | `17.128390` | `17.294340` |
  | P90 | `21.379730` | `21.278770` |
  | Min / max | `11.766600 / 23.429100` | `10.283100 / 23.459700` |
  | CV | `10.17745361%` | `9.69412920%` |

- All pair differences are within the combined MAD; retain the numeric score but classify `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`. `LOAD_QUALITY=NOISY`; all raw samples are preserved.
- Device 4 snapshot: AICore `62% -> 64%`; HBM `59192 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T02:28:20Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V080 probe command remained.
- `CONTROL_ONLY=YES`; this Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed both threshold-path shapes and passed only the unchanged control.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.982952181578x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V080 is not promoted. Continue the bounded threshold OFAT from exact `R31B-V011`; C15 remains excluded.
