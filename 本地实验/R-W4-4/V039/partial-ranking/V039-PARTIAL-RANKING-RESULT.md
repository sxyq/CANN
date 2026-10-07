# R-W4-4 V039 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V039`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `453f7acd16f48d5cc53879466883a09b8b4ee42daaa5a5415d3eab649fd08e8d`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3072 -> 3584`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V039 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial ranking.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. The probe emitted existing non-fatal attribute/format warnings; all targets returned `rc=0`.

Parent and Candidate both returned `rc=0`, `bad=0` on device 4 for all three FP32 widths.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x3576` | `1.66893e-06` | `1.66893e-06` |
| `128x3584` | `1.90735e-06` | `1.90735e-06` |
| `128x3592` | `1.66893e-06` | `1.66893e-06` |

Correctness run metadata, stdout/stderr, stats, raw event TSVs, binary hashes, and pre/post device/process snapshots are retained alongside this note.

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 sample values remain in raw TSV files. Every correctness-passing width was included; load and stability are context, not admission gates.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x3576` | `1.031363, 1.029063, 1.069832, 0.988313, 0.984671, 0.941685` | `1.007487635195x` | `10.073300 / 10.020800` | `7.826% / 13.390%` | `3/6; 6/6` |
| `128x3584` | `0.953688, 0.939518, 1.007277, 0.990597, 0.991351, 1.004976` | `0.981234417742x` | `9.044840 / 9.199685` | `38.823% / 10.381%` | `2/6; 6/6` |
| `128x3592` | `1.008066, 1.015320, 0.985321, 1.016321, 0.955080, 1.035631` | `1.002622842978x` | `9.319530 / 9.301405` | `45.852% / 10.232%` | `4/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.997049448562x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-0.295055144%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas were within their corresponding Parent-plus-Candidate MAD sums. Direction is mixed; the aggregate is a small negative and remains inside observed pair noise. No sample was removed.
- Retained pooled tails include Parent `128x3584` max `74.674100 us` (CV `38.823%`) and Parent `128x3592` max `69.107200 us` (CV `45.852%`). Candidate `128x3576` max was `28.260900 us`; all tails remain in the numeric score and raw data.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and latency jitter are context, not admission criteria.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{3576,3584,3592}-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v039-local-run-index.tsv` and `v039-interleaved-local.log`; correctness and build records are in the corresponding `v039-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v039/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` for the three tested widths; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `0.997049448562x` (partial route-local score only).
- LOCAL_DELTA: `-0.295055144%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V039 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
