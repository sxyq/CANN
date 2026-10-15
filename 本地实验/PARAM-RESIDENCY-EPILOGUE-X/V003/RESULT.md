# PARAM-RESIDENCY-EPILOGUE-X V003

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V003
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V003/R31B-V016-WIDE-TILE-SEED-V003_kernel.asc
CANDIDATE_SOURCE_SHA256=4e704718d0a268b5cc5e19bc26b59465b90f95845bc9a72ba66e3a65dbe9dfd4

## PRIOR REVISION CONFIRMATION

V001 removed the duplicate BF16 pass-2 Gamma/Bias `ToFloat` pair. V002 skipped the final parameter-tile `V_MTE2` release handshake when no successor DMA reused that slot. V003 keeps both changes unchanged.

## HYPOTHESIS

After pass 1, the input double-buffer slots are released while normalization reads only the FP32 value, input-conversion, and reduction buffers. Starting the first Gamma/Bias tile DMA at that boundary should overlap parameter movement with the scalar/vector normalization work. The existing `prd0` wait still completes before the epilogue consumes the tile.

## ONE CHANGE

In `ProcessWideLowPrecision`, allocate the existing parameter event IDs and issue the first Gamma/Bias tile Load immediately after pass-1 input-slot release and before the per-row normalization loop. The pass-2 loop consumes the already-issued `prd0`; later tile prefetch, final-tile handling, row geometry, UB buffers, reduction tree, and arithmetic order are unchanged.

Modified existing entry files:

- `本地实验/R31B/V016/support/paired/CMakeLists.txt`: default candidate source directory points to V003.
- `本地实验/R31B/V016/support/paired/paired_runner_v016.asc`: includes the V003 Candidate.

`paired_runner.cpp` and `paired_runner_abi.h` were reused without modification. No second Runner or measurement script was created.

## COMPILE

LOCAL_COMMAND=`cmake -S '本地实验/R31B/V016/support/paired' -B '本地实验/PARAM-RESIDENCY-EPILOGUE-X/V003/build' -DR31B_V011_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/线上结果/R31B/V011' -DR31B_V016_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/本地实验/PARAM-RESIDENCY-EPILOGUE-X/V003'`
LOCAL_RETURN_CODE=127
LOCAL_EVIDENCE=`zsh:1: command not found: cmake`

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v003.d4Pc3E
TOOLCHAIN_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
CANN_PACKAGE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC=Ascend910B3
REMOTE_CONFIG_COMMAND=`cmake -S /tmp/n05-param-residency-epilogue-v003.d4Pc3E/paired -B /tmp/n05-param-residency-epilogue-v003.d4Pc3E/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v003.d4Pc3E/parent -DR31B_V016_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v003.d4Pc3E/candidate`
REMOTE_CONFIG_RETURN_CODE=0
REMOTE_BUILD_COMMAND=`cmake --build /tmp/n05-param-residency-epilogue-v003.d4Pc3E/build --target paired_runner_v016 --parallel 1`
REMOTE_BUILD_RETURN_CODE=0
REMOTE_BINARY=/tmp/n05-param-residency-epilogue-v003.d4Pc3E/build/paired_runner_v016
REMOTE_BUILD_EVIDENCE=`Built target paired_runner_v016`

## CORRECTNESS

RUNNER=本地实验/R31B/V016/support/paired/paired_runner.cpp
ABI=本地实验/R31B/V016/support/paired/paired_runner_abi.h
RUNNER_MODES=`--qualify-v011; --paired`
RUNTIME_LIBRARY_ENV=`LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}`
QUALIFY_COMMAND=`/tmp/n05-param-residency-epilogue-v003.d4Pc3E/build/paired_runner_v016 --qualify-v011`
QUALIFY_RETURN_CODE=0
QUALIFY_RESULT=PASS_ALL_4_V011_CASES
PAIRED_COMMAND=`/tmp/n05-param-residency-epilogue-v003.d4Pc3E/build/paired_runner_v016 --paired`
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
| fp16-tail-d12288 | 6.180 | 5.680 | -0.200 | PASS/PASS |
| fp16-wide-d32768 | 10.020 | 9.820 | -0.280 | PASS/PASS |
| bf16-tail-d12288 | 7.220 | 6.700 | -0.460 | PASS/PASS |
| bf16-wide-d32768 | 14.280 | 12.960 | -1.340 | PASS/PASS |

LOCAL_SCORE=NONE
LOCAL_INTERPRETATION=`The four paired medians moved lower for V003, but device 4 retained VLLMEngineCor and the raw samples contain large outliers; one run does not support a stable performance claim.`

## RESULT

RESULT=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NONE
INTERPRETATION=`The early first-tile parameter prefetch compiled and preserved correctness. The observed medians are directionally lower, but the device load and outliers leave the performance result unqualified.`

## EVIDENCE

Original remote logs and device snapshots were copied to:

`本地实验/PARAM-RESIDENCY-EPILOGUE-X/V003/remote/`

Files include `env-snapshot.log`, `source-sha256.log`, compile configure/build logs, qualify and paired Runner output, and pre/post device snapshots.

## COMMIT

COMMIT=PENDING
PUSH=NO
OFFICIAL=NO

## NEXT

Keep V003 performance status as `NONE`. If another revision is attempted, it must use a fourth parameter residency or tail organization factor and preserve the fixed output formula, split geometry, UB intermediate residency, reduction tree, and arithmetic order.

ISOLATION_NOT_ENFORCED=YES
