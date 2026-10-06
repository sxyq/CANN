# R-W4-4 V028 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V028
- DIRECT_PARENT: R-W4-4 V027
- PARENT_SOURCE_SHA: f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033
- SOURCE_SHA: 06384465fe0e3831048fc15903807ec3aeae55cdcb1852f5d85d60fbb9154ffb
- CHANGE: `kSmallFp32BatchMaxWidth`, 512 -> 256 (only source delta from V027)
- EDIT_START_TIMESTAMP: 2026-10-06T21:15:13.674816492Z
- EDIT_SLA: MISSED (started 182.640153733 seconds after V027 Compile PASS; deadline 2026-10-06T21:15:11.034662759Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 182.640153733 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:15:56.504022693Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V028' -B '本地实验/R-W4-4/V028/build-cmake' && cmake --build '本地实验/R-W4-4/V028/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
