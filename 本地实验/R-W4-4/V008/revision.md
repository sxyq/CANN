# R-W4-4 V008 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V008
- DIRECT_PARENT: R-W4-4 V007
- PARENT_SOURCE_SHA: ec6faa3df7a7afc256631a8ea03968198bf58117777095edfa5319836a8bf902
- SOURCE_SHA: 94f1f917bffa7bbce3379811d396155d06a718528dccc9b07fe6bd9a8e1cf2fc
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 16 -> 8 (only source delta from V007)
- EDIT_START_TIMESTAMP: 2026-10-06T19:54:11.075424347Z
- EDIT_SLA: PASS (next edit started within 180 seconds)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES (inherited from the V007 SLA miss)
- COMPILE: PASS
- COMPILE_TIMESTAMP: 2026-10-06T19:55:55.465317539Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V008' -B '本地实验/R-W4-4/V008/build-cmake' && cmake --build '本地实验/R-W4-4/V008/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- NEXT_EDIT_DEADLINE: 2026-10-06T19:58:55.465317539Z
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
