# R-W4-4 V081 Partial Ranking Result

- `REVISION=V081`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `be67a911e055785644a16f4e507c9a12f88ed4b66253d22435387457b8bd7398`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 7168`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`). Correctness used the established C++ include/runtime paths and fixed device 4.

| FP32 shape | Exact Parent | V081 Candidate | Candidate bad / max abs | Local |
|---|---|---|---:|---|
| `128x7160` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `915631 / 4.59165` | not run |
| `128x7168` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `916781 / 4.79617` | not run |
| `128x7176` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0 / 2.14577e-06` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x7176` FP32. This width is above the V081 cutoff and uses the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; all samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v081-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.989459941349x`; `PARTIAL_ROUTE_LOCAL_DELTA=-1.0540058651%` (one unchanged control shape only).
- Candidate was faster in 1/6 pairs and slower in 5/6. Every pair's median difference is within the sum of its Parent and Candidate MADs.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean | `21.851899` | `20.667100` |
  | Median | `20.618900` | `20.762000` |
  | MAD | `1.296900` | `1.018100` |
  | P10 | `18.388750` | `18.606030` |
  | P90 | `23.456610` | `22.629400` |
  | Min / max | `16.135000 / 137.342000` | `13.102500 / 25.577500` |
  | CV | `45.16838676%` | `8.03252512%` |

- The Parent maximum is retained; paired deltas are small relative to combined MAD. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`; no sample was removed.
- Device 4 snapshot: AICore `65% -> 57%`; HBM `59192 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T02:38:51Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V081 probe command remained.
- `CONTROL_ONLY=YES`; this Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed both threshold-path shapes and passed only the unchanged control.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.989459941349x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V081 is not promoted. Continue the bounded threshold OFAT from exact `R31B-V011`; C15 remains excluded.
