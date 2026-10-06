# R-W4-4 V019 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V019
- DIRECT_PARENT: R-W4-4 V018
- PARENT_SOURCE_SHA: 9a19aa941542cef7fd5a5fc674f2950b80d64790a17123669e2c48beee0b1d70
- SOURCE_SHA: 2f2ccad374972de6d9e19839ae8419eddbe61b8211c8ca53c0c6d9839e818971
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 32 -> 16 (only source delta from V018)
- EDIT_START_TIMESTAMP: 2026-10-06T20:44:32.074811264Z
- EDIT_SLA: PASS (started within 180 seconds of V018 Compile PASS; deadline 2026-10-06T20:45:36.368025870Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 115.706785394 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:45:33.115786805Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V019' -B '本地实验/R-W4-4/V019/build-cmake' && cmake --build '本地实验/R-W4-4/V019/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
