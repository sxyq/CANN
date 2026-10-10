# W5-R01 CROSSROW-EPI-PROLOGUE-X V003 Build / Correctness receipt

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V003
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_COMMIT: 5347a5606cdc8aec4c350c7d6e0b987b94346a8c

## Source identity and single-point diff

PARENT_SOURCE: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/Parent.asc`
PARENT_SOURCE_SHA256: `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`
CANDIDATE_SOURCE: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/Candidate.asc`
CANDIDATE_SOURCE_SHA256: `c6d8da88a9ba03e6c730733c52bed051b54ebc94629196636ea415599b96eb8c`
SINGLE_POINT_DIFF: `AddRmsNormBiasKernel::Init()` only, `valueFp32Buf_` allocation:

```diff
- pipe_.InitBuffer(valueFp32Buf_, kCacheElems * sizeof(float));
+ pipe_.InitBuffer(valueFp32Buf_, static_cast<uint32_t>(rowWidth * sizeof(float)));
```

No V002 file was modified. No second Kernel location, ABI, runner, shape, arithmetic, Load/Store helper, or event/synchronization change was made.

## Temporary adapter selector and restoration

The existing V001 support adapters were temporarily changed only for the V003 Build/Correctness invocation:

```diff
- #include "../Parent.asc"
+ #include "../../V003/Parent.asc"
- #include "../Candidate.asc"
+ #include "../../V003/Candidate.asc"
```

These selector edits were not staged or committed. They were restored to `../Parent.asc` and `../Candidate.asc`; post-restore `git diff` for both adapter paths is empty.

## V003 Build

BUILD_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH and CPATH to the HCC C++ root/target/backward; cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1`
BUILD_RC: `0`
BUILD_LOG: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/build-v003.log`
V003_BOUND_PARENT_SO_SHA256: `291eabc271043752cc48466aea0d3ec994476cd3340a4e97f3143efe69dcb7bc`
V003_BOUND_CANDIDATE_SO_SHA256: `502006729d37f9ab654ab460efef9aa4318bb7fa9cb107c066e0e0ad6b1bf342`
V003_BOUND_RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`

## Correctness

TARGET_COMMAND: `crossrow_v001_paired_runner --mode correctness --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16`
TARGET_RESULT: `PASS`, RC `0`; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`
TARGET_LOG: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/correctness-r16-d6144.log`

M1_COMMAND: `crossrow_v001_paired_runner --mode correctness --device 3 --rows 1 --width 6144 --blocks 1 --dtype fp16`
M1_RESULT: `PASS`, RC `0`; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`
M1_LOG: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/correctness-m1-d6144.log`

The existing runner prints its historical V001 metadata labels; source identity for this run is established by the temporary adapter selector and the V003-bound artifact hashes above.

## Restore

RESTORE_BUILD_COMMAND: same existing CMake command after restoring both adapters to V001 paths
RESTORE_BUILD_RC: `0`
RESTORE_LOG: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/restore-v001-support-build.log`
RESTORED_PARENT_SO_SHA256: `61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944`
RESTORED_CANDIDATE_SO_SHA256: `278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b`
RESTORED_RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`
RESTORED_ADAPTER_TRACKED_DIFF: `EMPTY`

## Local gate

LOCAL: `NOT_RUN`
LOCAL_SCOPE: `SHAPE_LOCAL_ONLY`
FULL_LOCAL_SCORE: `NOT_COMPUTABLE`
LOCAL_BEST: `UNCHANGED`
REASON: Build and Correctness were the requested gates; no complete current Local matrix exists, and no diagnostic Local was run in this receipt.

NEXT_ACTION: review the V003 Build/Correctness evidence; any optional shape-level Local must reuse the existing runner and remain diagnostic only, with no `COMPLETE` or `LOCAL_BEST` claim.
