# R-W4-4 V012 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V012
- DIRECT_PARENT: R-W4-4 V011
- PARENT_SOURCE_SHA: 5e8da3dab8df9b7c949b4b256fa822ef350c4773c24acff9f456fc53c0314680
- SOURCE_SHA: b2c3925b907aa085a0cdc784a809ede41c513a7278663b0ee028bc85f9b47666
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 1 -> 0 (only source delta from V011)
- EDIT_START_TIMESTAMP: 2026-10-06T20:15:51.073453839Z
- EDIT_SLA: MISSED (V011 deadline 2026-10-06T20:15:30.439Z; edit started 20.635 seconds late)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:18:52.482433851Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V012' -B '本地实验/R-W4-4/V012/build-cmake' && cmake --build '本地实验/R-W4-4/V012/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
