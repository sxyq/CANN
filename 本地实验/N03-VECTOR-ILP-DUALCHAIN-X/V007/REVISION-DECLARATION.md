# N03 V007 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V007
PARENT=989b9b1973ce9cb86523fa692747dbe3696d3e88
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V007/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V006/submission.asc
SOURCE_SHA256_BEFORE=7878c225e01b20f8ab8e1bad0bfb52222532e841520c115ce7aa5e9bcc29cbe1
SOURCE_SHA256_AFTER=f4c88d720f27466003003d87c2a4468a12932042537003822f9e7642cbdf9ea1

## Single change

Function: `ProcessWideLowPrecision`, FP16 Pass 2, `batchRows == 2`.

V007 keeps the existing FP16 row and tile path but stages the two independent
rows through the existing FP32 scratch tiles before draining either row. It
executes row zero and row one `ToFloat`, then row zero and row one `Muls`, and
then retains the original per-row `FromFloat`, gamma `Mul`, bias `Add`, and
Store sequence. The non-two-row FP16 path is unchanged.

The existing `col`, `valid`, `rowWidth`, `batchBegin`, `batchRow`, GM offsets,
tile loop, GM Load/Store calls, DMA/event operations, ownership, arithmetic
operands, output order, and `SyncVToMTE2`/`SyncVToMTE3`/`SyncMTE3ToV` calls are
unchanged. The dual-row branch keeps the same ten `PIPE_V` barriers and the
same two output stores; only independent Vector dependency and issue order
changes. V006 BF16 scheduling is retained as the parent and is not modified.

## Mechanism boundary

N03 studies Vector instruction-level parallelism only. N05 Scalar address
generation and N04 GM/L2 cache streaming are excluded. No formal Runner or new
CMake input was created; the existing V016 probe entry is reused.

## Validation

COMPILE=PASS
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
CONFIGURE_RC=0
BUILD_RC=0
PROBE_V016=PASS; Candidate built and linked
PROBE_V011=PASS; Parent built and linked
COMPILE_SOURCE_SHA256=f4c88d720f27466003003d87c2a4468a12932042537003822f9e7642cbdf9ea1
COMPILE_LOG=V007/support/compile-v007-probe.log
COMPILE_LOG_SHA256=7c756cfdf35c031c230979088290f0f0351cb192572ada4f90f535f5c6aec6f0
CODEGEN_EXPORT_LOG=V007/support/codegen-v007-export.log
CODEGEN_EXPORT_LOG_SHA256=aebe52de9d45c080412aa392039eab60763fc8626cb40ec264de9dbf017764f9
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
CANDIDATE_OBJECT_SHA256=e5ce46584e28bbb9b8a3b83b338ba5fe85a87534a5c98c5a1fe3f02b2acabf77
PARENT_OBJECT_SHA256=b3ffccdd6316a0c5eae67f4a278faba1fcb8d2aca262d05b18e3302ada0f0a25
CANDIDATE_OBJECT_BYTES=158952
PARENT_OBJECT_BYTES=155840
CANDIDATE_ELF_SHA256=c117580570eb24385f09bf53921eb577239d76b10f599c39a942d4d1150bfbb0
PARENT_ELF_SHA256=4cd71a69fb936b7d09e1dff7ef07d85d5637d656600654d2011fdbc4012a763a
CANDIDATE_AICORE_SHA256=b043f645faf04c9f8c2d589fb62e864a3ecfe1ab7f21e41b91dd56c7602afb23
PARENT_AICORE_SHA256=930caebea319960182c5a392c131103519c8d57acd8f7a7b1297ee3a92cfc2a5
CANDIDATE_AICORE_BYTES=45808
PARENT_AICORE_BYTES=44248
AICORE_OBJECT_DIFF=YES; V007/support/probe-v011-v016-object.diff; diff return code 1
NO_RESET_RUNTIME_SHA256=2661154f14214c244793309dda0ec30e390906e046e2c2b65f73f9b2bf560581
NO_RESET_RUNTIME_SOURCE=V006/support/runtime_probe-v006-no-reset.cpp reused for the V007 temporary probe copy
NO_RESET_LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
NO_RESET_LD_LIBRARY_PATH_PREFIX=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11
NO_RESET_RUNTIME=PASS; both compiled probe binaries have no aclrtResetDevice symbol
CORRECTNESS=PASS
CORRECTNESS_COMMAND=probe_v016 and probe_v011 1 16 12288 6 0 0 with temporary no-reset runtime
CORRECTNESS_LOG=V007/support/correctness-v007-no-reset.log
CORRECTNESS_LOG_SHA256=f8224431ff6568067c5cf447b288bb5dc113db97eab0ce53fd496e0eaf887c98
CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.001953125; max_rel=0.052631579
LOCAL_EXECUTION=PASS
LOCAL_COMMAND=two interleaved Parent/Candidate pairs; dtype=FP16 rows=16 width=12288 warmup=5 repeats=5
LOCAL_METHOD=host steady_clock median from the existing no-reset probe
LOCAL_LOG=V007/support/local-v007-no-reset.log
LOCAL_LOG_SHA256=e3a3b74537e30827f847b645aadb114fcc6f5c68cb27d172e9801e81d87df263
LOCAL_SCORE=NONE
DEVICE_ID=6
FREE_HBM=NOT_EXACT; HBM capacity 65536 MB, usage rate 91%, proc-mem reports 56355 MB for PID 3357697
DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=6%
AICORE_LOAD=0%
OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697
NPU6_PRE_CORRECTNESS_SNAPSHOT=V007/support/npu6-precorrectness-v007.log
NPU6_PRE_CORRECTNESS_SNAPSHOT_SHA256=64cf4604bbaafe51d598a852aa9aa9a6e33873d456d67962c9bcece8d44cc5f2
LOCAL_REASON=Two paired samples completed, but sample count is insufficient for a score and NPU6 retained a VLLMWorker_TP process at HBM usage rate 91%.
REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V007
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
EXISTING_CMAKE=V007/probe-src/CMakeLists.txt copied from the existing V016 input
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
DEVICE_PROGRAM=TEMPORARY_NO_RESET_COPY_ONLY; original reset runtime was not run
RESULT=Compile PASS; Candidate/Parent FP16 Correctness PASS; Local execution completed with score NONE.
NEXT_ACTION=No additional V007 samples or mechanism changes in this revision.
