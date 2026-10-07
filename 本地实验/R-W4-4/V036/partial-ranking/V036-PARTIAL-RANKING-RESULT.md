# R-W4-4 V036 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V036`.
- DIRECT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `f4b3623b4a8cb86a43328f5dbc15fe6826ab8f13cbcc88745d0e23aa9da2b49f`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 1024`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V036 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; C15 is excluded from this partial ranking.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Correctness

On local host `hwnput3`, device 4, Parent and Candidate both returned `rc=0`, `bad=0` for the three tested FP32 shapes.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x1016` | `1.19209e-06` | `1.19209e-06` |
| `128x1024` | `1.66893e-06` | `1.66893e-06` |
| `128x1032` | `1.43051e-06` | `1.43051e-06` |

## Local ranking

Local used fixed device 4, FP32, warmup 45, 31 samples per block, two blocks, gap 0, batch 64, and six interleaved Parent/Candidate pairs per shape. Odd pairs ran Parent/Candidate; even pairs ran Candidate/Parent. All 36 invocations returned `rc=0`, `bad=0`. Each invocation retained 62 device-event samples; all 2,232 event values remain in the raw TSVs. No width or sample was omitted.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape score is the arithmetic mean of its six pair speedups. `PARTIAL_ROUTE_LOCAL_SCORE` is the equal-weight geometric mean of the three shape scores; `delta_pct = (score - 1) * 100`.

| Shape | Pair speedups 1-6 | Mean pair speedup | P/C pooled median us | P/C pooled CV | Candidate faster; within P/C MAD sum |
|---|---|---:|---:|---:|---:|
| `128x1016` | `1.014119, 0.902318, 1.017341, 0.968774, 0.991485, 0.973254` | `0.977881794338x` | `8.534220 / 8.707970` | `9.504% / 11.610%` | `2/6; 6/6` |
| `128x1024` | `0.953403, 0.981028, 0.965412, 0.972629, 0.964461, 1.016636` | `0.975594752733x` | `8.236875 / 8.452655` | `11.864% / 12.194%` | `1/6; 6/6` |
| `128x1032` | `0.945915, 1.002202, 0.967659, 0.967174, 1.023481, 0.932348` | `0.973129830363x` | `8.235315 / 8.481875` | `10.728% / 10.321%` | `2/6; 6/6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.975533529168x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-2.446647083%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- Every shape's six paired deltas were within the corresponding Parent-plus-Candidate MAD sum; paired direction was mixed and Parent-favoring overall. These are numeric results, not a Local admission gate.

Parent-only same-binary stability is context, not a Local gate. `128x1016` failed block stability: block medians were `8.82 / 16.14 us`, with device CV `1.64646 / 1.46764` and max/min `15.3518 / 33.2847`. It was still included in all six P/C pairs. `128x1024` block medians were `7.50 / 7.72 us`; `128x1032` were `8.94 / 8.84 us`; both had extreme same-binary CV/outlier ranges (`ALL_DEVICE CV / max_min`: `2.10689 / 40.687` and `2.16519 / 47.1546`, respectively). No same-binary result was used to suppress a correctness-passing shape.

NPU4 pre/post snapshots showed AICore `0%`, HBM `59193/65536 MB` then `59195/65536 MB`, and the same existing VLLM process PID `2999855` using `55666 MB`. That process was left untouched. The high resident load and observed jitter are retained as quality context, not an admission condition.

Raw runs are `local-pairNN-{parent,candidate}-r128-w{1016,1024,1032}-fp32-{raw.tsv,stats.txt,stdout.log}`. Run parameters/index and pre/post snapshots are in `local-run-manifest.txt`, `local-run-index.tsv`, `local-pre-npu-smi.txt`, and `local-post-npu-smi.txt`. The first setup attempt exited `rc=1` before launching a measurement because `set_env.sh` encountered `LD_LIBRARY_PATH: unbound variable` under shell nounset; its output is preserved in `local-env.log`. The successful setup output is `local-env-retry1.log`.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three tested shapes; partial because exact Parent C15 remains a known failure.
- LOCAL_SCORE: `0.975533529168x` (partial route-local score only).
- LOCAL_DELTA: `-2.446647083%`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: unchanged; V036 is not promoted.
- NEXT_PARENT: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
