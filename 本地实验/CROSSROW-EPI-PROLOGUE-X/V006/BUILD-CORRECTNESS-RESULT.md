# W5-R01 CROSSROW-EPI-PROLOGUE-X V006 Build / Correctness

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V006
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_COMMIT: 8b1e6ae0a9dd0ce0136979db6bf5eb3cc4e2765a
PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256: d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c
PARENT_ARTIFACT_SHA256: e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64
CANDIDATE_ARTIFACT_SHA256: 59f9c8ab75fb1b2d91c9dc2d1e6cf4c4461b1481acb931c29a6314c4a4b7662a
RUNNER_REF_SHA256: 2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7

## Build

Initial command: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1`

Initial result: `RC=2`. The exact fatal error was `/tmp/asc_plugin_binary_register_code-f5c44d.c:3:10: fatal error: 'vector' file not found`; the toolkit `vector` header existed, but the generated C++ registration compile did not inherit the HCC C++ include search path. The subsequent cceld/link errors were cascading build errors.

Build-input repair: exported the existing toolkit HCC C++ include roots through `CPLUS_INCLUDE_PATH` and `CPATH` in the Build shell only. No Runner, CMake, ABI, or Kernel file was changed.

Repair command: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH/CPATH from /usr/local/Ascend/ascend-toolkit/latest/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0; cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1`

Final Build: `PASS`, `RC=0`; log: `build-v006-canonical-fix-20261011.log`. The `/tmp/.../V003` name is only the existing runner staging directory; the bound Candidate source SHA and artifact SHA above are V006.

## Correctness

Command form for all cases: `<probe> 7 <rows> <width> <dtype> <raw-prefix> 0 1 1 0 1`, with a fresh `source /usr/local/Ascend/ascend-toolkit/set_env.sh` in each runner shell.

Suite: C01/C08/C11/C12/C13/C14/C16, Parent and Candidate. Result: `PASS_7_OF_7`, `14/14` invocations `RC=0`, `bad=0`; 14 raw and 14 stats files are retained under `canonical-correctness-20261011/`.

## Local gate

Correctness passed before Local. The complete Local evidence and score are under `canonical-paired-20261011/`; no V007 was started.
