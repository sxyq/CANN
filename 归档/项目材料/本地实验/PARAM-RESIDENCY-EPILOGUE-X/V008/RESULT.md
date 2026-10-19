# PARAM-RESIDENCY-EPILOGUE-X V008

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V008
PARENT=V006 (V007 output-store schedule excluded)
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V008/R31B-V016-WIDE-TILE-SEED-V008_kernel.asc

## HYPOTHESIS

For width 12,288, the existing low-precision wide path uses 4,096-element tiles and retains enough UB for at least two complete rows in both FP16 and BF16. The paired Runner has two rows and passes eight available cores; `run_kernel` therefore launches two blocks, each receiving one row. `ProcessWideLowPrecision` loads each Gamma/Bias tile once per row batch, so the same three parameter tiles are fetched once by each block. Grouping two rows per block should reduce each Gamma and Bias tile's Load count from six to three across the invocation. The tradeoff is one active block instead of two.

For BF16, the existing two-row UB estimate is `8 * width + 4 * 4096 * 2 + 2 * 4096 * 4 + 2 * 16 * 4` bytes. It fits the 176 KiB budget through width 14,320. FP16's two-row estimate also fits through that width. Width 32,768 does not enter the grouping branch because the existing UB plan retains one row there.

This candidate starts from V006 and does not include V007's deferred output-store drain change. Arithmetic, parameter tile loops, tile size, row math, and store ordering are unchanged by the kernel delta.

## CHANGE

Cap `blockCount` at `ceil(rowCount / 2)` for FP16/BF16 wide rows with width 8,193 through 14,320, allowing one parameter-tile traversal to serve two rows where the existing UB plan permits it. Reuse the existing paired Runner and build target. The Runner now completes Parent/Candidate correctness for all four cases before starting paired timing.

## COMPILE

HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
BUILD_TARGET=paired_runner_v016
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v008.MGLmq5
CONFIGURE_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11 cmake -S ${REMOTE_TEMP}/paired -B ${REMOTE_TEMP}/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=${REMOTE_TEMP}/parent -DR31B_V016_SOURCE_DIR=${REMOTE_TEMP}/candidate`
CONFIGURE_RC=0
BUILD_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11 cmake --build ${REMOTE_TEMP}/build --target paired_runner_v016 --parallel 1`
BUILD_RC=0
REMOTE_BINARY=${REMOTE_TEMP}/build/paired_runner_v016
DEVICE_EXECUTION=COMPLETED_ON_NPU3

Evidence: `remote/compile-config.log`, `remote/compile-build.log`.

## CORRECTNESS AND LOCAL

HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
DEVICE_ID=3
LEASE=N05-V008-D3-CORR-LOCAL-20261016T102932Z
LEASE_START_UTC=2026-10-16T10:29:32Z
PRE_SNAPSHOT_UTC=2026-10-16T10:33:17Z
PRE_STATE=Health:OK; HBM:3428/65536 MiB; AICore:0; no NPU3 process
RUN_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}; cd /tmp/n05-param-residency-epilogue-v008.MGLmq5/build; ./paired_runner_v016 --paired 3`
RUNNER_START_UTC=2026-10-16T10:34:06Z
RUNNER_END_UTC=2026-10-16T10:34:12Z
RUNNER_RC=0
POST_SNAPSHOT_UTC=2026-10-16T10:34:24Z
POST_STATE=Health:OK; HBM:3429/65536 MiB; AICore:0; no NPU3 process

CORRECTNESS=PASS 8/8 (Parent and Candidate for four cases)
LOCAL=RUN 4 cases x 21 paired samples; warmup_each=45
LOCAL_SCORE=NONE
OFFICIAL=NOT_RUN

| Case | Parent median (us) | Candidate median (us) | Paired median V016-V011 (us) | P10 (us) | P90 (us) | P10-P90 observation |
|---|---:|---:|---:|---:|---:|---|
| fp16-tail-d12288 | 6.600 | 8.840 | +2.300 | +1.760 | +3.460 | positive; Candidate slower |
| fp16-wide-d32768 | 11.960 | 9.960 | -0.540 | -7.280 | -0.080 | negative central interval; wide spread and outliers |
| bf16-tail-d12288 | 14.060 | 9.700 | +2.080 | -6.860 | +9.160 | crosses zero |
| bf16-wide-d32768 | 13.880 | 13.320 | -0.620 | -0.780 | -0.340 | negative central interval |

The targeted width-12,288 cases do not establish a stable benefit from grouping two rows per block: FP16 is consistently slower in the central interval, while BF16 is noisy and crosses zero. The width-32,768 cases are outside the grouping branch; their timing cannot support that mechanism. BF16 wide has a small negative paired interval, while FP16 wide has substantial spread and outliers. Overall directions are mixed, so no Local gain is claimed and LOCAL_SCORE remains NONE. This Runner compares V011 with the full V008 candidate, including inherited V001-V006 changes, and is not an isolated V006-versus-V008 measurement.

Evidence: `remote/pre-npu3-corr-local-snapshot.log`, `remote/npu3-paired.log`, `remote/post-npu3-corr-local-snapshot.log`. The raw output contains all four Parent/Candidate correctness results and all 21 interleaved pairs per case. Other cards showed unrelated processes in both snapshots; they were not used or touched.
