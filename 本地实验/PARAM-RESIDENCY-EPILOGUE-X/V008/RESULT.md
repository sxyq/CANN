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
DEVICE_EXECUTION=NOT_RUN_NO_CURRENT_N05_ALLOCATION_RECORD

Evidence: `remote/compile-config.log`, `remote/compile-build.log`.

## CORRECTNESS AND LOCAL

CORRECTNESS=NOT_RUN
LOCAL=NOT_RUN
LOCAL_SCORE=NONE
OFFICIAL=NOT_RUN

The existing paired Runner compares V011 with the full V008 candidate; its Local delta therefore includes inherited V001-V006 changes and is not an isolated V006-versus-V008 measurement. The device record available inside this Clone contains no current N05 V006/V007/V008 allocation entry, so no device was selected or sampled. Before a later run, use a current route allocation record and capture a fresh device snapshot; paired mode performs all four Parent/Candidate correctness checks before starting any Local timing.
