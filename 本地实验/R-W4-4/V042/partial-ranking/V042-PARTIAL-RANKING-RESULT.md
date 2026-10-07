# R-W4-4 V042 threshold endpoint/control result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V042`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3968 -> 4096`, restoring the exact Parent threshold.
- SOURCE_IDENTITY: Candidate source is byte-identical to Parent; this is an endpoint/control measurement, not a distinct Candidate revision or source-level performance gain.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V042 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial endpoint measurement.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. The probe emitted existing non-fatal attribute/format warnings; all targets returned `rc=0`.

Parent and Candidate both returned `rc=0`, `bad=0` on device 4 for the tested FP32 widths. Both source hashes are identical.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x4088` | `2.14577e-06` | `2.14577e-06` |
| `128x4096` | `2.86102e-06` | `2.86102e-06` |
| `128x4104` | `2.38419e-06` | `2.38419e-06` |

Correctness run metadata, stdout/stderr, stats, raw event TSVs, binary hashes, and pre/post device/process snapshots are retained alongside this note.

## Local endpoint/control measurement

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 sample values remain in raw TSV files. The Candidate and Parent sources are identical, so pair differences represent measurement variability, not an implementation effect.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. The numeric partial route-local endpoint score is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`. This numeric receipt is retained for the threshold sweep but is not attributed to a Candidate source change.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x4088` | `1.040746, 1.010164, 1.084116, 1.085399, 0.981172, 0.952448` | `1.025674218426x` | `10.704850 / 10.411550` | `8.943% / 39.735%` | `4/6; 6/6` |
| `128x4096` | `1.013167, 0.999225, 0.933266, 0.980617, 1.013663, 0.955099` | `0.982506216062x` | `8.971095 / 9.151875` | `9.899% / 9.222%` | `2/6; 6/6` |
| `128x4104` | `0.991343, 0.933546, 1.042614, 0.998925, 1.030874, 1.095025` | `1.015387782100x` | `10.752200 / 10.635950` | `10.566% / 8.776%` | `3/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `1.007686777013x` (noisy endpoint/control statistic only).
- PARTIAL_ROUTE_LOCAL_DELTA: `+0.768677701%` (not attributable to source change).
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES` for an attributable Candidate comparison; this endpoint does not establish a Local Best.
- All 18 paired deltas were within their corresponding Parent-plus-Candidate MAD sums. Direction is mixed despite source identity; no sample was removed.
- Pooled values use all 372 `device_us` samples per side and shape. The retained Candidate `128x4088` max is `67.702500 us` (CV `39.735%`); it remains included.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and latency jitter are context, not admission criteria.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{4088,4096,4104}-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v042-local-run-index.tsv` and `v042-interleaved-local.log`; correctness and build records are in the corresponding `v042-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v042/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` for the three tested widths; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `1.007686777013x` (partial endpoint/control statistic only).
- LOCAL_DELTA: `+0.768677701%` (no source-level gain claim).
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V042 is source-identical to Parent and is not promoted.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
