# R-W4-4 V043 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V043`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `3d071857a9c345a555e08d10d44fb5c86d36da90decd3753ba717cc1b9259de7`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 4224`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and distinct from the V043 Candidate-only failures at 4216 and 4224.
- PARTIAL_CORRECTNESS: `YES`; of tested widths 4216/4224/4232, both sides pass only at 4232.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. The probe emitted existing non-fatal attribute/format warnings; all compile targets returned `rc=0`.

Correctness ran on device 4 with exact Parent R31B-V011. Parent passed all three widths (`rc=0`, `bad=0`). Candidate failed at 4216 and 4224 (`rc=3`) and passed at 4232 (`rc=0`, `bad=0`). The failures are retained as a Candidate regression at those widths, not attributed to the known Parent C15 failure.

| Shape | Parent result | Candidate result | Parent max_abs | Candidate max_abs |
|---|---|---|---:|---:|
| `128x4216` | PASS, `bad=0` | FAIL, `bad=524613` | `2.6226e-06` | `4.92167` |
| `128x4224` | PASS, `bad=0` | FAIL, `bad=505279` | `2.38419e-06` | `4.50644` |
| `128x4232` | PASS, `bad=0` | PASS, `bad=0` | `2.6226e-06` | `2.6226e-06` |

Local was not run on 4216 or 4224 because Candidate correctness failed there. It was run only on 4232, where both sides passed.

## Local ranking on the sole jointly passing shape

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs at 128x4232. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 744 samples remain in raw TSV files.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. The only jointly passing shape score is the arithmetic mean of its six pair speedups. Since only one of the three planned shapes is jointly correctness-passing, this is a one-shape partial score, not a full three-shape route score.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x4232` | `0.985301, 1.099083, 0.991349, 0.975835, 1.032860, 0.971663` | `1.009348519148x` | `11.177050 / 11.107950` | `8.054% / 15.996%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `1.009348519148x` for the sole eligible shape only; no three-shape aggregate is claimed.
- PARTIAL_ROUTE_LOCAL_DELTA: `+0.934851915%` for that shape only.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`; all six paired deltas were within their P/C MAD sums and direction was mixed. All numeric data is retained without filtering.
- Pooled ranges include Candidate max `29.808400 us` (CV `15.996%`); it remains in the sample set.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and jitter are context, not admission criteria.

Correctness files preserve stdout/stderr, stats and raw event TSVs for all six runs. Local raw files are `local-pairNN-{parent,candidate}-r128-w4232-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v043-local-run-index.tsv` and `v043-interleaved-local.log`; build and correctness records are in corresponding `v043-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v043/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PARTIAL`; Parent passes all three tested widths, Candidate passes 4232 and fails 4216/4224.
- LOCAL_SCORE: `1.009348519148x` for 128x4232 only.
- LOCAL_DELTA: `+0.934851915%` for 128x4232 only.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V043 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is neither an Official score nor a full Candidate ranking.
