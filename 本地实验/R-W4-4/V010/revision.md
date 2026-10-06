# R-W4-4 V010 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V010
- DIRECT_PARENT: R-W4-4 V009
- PARENT_SOURCE_SHA: b6f63b86259e516c0b5242344ec3345ecc78a453467e65e947fe7707b61b5ef7
- SOURCE_SHA: af92d3c70d77e1f267ba0dbb8ddb850bf39f4cf4ef30d9c9888de066a386607c
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 4 -> 2 (only source delta from V009)
- EDIT_START_TIMESTAMP: 2026-10-06T20:06:31.924Z
- EDIT_SLA: MISSED (deadline 2026-10-06T20:04:20Z; edit started after deadline)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_TIMESTAMP: 2026-10-06T20:07:40.201Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V010' -B '本地实验/R-W4-4/V010/build-cmake' && cmake --build '本地实验/R-W4-4/V010/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
