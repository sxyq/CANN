# R-W4-4 V027 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V027
- DIRECT_PARENT: R-W4-4 V026
- PARENT_SOURCE_SHA: 5c32129ddb1904b437f1cb22098c6b7fbebf5ac12a10e79761ece39895f398a2
- SOURCE_SHA: f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033
- CHANGE: `kSmallFp32BatchMaxWidth`, 1024 -> 512 (only source delta from V026)
- EDIT_START_TIMESTAMP: 2026-10-06T21:11:30.198410507Z
- EDIT_SLA: PASS (started within 180 seconds of V026 Compile PASS; deadline 2026-10-06T21:11:39.909635676Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 170.288774831 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:12:11.034662759Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V027' -B '本地实验/R-W4-4/V027/build-cmake' && cmake --build '本地实验/R-W4-4/V027/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
