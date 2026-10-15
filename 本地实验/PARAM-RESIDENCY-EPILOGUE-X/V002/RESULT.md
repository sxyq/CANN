# PARAM-RESIDENCY-EPILOGUE-X V002

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V002
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V002/R31B-V016-WIDE-TILE-SEED-V002_kernel.asc
CANDIDATE_SOURCE_SHA256=3e9e5b14208f3a3867bb95bf0eb9b614d139f02846cba8ef7766c9dfadd27c56

## V001 DIFF CONFIRMATION

V001 removed the duplicate BF16 pass-2 `ToFloat(gammaLocal, biasLocal)` pair in `ProcessWideLowPrecision`. V002 retains that V001 change and does not modify the BF16 conversion block.

## HYPOTHESIS

In the wide low-precision epilogue, the final Gamma/Bias tile has no successor parameter DMA. Skipping its `V_MTE2` release event should remove one unused parameter-slot release handshake while preserving every parameter load, the two-slot parameter staging, row/tile geometry, retained `y`, reduction tree, and mathematical order.

## ONE CHANGE

In `ProcessWideLowPrecision` pass 2, emit and track the `V_MTE2` parameter-slot release event only when `tile + 1 < tileCount`. The final tile keeps its existing output operations and store, but does not release a slot that will not be overwritten.

Modified existing entry files:

- `本地实验/R31B/V016/support/paired/CMakeLists.txt`: default candidate source directory points to V002.
- `本地实验/R31B/V016/support/paired/paired_runner_v016.asc`: includes the V002 Candidate.

`paired_runner.cpp` and `paired_runner_abi.h` were reused without modification. No second Runner or measurement script was created.

## COMPILE

LOCAL_COMMAND=`cmake -S '本地实验/R31B/V016/support/paired' -B '本地实验/PARAM-RESIDENCY-EPILOGUE-X/V002/build' -DR31B_V011_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/线上结果/R31B/V011' -DR31B_V016_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/本地实验/PARAM-RESIDENCY-EPILOGUE-X/V002'`
LOCAL_RETURN_CODE=127
LOCAL_EVIDENCE=`zsh: command not found: cmake`

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v002.CgAp6D
TOOLCHAIN_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
CANN_PACKAGE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC=Ascend910B3
REMOTE_CONFIG_COMMAND=`cmake -S /tmp/n05-param-residency-epilogue-v002.CgAp6D/paired -B /tmp/n05-param-residency-epilogue-v002.CgAp6D/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v002.CgAp6D/parent -DR31B_V016_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v002.CgAp6D/candidate`
REMOTE_CONFIG_RETURN_CODE=0
REMOTE_BUILD_COMMAND=`cmake --build /tmp/n05-param-residency-epilogue-v002.CgAp6D/build --target paired_runner_v016 --parallel 1`
REMOTE_BUILD_RETURN_CODE=0
REMOTE_BINARY=/tmp/n05-param-residency-epilogue-v002.CgAp6D/build/paired_runner_v016
REMOTE_BUILD_EVIDENCE=`Built target paired_runner_v016`
REMOTE_FIRST_ATTEMPT=`source with set -u`
REMOTE_FIRST_ATTEMPT_RETURN_CODE=1
REMOTE_FIRST_ATTEMPT_EVIDENCE=`/usr/local/Ascend/ascend-toolkit/set_env.sh: line 1: LD_LIBRARY_PATH: unbound variable`
REMOTE_COMPILE_RETRY=`same compile after disabling set -u and setting CPLUS_INCLUDE_PATH`

## CORRECTNESS

RUNNER=本地实验/R31B/V016/support/paired/paired_runner.cpp
ABI=本地实验/R31B/V016/support/paired/paired_runner_abi.h
RUNNER_MODES=`--qualify-v011; --paired`
RUNTIME_LIBRARY_ENV=`LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}`
QUALIFY_COMMAND=`/tmp/n05-param-residency-epilogue-v002.CgAp6D/build/paired_runner_v016 --qualify-v011`
QUALIFY_RETURN_CODE=0
QUALIFY_RESULT=PASS_ALL_4_V011_CASES
PAIRED_COMMAND=`/tmp/n05-param-residency-epilogue-v002.CgAp6D/build/paired_runner_v016 --paired`
PAIRED_RETURN_CODE=0

| CASE | PARENT | CANDIDATE | CORRECTNESS |
|---|---:|---:|---|
| fp16-tail-d12288 | V011 | V016 | PASS/PASS |
| fp16-wide-d32768 | V011 | V016 | PASS/PASS |
| bf16-tail-d12288 | V011 | V016 | PASS/PASS |
| bf16-wide-d32768 | V011 | V016 | PASS/PASS |

## LOCAL

METHOD=`existing paired_runner_v016 --paired; 45 warmups per binary; 21 interleaved pairs per case; device event primary; wall time diagnostic`
DEVICE_ID=4
DEVICE=Ascend910B3
FREE_HBM_MB=UNKNOWN
DEVICE_LOAD=`VLLMEngineCor on NPU 4; VLLMEngineCor also present on NPU 0/1; VLLMWorker_TP on NPU 5/6`

| CASE | PARENT_MEDIAN_US | CANDIDATE_MEDIAN_US | PAIRED_DELTA_US | CORRECTNESS |
|---|---:|---:|---:|---|
| fp16-tail-d12288 | 11.340 | 15.320 | +2.680 | PASS/PASS |
| fp16-wide-d32768 | 16.940 | 19.800 | +1.820 | PASS/PASS |
| bf16-tail-d12288 | 12.940 | 13.140 | +0.180 | PASS/PASS |
| bf16-wide-d32768 | 25.240 | 26.040 | +0.320 | PASS/PASS |

LOCAL_SCORE=NONE
LOCAL_INTERPRETATION=`No performance claim: device 4 retained VLLMEngineCor and the paired samples contain large outliers; all four paired medians were slower for V002.`

## RESULT

RESULT=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NONE
INTERPRETATION=`The final-tile parameter release-handshake change compiled and preserved correctness. The measured paired medians do not support a performance benefit under the observed device load.`

## EVIDENCE

Original remote logs and device snapshots were copied to:

`本地实验/PARAM-RESIDENCY-EPILOGUE-X/V002/remote/`

Files include `env-snapshot.log`, `source-sha256.log`, compile configure/build logs, qualify and paired Runner output, and pre/post device snapshots.

## COMMIT

COMMIT=PENDING
PUSH=NO
OFFICIAL=NO

## NEXT

Record V002 as `NONE` for performance and do not claim a gain. Any later revision must use a different Gamma/Bias residency or epilogue organization factor and must preserve the fixed output formula, split geometry, UB intermediate residency, reduction tree, and mathematical order.

ISOLATION_NOT_ENFORCED=YES
