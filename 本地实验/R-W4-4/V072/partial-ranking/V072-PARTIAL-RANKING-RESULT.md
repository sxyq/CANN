# R-W4-4 V072 Partial Ranking Result

- `REVISION=V072`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `f3639b16113de579efe221aa8aacd01448ea3d3dd7f471cb7811dde8b0517fcd`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 7936`; the Candidate differs from exact V011 only at this constant.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; excluded and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build passed (`ROUTE_BUILD_RC=0`) and the `clx_ref_parent_probe` / `clx_ref_candidate_probe` reference-harness targets built successfully (`REF_PROBE_BUILD_RC=0`). Correctness was run on fixed device 4 for only the authorized FP32 shapes; C15 was not run.

| FP32 shape | Exact Parent | V072 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x7928` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1014056` | not run |
| `128x7936` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1015130` | not run |
| `128x7944` | PASS, `rc=0`, `bad=0` | PASS, `rc=0` | `0` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x7944` FP32. This width is above the V072 cutoff and exercises the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side; 744 total); all invocations returned `rc=0,bad=0`. All samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Per-pair values are in `v072-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.995806084388x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.4193915612%` (one unchanged control shape only).
- Median-based direction: Candidate faster in 3/6 pairs and slower in 3/6. All six paired median deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 nearest-rank; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `22.760650` | `22.900000` |
  | MAD | `1.740500` | `1.725950` |
  | P10 | `19.694400` | `20.149100` |
  | P90 | `26.549700` | `26.203700` |
  | Min / max | `11.295300 / 149.793000` | `11.247200 / 29.733100` |
  | CV | `45.85363627%` | `12.53505077%` |

- The Parent maximum is retained; no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `57% -> 58%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after, untouched.
- `CONTROL_ONLY=YES`; Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate failed both tested threshold-path shapes and passed only the unchanged control; exact Parent passed all three.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.995806084388x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V072 is not promoted. Use exact `R31B-V011` as Parent for the next threshold OFAT.
