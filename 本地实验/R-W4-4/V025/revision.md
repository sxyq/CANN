# R-W4-4 V025 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V025
- DIRECT_PARENT: R-W4-4 V024
- PARENT_SOURCE_SHA: 21243b31cedcace12178a706beb2327e26ca317cd327723201e3b36b903a3a98
- SOURCE_SHA: aebdb35681511e4c7cb00a2abc9e02694a1eeabd4409774eda9d54b3eeb84e69
- CHANGE: `kSmallFp32BatchMaxWidth`, 4096 -> 2048 (only source delta from V024)
- EDIT_START_TIMESTAMP: 2026-10-06T21:04:43.168291899Z
- EDIT_SLA: PASS (started within 180 seconds of V024 Compile PASS; deadline 2026-10-06T21:04:58.784835836Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 164.383456063 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:05:20.568996291Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V025' -B '本地实验/R-W4-4/V025/build-cmake' && cmake --build '本地实验/R-W4-4/V025/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
