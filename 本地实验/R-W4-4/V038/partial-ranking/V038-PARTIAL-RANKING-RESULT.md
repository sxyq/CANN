# R-W4-4 V038 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V038`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `2d7f27e684d1a07a35a7a0ec7178a4589dbf3acf975ac77dd9a5a3994c00bdea`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 3072`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V038 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial ranking.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Compile and correctness

On local host `hwnput3`, route `device` and `submission` compile targets passed, as did the isolated Parent/Candidate probe targets. Probe build warnings were non-fatal; all targets returned `rc=0`.

Parent and Candidate both returned `rc=0`, `bad=0` on device 4 for all three FP32 widths.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x3064` | `2.86102e-06` | `2.86102e-06` |
| `128x3072` | `2.86102e-06` | `2.86102e-06` |
| `128x3080` | `3.09944e-06` | `3.09944e-06` |

Correctness run metadata, stdout/stderr, stats, raw event TSVs, binary hashes, and pre/post device/process snapshots are retained alongside this note.

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 sample values remain in raw TSV files. Every correctness-passing width was included; load and stability are context, not admission gates.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x3064` | `0.918835, 0.938257, 0.964339, 0.916845, 0.962277, 0.968870` | `0.944903838308x` | `9.494685 / 10.069050` | `8.787% / 11.511%` | `0/6; 6/6` |
| `128x3072` | `1.049067, 0.976196, 0.971233, 1.042189, 0.960905, 0.968295` | `0.994647333146x` | `9.000780 / 9.007500` | `10.589% / 44.585%` | `2/6; 6/6` |
| `128x3080` | `1.085830, 0.993500, 0.994841, 0.880778, 1.010598, 0.980505` | `0.991008752555x` | `9.074375 / 9.330470` | `49.999% / 16.977%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.976588056465x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-2.341194354%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas were within their corresponding Parent-plus-Candidate MAD sums. Pair direction was mixed on two widths and Parent-favoring on `128x3064`; the measured aggregate is negative. No sample was removed.
- Pooled ranges expose retained tails: Candidate `128x3072` max `79.602800 us` (CV `44.585%`); Parent `128x3080` max `75.639100 us` (CV `49.999%`).

NPU4 pre/post Local snapshots show AICore `0%`, HBM `59192/65536 MB` then `59194/65536 MB`, and the same existing VLLM PID `2999855` using `55666 MB`; it was left untouched. The 6+ GB resident HBM use and observed latency tails are recorded as context. No process was stopped or modified.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{3064,3072,3080}-fp32-{raw.tsv,stats.txt,stdout.log,stderr.log}`. Invocation order and return codes are in `v038-local-run-index.tsv` and `v038-interleaved-local.log`; correctness and environment/build records are in the corresponding `v038-*.log` files. Generated `build-cmake/` and `partial-ranking/stage/build-v038/` directories are not part of the evidence commit.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` for the three tested widths; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `0.976588056465x` (partial route-local score only).
- LOCAL_DELTA: `-2.341194354%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V038 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
