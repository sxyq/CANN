# R-W4-4 V037 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V037`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `b165a8d60e03ad620dc8129b97b433a2a18970b03ca43b539a0b19c2cb5e9f3c`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 2048`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; it was not rerun and is not a V037 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 remains excluded from this partial result.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The route `device` and `submission` targets passed. The isolated Parent/Candidate correctness probe targets also passed. Retained build failures include a stale copied CMake cache (`rc=1`) and an initial wrong target selection that referenced the absent `runner_main.inc` (`rc=2`); a clean build directory and the existing `ref_*` runner targets passed (`rc=0`). No Candidate source fix beyond the single threshold change was made.

On local host `hwnput3`, device 4, Parent and Candidate both returned `rc=0`, `bad=0` for all tested FP32 shapes.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x2040` | `2.14577e-06` | `2.14577e-06` |
| `128x2048` | `2.14577e-06` | `2.14577e-06` |
| `128x2056` | `2.38419e-06` | `2.38419e-06` |

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 event values remain in raw TSV files. All correctness-passing shapes were included regardless of load or same-binary stability; no separate same-binary qualification was used as an admission gate.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. `PARTIAL_ROUTE_LOCAL_SCORE` is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x2040` | `0.922751, 0.953797, 1.056194, 1.009971, 1.003836, 1.021245` | `0.994632255780x` | `9.210315 / 9.296405` | `12.559% / 35.151%` | `4/6; 6/6` |
| `128x2048` | `0.923767, 0.991116, 1.050957, 1.000727, 0.986719, 1.006954` | `0.993373555945x` | `8.661245 / 8.720155` | `12.911% / 13.005%` | `3/6; 6/6` |
| `128x2056` | `0.989236, 1.060399, 1.052657, 1.031121, 0.961954, 1.000632` | `1.015999936613x` | `8.914685 / 8.749375` | `14.543% / 16.958%` | `4/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `1.001281683334x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `+0.128168333%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 pair deltas were within the corresponding Parent-plus-Candidate MAD sum. The aggregate gain is small relative to observed jitter; the numeric result is retained without filtering.
- The `128x2040` Candidate pooled samples include a `56.1203 us` maximum (`7.768x` min-to-max range), reflected in its `35.151%` CV. This tail value is retained, not removed.

NPU4 pre/post snapshots show HBM `59192/65536 MB` then `59195/65536 MB`; existing VLLM PID `2999855` remained at `55666 MB` and was untouched. The device snapshots report AICore `0%`. Load is recorded as context, not an admission gate.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{2040,2048,2056}-fp32-{raw.tsv,stats.txt,stdout.log}`. Run parameters and order are in `v037-local-run-manifest.txt`, `v037-local-run-index.tsv`, and `v037-interleaved-local.log`; snapshots are `v037-local-pre-npu-smi.txt` and `v037-local-post-npu-smi.txt`. Correctness files use `parent-correctness-*` and `candidate-correctness-*`. All failed build attempts remain in `v037-probe-build*.log` and `v037-route-compile.log`.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three tested shapes; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `1.001281683334x` (partial route-local score only).
- LOCAL_DELTA: `+0.128168333%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: unchanged; V037 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
