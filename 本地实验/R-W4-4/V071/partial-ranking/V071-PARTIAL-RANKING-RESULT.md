# R-W4-4 V071 Partial Ranking Result

- `REVISION=V071`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `29f08fb809540a0ae7c8208ba5f909b1780ce7bcd09d5698efe62b780501e822`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 7808`; the Candidate differs from exact V011 only at this constant.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; excluded and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build passed (`ROUTE_BUILD_RC=0`). The first probe-build attempt selected legacy `clx_parent_probe` / `clx_candidate_probe` targets and failed with `PROBE_BUILD_RC=2` because those targets include unavailable `runner_main.inc`; that failed log is retained. The correct `clx_ref_parent_probe` / `clx_ref_candidate_probe` reference-harness targets built successfully (`REF_PROBE_BUILD_RC=0`).

Correctness was run on fixed device 4 for only the authorized FP32 shapes; C15 was not run.

| FP32 shape | Exact Parent | V071 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x7800` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `997666` | not run |
| `128x7808` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `998677` | not run |
| `128x7816` | PASS, `rc=0`, `bad=0` | PASS, `rc=0` | `0` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x7816` FP32. This width is above the V071 cutoff and exercises the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side; 744 total); all invocations returned `rc=0,bad=0`. All samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Per-pair values are in `v071-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.987424014760x`; `PARTIAL_ROUTE_LOCAL_DELTA=-1.2575985240%` (one unchanged control shape only).
- Median-based direction: Candidate faster in 2/6 pairs and slower in 4/6. All six paired median deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 nearest-rank; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `22.131250` | `22.596250` |
  | MAD | `1.675450` | `1.601050` |
  | P10 | `19.636600` | `19.551900` |
  | P90 | `26.415900` | `25.719400` |
  | Min / max | `11.590000 / 152.785000` | `11.322500 / 52.544400` |
  | CV | `52.79541060%` | `14.47710157%` |

- The Parent maximum is retained; no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `60% -> 59%`; HBM `59192 -> 59194 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after, untouched.
- `CONTROL_ONLY=YES`; Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; correct reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate failed both tested threshold-path shapes and passed only the unchanged control; exact Parent passed all three.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.987424014760x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V071 is not promoted. The next threshold OFAT must use exact `R31B-V011` as Parent.
