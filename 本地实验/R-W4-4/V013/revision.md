# R-W4-4 V013 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V013
- DIRECT_PARENT: R-W4-4 V012
- PARENT_SOURCE_SHA: b2c3925b907aa085a0cdc784a809ede41c513a7278663b0ee028bc85f9b47666
- SOURCE_SHA: 04f996ece341d286c9f481de65c6dd2e435c285263c0e602ff93558dd885fd82
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 2048 -> 1024 (only source delta from V012)
- EDIT_START_TIMESTAMP: 2026-10-06T20:26:31.891761712Z
- EDIT_SLA: MISSED (V012 deadline 2026-10-06T20:21:52.482433851Z; edit started 279.409 seconds late)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:27:30.785662513Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V013' -B '本地实验/R-W4-4/V013/build-cmake' && cmake --build '本地实验/R-W4-4/V013/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
