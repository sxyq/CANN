# N03 V006 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V006
PARENT=2c2c9c678a2e52e2751785d8aae3f87c029c40fd
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V006/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V005/submission.asc
SOURCE_SHA256_BEFORE=13a0d610cad47fa920d704b10b4f66efa9eabe0e7b242ec3aab3f612c62d22f1
SOURCE_SHA256_AFTER=7878c225e01b20f8ab8e1bad0bfb52222532e841520c115ce7aa5e9bcc29cbe1

## Single change

Function: `ProcessWideLowPrecision`, BF16 two-row Pass1 and Pass2.

V006 keeps the existing tile and row ownership while making the retained two-row
path an explicit multi-stage Vector schedule. Pass1 widens and adds row zero,
then forms row one before the two retained value tiles run their square and
partial-reduction stages. Pass2 executes the two retained rows through six
explicit Vector stages: row-zero scale, row-one scale, row-zero multiply,
row-zero add, row-one multiply, and row-one add. The Vector operations and
element counts remain the same as the parent schedule.

The existing `col`, `valid`, `rowWidth`, `rowOffset`, `batchRow`, GM offsets,
tile loop, GM Load/Store calls, DMA/event operations, barriers, reduction slot
mapping, arithmetic operands, and output order are unchanged. The change only
organizes independent Vector dependencies and their issue order. No Scalar
address cursor, traversal change, cache/streaming mechanism, helper-only code,
or dead code was introduced.

## Mechanism boundary

N03 studies Vector instruction-level parallelism only. N05 Scalar address
generation and N04 GM/L2 cache streaming are excluded. No Runner or new CMake
input was created.

## Validation

COMPILE=PASS
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng; cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
CONFIGURE_RC=0
BUILD_RC=0
PROBE_V016=PASS; Candidate built and linked
PROBE_V011=PASS; Parent built and linked
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_SOURCE_SHA256=7878c225e01b20f8ab8e1bad0bfb52222532e841520c115ce7aa5e9bcc29cbe1
COMPILE_LOG=V006/support/compile-probe-v006.log
COMPILE_LOG_SHA256=0515395f24a5a8af882b6f1eb6650dc7e71a47f36fae96d0c457618b24899fee
CODEGEN_EXPORT_LOG=V006/support/codegen-export-v006.log
CODEGEN_EXPORT_LOG_SHA256=388890b39fffa0f5f13dee034eb0c5f4f8bffdbbc9414ae4e798588ed2fefbda
PARENT_INPUT=/home/data4t2/lelinfeng/phase4-workspaces/R31B
EXISTING_CMAKE=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V006/probe-src/CMakeLists.txt
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
CANDIDATE_OBJECT_SHA256=02e7e98595cd16ee15be37fa1a1294a5d7ed9349ea28061f79f3c0a879dc5d81
PARENT_OBJECT_SHA256=ee911ef3908a977f9c781c67cbfea365fa00f03bdc3ea24fbaccac814ce076c4
CANDIDATE_OBJECT_BYTES=158192
PARENT_OBJECT_BYTES=156144
CANDIDATE_ELF_SHA256=54ebcfe60cf52fa3941e567eb2db1a48cc6350d9b88366ae15dac86997c331f0
PARENT_ELF_SHA256=f9f564dbf3cc68d86fc1f9d091005959c05ba3b0078c11fbd2a784c5d429eaa5
CANDIDATE_AICORE_SHA256=d9dfbf18fe725c2b4b8e44f698b9a7e315616d77bd4574c8809b2f1a9059e49d
PARENT_AICORE_SHA256=930caebea319960182c5a392c131103519c8d57acd8f7a7b1297ee3a92cfc2a5
CANDIDATE_AICORE_BYTES=45272
PARENT_AICORE_BYTES=44248
AICORE_OBJECT_DIFF=YES; V006/support/probe-v011-v016-object.diff; diff return code 1
NO_RESET_ADAPTATION=TEMPORARY_EXISTING_V016_PROBE_COPY; only the runtime reset call was removed
NO_RESET_ORIGINAL_RUNTIME_SHA256=e57518d3027097bc398f367486efdb46187de37d5d59fd26b43ed748feb53570
NO_RESET_ADAPTED_RUNTIME_SHA256=2661154f14214c244793309dda0ec30e390906e046e2c2b65f73f9b2bf560581
NO_RESET_ADAPTATION_DIFF=V006/support/runtime_probe-v006-no-reset.diff; one aclrtResetDevice line removed
NO_RESET_ADAPTER_COMPILE=PASS
NO_RESET_ADAPTER_COMPILE_LOG=V006/support/compile-no-reset-adapter-v006.log
NO_RESET_ADAPTER_COMPILE_LOG_SHA256=03ecdc946acd8ed98ea57ba92bc10406e6f35bf06d7bf937ee542a01c9d94d83
NO_RESET_CANDIDATE_BINARY_SHA256=473dfcf1a4ecea25ece490c76caa83025626d1fb9b60bae3c8b8b0236c6f2261
NO_RESET_PARENT_BINARY_SHA256=4cd71a69fb936b7d09e1dff7ef07d85d5637d656600654d2011fdbc4012a763a
FIRST_CORRECTNESS_ATTEMPT=RUNTIME_ERROR; libstdc++.so.6 lacked GLIBCXX_3.4.29 before device launch
FIRST_CORRECTNESS_LOG=V006/support/correctness-v006-no-reset.log
FIRST_CORRECTNESS_LOG_SHA256=255124b7b6f5d461ebfe2dc2472163c5f3562a1a00ad23f1460cb00bf2d2d6f6
CORRECTNESS=PASS
CORRECTNESS_COMMAND=probe_v016 and probe_v011 2 16 12288 6 0 0 with no-reset adaptation
CORRECTNESS_LOG=V006/support/correctness-v006-no-reset-libstdcxx-retry.log
CORRECTNESS_LOG_SHA256=47a6ffe48e1df997f025578230c78d7f183a7a26121d44fd674d041936eee2e8
CORRECTNESS_RESULT=Candidate and Parent PASS; failures=0 for both; max_abs=0.0078125; max_rel=0.017964073
LOCAL_EXECUTION=PASS
LOCAL_METHOD=three interleaved Parent/Candidate pairs; host steady_clock median; warmup=5; repeats=5
LOCAL_LOG=V006/support/local-v006-no-reset.log
LOCAL_LOG_SHA256=0a7faeb8a410f6170a9cce61ae9f8363d985589507bd8b101a76f959224a5e23
LOCAL_SCORE=NONE
LOCAL_REASON=Three paired samples completed, but sample count is insufficient for a score and NPU6 retained a VLLMWorker_TP process at HBM usage rate 91%.
DEVICE_ID=6
FREE_HBM=NOT_EXACT; HBM capacity 65536 MB, usage rate 91%, proc-mem reports 56358 MB for PID 3357697
DEVICE_LOAD=HBM_USAGE_RATE=91%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; CTRLCPU_USAGE_RATE=6%
AICORE_LOAD=0%
OTHER_PROCESS_PRESENT=YES; VLLMWorker_TP PID 3357697
NPU_SNAPSHOT=V006/support/npu6-snapshot-v006.log
NPU_SNAPSHOT_SHA256=55cd604c6f0f9604e22d0133e9a3179566e5818165f570d478fa2d1f0b2e4e63
NPU6_PRE_CORRECTNESS_SNAPSHOT=V006/support/npu6-precorrectness-v006.log
NPU6_PRE_CORRECTNESS_SNAPSHOT_SHA256=82f57efde47906abdf83880ff61f82b255447884a467bae853fdaf97b88475dc
NPU6_FIRST_FAILURE_POST_SNAPSHOT=V006/support/npu6-post-correctness-failure-v006.log
NPU6_FIRST_FAILURE_POST_SNAPSHOT_SHA256=eb5a91a72d6353d6c923c22727c19c59a47ece773029d8ba0aca9824ce62a039
NPU_SMI_PROC=UNSUPPORTED; npu-smi reports that device 6 does not support proc info; proc-mem was available
RESET_ENTRY_EVIDENCE=V006/support/no-reset-entry-evidence-v006.txt
REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V006
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_PROGRAM=NO_RESET_TEMPORARY_COPY_ONLY; original reset binaries were not run
RESULT=Compile PASS; no-reset temporary probe Correctness PASS for Candidate and Parent; Local execution completed with score NONE.
NEXT_ACTION=No additional V006 samples or mechanism changes in this revision.
