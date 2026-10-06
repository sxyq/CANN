# R-W4-4 V015 declaration and compile result

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V015
- DIRECT_PARENT: R-W4-4 V014
- PARENT_SOURCE_SHA: f6f3df790290deafe2970a61ff76e419edb5273b633e0ad6bfeee9e6ea3a1e16
- SOURCE_SHA: 98b2b5c7b64ff40e9607870e55d6c5fe9ce3bfee64788a73574fc5e7e94c6587
- CHANGE: `kSmallFp32ContiguousMaxWidth`, 512 -> 256 (only source delta from V014)
- EDIT_START_TIMESTAMP: 2026-10-06T20:33:42.398651166Z
- EDIT_SLA: PASS (started within 180 seconds of V014 Compile PASS; deadline 2026-10-06T20:34:13.211645154Z)
- INHERITED: CHILD_SLA_FAIL=YES
- CHILD_SLA_FAIL: YES
- COMPILE: PASS
- COMPILE_OBSERVED_TIMESTAMP: 2026-10-06T20:34:27.501356016Z
- COMPILE_COMMAND: `cmake -S '本地实验/R-W4-4/V015' -B '本地实验/R-W4-4/V015/build-cmake' && cmake --build '本地实验/R-W4-4/V015/build-cmake' --target device submission --parallel 2`
- COMPILE_RESULT: `device` and `submission` targets built successfully; exit code 0
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- CORRECTNESS / LOCAL / ONLINE: NOT RUN (out of scope)
