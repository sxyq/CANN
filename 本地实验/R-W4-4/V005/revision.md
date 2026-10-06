# R-W4-4 V005 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V005
- DIRECT_PARENT: R-W4-4 V004
- PARENT_SOURCE_SHA: c3b4a4453a528c3cea29fb481b0345b466c61a0e6619fd5c954d7764ed5d9eef
- SOURCE_SHA: 8c3aea493c1ae59681e0e3c033c372edf4aceb0c1ae1d32d723439dfb0087df9
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 128 -> 64 (only source delta from V004)
- EDIT_TIMESTAMP: 2026-10-06T19:32:56.392308827Z
- INHERITED: CHILD_SLA_FAIL=YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T19:36:17.243360054Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V005' -B '本地实验/R-W4-4/V005/build-cmake' && cmake --build '本地实验/R-W4-4/V005/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
