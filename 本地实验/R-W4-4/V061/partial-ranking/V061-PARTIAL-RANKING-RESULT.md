# R-W4-4 V061 Partial Ranking Result

- `REVISION=V061`; `DIRECT_PARENT=R31B-V011` exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `38eaa1e5b97f4d62799120a3c5e0d9e6b7e4e164b2c86689002da1e2dddf42ee`.
- Single change from the previous threshold step: `kSmallFp32BatchMaxWidth`, `6400 -> 6528`; the V061 source differs from exact V011 only at this constant (`4096 -> 6528`).
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route `device`/`submission` and isolated Parent/Candidate probe targets compiled successfully (`ROUTE_BUILD_RC=0`, `PROBE_BUILD_RC=0`). See `../v061-compile.log`.

The first probe launch attempt exited `127` because `libgraph.so` was not on the runtime library path. After sourcing the toolkit environment, retry 1 exited before device execution because the toolchain RPATH selected a `libstdc++.so.6` without `GLIBCXX_3.4.29`. Both failed launch attempts are retained. Retry 2 sourced the same toolkit environment and placed the system ARM64 library directory first for these probe processes; all six correctness runs then reached device 4.

| FP32 shape | Exact Parent | V061 Candidate | Candidate bad / max_abs | Local |
|---|---|---|---:|---|
| `128x6520` | PASS, `rc=0`, max_abs `4.29153e-06` | FAIL, `rc=3` | `833504 / 4.73264` | not run |
| `128x6528` | PASS, `rc=0`, max_abs `4.52995e-06` | FAIL, `rc=3` | `834742 / 4.75968` | not run |
| `128x6536` | PASS, `rc=0`, max_abs `3.8147e-06` | PASS, `rc=0` | `0 / 3.8147e-06` | control-only Local below |

Raw stdout/stderr, stats, correctness event files, failed launch diagnostics, and pre/post device/process snapshots remain in this directory. No C15 case was run.

## Partial Local

Host `hwnput3`, fixed device 4, `128x6536` FP32. This width is outside the V061 cutoff and exercises the unchanged control path; its Local number is not evidence of threshold-path benefit. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side total); all invocations returned `rc=0,bad=0`. No sample or pair was removed.

For each pair, the Parent and Candidate medians are computed from their 62 raw `device_us` values. `pair_speedup = Parent_median / Candidate_median`; the one-shape partial score is the arithmetic mean of the six pair speedups, with `delta_pct = (score - 1) * 100`. Exact pair values are in `v061-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.999641978447x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.0358021553%` (one unchanged control shape only).
- Candidate was faster in 3/6 pairs and slower in 3/6. All six paired deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw CV: Parent `14.8296768%`, Candidate `14.2805125%`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `57% -> 58%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) remained untouched.
- `CURRENT_LOCAL_BEST=NONE`. This control-only partial score does not promote V061; use exact R31B-V011 as the next threshold OFAT baseline.

## Result Flags

- `COMPILE=PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate failed both tested threshold-path shapes and passed only the unchanged control; the exact Parent passed all three.
- `LOCAL_SCORE=0.999641978447x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; no Candidate Local Best is established.
