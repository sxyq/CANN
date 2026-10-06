# R-W4-4 V024 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V024
- DIRECT_PARENT: R-W4-4 V023
- PARENT_SOURCE_SHA: 8480996f6969b0db344799f6562f86f38875ef965bf5fe9f5f9ecc31536d1e27
- SOURCE_SHA: 21243b31cedcace12178a706beb2327e26ca317cd327723201e3b36b903a3a98
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 1 -> 0 (only source delta from V023)
- EDIT_START_TIMESTAMP: 2026-10-06T21:01:21.382576659Z
- EDIT_SLA: PASS (started within 180 seconds of V023 Compile PASS; deadline 2026-10-06T21:01:40.961380852Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 160.421195807 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:01:58.784835836Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V024' -B '本地实验/R-W4-4/V024/build-cmake' && cmake --build '本地实验/R-W4-4/V024/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
