# R-W4-4 V006 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V006
- DIRECT_PARENT: R-W4-4 V005
- PARENT_SOURCE_SHA: 8c3aea493c1ae59681e0e3c033c372edf4aceb0c1ae1d32d723439dfb0087df9
- SOURCE_SHA: 01715186c6bea9dd127dabf654cffe2927a15643b942da0a15a9a6420bfbc883
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 64 -> 32 (only source delta from V005)
- EDIT_TIMESTAMP: 2026-10-06T19:45:22.849039357Z
- INHERITED: CHILD_SLA_FAIL=YES
- PREVIOUS_EDIT_SLA: MISSED (V005 deadline 2026-10-06T19:39:17.243360054Z)
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: not captured; PASS was observed before 2026-10-06T19:47:05.572580973Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V006' -B '本地实验/R-W4-4/V006/build-cmake' && cmake --build '本地实验/R-W4-4/V006/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
