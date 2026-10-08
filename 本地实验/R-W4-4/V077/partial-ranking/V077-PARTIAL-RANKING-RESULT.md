# R-W4-4 V077 Partial Ranking Result

- `REVISION=V077`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `194648a77e5ccecf0badf98a8e9fdde5d6fb6f72dd5327d13ec9babd1b355d8e`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 8160`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`). All device correctness commands used fixed device 4 and the exact Parent source above.

| FP32 shape | Exact Parent | V077 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x8152` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1042744` | not run |
| `128x8160` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1043810` | not run |
| `128x8168` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x8168` FP32. This width is above the V077 cutoff and uses the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; no samples were discarded.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v077-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=1.003389141975x`; `PARTIAL_ROUTE_LOCAL_DELTA=+0.3389141975%` (one unchanged control shape only).
- Candidate was faster in 4/6 pairs and slower in 2/6. Every pair's median difference is within the sum of its Parent and Candidate MADs.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `21.705000` | `21.696400` |
  | MAD | `1.788250` | `2.166700` |
  | P10 | `19.616890` | `19.362510` |
  | P90 | `44.488370` | `46.733670` |
  | Min / max | `17.627200 / 231.006000` | `17.069100 / 126.741000` |
  | CV | `76.17172043%` | `70.44574215%` |

- The pooled distributions and device load are noisy. Maxima were retained; no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `65% -> 72%`; HBM `59192 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T01:37:06Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V077 probe command remained.
- `CONTROL_ONLY=YES`; this Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed both tested threshold-path widths and passed only the unchanged control width.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=1.003389141975x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V077 is not promoted. Next threshold OFAT is V078 from exact `R31B-V011`, cutoff `8176`, testing `128x8168/8176/8184` FP32; do not test C15.
