# R-W4-4 V014 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V014
- DIRECT_PARENT: R-W4-4 V013
- PARENT_SOURCE_SHA: 04f996ece341d286c9f481de65c6dd2e435c285263c0e602ff93558dd885fd82
- SOURCE_SHA: f6f3df790290deafe2970a61ff76e419edb5273b633e0ad6bfeee9e6ea3a1e16
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 1024 -> 512 (only source delta from V013)
- EDIT_START_TIMESTAMP: 2026-10-06T20:30:23.994013595Z
- EDIT_SLA: PASS (started within 180 seconds of V013 Compile PASS; deadline 2026-10-06T20:30:30.785662513Z)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:31:13.211645154Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V014' -B '本地实验/R-W4-4/V014/build-cmake' && cmake --build '本地实验/R-W4-4/V014/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
