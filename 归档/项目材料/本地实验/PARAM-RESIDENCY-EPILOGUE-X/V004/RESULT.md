# PARAM-RESIDENCY-EPILOGUE-X V004

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V004
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V004/R31B-V016-WIDE-TILE-SEED-V004_kernel.asc
CANDIDATE_SOURCE_SHA256=86ffc109f493feafb022016acbf685a1e6f1cc2827b94645c8a1b7f735d0ea9c

## PRIOR REVISION CONFIRMATION

V001 removed the duplicate BF16 pass-2 Gamma/Bias `ToFloat` pair. V002 skipped the final parameter-tile release handshake when no successor DMA reused that slot. V003 moved the first Gamma/Bias tile DMA before the per-row normalization loop. V004 keeps all three changes unchanged.

## HYPOTHESIS

For a one-row final BF16 tile, Gamma has already been promoted into the FP32 parameter scratch and is no longer needed in its native slot. Using that native Gamma slot as the `FromFloat` destination for the final output tile should organize the tail Store without changing the FP32 multiply/add order or the valid element count.

## ONE CHANGE

In `ProcessWideLowPrecision`, only when `batchRows == 1`, the type is the BF16 low-precision branch, and `tile + 1 == tileCount`, assign `outputLocal` to `gammaLocal`. FP16 and all non-final tiles keep `outputBuf_`; the output formula, tile geometry, retained y, UB allocation, and reduction tree remain unchanged.

Modified existing entry files:

- `本地实验/R31B/V016/support/paired/CMakeLists.txt`: default candidate source directory points to V004.
- `本地实验/R31B/V016/support/paired/paired_runner_v016.asc`: includes the V004 Candidate.

`paired_runner.cpp` and `paired_runner_abi.h` were reused without modification. No second Runner, script, or ledger was created.

## COMPILE

LOCAL_COMMAND=`cmake -S '本地实验/R31B/V016/support/paired' -B '本地实验/PARAM-RESIDENCY-EPILOGUE-X/V004/build' -DR31B_V011_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/线上结果/R31B/V011' -DR31B_V016_SOURCE_DIR='/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n05-param-residency-epilogue/本地实验/PARAM-RESIDENCY-EPILOGUE-X/V004'`
LOCAL_RETURN_CODE=127
LOCAL_EVIDENCE=`zsh:2: command not found: cmake`

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v004.O1zgRO
TOOLCHAIN_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
CANN_PACKAGE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC=Ascend910B3
REMOTE_CONFIG_COMMAND=`cmake -S /tmp/n05-param-residency-epilogue-v004.O1zgRO/paired -B /tmp/n05-param-residency-epilogue-v004.O1zgRO/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v004.O1zgRO/parent -DR31B_V016_SOURCE_DIR=/tmp/n05-param-residency-epilogue-v004.O1zgRO/candidate`
REMOTE_CONFIG_RETURN_CODE=0
REMOTE_BUILD_COMMAND=`cmake --build /tmp/n05-param-residency-epilogue-v004.O1zgRO/build --target paired_runner_v016 --parallel 1`
REMOTE_BUILD_RETURN_CODE=0
REMOTE_BINARY=/tmp/n05-param-residency-epilogue-v004.O1zgRO/build/paired_runner_v016
REMOTE_BUILD_EVIDENCE=`Built target paired_runner_v016`

## CORRECTNESS

RUNNER=本地实验/R31B/V016/support/paired/paired_runner.cpp
ABI=本地实验/R31B/V016/support/paired/paired_runner_abi.h
RUNNER_MODES=`--qualify-v011; --paired`
RUNTIME_LIBRARY_ENV=`LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}`
QUALIFY_COMMAND=`/tmp/n05-param-residency-epilogue-v004.O1zgRO/build/paired_runner_v016 --qualify-v011`
QUALIFY_RETURN_CODE=0
QUALIFY_RESULT=PASS_ALL_4_V011_CASES
PAIRED_COMMAND=`/tmp/n05-param-residency-epilogue-v004.O1zgRO/build/paired_runner_v016 --paired`
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
| fp16-tail-d12288 | 12.820 | 11.520 | +0.020 | PASS/PASS |
| fp16-wide-d32768 | 20.320 | 21.120 | -1.500 | PASS/PASS |
| bf16-tail-d12288 | 12.000 | 12.000 | +0.080 | PASS/PASS |
| bf16-wide-d32768 | 24.260 | 21.900 | -1.220 | PASS/PASS |

LOCAL_SCORE=NONE
LOCAL_INTERPRETATION=`The measured deltas are mixed across cases and the device retained VLLMEngineCor with large outliers; the run does not clearly support a performance gain.`

## RESULT

RESULT=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NONE
INTERPRETATION=`The BF16 one-row tail staging change compiled and preserved correctness. Local evidence is mixed and affected by device load, so no performance claim is made.`

## EVIDENCE

Original remote logs and device snapshots were copied to:

`本地实验/PARAM-RESIDENCY-EPILOGUE-X/V004/remote/`

Files include `env-snapshot.log`, `source-sha256.log`, compile configure/build logs, qualify and paired Runner output, and pre/post device snapshots.

## COMMIT

COMMIT=NOT_CREATED_LOCAL_SCORE_NONE
PUSH=NO
OFFICIAL=NO

## NEXT

Do not return an N06 candidate. Keep V004 as an uncommitted measured result because Local evidence does not clearly support the mechanism. A later attempt requires a separate fourth parameter or tail organization factor and a run with interpretable device conditions.

ISOLATION_NOT_ENFORCED=YES
