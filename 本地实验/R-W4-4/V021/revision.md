# R-W4-4 V021 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V021
- DIRECT_PARENT: R-W4-4 V020
- PARENT_SOURCE_SHA: 8de376a2814f64f013166ec537af1946ce84d383bc207712b97d1c71bb822955
- SOURCE_SHA: 3137357387f46ccbe61cef53d6f8925e6a965ac80fc0b2cea79f642431bb0091
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 8 -> 4 (only source delta from V020)
- EDIT_START_TIMESTAMP: 2026-10-06T20:50:32.948507029Z
- EDIT_SLA: PASS (started within 180 seconds of V020 Compile PASS; deadline 2026-10-06T20:50:49.644488460Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 163.304018569 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:51:10.106896583Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V021' -B '本地实验/R-W4-4/V021/build-cmake' && cmake --build '本地实验/R-W4-4/V021/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
