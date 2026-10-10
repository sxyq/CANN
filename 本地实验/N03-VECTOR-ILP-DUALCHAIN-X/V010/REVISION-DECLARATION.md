# N03 V010 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V010
PARENT=a68102576d5ee991939235a5affd6e82fbfbeee9
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V010/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V009/submission.asc
SOURCE_SHA256_BEFORE=1a1b5d42d9a06507bb0b9d06f21202c7bbf8748f7e031e388e0036d285de77cf
SOURCE_SHA256_AFTER=852b0ec6cbfd7f3f847176737384eda3d2e5d196dd0443892b60cda9aaa25f7b
DIFF_SHA256=a326281e022caee9a7d0200959401359639247c8c7ad5e207daa61c8102cf49e

## Single change

Function: `ProcessWideLowPrecision`, FP16 Pass 1, one `(batchRow, tile)` unit.

After the existing `Add(xLocal, xLocal, residualLocal, valid)` and
`ToFloat(valueFp32, xLocal, valid)`, V010 changes one independent Vector issue
pair from `Muls(yTile, xLocal, 1)` then `Mul(xFp32, valueFp32, valueFp32)` to
`Mul(xFp32, valueFp32, valueFp32)` then `Muls(yTile, xLocal, 1)`. `Mul` reads
the widened value tile and writes the square tile; `Muls` reads `xLocal` and
writes the retained half tile. They have disjoint Vector inputs and outputs.
The existing `PIPE_V` barriers, following `ReduceSum`, retained y tile, and
all element counts are unchanged.

Tile/Row traversal, `col`, `valid`, `rowWidth`, `batchBegin`, `batchRow`, GM
offsets, GM Load/Store calls, DMA/event operations, synchronization, ownership,
Scalar indexing, arithmetic operands, output order, BF16 paths, V007 FP16
dual-row staging, V008 BF16 Pass2 scheduling, and V009 `ToFloat/Muls` ordering
are unchanged. No N05 address mechanism, N01 FP32 lifetime change, N04
GM/L2 change, Runner, CMake input, or runtime reset path was added.

## Compile

COMPILE=PASS
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
NO_RESET_COMPILE_COMMAND=cmake -S no-reset-src -B no-reset-build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build no-reset-build --target probe_v016 probe_v011 --parallel 1
COMPILE_RETRY_ENV=CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include; CPATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include; ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; source_rc=0
COMPILE_CMAKE_VERSION=cmake version 3.22.1
COMPILE_SOC_VERSION=Ascend910B3
COMPILE_EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
COMPILE_FIRST_RESULT=FAIL; standard configure rc=0; standard build rc=2; no-reset configure rc=0; no-reset build rc=2
COMPILE_FIRST_CAUSE=toolchain plugin registration compile could not find `<vector>`; raw log retained
COMPILE_FIRST_LOG=V010/support/compile-v010-probe.log
COMPILE_FIRST_LOG_SHA256=9137843c0774c33419a6929f6f0e084c7b3a0898502706d128ef5ef817180ff8
COMPILE_RETRY_RESULT=PASS; standard configure/build rc=0/0; no-reset configure/build rc=0/0
COMPILE_RETRY_LOG=V010/support/compile-v010-retry.log
COMPILE_RETRY_LOG_SHA256=06a7926c87404257bbf5a674f4857eaa7a1c5cbc9a858776b47c1551f1d563e3
PROBE_V016=PASS; Candidate built and linked
PROBE_V011=PASS; Parent built and linked
NO_RESET_PROBE_V016=PASS; Candidate built and linked
NO_RESET_PROBE_V011=PASS; Parent built and linked
CANDIDATE_BINARY_SHA256=3d3bddc9636f234f265350f12367b18c9278651f97b731e88bb4046590ad4aa2
PARENT_BINARY_SHA256=c5f855f4a729333f811a48906b3de3d9f560819bfdb6710f802a3607bca40d6c
NO_RESET_CANDIDATE_RESET_SYMBOL=ABSENT
NO_RESET_PARENT_RESET_SYMBOL=ABSENT

## Correctness

CORRECTNESS=PASS
CORRECTNESS_COMMAND=no-reset-build/probe_v016 and probe_v011 1 16 12288 6 0 0
CORRECTNESS_LOG=V010/support/correctness-v010-no-reset.log
CORRECTNESS_LOG_SHA256=0767c418948e4e83b5362c62d747c6fd73c5950320f1cdbd6cbca66d067ef2cb
CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.001953125; max_rel=0.052631579
CORRECTNESS_DEVICE=6
CORRECTNESS_DEVICE_PROCESS=VLLMWorker_TP PID 3357697; process untouched

## Local

LOCAL_EXECUTION=PASS
LOCAL_COMMAND=two interleaved Parent/Candidate pairs; dtype=FP16 rows=16 width=12288 warmup=5 repeats=5
LOCAL_METHOD=host steady_clock median from the existing no-reset probe
LOCAL_LOG=V010/support/local-v010-no-reset.log
LOCAL_LOG_SHA256=f548e35bc345ff05d503bf77b98ec3b3b44f9d7c198842f25d76292d6fe178fe
LOCAL_PAIR_COUNT=2
LOCAL_PAIR_1_PARENT_MEDIAN_US=149.033
LOCAL_PAIR_1_CANDIDATE_MEDIAN_US=197.633
LOCAL_PAIR_2_PARENT_MEDIAN_US=103.991
LOCAL_PAIR_2_CANDIDATE_MEDIAN_US=104.372
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOCAL_REASON=Only two paired samples; NPU6 HBM usage remained 91% and VLLMWorker_TP PID 3357697 remained present.
DEVICE_ID=6
FREE_HBM=NOT_EXACT; HBM capacity 65536 MB; usage rate 91%; proc-mem 56355 MB
DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=6% before and after
AICORE_LOAD=0%
OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697; process untouched
LOCAL_USAGE_SNAPSHOT_COMMAND=npu-smi info -t usages -i 6; npu-smi info -t proc-mem -i 6
LOCAL_USAGE_SNAPSHOT_RC=0

REMOTE_HOST=hwnput3
REMOTE_COMPILE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V010
REMOTE_RUNTIME_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V010/no-reset-build
RUNTIME_SOC_VERSION=(empty after source /usr/local/Ascend/ascend-toolkit/set_env.sh; CMake SOC_VERSION=Ascend910B3)
RUNTIME_ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
RUNTIME_LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
RUNTIME_LD_LIBRARY_PATH_PREFIX=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11
RESULT=Compile PASS after existing environment include-path repair; FP16 Candidate/Parent Correctness PASS; Local execution completed with LOCAL_SCORE=NONE.
NEXT_ACTION=V010 evidence is ready for local commit; next revision may study one different independent Vector issue-order factor without changing addresses, DMA, synchronization, ownership, Scalar indexing, or arithmetic.
