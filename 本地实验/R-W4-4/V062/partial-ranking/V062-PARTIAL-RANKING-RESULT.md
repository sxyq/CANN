# R-W4-4 V062 Partial Ranking Result

- `REVISION=V062`; `DIRECT_PARENT=R31B-V011` exact source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `7a602ba0555b7d9b117bb72c55a75b5480d31618defb30f37f3b100fb32ce18d`.
- Single change: `kSmallFp32BatchMaxWidth`, `4096 -> 6656`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route `device`/`submission` compiled successfully (`ROUTE_BUILD_RC=0`). Isolated exact-Parent and Candidate correctness probe targets also compiled (`PROBE_BUILD_RC=0`). The first probe-build log capture hit a missing output-directory error in `tee`; the underlying build completed PASS. A captured repeat records that event and confirms both probe targets PASS in `probe-build.log`.

On local host `hwnput3`, device 4, exact R31B-V011 Parent passed all three FP32 widths. Candidate failed in the changed threshold path and passed only the outside control:

| Shape | Parent | Candidate | Candidate bad / max_abs | Local |
|---|---|---|---:|---|
| `128x6648` | PASS, `rc=0`, max_abs `4.29153e-06` | FAIL, `rc=3` | `850132 / 4.71808` | not run |
| `128x6656` | PASS, `rc=0`, max_abs `3.8147e-06` | FAIL, `rc=3` | `851150 / 4.80237` | not run |
| `128x6664` | PASS, `rc=0`, max_abs `3.57628e-06` | PASS, `rc=0` | `0 / 3.57628e-06` | control-only Local below |

C15 was excluded. Correctness stdout/stderr, raw event files, stats, identity, and pre/post device/process snapshots are retained in this directory.

## Partial Local

Only the jointly passing `128x6664` FP32 shape was measured. This is outside the V062 cutoff and takes the unchanged control path; the result is not evidence for threshold-path benefit. Device 4 used device events, warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, and six interleaved P/C pairs in alternating order. All 12 invocations returned `rc=0,bad=0`; each side retained 372 raw device-event samples. No sample or pair was excluded.

For each pair, compute the Parent and Candidate medians from all 62 raw `device_us` values, then `pair_speedup = Parent_median / Candidate_median`. The one-shape partial score is the arithmetic mean of all six pair speedups; `delta_pct = (score - 1) * 100`. Pair medians, MADs, deltas, and MAD-envelope flags are in `v062-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.996157857718x`.
- `PARTIAL_ROUTE_LOCAL_DELTA=-0.3842142282%`.
- Candidate was faster in 4/6 pairs; slower in 2/6. All paired deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw jitter (372 samples/side): Parent median `20.59125 us`, MAD `1.69765 us`, p10/p90 `17.8022/25.0919 us`, CV `44.2493270%`; Candidate median `20.6292 us`, MAD `1.477 us`, p10/p90 `17.4231/23.9356 us`, CV `14.4988783%`.
- Device 4 load: AICore `57% -> 59%`; HBM `59192 -> 59194 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was untouched.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`.
- `CURRENT_LOCAL_BEST=NONE`. Do not promote this partial/control-only result. Continue threshold OFAT from exact R31B-V011.

## Result Flags

- `COMPILE=PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passes all three widths; Candidate fails both changed-path widths and passes only the unchanged control.
- `LOCAL_SCORE=0.996157857718x` (partial, control-only, noisy; not an Official-comparable score).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; no Candidate Local Best is established.
