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

## CORRECTNESS AND PAIRED LOCAL

HOST=cann-server3
DEVICE_ID=3
RUN_COMMAND=paired_runner_v016 --paired 3
RUNNER_EXIT_CODE=0
END_UTC=2026-10-16T08:42:23Z
CORRECTNESS=PASS (8/8: V011 and V016 passed in all four cases)
LOCAL=COMPLETE (21 paired samples per case; 84 pairs total)
LOCAL_SCORE=NONE (per-case paired measurements recorded; no combined scalar computed)
OFFICIAL=NOT_RUN

Runner mode: paired; 45 warmups per binary/case; device events are primary timing and wall times are diagnostic. Parent is V011 and candidate is V016. Paired delta is V016 minus V011; negative values favor V016.

| Case | V011 median (us) | V016 median (us) | Paired delta median (us) | Paired delta P10 (us) | Paired delta P90 (us) | Correctness |
|---|---:|---:|---:|---:|---:|---|
| fp16-tail-d12288 | 16.980 | 19.460 | +0.160 | -10.520 | +15.720 | PASS |
| fp16-wide-d32768 | 11.060 | 11.840 | -0.200 | -5.440 | +10.860 | PASS |
| bf16-tail-d12288 | 8.300 | 7.800 | -0.200 | -11.860 | +2.200 | PASS |
| bf16-wide-d32768 | 20.500 | 18.160 | -1.240 | -9.800 | +6.220 | PASS |

All four paired-delta P10–P90 ranges cross zero. The medians are +0.160, -0.200, -0.200, and -1.240 us, with mixed direction and ranges much wider than the medians. Although both BF16 unpaired candidate medians are lower, paired samples do not show a consistent separation; the measurements do not establish a stable Local gain. Record this as a negative/inconclusive Local result and retain LOCAL_SCORE=NONE. No Official result was run.

QUALIFICATION_ATTEMPT=paired_runner_v016 --qualify-v011 3
QUALIFICATION_EXIT_CODE=127
QUALIFICATION_RESULT=Loader could not find libgraph.so; Runner did not start and no NPU work occurred. The qualification command was not repeated.

PRE_RUN_SNAPSHOT_UTC=2026-10-16T08:41:42Z; Health=OK; HBM=3429/65536 MiB; AICore=0%; NPU3 processes=none.
RUNNER_END_UTC=2026-10-16T08:42:23Z; RUNNER_EXIT_CODE=0.
MAIN_ENVFIX_LEASE_TIME=2026-10-16T08:42:33Z (reported by Main; 10 seconds after Runner end; timestamp retained as reported).
POST_RUN_SNAPSHOT_UTC=2026-10-16T08:42:55Z; Health=OK; HBM=3429/65536 MiB; AICore=0%; NPU3 processes=none.

Evidence: `remote/compile-config-runner-npu3.log`, `remote/compile-build-runner-npu3.log`, `remote/npu3-qualify-v011.log`, `remote/pre-npu3-paired-snapshot-run.log`, `remote/npu3-paired.log`, `remote/post-npu3-paired-snapshot.log`.

## RUNNER DEVICE ARGUMENT UPDATE

Updated the existing `本地实验/R31B/V016/support/paired/paired_runner.cpp` only. Device ID is now the required second argument after the mode, for example `paired_runner_v016 --paired 7` or `paired_runner_v016 --qualify-v011 7`. The case output reports that ID, `aclrtSetDevice` uses it, and the `aclrtResetDevice` call was removed. The V006 kernel source was not changed.

CONFIGURE_RC=0
BUILD_TARGET=paired_runner_v016
BUILD_RC=0
BUILD_OBJECT_BYTES=269896
RUNNER_BYTES=581248
NPU_RUN=NOT_RUN

Evidence: `remote/compile-config-runner-device-arg.log`, `remote/compile-build-runner-device-arg.log`.
