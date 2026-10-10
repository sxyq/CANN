# R-W4-4 V016 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V016
- DIRECT_PARENT: R-W4-4 V015
- PARENT_SOURCE_SHA: 98b2b5c7b64ff40e9607870e55d6c5fe9ce3bfee64788a73574fc5e7e94c6587
- SOURCE_SHA: 6204790d72b7ad1e1a21c40cdb6919f6da047c742973a0af2e552d85606343de
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 256 -> 128 (only source delta from V015)
- EDIT_START_TIMESTAMP: 2026-10-06T20:36:55.701693806Z
- EDIT_SLA: PASS (started within 180 seconds of V015 Compile PASS; deadline 2026-10-06T20:37:27.501356016Z)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:37:50.154630896Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V016' -B '本地实验/R-W4-4/V016/build-cmake' && cmake --build '本地实验/R-W4-4/V016/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
