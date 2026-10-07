# R-W4-4 V041 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V041`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `bb051a75e2aee04938473de4cfab35dcdfc89769dd6e7d85c12563b744ea4a47`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3840 -> 3968`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V041 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial ranking.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. The probe emitted existing non-fatal attribute/format warnings; all targets returned `rc=0`.

Parent and Candidate both returned `rc=0`, `bad=0` on device 4 for all three FP32 widths.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x3960` | `2.38419e-06` | `2.38419e-06` |
| `128x3968` | `2.14577e-06` | `2.14577e-06` |
| `128x3976` | `2.38419e-06` | `2.38419e-06` |

Correctness run metadata, stdout/stderr, stats, raw event TSVs, binary hashes, and pre/post device/process snapshots are retained alongside this note.

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 sample values remain in raw TSV files. Every correctness-passing width was included; load and stability are context, not admission gates.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x3960` | `0.894385, 0.904249, 0.949048, 0.991445, 0.986663, 0.942548` | `0.944722964457x` | `9.897815 / 10.471100` | `9.186% / 10.964%` | `0/6; 6/6` |
| `128x3968` | `1.027151, 0.956542, 0.906612, 0.992558, 0.940722, 0.956795` | `0.963396737548x` | `8.950155 / 9.272970` | `9.170% / 51.045%` | `1/6; 6/6` |
| `128x3976` | `0.987171, 0.988067, 0.985686, 1.094226, 0.978625, 1.034708` | `1.011413761621x` | `9.294530 / 9.262030` | `27.305% / 16.325%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.972775970331x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-2.722402967%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas were within their corresponding Parent-plus-Candidate MAD sums. Direction was Parent-favoring at widths 3960 and 3968; the pooled three-shape score is negative. No sample was removed.
- Pooled values use all 372 `device_us` samples per side and shape. Retained tails include Candidate `128x3968` max `101.447000 us` (CV `51.045%`) and Parent `128x3976` max `53.015900 us` (CV `27.305%`); all remain included.

NPU4 pre/post Local snapshots and full process lists are preserved. Existing processes were left untouched. Device load and latency jitter are context, not admission criteria.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{3960,3968,3976}-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v041-local-run-index.tsv` and `v041-interleaved-local.log`; correctness and build records are in the corresponding `v041-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v041/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` for the three tested widths; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `0.972775970331x` (partial route-local score only).
- LOCAL_DELTA: `-2.722402967%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V041 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
