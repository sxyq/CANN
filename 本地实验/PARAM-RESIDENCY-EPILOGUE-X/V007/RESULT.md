# PARAM-RESIDENCY-EPILOGUE-X V007

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V007
PARENT=V006
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V007/R31B-V016-WIDE-TILE-SEED-V007_kernel.asc

## V001-V006 REVIEW

V001 removes the duplicate BF16 Gamma/Bias widening pair. V002 omits the final parameter-slot release when no later DMA uses the slot. V003 starts the first Gamma/Bias tile load before row normalization. V004 reuses the Gamma slot only for a one-row final BF16 output tile. V005 extends Gamma-slot output reuse to all BF16 wide tiles and removes the dedicated output tile. V006 fuses the BF16 affine Mul+Add into FusedMulAdd.

In V006 `ProcessWideLowPrecision`, Gamma/Bias are loaded once per parameter tile and widened before the retained-row loop. The output `Store` is followed by `SyncMTE3ToV` before the next row/tile can proceed. The existing `ProcessWideFp16BatchedOutputPipelined` demonstrates separate output staging and MTE3 release handling, but the BF16 wide path does not use that schedule.

## HYPOTHESIS

Test output Cast/Store scheduling as one factor: keep one independent BF16 output tile, issue its Store, and defer the MTE3-to-Vector drain until immediately before that tile is reused by the next `FromFloat`. This should let the next tile's parameter work and Vector arithmetic overlap the prior output transfer. Gamma/Bias load count and widening, V006 FusedMulAdd, arithmetic order, and tile selection remain unchanged. The dedicated tile is the same single-tile allocation used by V004; this version makes no UB-saving claim and does not repeat the V005 Gamma-slot allocation-saving hypothesis.

## CHANGE

- Restore the existing `outputBuf_` allocation for the BF16 wide path.
- Use it as the output destination; defer `SyncMTE3ToV` until before reuse, then drain the final pending Store before return.
- Keep the FP16 store synchronization unchanged.
- Reuse the existing `paired_runner_v016` target, with its source include and default candidate directory pointing to V007.

## COMPILE

HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v007.HfMXDc
BUILD_TARGET=paired_runner_v016
CONFIGURE_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake -S ${REMOTE_TEMP}/paired -B ${REMOTE_TEMP}/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=${REMOTE_TEMP}/parent -DR31B_V016_SOURCE_DIR=${REMOTE_TEMP}/candidate`
CONFIGURE_RC=0
BUILD_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake --build ${REMOTE_TEMP}/build --target paired_runner_v016 --parallel 1`
BUILD_RC=0
DEVICE_EXECUTION=NOT_RUN_WAITING_FOR_MAIN_DEVICE_ASSIGNMENT

Evidence: `remote/compile-config.log`, `remote/compile-build.log`.

## CORRECTNESS AND LOCAL

HOST=cann-server3
DEVICE_ID=3
PRE_SNAPSHOT_UTC=2026-10-16T09:25:51Z
PRE_STATE=Health:OK; HBM:3429/65536 MiB; AICore:0; no NPU3 process
RUN_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}; /tmp/n05-param-residency-epilogue-v007.HfMXDc/build/paired_runner_v016 --paired 3`
RUNNER_START_UTC=2026-10-16T09:28:47Z
RUNNER_END_UTC=2026-10-16T09:28:53Z
RUNNER_RC=0
POST_SNAPSHOT_UTC=2026-10-16T09:29:15Z
POST_STATE=Health:OK; HBM:3429/65536 MiB; AICore:0; no NPU3 process

CORRECTNESS=PASS 8/8 (Parent and Candidate for four cases)
LOCAL=RUN 4 cases x 21 paired samples
LOCAL_SCORE=NONE
OFFICIAL=NOT_RUN

| Case | Paired median (V016-V011, us) | P10 (us) | P90 (us) | P10-P90 |
|---|---:|---:|---:|---|
| fp16-tail-d12288 | -0.840 | -9.800 | 7.220 | crosses zero |
| fp16-wide-d32768 | -0.220 | -8.600 | 7.080 | crosses zero |
| bf16-tail-d12288 | +0.240 | -9.780 | 9.700 | crosses zero |
| bf16-wide-d32768 | -0.080 | -6.900 | 4.400 | crosses zero |

All four P10-P90 intervals cross zero. The paired medians are small and mixed in direction, so the data show no stable latency gain for deferred output-store synchronization. Correctness passed, but this local run does not support advancing a performance claim; LOCAL_SCORE remains NONE. Official was not run.

Evidence: `remote/pre-npu3-paired-snapshot.log`, `remote/npu3-paired.log`, `remote/post-npu3-paired-snapshot.log`. The raw paired output records all 21 pairs per case and both Parent/Candidate correctness results. The post snapshot also shows a process on NPU7; it was not used or touched.
