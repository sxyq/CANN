# R-W4-4 V068 Partial Ranking Result

- `REVISION=V068`; `DIRECT_PARENT=R31B-V011` exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `5acd010f4ed60c0288c154cab6c84904edb7871b61c1dd6ef0f51edd78ed9eb7`.
- Single change: `kSmallFp32BatchMaxWidth`, `4096 -> 7424`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route `device`/`submission` and isolated Parent/Candidate correctness probes compiled successfully (`ROUTE_BUILD_RC=0`, `PROBE_BUILD_RC=0`).

On local host `hwnput3`, device 4, exact R31B-V011 Parent passed all three FP32 shapes. Candidate failed in the changed threshold path and passed only the outside control:

| Shape | Parent | Candidate | Candidate bad / max_abs | Local |
|---|---|---|---:|---|
| `128x7416` | PASS, `rc=0`, max_abs `1.66893e-06` | FAIL, `rc=3` | `948498 / 4.72435` | not run |
| `128x7424` | PASS, `rc=0`, max_abs `1.66893e-06` | FAIL, `rc=3` | `949573 / 4.77558` | not run |
| `128x7432` | PASS, `rc=0`, max_abs `1.90735e-06` | PASS, `rc=0` | `0 / 1.90735e-06` | control-only Local below |

C15 was excluded. Correctness stdout/stderr, raw event files, stats, source identities, and pre/post device/process snapshots remain in this directory.

## Partial Local

Only jointly passing `128x7432` FP32 was measured. This is outside the V068 threshold and uses the unchanged control path; it is not evidence of threshold-path benefit. Device 4 used device events, warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, and six interleaved P/C pairs in alternating order. All 12 invocations returned `rc=0,bad=0`; each side retained 372 raw device-event samples. No sample or pair was removed.

For each pair, compute Parent and Candidate medians from all 62 raw `device_us` values, then `pair_speedup = Parent_median / Candidate_median`. The one-shape partial score is the arithmetic mean of all six pair speedups; `delta_pct = (score - 1) * 100`. Pair medians, MADs, deltas, and MAD-envelope flags are in `v068-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.993107010002x`.
- `PARTIAL_ROUTE_LOCAL_DELTA=-0.6892989998%`.
- Candidate was faster in 2/6 pairs and slower in 4/6. All paired deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw jitter (372 samples/side): Parent median `21.6153 us`, MAD `1.6312 us`, p10/p90 `19.1597/25.0891 us`, CV `41.9910311%`; Candidate median `21.70375 us`, MAD `1.39485 us`, p10/p90 `19.55/24.5391 us`, CV `8.7734886%`.
- Device 4 load: AICore `64% -> 61%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was untouched.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`.
- `CURRENT_LOCAL_BEST=NONE`. Do not promote this partial/control-only result. Continue threshold OFAT from exact R31B-V011.

## Result Flags

- `COMPILE=PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passes all three shapes; Candidate fails both changed-path shapes and passes only the unchanged control.
- `LOCAL_SCORE=0.993107010002x` (partial, control-only, noisy; not an Official-comparable score).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; no Candidate Local Best is established.
