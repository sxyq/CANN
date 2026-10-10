# N03 V009 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V009
PARENT=eba546bd88884170720df150d77a498464a490d5
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V009/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V008/submission.asc
SOURCE_SHA256_BEFORE=79ccb6e0e55452bb5227f30510f1ed59fd73e55b753c3112fd2c44ec4c0a5f6b
SOURCE_SHA256_AFTER=1a1b5d42d9a06507bb0b9d06f21202c7bbf8748f7e031e388e0036d285de77cf
DIFF_SHA256=d630e399043816accf2cc53ad3a096065b4038b3e3aaef8a82b705f1ceb9775c

## Single change

Function: `ProcessWideLowPrecision`, FP16 Pass 1.

V009 swaps the issue order of two independent Vector operations after the
existing `Add(xLocal, xLocal, residualLocal, valid)`: `ToFloat(valueFp32,
xLocal, valid)` now precedes the retained-half `Muls(yTile, xLocal,
half(1.0f), valid)`. Both operations read `xLocal`, write different local
tiles, and retain their original `PIPE_V` barriers. The following square,
ReduceSum, output, and parameter operations are unchanged.

Tile and row traversal, `col`, `valid`, `rowWidth`, `batchBegin`, `batchRow`,
GM offsets, GM Load/Store calls, DMA/event operations, synchronization,
ownership, arithmetic operands, element counts, and all BF16 paths are
unchanged. V007 FP16 dual-row staging and V008 BF16 Pass2 cross-row
`Mul/Add` scheduling are not reused.

## Compile

COMPILE=PASS
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
COMPILE_ENV=SOC_VERSION=Ascend910B3; ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; existing V016 C++ and CANN include paths
CONFIGURE_RC=0
BUILD_RC=0
PROBE_V016=PASS; Candidate built and linked
PROBE_V011=PASS; Parent built and linked
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
COMPILE_LOG=V009/support/compile-v009-probe.log
COMPILE_LOG_SHA256=e22bd51333439b64c2c0854f4029d0c256800dd32b7d7ffe809677c3e6cff6c2
NO_RESET_COMPILE_LOG=V009/support/compile-v009-no-reset.log
NO_RESET_COMPILE_LOG_SHA256=1b8199feb65d438ed7879f18a4f8ddba3503a185529bfd8957e5b4a4a233e31c
NO_RESET_BUILD_RC=0

## Generated artifacts

CODEGEN_EXPORT_LOG=V009/support/codegen-v009-export.log
CODEGEN_EXPORT_LOG_SHA256=9e80a57a3997a3c7c62530996383bbdb05ea68720dfc94bb2a9dfac1979d82fc
CANDIDATE_BINARY_SHA256=054e9aaacc2cbace311f30c28ad77d980ba69f93e6eb6477101ee87ebbed2d20
PARENT_BINARY_SHA256=4cd71a69fb936b7d09d1dff7ef07d85d5637d656600654d2011fdbc4012a763a
CANDIDATE_AICORE_SHA256=b94a17f94d5a0711b59e9c450023a20080c1c0de2e4ea05f7adbc93d642d0258
PARENT_AICORE_SHA256=930caebea319960182c5a392c131103519c8d57acd8f7a7b1297ee3a92cfc2a5
CANDIDATE_AICORE_BYTES=45824
PARENT_AICORE_BYTES=44248
AICORE_OBJECT_DIFF=YES; V009/support/probe-v011-v016-object.diff; diff return code 1
AICORE_OBJECT_DIFF_SHA256=0367645cc7f94a3d47c7966b8813864a5e13f8a964b9013b7f3874e9067419d4

## No-reset runtime

NO_RESET_RUNTIME=TEMPORARY_EXISTING_V016_ADAPTATION_ONLY
NO_RESET_RUNTIME_SOURCE=V006/support/runtime_probe-v006-no-reset.cpp
NO_RESET_RUNTIME_SHA256=2661154f14214c244793309dda0ec30e390906e046e2c2b65f73f9b2bf560581
NO_RESET_LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
NO_RESET_LD_LIBRARY_PATH_PREFIX=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11
NO_RESET_CANDIDATE_RESET_SYMBOL=ABSENT
NO_RESET_PARENT_RESET_SYMBOL=ABSENT
ORIGINAL_RESET_RUNTIME=NOT_RUN

## Correctness

CORRECTNESS=PASS
CORRECTNESS_COMMAND=no-reset-build/probe_v016 and probe_v011 1 16 12288 6 0 0
CORRECTNESS_LOG=V009/support/correctness-v009-no-reset.log
CORRECTNESS_LOG_SHA256=727ee417a73b787ecd1ead09a03802052ec79078573fd62bc27414bac71b788b
CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.001953125; max_rel=0.052631579
CORRECTNESS_DEVICE=6
CORRECTNESS_DEVICE_PROCESS=VLLMWorker_TP PID 3357697; process untouched

## Local

LOCAL_EXECUTION=PASS
LOCAL_COMMAND=two interleaved Parent/Candidate pairs; dtype=FP16 rows=16 width=12288 warmup=5 repeats=5
LOCAL_METHOD=host steady_clock median from the existing no-reset probe
LOCAL_LOG=V009/support/local-v009-no-reset.log
LOCAL_LOG_SHA256=7b8880c193ed7f120fa51ebeb7fe1cae49ff10f16b73ce8a35ffc6f8170b5c17
LOCAL_PAIR_1_PARENT_MEDIAN_US=75.880
LOCAL_PAIR_1_CANDIDATE_MEDIAN_US=74.010
LOCAL_PAIR_2_PARENT_MEDIAN_US=124.561
LOCAL_PAIR_2_CANDIDATE_MEDIAN_US=168.110
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOCAL_REASON=Only two paired samples; NPU6 HBM usage remained 91% and VLLMWorker_TP PID 3357697 remained present.
DEVICE_ID=6
FREE_HBM=NOT_EXACT; HBM capacity 65536 MB; usage rate 91%; proc-mem 56355 MB
DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=3% before and 6% after
AICORE_LOAD=0%
OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697
LOCAL_USAGE_SNAPSHOT_COMMAND=npu-smi info -t usages -i 6
LOCAL_USAGE_SNAPSHOT_RC=0

REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V009
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
RESULT=Compile PASS; FP16 Candidate/Parent Correctness PASS; Local execution completed with LOCAL_SCORE=NONE.
NEXT_ACTION=V009 closed; no additional samples or mechanism changes in this revision.

## Requalification

REQUAL_RUN_TIME=2026-10-10T20:54:30Z
REQUAL_SOURCE_SHA256=1a1b5d42d9a06507bb0b9d06f21202c7bbf8748f7e031e388e0036d285de77cf
REQUAL_REMOTE_HOST=hwnput3
REQUAL_REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V009/no-reset-build
REQUAL_COMMAND_CORRECTNESS=./probe_v016 1 16 12288 6 0 0; ./probe_v011 1 16 12288 6 0 0
REQUAL_COMMAND_LOCAL=two interleaved pairs; ./probe_v011 1 16 12288 6 5 5 and ./probe_v016 1 16 12288 6 5 5
REQUAL_DEVICE_ID=6
REQUAL_SOC_VERSION=(empty after source /usr/local/Ascend/ascend-toolkit/set_env.sh)
REQUAL_ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
REQUAL_LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
REQUAL_LD_LIBRARY_PATH_PREFIX=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11
REQUAL_ENV_SOURCE_RC=0
REQUAL_RESET_RUNTIME=NOT_USED; temporary no-reset adaptation retained
REQUAL_CORRECTNESS=PASS
REQUAL_CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.001953125; max_rel=0.052631579
REQUAL_CORRECTNESS_LOG=V009/support/requal-v009-no-reset.log
REQUAL_CORRECTNESS_LOG_SHA256=8024aa278e4164f495e8dccabc3e1a910226f04d220da5ab9b95e4c7ceae572d
REQUAL_CORRECTNESS_RC=0
REQUAL_LOCAL_EXECUTION=PASS
REQUAL_LOCAL_LOG=V009/support/requal-v009-no-reset.log
REQUAL_LOCAL_LOG_SHA256=8024aa278e4164f495e8dccabc3e1a910226f04d220da5ab9b95e4c7ceae572d
REQUAL_LOCAL_PAIR_COUNT=2
REQUAL_PAIR_1_PARENT_MEDIAN_US=154.331
REQUAL_PAIR_1_CANDIDATE_MEDIAN_US=122.451
REQUAL_PAIR_2_PARENT_MEDIAN_US=76.351
REQUAL_PAIR_2_CANDIDATE_MEDIAN_US=105.040
REQUAL_LOCAL_SCORE=NONE
REQUAL_LOCAL_DELTA=NONE
REQUAL_LOCAL_REASON=Two paired samples only; NPU6 HBM usage remained 91% and VLLMWorker_TP PID 3357697 remained present.
REQUAL_FREE_HBM=NOT_EXACT; HBM capacity 65536 MB; usage rate 91%; proc-mem 56355 MB
REQUAL_DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=3% before and 14% after
REQUAL_AICORE_LOAD=0%
REQUAL_OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697; process untouched
REQUAL_USAGE_SNAPSHOT_COMMAND=npu-smi info -t usages -i 6; npu-smi info -t proc-mem -i 6
REQUAL_USAGE_SNAPSHOT_RC=0
REQUAL_LOCAL_RESULT=Execution completed with two paired samples; LOCAL_SCORE=NONE due to sample count and load.
REQUAL_NEXT_ACTION=V009 requalification closed; next revision may study one new independent Vector ILP emission-order factor without changing addresses, DMA, synchronization, ownership, Scalar indexing, or arithmetic.
