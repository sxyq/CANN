# R-W4-4 V035 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V035`
- DIRECT_PARENT: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `f9c24130799116c202f6f4c499b314ba65c320d7e7f5d517a24699875eb104ce`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 512`; no other source delta.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for the known exact route Parent C15 FP32 `1x32768` failure. C15 was not rerun and is not a V035 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The local `device` and `submission` targets passed, as did the isolated Parent/Candidate probe build. Logs are `../v035-build.log` and `probe-build.log`; source and probe identities are in `identity.sha256`.

Parent and Candidate both passed on local host `hwnput3`, device 4, FP32. Each call returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x504` | `1.66893e-06` | `1.66893e-06` |
| `128x512` | `1.66893e-06` | `1.66893e-06` |
| `128x520` | `1.19209e-06` | `1.19209e-06` |

## Local ranking

The retained device-event run used warmup 45, 31 samples per block, two blocks, batch 64, and six interleaved Parent/Candidate pairs per shape. All 36 paired invocations returned `rc=0`, `bad=0`; every pair-side raw file contains 62 device-event samples. No sample was removed.

For each pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape aggregate is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean paired speedup | Candidate faster/slower | Within P/C MAD sum |
|---|---:|---:|---:|
| `128x504` | `0.986135020591x` (`-1.386497941%`) | `3 / 3` | `6 / 6` |
| `128x512` | `0.984706353315x` (`-1.529364668%`) | `2 / 4` | `6 / 6` |
| `128x520` | `0.967030518350x` (`-3.296948165%`) | `1 / 5` | `6 / 6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.979251925323x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-2.074807468%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- Combined Parent/Candidate CVs were `14.211% / 13.594%` (`128x504`), `15.442% / 14.196%` (`128x512`), and `15.542% / 16.017%` (`128x520`). Every pair delta fell within its measured P/C MAD sum and direction was mixed or Parent-favoring.
- An independent Parent-only same-binary qualification for these exact shapes is not present in the retained V035 evidence. The numeric score is retained as diagnostic partial ranking, not as a qualification-complete result or Local Best.

Device 4 HBM was `64888/65536 MB` before and `64887/65536 MB` after; AICore was `13%` before and `16%` after. Existing processes, including VLLM PID `2999855`, were left untouched. The snapshots and all raw files remain in this directory.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three tested partial shapes; `PARTIAL_CORRECTNESS=YES` because C15 was not rerun.
- LOCAL_SCORE: `0.979251925323x` (partial route-local speedup geomean only).
- LOCAL_DELTA: `-2.074807468%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`; standalone exact-shape Parent qualification is missing.
- CURRENT_LOCAL_BEST: `NONE`; V035 is not promoted.
- NEXT_PARENT: use exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFFICIAL_COMPARISON: `NO`; this partial result is not an Official score or candidate.
