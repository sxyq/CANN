# MODE V074 Finite Retest

## Identity and Dispatch

- Route branch: `routes/r-w4-4-mode-dispatch-cutoff-x`; worktree HEAD at retest start: `2d73f04c66376b1aced12937d3b23d8590ef8a79`.
- Candidate revision commit: `843f6cee1ebf0caca9ff4539bef51e1c1273112c`.
- Exact Candidate path: `本地实验/R-W4-4/V074/submission.asc`; SHA256: `d3a471ccd4e82bb1244ecb0b991f693d52e5e674cd2def36a19f4b62a34d8592`.
- Exact comparison Parent: `R31B-V011`; source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 8192`; Candidate and Parent probe inputs were identity-checked in `../v074-correctness-identities.sha256` and the staged source files were checked byte-for-byte before compile.
- For FP32 width 8192, the earlier contiguous branch is limited to width `<=2048`; the aligned width 8192 with multiple local rows reaches the changed batch branch (`width <= 8192`, stride 8). No source algorithm edit was made.

## Compile and Correctness

- Reference-harness Parent and Candidate probe targets: `PASS`, built from the existing V074 `stage/CMakeLists.txt` and runner sources.
- Initial compile failure (`<vector>` not found) is retained in `compile.log`; the known V074/V029-V035 `CPLUS_INCLUDE_PATH` fix was applied once, and `compile-retry1.log` records the successful rebuild. `configure.log` records the configure step.
- Loader-only attempts are retained: first missing `libgraph.so`, then HCC `libstdc++.so.6` missing `GLIBCXX_3.4.29`. Reusing V074's recorded `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH` resolved the loader. No additional loader retry was made.
- Correctness scope: FP32 `128x8184`, `128x8192`, `128x8200`; device 4. Final calls are in `correctness-retry2-events.tsv` and their stdout/stderr files.

| Shape | Parent | Candidate | Local |
|---|---|---|---|
| `128x8184` | PASS, `bad=0` | FAIL, `bad=1046887` | Not run |
| `128x8192` | PASS, `bad=0` | PASS, `bad=0` | Measured below |
| `128x8200` | FAIL, `bad=377793` | FAIL, `bad=489217` | Not run |

`128x8200` is a Parent failure as well as a Candidate failure; it is not evidence of a Candidate-only regression. The known exact-Parent C15 failure remains excluded and was not rerun. Therefore `PARTIAL_CORRECTNESS=YES`; only `128x8192` is jointly correct in this retest.

## Local Result

- Host `hwnput3`, fixed device 4, FP32 `128x8192`; six interleaved pairs, order `P,C; C,P; P,C; C,P; P,C; C,P`.
- Per invocation: warmup 45, 31 samples/block, 2 blocks, batch 64. Each side has 372 device-event samples across six invocations; 744 total. Every Local invocation returned `rc=0,bad=0`.
- Local window: `2026-10-09T10:06:21Z` to `2026-10-09T10:07:48Z`. Start/end device snapshots and per-call events are retained in `pre-local-npu-smi.txt`, `post-local-npu-smi.txt`, `pre-local-processes.txt`, `post-local-processes.txt`, `local-events.log`, and `local-events.tsv`.
- Formula reused from the existing V074 result: for each pair, `Parent median(device_us) / Candidate median(device_us)` over that pair's 62 raw samples; score is the arithmetic mean of six pair speedups. `LOCAL_DELTA=(score-1)*100`. This is a route-local partial metric, not an Official-equivalent score or new scorer.

| Pair | Parent median (us) | Candidate median (us) | Pair speedup | MAD comparison |
|---:|---:|---:|---:|---|
| 01 | 21.128400 | 20.738800 | 1.018786043551x | Within combined MAD |
| 02 | 21.170800 | 21.198000 | 0.998716860081x | Within combined MAD |
| 03 | 21.646900 | 21.450300 | 1.009165372978x | Within combined MAD |
| 04 | 21.119400 | 21.083000 | 1.001726509510x | Within combined MAD |
| 05 | 21.299500 | 21.277300 | 1.001043365465x | Within combined MAD |
| 06 | 21.090900 | 21.617000 | 0.975662672896x | Within combined MAD |

- `PARTIAL_ROUTE_LOCAL_SCORE=1.000850137413x`; `PARTIAL_ROUTE_LOCAL_DELTA=+0.085013741%` (speedup direction).
- Candidate is faster in 4/6 pairs; all 6/6 pair median deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw `device_us` diagnostics (372 samples/side): Parent median `21.253600 us`, mean `21.062349 us`, MAD `1.244050 us`, CV `12.776374%`, P10/P90 `18.631590/24.018500 us`, min/max `10.875900/27.753400 us`; Candidate median `21.177500 us`, mean `20.962298 us`, MAD `1.224650 us`, CV `11.730011%`, P10/P90 `18.907900/23.618040 us`, min/max `10.735000/26.177500 us`. Pooled median latency reduction is `0.358056988%`; it is diagnostic and is not the pair-score formula.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`; `CURRENT_LOCAL_BEST=R31B-V011` unchanged. Do not promote V074.
- Device 4 snapshot: pre Local AICore `64%`, HBM `59193/65536 MB`; post Local AICore `56%`, HBM `59195/65536 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) remained present and untouched.

Raw files are `local-pair{01..06}-{parent|candidate}-r128-w8192-fp32-raw.tsv`; paired calculation is in `pair-summary.tsv`; each invocation also has matching `-stats.txt`, `-stdout.log`, and `-stderr.log`. Failed build/loader attempts and the unsupported `npu-smi info proc` query are preserved alongside the successful records.

## Coverage and Exportability

- The retest covers exactly the three listed FP32 shapes, not the Official 15-case set. Exact Official manifest/mapping is unavailable here: `OFFICIAL_15_CASE_COVERAGE=UNKNOWN`. No Official-equivalent score was computed.
- `EXPERIMENTAL_SOURCE_EXPORTABLE=YES`: exact committed V074 source identity is verified and the existing Parent/Candidate reference targets compile successfully. Export only as a clearly labeled experimental artifact.
- `OFFICIAL_SUBMISSION_READY=NO`; `ONLINE_READY=NO`: Candidate correctness fails at `128x8184`, Parent/Candidate both fail at `128x8200`, C15 remains excluded, and full official-case correctness is unverified. The noisy one-shape Local result does not qualify V074 as Local Best.
