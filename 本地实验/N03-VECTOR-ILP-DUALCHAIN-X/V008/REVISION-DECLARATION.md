# N03 V008 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V008
PARENT=4d23cb76af7d5171403de629f796667d27b3a805
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V008/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V007/submission.asc
SOURCE_SHA256_BEFORE=f4c88d720f27466003003d87c2a4468a12932042537003822f9e7642cbdf9ea1
SOURCE_SHA256_AFTER=79ccb6e0e55452bb5227f30510f1ed59fd73e55b753c3112fd2c44ec4c0a5f6b
DIFF_SHA256=944a2f8bf51e46a56dbc1f380a34be4c785866d83ab90b2222a29b809f876505

## Single change

Function: `ProcessWideLowPrecision`, BF16 Pass 2, `batchRows == 2`.

V008 changes only the cross-row Vector issue order in the existing six-stage
schedule. The order is now `Muls(row0), Muls(row1), Mul(row0), Mul(row1),
Add(row0), Add(row1)`. Each row retains its original `Muls -> Mul -> Add`
dependency and the same arithmetic operands, element counts, output order, and
barrier count. This is distinct from V007, whose change was the FP16 dual-row
staging path.

The existing tile and row traversal, `col`, `valid`, `rowWidth`, `batchBegin`,
`batchRow`, GM offsets, GM Load/Store calls, DMA/event operations, synchronization,
ownership, and non-BF16 paths are unchanged. No N05 Scalar address mechanism,
new Runner, or new CMake input was created.

## Compile

COMPILE=PASS
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
COMPILE_ENV=SOC_VERSION=Ascend910B3; ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; CANN include paths from the existing V016 input
CONFIGURE_RC=0
BUILD_RC=0
PROBE_V016=PASS; Candidate built and linked
PROBE_V011=PASS; Parent built and linked
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
COMPILE_LOG_FIRST=V008/support/compile-v008-probe.log; BUILD_RC=2; fatal error: vector file not found
COMPILE_LOG_RETRY=V008/support/compile-v008-retry.log; BUILD_RC=2; same existing-entry include failure retained
COMPILE_LOG_FINAL=V008/support/compile-v008-retry2.log
COMPILE_LOG_FINAL_SHA256=4b9a600ccc6ffaa355e840db124f14af8a632fc3214295c9433cc3efa06701d3
NO_RESET_COMPILE_LOG_FIRST=V008/support/compile-v008-no-reset.log
NO_RESET_COMPILE_LOG_FINAL=V008/support/compile-v008-no-reset-final.log
NO_RESET_COMPILE_LOG_FINAL_SHA256=17349d4c76be480a2853fd6aeaeb99a6cb33bd3193941b5eb10738ede0a05092
NO_RESET_BUILD_RC=0

## Generated artifacts

CODEGEN_EXPORT_LOG=V008/support/codegen-v008-export.log
CODEGEN_EXPORT_LOG_SHA256=2cbd2fd8c37a2408908b18575f732424e597b1154ddb92d301f32ea64304fec2
CANDIDATE_BINARY_SHA256=a9ac12a5d2f18b912de51b748a2871fa04900a1ed964e7a5bebbb8a4e055499a
PARENT_BINARY_SHA256=4cd71a69fb936b7d09e1dff7ef07d85d5637d656600654d2011fdbc4012a763a
CANDIDATE_AICORE_SHA256=3989d0830bceff53a2b4069471fbc9660f57a31d07e7bee6b87cc00eed0ceea3
PARENT_AICORE_SHA256=930caebea319960182c5a392c131103519c8d57acd8f7a7b1297ee3a92cfc2a5
CANDIDATE_AICORE_BYTES=45816
PARENT_AICORE_BYTES=44248
AICORE_OBJECT_DIFF=YES; V008/support/probe-v011-v016-object.diff; diff return code 1
AICORE_OBJECT_DIFF_SHA256=179ab34b7f1128351ef01ffa2dddfc4878cb5ee269c53193b4c21cdb0e25cd90

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
CORRECTNESS_COMMAND=no-reset-build/probe_v016 and probe_v011 2 16 12288 6 0 0
CORRECTNESS_LOG=V008/support/correctness-v008-no-reset.log
CORRECTNESS_LOG_SHA256=651ddde2f9646125bccea741f76a59f296145a1c896dd2ecd024e6c3bbf80093
CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.0078125; max_rel=0.017964073
CORRECTNESS_DEVICE=6
CORRECTNESS_DEVICE_PROCESS=VLLMWorker_TP PID 3357697; proc-mem snapshot available; process untouched

## Local

LOCAL_EXECUTION=PASS
LOCAL_COMMAND=two interleaved Parent/Candidate pairs; dtype=BF16 rows=16 width=12288 warmup=5 repeats=5
LOCAL_METHOD=host steady_clock median from the existing no-reset probe
LOCAL_LOG=V008/support/local-v008-no-reset.log
LOCAL_LOG_SHA256=600bc5bf9608bd3c69f2778373c20314411112d848c121277beab7ab4409afd9
LOCAL_PAIR_1_PARENT_MEDIAN_US=89.971
LOCAL_PAIR_1_CANDIDATE_MEDIAN_US=30.901
LOCAL_PAIR_2_PARENT_MEDIAN_US=32.000
LOCAL_PAIR_2_CANDIDATE_MEDIAN_US=132.102
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOCAL_REASON=Only two paired samples; NPU6 HBM usage remained 91% and VLLMWorker_TP PID 3357697 remained present.
DEVICE_ID=6
FREE_HBM=NOT_EXACT; HBM capacity 65536 MB; usage rate 91%; proc-mem 56355 MB
DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=4% before and 5% after
AICORE_LOAD=0%
OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697
LOCAL_USAGE_SNAPSHOT_COMMAND=npu-smi info -t usages -i 6
LOCAL_USAGE_SNAPSHOT_RC=0

REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V008
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
RESULT=Compile PASS; BF16 Candidate/Parent Correctness PASS; Local execution completed with LOCAL_SCORE=NONE.
NEXT_ACTION=V008 closed; no additional samples or mechanism changes in this revision.
