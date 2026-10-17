# PARAM-RESIDENCY-EPILOGUE-X V010

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V010
PARENT=V009
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V010/R31B-V016-WIDE-TILE-SEED-V010_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=1e6add8b
PUSH=NO
OFFICIAL=PASS_15_OF_15
OFFICIAL_SCORE=40.37

## HYPOTHESIS

V009 removes per-unit quotient/reconstruction from `ProcessWideLowPrecision` Pass 1, but its input addresses still combine `rowOffset + col`; the prefetched address additionally selects `nRowOffset` and combines it with `nCol`. If scalar address formation is still limiting the MTE2 load stream, carrying one row-major input offset should shorten that address path.

V010 initializes `inputOffset` to `batchBegin * rowWidth`, uses it for the first X/residual loads, and advances it by the current `valid` count when it enqueues the next tile. For an interior tile, `valid` is `tileWidth`; for a row's final tile, `valid` is the remaining row width, so the next offset equals the following row's start. This preserves the existing row-major element sequence while removing the explicit row-offset/column-offset combination from the load addresses.

Source evidence: V009 Pass 1 uses `rowOffset + col` for its initial loads and `nRowOffset + nCol` for prefetch; V010 uses `inputOffset` and `nextOffset`. Search of V001–V009 found no linear GM-offset cursor in this Pass 1. The hypothesis is source-based; no generated-instruction or device evidence is available yet.

## CHANGE

- Copied the V009 kernel into V010 and changed only Pass 1 X/residual GM offset progression.
- Updated the existing paired AscendC include and CMake candidate directory to V010. The C++ Runner logic is unchanged.
- DMA order, A/B ping-pong, event sequence, valid-count calculation, Vector operations, reductions, and output path remain as in V009.

## VALIDATION

REMOTE_HOST=cann-server3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V010
HOST=hwnput3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_ID=3
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
COMPILE_LOG=remote/compile-attempt5.log
COMPILE_ENV=remote/compile-attempt5-env.txt
COMPILE_NOTE=Attempts 1-4 retained; attempt4 returned 2 before the CPLUS_INCLUDE_PATH fix, attempt5 returned 0 with the existing server3 set_env.sh and CPLUS_INCLUDE_PATH environment.

CORRECTNESS=PASS_4_OF_4
LOCAL=MEASURED
RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V010/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; rows=2; warmup_each=45; pairs_each=21; device event primary; wall time diagnostic`

| Shape | Parent median us | Candidate median us | Paired delta median us | Candidate minus Parent median |
|---|---:|---:|---:|---:|
| fp16-tail-d12288 | 9.880 | 8.780 | -0.540 | -1.100 us (-11.1%) |
| fp16-wide-d32768 | 10.920 | 11.760 | 0.000 | +0.840 us (+7.7%) |
| bf16-tail-d12288 | 7.100 | 6.740 | -0.240 | -0.360 us (-5.1%) |
| bf16-wide-d32768 | 14.400 | 13.860 | -0.640 | -0.540 us (-3.8%) |

SAMPLE_COUNT=21 pairs per shape
LOCAL_SCORE=NO_CLEAR_WIN
INTERPRETATION=Correctness passed for all four shapes. V010 is mixed: fp16-wide regressed, while the other three medians improved; one 21-pair run does not establish a stable broad improvement. Do not hand off V010 for online submission.

HBM_USAGE_PRE=5% (npu-smi device 3; exact free HBM not reported by the captured command)
HBM_USAGE_POST=5% (npu-smi device 3; exact free HBM not reported by the captured command)
DEVICE_LOAD_PRE=AICore 0%, AIVector 0%, AICPU 0%, CtrlCPU 1%
DEVICE_LOAD_POST=AICore 0%, AIVector 0%, AICPU 0%, CtrlCPU 2%
SYSTEM_LOAD_PRE=18:48:47, load average 23.65/31.83/31.27
SYSTEM_LOAD_POST=18:48:56, load average 22.91/31.40/31.14
OTHER_PROCESS_PRESENT=Device proc query unsupported; own-process snapshots retained. No N05 runner remained after exit.
RAW_REMOTE_LOGS=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V010/logs/
RAW_LOCAL_LOGS=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V010/remote/
ONLINE_SUBMIT=PASS_15_OF_15
ONLINE_SUBMIT_OWNER=Main using the existing online submission script; this Agent did not perform an online submission.

## Official 服务器事务结果

- 提交使用既有 `工具/cannjudge-submit.mjs`，结果文件：`线上结果/NEW-N05-PARAM-RESIDENCY-EPILOGUE-X/V010/result.json`。
- 状态：`Pass 15/15`；Official score：`40.37`；Submission ID：`6ad27516193e507d8ec24022`。
- 源码身份：本地 SHA256 `d467e69b553e269e975728d492e8ecc2da03f6488c320ccb4db1c133db0cc023`，平台返回 `LOCAL_SHA_EQ_REMOTE_SHA`。
- 该分数低于 R31B V011 的 `45.16`，保留为真实服务器事务结果。

## COMMIT

SOURCE_COMMIT=1e6add8b
PUSH=NO
