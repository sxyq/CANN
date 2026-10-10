# R-W4-4 V029 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V029
- DIRECT_PARENT: R-W4-4 V028
- PARENT_SOURCE_SHA: 06384465fe0e3831048fc15903807ec3aeae55cdcb1852f5d85d60fbb9154ffb
- SOURCE_SHA: 9e9a4a17e0079b77e8f9b41f26e57653e70128081da27c5ffff0b064375bb978
- CHANGE: `kSmallFp32BatchMaxWidth`, 256 -> 128 (only source delta from V028)
- EDIT_START_TIMESTAMP: 2026-10-06T21:18:28.729116924Z
- EDIT_SLA: PASS (started within 180 seconds of V028 Compile PASS; deadline 2026-10-06T21:18:56.504022693Z)
- EDIT_ELAPSED_FROM_PARENT_COMPILE: 152.225094231 seconds
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- INHERITED_SLA_NOTE: V023 edit SLA miss preserved; it started 0.641409693 seconds late.
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T21:19:10.809339002Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V029' -B '本地实验/R-W4-4/V029/build-cmake' && cmake --build '本地实验/R-W4-4/V029/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
