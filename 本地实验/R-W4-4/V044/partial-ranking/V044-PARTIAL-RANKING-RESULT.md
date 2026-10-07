# R-W4-4 V044 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V044`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `370cce9ee13c13ac793de74ca936ddfa177a259383c12e7a395c34fdfe7251a1`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4224 -> 4352`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and distinct from V044 Candidate-only failures at 4344 and 4352.
- PARTIAL_CORRECTNESS: `YES`; of tested widths 4344/4352/4360, both sides pass only at 4360.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. Probe warnings were non-fatal; all compile targets returned `rc=0`.

Correctness ran on device 4 with exact Parent R31B-V011. Parent passed all three widths (`rc=0`, `bad=0`). Candidate failed at 4344 and 4352 (`rc=3`) and passed at 4360 (`rc=0`, `bad=0`). These are Candidate-only failures, distinct from the known exact-Parent C15 failure.

| Shape | Parent result | Candidate result | Parent max_abs | Candidate max_abs |
|---|---|---|---:|---:|
| `128x4344` | PASS, `bad=0` | FAIL, `bad=546642` | `2.86102e-06` | `4.57812` |
| `128x4352` | PASS, `bad=0` | FAIL, `bad=551155` | `2.6226e-06` | `4.84161` |
| `128x4360` | PASS, `bad=0` | PASS, `bad=0` | `3.09944e-06` | `3.09944e-06` |

Local was not run on 4344 or 4352 because Candidate correctness failed there. It was run only on the jointly passing 4360 shape.

## Local ranking on the sole jointly passing shape

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs at 128x4360. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 744 samples remain in raw TSV files.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. The sole jointly passing shape score is the arithmetic mean of its six pair speedups. This one-shape partial score is not a full three-shape route score.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x4360` | `0.969175, 1.030878, 0.972272, 0.941567, 0.995419, 1.004387` | `0.985616398160x` | `10.765150 / 10.975150` | `9.013% / 15.226%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.985616398160x` for the sole eligible shape only; no three-shape aggregate is claimed.
- PARTIAL_ROUTE_LOCAL_DELTA: `-1.438360184%` for that shape only.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`; all six paired deltas were within their P/C MAD sums and direction was mixed. All numeric data is retained without filtering.
- Pooled data includes Candidate max `37.229400 us` (CV `15.226%`); it remains in the raw set and score.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and jitter are context, not admission criteria.

Correctness outputs, stats, and raw files are retained for every Parent/Candidate shape. Local raw files are `local-pairNN-{parent,candidate}-r128-w4360-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v044-local-run-index.tsv` and `v044-interleaved-local.log`; build/correctness records are in corresponding `v044-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v044/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PARTIAL`; Parent passes all tested widths, Candidate passes 4360 and fails 4344/4352.
- LOCAL_SCORE: `0.985616398160x` for 128x4360 only.
- LOCAL_DELTA: `-1.438360184%` for 128x4360 only.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V044 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is neither an Official score nor a full Candidate ranking.
