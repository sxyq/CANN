# R-W4-4 V007 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V007
- DIRECT_PARENT: R-W4-4 V006
- PARENT_SOURCE_SHA: 01715186c6bea9dd127dabf654cffe2927a15643b942da0a15a9a6420bfbc883
- SOURCE_SHA: ec6faa3df7a7afc256631a8ea03968198bf58117777095edfa5319836a8bf902
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 32 -> 16 (only source delta from V006)
- EDIT_TIMESTAMP: 2026-10-06T19:49:41.757325117Z
- EDIT_SLA: MISSED (requested deadline 2026-10-06T19:48:22.850Z)
- INHERITED: CHILD_SLA_FAIL=YES
- COMPILE: PASS
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V007' -B '本地实验/R-W4-4/V007/build-cmake' && cmake --build '本地实验/R-W4-4/V007/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
