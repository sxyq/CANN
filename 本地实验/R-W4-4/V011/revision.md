# R-W4-4 V011 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V011
- DIRECT_PARENT: R-W4-4 V010
- PARENT_SOURCE_SHA: af92d3c70d77e1f267ba0dbb8ddb850bf39f4cf4ef30d9c9888de066a386607c
- SOURCE_SHA: 5e8da3dab8df9b7c949b4b256fa822ef350c4773c24acff9f456fc53c0314680
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 2 -> 1 (only source delta from V010)
- EDIT_START_TIMESTAMP: 2026-10-06T20:12:01.778Z
- EDIT_SLA: MISSED (V010 deadline 2026-10-06T20:10:40.201Z; edit started 81.577 seconds late)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:12:30.439Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V011' -B '本地实验/R-W4-4/V011/build-cmake' && cmake --build '本地实验/R-W4-4/V011/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
