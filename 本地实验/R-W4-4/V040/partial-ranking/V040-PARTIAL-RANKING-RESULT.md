# R-W4-4 V040 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V040`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `e06436370b797fe9d26c52354f328b1023e4dfee2192eac1880efe7f23d6c4ee`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3584 -> 3840`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V040 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial ranking.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. The probe emitted existing non-fatal attribute/format warnings; all targets returned `rc=0`.

Parent and Candidate both returned `rc=0`, `bad=0` on device 4 for all three FP32 widths.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x3832` | `2.14577e-06` | `2.14577e-06` |
| `128x3840` | `2.14577e-06` | `2.14577e-06` |
| `128x3848` | `1.90735e-06` | `1.90735e-06` |

Correctness run metadata, stdout/stderr, stats, raw event TSVs, binary hashes, and pre/post device/process snapshots are retained alongside this note.

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 sample values remain in raw TSV files. Every correctness-passing width was included; load and stability are context, not admission gates.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x3832` | `0.951677, 0.946623, 0.974869, 0.954687, 0.987702, 1.033782` | `0.974890119269x` | `10.096550 / 10.306400` | `12.451% / 11.807%` | `1/6; 6/6` |
| `128x3840` | `1.000018, 1.071803, 1.025443, 0.966552, 0.952942, 1.006949` | `1.003951091702x` | `9.177970 / 9.224375` | `9.393% / 10.529%` | `4/6; 6/6` |
| `128x3848` | `0.996059, 1.057331, 0.974163, 0.999780, 1.026422, 0.948402` | `1.000359438960x` | `9.212810 / 9.221565` | `9.063% / 10.601%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.992982130565x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-0.701786944%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas were within their corresponding Parent-plus-Candidate MAD sums. Direction is mixed and aggregate is a small negative relative to observed jitter. No sample was removed.
- Pooled values use all 372 `device_us` samples per side and shape. Retained ranges include Candidate `128x3832` max `17.565600 us` and Candidate `128x3840` max `14.815000 us`; all tails remain included.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and latency jitter are context, not admission criteria.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{3832,3840,3848}-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v040-local-run-index.tsv` and `v040-interleaved-local.log`; correctness and build records are in the corresponding `v040-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v040/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` for the three tested widths; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `0.992982130565x` (partial route-local score only).
- LOCAL_DELTA: `-0.701786944%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V040 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
