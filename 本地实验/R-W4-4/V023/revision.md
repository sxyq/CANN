# R-W4-4 V023 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V023
- DIRECT_PARENT: R-W4-4 V022
- PARENT_SOURCE_SHA: 429735bc639538845d9a02f9bb6225eb4d4c37b49e235fa7785d0bc5b6af3d99
- SOURCE_SHA: 8480996f6969b0db344799f6562f86f38875ef965bf5fe9f5f9ecc31536d1e27
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 2 -> 1 (only source delta from V022)
- EDIT_START_TIMESTAMP: 2026-10-06T20:57:11.001287848Z
- EDIT_SLA: MISSED (started 180.641409693 seconds after V022 Compile PASS; deadline 2026-10-06T20:57:10.359878155Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 180.641409693 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:58:40.961380852Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V023' -B '本地实验/R-W4-4/V023/build-cmake' && cmake --build '本地实验/R-W4-4/V023/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
