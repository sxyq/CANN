# R-W4-4 V017 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V017
- DIRECT_PARENT: R-W4-4 V016
- PARENT_SOURCE_SHA: 6204790d72b7ad1e1a21c40cdb6919f6da047c742973a0af2e552d85606343de
- SOURCE_SHA: ce46abb9621b00f4ccb408586b9cd82c47d6f7bbb07a40088c2167b39914356d
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 128 -> 64 (only source delta from V016)
- EDIT_START_TIMESTAMP: 2026-10-06T20:39:21.076120076Z
- EDIT_SLA: PASS (started within 180 seconds of V016 Compile PASS; deadline 2026-10-06T20:40:50.154630896Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 90.921489180 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:40:10.224478801Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V017' -B '本地实验/R-W4-4/V017/build-cmake' && cmake --build '本地实验/R-W4-4/V017/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
