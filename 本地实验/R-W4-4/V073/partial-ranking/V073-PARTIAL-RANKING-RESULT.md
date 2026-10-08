# R-W4-4 V073 Partial Ranking Result

- `REVISION=V073`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `4f59d75f33d629fee975d97be49b6ebdcb0505300892ca842320da4500a58570`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 8064`; the Candidate differs from exact V011 only at this constant.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; excluded and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build passed (`ROUTE_BUILD_RC=0`) and the `clx_ref_parent_probe` / `clx_ref_candidate_probe` reference-harness targets built successfully (`REF_PROBE_BUILD_RC=0`). Correctness was run on fixed device 4 for only the authorized FP32 shapes; C15 was not run.

| FP32 shape | Exact Parent | V073 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x8056` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1030468` | not run |
| `128x8064` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1031499` | not run |
| `128x8072` | PASS, `rc=0`, `bad=0` | PASS, `rc=0` | `0` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x8072` FP32. This width is above the V073 cutoff and exercises the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side; 744 total); all invocations returned `rc=0,bad=0`. All samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Per-pair values are in `v073-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.991414587078x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.8585412922%` (one unchanged control shape only).
- Median-based direction: Candidate faster in 2/6 pairs and slower in 4/6. All six paired median deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 nearest-rank; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `21.352200` | `21.538300` |
  | MAD | `2.584400` | `2.000650` |
  | P10 | `17.928100` | `19.143100` |
  | P90 | `44.095000` | `50.357500` |
  | Min / max | `11.424700 / 204.675000` | `15.599100 / 201.040000` |
  | CV | `86.28945375%` | `85.29802211%` |

- Both pooled distributions are highly noisy; all maxima are retained and no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `54% -> 55%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after, untouched.
- `CONTROL_ONLY=YES`; Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate failed both tested threshold-path shapes and passed only the unchanged control; exact Parent passed all three.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.991414587078x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V073 is not promoted. Use exact `R31B-V011` as Parent for the next threshold OFAT.
