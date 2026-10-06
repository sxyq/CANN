# R-W4-4 V026 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V026
- DIRECT_PARENT: R-W4-4 V025
- PARENT_SOURCE_SHA: aebdb35681511e4c7cb00a2abc9e02694a1eeabd4409774eda9d54b3eeb84e69
- SOURCE_SHA: 5c32129ddb1904b437f1cb22098c6b7fbebf5ac12a10e79761ece39895f398a2
- CHANGE: `kSmallFp32BatchMaxWidth`, 2048 -> 1024 (only source delta from V025)
- EDIT_START_TIMESTAMP: 2026-10-06T21:08:02.708147198Z
- EDIT_SLA: PASS (started within 180 seconds of V025 Compile PASS; deadline 2026-10-06T21:08:20.568996291Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 162.139150907 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:08:39.909635676Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V026' -B '本地实验/R-W4-4/V026/build-cmake' && cmake --build '本地实验/R-W4-4/V026/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
