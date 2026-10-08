# R-W4-4 V078 Partial Ranking Result

- `REVISION=V078`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `cb80f94cd04dc6ab4e060b9d472fc533c7f7b9e6964c3097495a8df65c0b43a0`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 8176`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`) using the known C++ include path and sourced Ascend environment. Initial probe launches all returned `rc=1` before execution because the toolkit `libstdc++.so.6` lacked `GLIBCXX_3.4.29`; those logs are preserved. Retry 1 set `/usr/lib/aarch64-linux-gnu` ahead of toolkit libraries and completed on fixed device 4.

| FP32 shape | Exact Parent | V078 Candidate | Candidate bad / max abs | Local |
|---|---|---|---:|---|
| `128x8168` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1044850 / 4.48806` | not run |
| `128x8176` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1045877 / 4.49447` | not run |
| `128x8184` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0 / 2.6226e-06` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x8184` FP32. This width is above the V078 cutoff and uses the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; all samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v078-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=1.030244544464x`; `PARTIAL_ROUTE_LOCAL_DELTA=+3.0244544464%` (one unchanged control shape only).
- Candidate was faster in 5/6 pairs and slower in 1/6. Every pair's median difference is within the sum of its Parent and Candidate MADs.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean | `28.442494` | `28.744411` |
  | Median | `23.020300` | `22.182700` |
  | MAD | `3.108950` | `2.396950` |
  | P10 | `19.189500` | `19.514280` |
  | P90 | `42.599740` | `40.773040` |
  | Min / max | `13.003700 / 104.783000` | `17.176600 / 107.232000` |
  | CV | `60.83907508%` | `62.91201137%` |

- The pooled distributions and device load are noisy. Maxima were retained; no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `54% -> 74%`; HBM `59193 -> 59194 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T01:50:46Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V078 probe command remained.
- `CONTROL_ONLY=YES`; this Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed both tested threshold-path widths and passed only the unchanged control width.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=1.030244544464x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V078 is not promoted. Continue from exact `R31B-V011` with V079's next bounded threshold OFAT, cutoff `8192`, testing `128x8184/8192/8200` FP32; do not test C15.
