# R-W4-4 V045 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V045`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `01ce40919026ab8a312fcf6e2a5aa6a4d1d44b3dfcace36a4fe00afa6aaa14f1`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4352 -> 4480`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and distinct from V045 Candidate-only failures at 4472 and 4480.
- PARTIAL_CORRECTNESS: `YES`; of tested widths 4472/4480/4488, both sides pass only at 4488.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. Probe warnings were non-fatal; all compile targets returned `rc=0`.

Correctness ran on device 4 with exact Parent R31B-V011. Parent passed all three widths (`rc=0`, `bad=0`). Candidate failed at 4472 and 4480 (`rc=3`) and passed at 4488 (`rc=0`, `bad=0`). These Candidate-only failures are distinct from the known exact-Parent C15 failure.

| Shape | Parent result | Candidate result | Parent max_abs | Candidate max_abs |
|---|---|---|---:|---:|
| `128x4472` | PASS, `bad=0` | FAIL, `bad=569420` | `3.09944e-06` | `4.88604` |
| `128x4480` | PASS, `bad=0` | FAIL, `bad=570073` | `2.86102e-06` | `4.86521` |
| `128x4488` | PASS, `bad=0` | PASS, `bad=0` | `2.86102e-06` | `2.86102e-06` |

Local was not run on 4472 or 4480 because Candidate correctness failed there. It was run only on the jointly passing 4488 shape.

## Local ranking on the sole jointly passing shape

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs at 128x4488. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 744 samples remain in raw TSV files.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. The sole jointly passing shape score is the arithmetic mean of its six pair speedups. This one-shape partial score is not a full three-shape route score.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x4488` | `0.915362, 0.976787, 0.992267, 0.994523, 0.989503, 1.000876` | `0.978219664940x` | `10.869850 / 11.018600` | `77.259% / 10.812%` | `1/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.978219664940x` for the sole eligible shape only; no three-shape aggregate is claimed.
- PARTIAL_ROUTE_LOCAL_DELTA: `-2.178033506%` for that shape only.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`; all six paired deltas were within their P/C MAD sums and only one pair favored Candidate. All numeric data is retained without filtering.
- Pooled data includes Parent max `169.671000 us` (CV `77.259%`); it is retained in the raw sample set and summaries.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and jitter are context, not admission criteria.

Correctness outputs, stats, and raw files are retained for all Parent/Candidate shapes. Local raw files are `local-pairNN-{parent,candidate}-r128-w4488-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v045-local-run-index.tsv` and `v045-interleaved-local.log`; build/correctness records are in corresponding `v045-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v045/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PARTIAL`; Parent passes all tested widths, Candidate passes 4488 and fails 4472/4480.
- LOCAL_SCORE: `0.978219664940x` for 128x4488 only.
- LOCAL_DELTA: `-2.178033506%` for 128x4488 only.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V045 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is neither an Official score nor a full Candidate ranking.
