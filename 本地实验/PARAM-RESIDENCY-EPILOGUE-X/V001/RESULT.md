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
