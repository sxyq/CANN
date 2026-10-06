# R-W4-4 V020 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V020
- DIRECT_PARENT: R-W4-4 V019
- PARENT_SOURCE_SHA: 2f2ccad374972de6d9e19839ae8419eddbe61b8211c8ca53c0c6d9839e818971
- SOURCE_SHA: 8de376a2814f64f013166ec537af1946ce84d383bc207712b97d1c71bb822955
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 16 -> 8 (only source delta from V019)
- EDIT_START_TIMESTAMP: 2026-10-06T20:47:12.367964963Z
- EDIT_SLA: PASS (started within 180 seconds of V019 Compile PASS; deadline 2026-10-06T20:48:33.115786805Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 99.252178158 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:47:49.644488460Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V020' -B '本地实验/R-W4-4/V020/build-cmake' && cmake --build '本地实验/R-W4-4/V020/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
