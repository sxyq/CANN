# PARAM-RESIDENCY-EPILOGUE-X V006

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V006
PARENT=V005
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V006/R31B-V016-WIDE-TILE-SEED-V006_kernel.asc
CANDIDATE_SOURCE_SHA256=8c6ba3ffa6dca74c424bf456927d148625fb994fe35f2e4dafd5c0389ba000d3

## PREVIOUS REVISION

V005 Official PASS 15/15, score 40.92, SID `6ad1a8a5aad397b6d9f71c37`, source SHA256 `11f20d8fc1cb3b07d872832f3ec9532aff7c292c57acd4184d9fee64fa285d79`, Correctness 8/8, Local=NONE. V005 was not resubmitted.

## CHANGE

V005 BF16 wide-path epilogue applies the row scale with `Muls`, then applies Gamma and Bias using separate FP32 `Mul` and `Add` operations before the BF16 output conversion. V006 retains the row-scale `Muls`, FP32 Gamma/Bias values, `FromFloat`, store path, tile boundaries, and synchronization around the result. It replaces the separate `Mul` plus `Add` with `FusedMulAdd(valueTile, xFp32, residualFp32, valid)`.

CANN 8.5.0.alpha002 DAV-C220 headers define this operation as `dst = src0 * dst + src1`; the implementation maps it to `vmadd`. For each BF16 output tile, this reduces the Gamma/Bias affine stage from two vector operations to one. It does not reuse the Gamma buffer as output storage.

Fused multiply-add can round differently from separate multiply and add. Device Correctness remains necessary before any performance run.

## COMPILE

HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
CONFIGURE_RC=0
BUILD_TARGET=paired_runner_v016
BUILD_RC=0
BUILD_OBJECT_BYTES=270144
RUNNER_BYTES=581264
DEVICE_EXECUTION=NOT_RUN
CANDIDATE_TRANSFER_IDENTITY=LOCAL_EQUALS_REMOTE; SHA256 `8c6ba3ffa6dca74c424bf456927d148625fb994fe35f2e4dafd5c0389ba000d3`

The first build attempt returned 2 because `ASCEND_HOME_PATH` was unset. The next attempt returned 2 because compiler-generated registration code could not locate `<vector>`. The successful build loaded `/usr/local/Ascend/ascend-toolkit/set_env.sh` and set `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11`.

Evidence: `remote/compile-config.log`, `remote/compile-build.log`.

## CORRECTNESS AND LOCAL

CORRECTNESS=NOT_RUN_WAITING_FOR_MAIN_DEVICE_ASSIGNMENT
LOCAL=NOT_RUN_WAITING_FOR_MAIN_DEVICE_ASSIGNMENT
LOCAL_SCORE=NONE
OFFICIAL=NOT_RUN

No NPU Runner was started. Continue only after Main assigns a device; first run Correctness, then record any Local result separately.

## RUNNER DEVICE ARGUMENT UPDATE

Updated the existing `本地实验/R31B/V016/support/paired/paired_runner.cpp` only. Device ID is now the required second argument after the mode, for example `paired_runner_v016 --paired 7` or `paired_runner_v016 --qualify-v011 7`. The case output reports that ID, `aclrtSetDevice` uses it, and the `aclrtResetDevice` call was removed. The V006 kernel source was not changed.

CONFIGURE_RC=0
BUILD_TARGET=paired_runner_v016
BUILD_RC=0
BUILD_OBJECT_BYTES=269896
RUNNER_BYTES=581248
NPU_RUN=NOT_RUN

Evidence: `remote/compile-config-runner-device-arg.log`, `remote/compile-build-runner-device-arg.log`.
