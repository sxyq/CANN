# R-W4-4 V022 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V022
- DIRECT_PARENT: R-W4-4 V021
- PARENT_SOURCE_SHA: 3137357387f46ccbe61cef53d6f8925e6a965ac80fc0b2cea79f642431bb0091
- SOURCE_SHA: 429735bc639538845d9a02f9bb6225eb4d4c37b49e235fa7785d0bc5b6af3d99
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 4 -> 2 (only source delta from V021)
- EDIT_START_TIMESTAMP: 2026-10-06T20:53:28.375083532Z
- EDIT_SLA: PASS (started within 180 seconds of V021 Compile PASS; deadline 2026-10-06T20:54:10.106896583Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 138.268186949 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:54:10.359878155Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V022' -B '本地实验/R-W4-4/V022/build-cmake' && cmake --build '本地实验/R-W4-4/V022/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
