# V034 Correctness Blocker

ROUTE: STAGING-LIVENESS-X
REVISION: V034
STAGE: CORRECTNESS
STATUS: BLOCKED — no executable correctness runner in this Route's V034 assets
COMPILE: PASS at 2026-10-06T21:19:30.322406265Z (device/submission object targets; full_link not run)
CANDIDATE_SOURCE_SHA256: 75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33
LOCAL_SCORE: NONE
ONLINE: FORBIDDEN

## Evidence

- `git ls-tree -r --name-only HEAD -- '本地实验/STAGING-LIVENESS-X/V034'` lists only the V034 declaration, build record, CMake file, adapter, device include, and candidate source copies.
- A bounded search of V034 for runner/correctness/local programs, Python files, shared libraries, and JSON artifacts returned no files.
- V034's `CMakeLists.txt` defines object targets and a `full_link` target. `src/adapter.cpp` ends with `int main(){return 0;}` and does not invoke the kernel; the build record confirms `full_link` was not run.
- The environment receipt reports no built canonical runner artifact and no runner source/SHA variables. No exact runner is supplied by V034's own assets.

## Disposition

Correctness was not executed; this is not a correctness pass or failure. Local measurement is consequently not run and no numeric Local score is claimed. Preserve V034 unchanged. Resume at Correctness when an executable runner bound to this exact candidate source SHA (or route-local runner source and build instructions) is available; then proceed directly to Local on assigned device 1 and record load/HBM. Do not wait for device idleness and do not inspect other Routes for runner provenance.

## Bounded route-owned direct-entry recheck (2026-10-07)

V034 candidate source remains frozen at SHA-256 `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`. The bounded checks and exact outputs were:

- `rg --files 本地实验/STAGING-LIVENESS-X/V034 -g '*.sh' -g '*.py' -g '*runner*' -g '*correct*'` — exit 1; no matching route-owned runner/correctness source or script.
- `find 本地实验/STAGING-LIVENESS-X/V034/build -maxdepth 3 -type f -executable -printf '%P\n'` — exit 0; only `CMakeFiles/3.22.1/CMakeDetermineCompilerABI_CXX.bin` was executable.
- `rg -n 'int main|run_kernel|add_executable|full_link' 本地实验/STAGING-LIVENESS-X/V034/CMakeLists.txt 本地实验/STAGING-LIVENESS-X/V034/src/adapter.cpp 本地实验/STAGING-LIVENESS-X/V034/src/submission.asc` — found `src/adapter.cpp:17:int main(){return 0;}`, `CMakeLists.txt:39:add_executable(full_link src/adapter.cpp)`, and the `run_kernel` declaration/ABI documentation in `src/submission.asc`; no correctness invocation or result comparison exists.

Exact blocker: V034 has a compile-only `full_link` target whose host `main` is a no-op, but no executable correctness runner. Do not treat `full_link` or the CMake compiler-ABI probe as correctness evidence. Correctness remains `NOT_RUN`; Local remains `NONE`; Official score is not claimed; V034 stays frozen and no V035 is created. Next action is to resume Correctness only when a route-owned executable runner/build entry bound to the frozen V034 source is supplied.

## Superseding status after route-local runner authorization (2026-10-07)

The previous no-runner disposition and Support-handoff gate above are historical. Planning authorized route-local runner/build support; the direct-invocation runner is now built and both the recorded Parent and frozen Candidate were run through the route-local T01-T15 harness. The harness identifies this as synthetic route coverage, not the unpublished official mapping.

- V033 Parent: T01-T13 and T15 PASS; T14 FP32 `[2,1,2,32768]` FAIL (`matched_ratio=0.172195`, `max_abs_error=3.5058006`).
- V034 Candidate SHA-256 `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`: T01-T13 and T15 PASS; T14 FP32 `[2,1,2,32768]` FAIL (`matched_ratio=0.155510`, `max_abs_error=3.4189978`). Runner identity matched the frozen SHA and the case completed; this is a numerical failure, not a setup failure.
- Full Parent/Candidate case logs and JSON summaries: `support/results/correctness-pair-20261007T231845Z/`.
- Current status: Candidate CORRECTNESS_FAILED; Local NOT_RUN and score NONE; Online FORBIDDEN. Keep V034 frozen and await Planning direction. Do not create V035 or treat this route receipt as route closure.

## T14 contract recheck (2026-10-07)

The isolated-case selector previously renumbered a selected case's seed. It now retains the original `CASES` index, so T14-only execution uses the same seed 341047 and fixture as the preserved full-suite run. The change does not alter all-cases seeds. The corrected Parent and Candidate T14 recheck is recorded at `support/results/correctness-t14-contract-20261007T234700Z/RESULT.md`; the first attempt without the CANN runtime environment is also retained at `support/results/correctness-t14-contract-20261007T234229Z/`.

The binary inputs and golden match the original run byte-for-byte, and independent FP32 recomputation matches the saved golden exactly. The direct runner supplies rank-4 `[2,1,2,32768]` FP32 metadata, rank-1 width-32768 parameter metadata, epsilon `1e-5`, and 40 available vector cores; `run_kernel` derives 4 rows and launches 4 blocks. No fixture, golden, launch, or ABI mismatch was found.

T14 dispatches through the same `ProcessWideFp32FullCacheRows` body in V033 and V034; V034's changed wait is in a separate path. The exact Parent and Candidate both fail again. Their actual-output hashes also differ from their original full-suite outputs despite identical fixture/golden hashes. Treat the Parent as invalid for this T14 check; the evidence localizes the unstable numerical failure to the shared wide-FP32 path but does not establish a specific device-side race mechanism.

V034 remains `CORRECTNESS_FAILED` under this route-local synthetic suite, not an isolated Parent/Candidate regression. Local remains `NOT_RUN`; V034 source remains frozen. No valid STAGING-LIVENESS-X Local Best is currently evidenced here (V001-V004 are `NOT_COMPLETE` / `SUSPENDED_FOR_W4`, with no later route-local result). Do not claim Local, start a new revision, or close the Route based on this receipt; Planning direction is needed to identify a valid base and resolve the next step.
