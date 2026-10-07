# R-W4-4 V063 Partial Ranking Result

- `REVISION=V063`; `DIRECT_PARENT=R31B-V011` exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `4fc39adab2dbb187e9ab27d4e0dc690b35b747870af9eb39bb2d050214c7dee8`.
- Single change: `kSmallFp32BatchMaxWidth`, `4096 -> 6784`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route `device`/`submission` compiled successfully (`ROUTE_BUILD_RC=0`). Isolated exact-Parent and Candidate correctness probes also compiled successfully (`PROBE_BUILD_RC=0`).

On local host `hwnput3`, device 4, exact R31B-V011 Parent passed all tested FP32 widths. Candidate failed within the changed threshold path and passed only the outside control:

| Shape | Parent | Candidate | Candidate bad / max_abs | Local |
|---|---|---|---:|---|
| `128x6776` | PASS, `rc=0`, max_abs `3.57628e-06` | FAIL, `rc=3` | `866397 / 4.59968` | not run |
| `128x6784` | PASS, `rc=0`, max_abs `3.57628e-06` | FAIL, `rc=3` | `867506 / 4.72999` | not run |
| `128x6792` | PASS, `rc=0`, max_abs `3.57628e-06` | PASS, `rc=0` | `0 / 3.57628e-06` | control-only Local below |

C15 was excluded. Correctness stdout/stderr, raw event files, stats, source identities, and pre/post device/process snapshots remain in this directory.

## Partial Local

Only jointly passing `128x6792` FP32 was measured. This width is outside the V063 threshold and follows the unchanged control path; it is not evidence of threshold-path benefit. Device 4 used device events, warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, and six interleaved P/C pairs in alternating order. All 12 invocations returned `rc=0,bad=0`; each side retained 372 raw device-event samples. No sample or pair was removed.

For each pair, compute Parent and Candidate medians from all 62 raw `device_us` values, then `pair_speedup = Parent_median / Candidate_median`. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Pair-level medians, MADs, deltas, and MAD-envelope flags are in `v063-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.998072450490x`.
- `PARTIAL_ROUTE_LOCAL_DELTA=-0.1927549510%`.
- Candidate was faster in 3/6 pairs and slower in 3/6. All paired deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw jitter (372 samples/side): Parent median `20.538 us`, MAD `1.52785 us`, p10/p90 `18.2622/24.8216 us`, CV `14.2900728%`; Candidate median `20.7014 us`, MAD `1.8064 us`, p10/p90 `17.3903/24.9972 us`, CV `16.0367313%`.
- Device 4 load: AICore `61% -> 58%`; HBM `59192 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was untouched.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`.
- `CURRENT_LOCAL_BEST=NONE`. Do not promote this partial/control-only result. Continue threshold OFAT from exact R31B-V011.

## Result Flags

- `COMPILE=PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passes all three widths; Candidate fails both changed-path widths and passes only the unchanged control.
- `LOCAL_SCORE=0.998072450490x` (partial, control-only, noisy; not an Official-comparable score).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; no Candidate Local Best is established.
