# PARAM-RESIDENCY-EPILOGUE-X V001

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V001
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V001/R31B-V016-WIDE-TILE-SEED_kernel.asc
CANDIDATE_SOURCE_SHA256=d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c

## HYPOTHESIS

BF16 wide-row epilogue currently widens each loaded Gamma/Bias tile twice with identical `ToFloat` operations. Removing the second conversion pair should reduce epilogue vector work while preserving parameter GM movement, double-buffer residency, tail length, row geometry, retained `y`, and the required output formula.

## ONE CHANGE

Removed the duplicate BF16 Gamma/Bias `ToFloat` block at candidate lines 3308-3312 relative to the parent source.

## COMPILE

COMMAND=`cmake -S '本地实验/R31B/V016/support/paired' -B '本地实验/PARAM-RESIDENCY-EPILOGUE-X/V001/build' -DR31B_V011_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/线上结果/R31B/V011' -DR31B_V016_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/本地实验/PARAM-RESIDENCY-EPILOGUE-X/V001'`
RETURN_CODE=127
EVIDENCE=`zsh:1: command not found: cmake`
COMPILE=NOT_AVAILABLE_LOCAL_TOOL

## CORRECTNESS

CORRECTNESS=NOT_RUN_COMPILE_UNAVAILABLE
RUNNER=本地实验/R31B/V016/support/paired/paired_runner.cpp
ABI=本地实验/R31B/V016/support/paired/paired_runner_abi.h
CASES=fp16-tail-d12288,fp16-wide-d32768,bf16-tail-d12288,bf16-wide-d32768

## LOCAL

LOCAL=NOT_RUN_COMPILE_UNAVAILABLE
DEVICE_ID=NONE
FREE_HBM_MB=UNKNOWN
DEVICE_LOAD=UNKNOWN

## RESULT

RESULT=UNMEASURED
INTERPRETATION=Source change is isolated to duplicate BF16 parameter conversion; compile and device measurements remain unavailable because the local Clone host has no cmake or Ascend compiler.

## COMMIT

COMMIT=NOT_CREATED
PUSH=NO

## NEXT

Use the existing paired CMake/Runner entry when a host with the recorded Ascend toolchain is available. Do not add another Runner or measurement script.

ISOLATION_NOT_ENFORCED=YES

## REMOTE_ENV

HOST=cann-server3
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v001.aBcsbv
TOOLCHAIN_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
CANN_PACKAGE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC=Ascend910B3
DEVICE_ID=4
RUNNER_DEVICE_PROCESS=VLLMEngineCor
RUNNER_DEVICE_PROCESS_MEMORY_MB=55686
RAW_LOG_DIR=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V001/remote

## REMOTE_COMPILE

CONFIG_COMMAND=`cmake -S /tmp/n05-param-residency-epilogue-v001.aBcsbv/paired -B /tmp/n05-param-residency-epilogue-v001.aBcsbv/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v001.aBcsbv/parent -DR31B_V016_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v001.aBcsbv/candidate`
CONFIG_RETURN_CODE=0
BUILD_COMMAND=`cmake --build /tmp/n05-param-residency-epilogue-v001.aBcsbv/build --target paired_runner_v016 --parallel 1`
BUILD_ATTEMPT_1_RETURN_CODE=2
BUILD_ATTEMPT_1_EVIDENCE=`fatal error: vector file not found`
BUILD_ATTEMPT_2_ENV=`CPLUS_INCLUDE_PATH=system-c++11 plus CANN hcc c++7.3.0 headers`
BUILD_ATTEMPT_2_RETURN_CODE=0
REMOTE_BINARY=/tmp/n05-param-residency-epilogue-v001.aBcsbv/build/paired_runner_v016

## REMOTE_CORRECTNESS

QUALIFY_ATTEMPT_1_RETURN_CODE=1
QUALIFY_ATTEMPT_1_EVIDENCE=`toolchain libstdc++ lacked GLIBCXX_3.4.29`
QUALIFY_RUNTIME_ENV=`LD_LIBRARY_PATH puts /usr/lib/aarch64-linux-gnu before CANN libraries`
QUALIFY_ATTEMPT_2_RETURN_CODE=0
QUALIFY_V011_CORRECTNESS=PASS_ALL_4_CASES
PAIRED_RETURN_CODE=0
PAIRED_CORRECTNESS=PASS_PARENT_AND_CANDIDATE_ALL_4_CASES

## REMOTE_LOCAL

METHOD=`existing paired_runner_v016 --paired; 45 warmups per binary; 21 interleaved pairs per case; device event primary; wall time diagnostic`
DEVICE_LOAD_NOTE=`device 4 retained VLLMEngineCor during snapshots`
LOCAL_SCORE=NONE
LOCAL_INTERPRETATION=`raw samples retained; existing device process and outliers make a stable performance claim unavailable`

| CASE | PARENT_MEDIAN_US | CANDIDATE_MEDIAN_US | PAIRED_DELTA_US | CORRECTNESS |
|---|---:|---:|---:|---|
| fp16-tail-d12288 | 10.700 | 10.960 | -0.100 | PASS/PASS |
| fp16-wide-d32768 | 19.540 | 22.000 | 0.720 | PASS/PASS |
| bf16-tail-d12288 | 12.540 | 14.160 | 1.080 | PASS/PASS |
| bf16-wide-d32768 | 22.880 | 20.020 | -1.180 | PASS/PASS |

## REMOTE_EVIDENCE

The remote directory was used only for this V001 run. Its configuration, build attempts, runtime failures, runner output, and pre/post device snapshots were copied without filtering into `RAW_LOG_DIR`.

## NEXT

V001 proves Compile and Correctness. Local timing is recorded as unqualified due to the observed device load. No second Runner or measurement script was created.
