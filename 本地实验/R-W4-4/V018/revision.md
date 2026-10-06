# R-W4-4 V018 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V018
- DIRECT_PARENT: R-W4-4 V017
- PARENT_SOURCE_SHA: ce46abb9621b00f4ccb408586b9cd82c47d6f7bbb07a40088c2167b39914356d
- SOURCE_SHA: 9a19aa941542cef7fd5a5fc674f2950b80d64790a17123669e2c48beee0b1d70
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 64 -> 32 (only source delta from V017)
- EDIT_START_TIMESTAMP: 2026-10-06T20:41:44.879415666Z
- EDIT_SLA: PASS (started within 180 seconds of V017 Compile PASS; deadline 2026-10-06T20:43:10.224478801Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 94.654936865 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:42:36.368025870Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V018' -B '本地实验/R-W4-4/V018/build-cmake' && cmake --build '本地实验/R-W4-4/V018/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
