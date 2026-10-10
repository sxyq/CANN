# N03 V004 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V004
PARENT=945d5a29012a25ecbff01a9bf33a229b82f0a5ba
REJECTED_PREDECESSOR=V003
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V004/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V001/submission.asc
SOURCE_SHA256_BEFORE=e97152453e04037e2d638c4a3c4d59b03793660f0b37deb1aea36d76de9b8f97
SOURCE_SHA256_AFTER=815bdb2b150470c0152ca966a11894891449c9d6f80993b8750cbee67634f9bf

## Single change

Function: `ProcessWideLowPrecision`, BF16 Pass 1.

The two independent BF16 input conversions now issue in the opposite source order:
`residual -> FP32` is issued before `x -> FP32`. The following barrier, `Add`, square,
and `ReduceSum` sequence is unchanged. This changes only the source order of
independent Vector operations; it does not change tile/row traversal or any address
calculation.

Preserved facts: the same `batchRow` and `tile` mapping, `col`, `rowOffset`, valid
length, GM loads, output stores, event IDs, barriers, reduction slots, arithmetic,
memory traffic, synchronization, and compute amount.

## Mechanism boundary

N03 studies Vector instruction-level parallelism only. N05 Scalar address generation
is excluded. No new Runner, CMake input, or execution chain is introduced.

## Validation

COMPILE=PASS
REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V004
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
EXISTING_CMAKE=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V004/probe-src/CMakeLists.txt
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_SOURCE_SHA256=815bdb2b150470c0152ca966a11894891449c9d6f80993b8750cbee67634f9bf
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng && cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
COMPILE_ATTEMPT_1=FAIL; CONFIGURE_RC=0; PROBE_TARGETS_RC=2
COMPILE_ATTEMPT_1_ERROR=Host plugin link could not find the C++ header vector because the recorded CPLUS_INCLUDE_PATH was absent after environment initialization.
COMPILE_ATTEMPT_1_LOG=V004/support/compile-probe-v004-attempt1-env-fail.log
COMPILE_RETRY=PASS; CONFIGURE_RC=0; PROBE_TARGETS_RC=0
PROBE_V016=PASS; Candidate object and executable built
PROBE_V011=PASS; V011 Parent object and executable built
COMPILE_RETRY_LOG=V004/support/compile-probe-v004-retry.log
CANDIDATE_OBJECT_SHA256=7773d6b77f2d7c386bf24a043550e13c0317f666fee5041ff1e94ebf2c014a3f
PARENT_OBJECT_SHA256=ee911ef3908a977f9c781c67cbfea365fa00f03bdc3ea24fbaccac814ce076c4
CANDIDATE_ELF_SHA256=1adb5f71e18cd7eda49abec2bf96ea2d409196b49ecf8fed5b43318971839573
PARENT_ELF_SHA256=f9f564dbf3cc68d86fc1f9d091005959c05ba3b0078c11fbd2a784c5d429eaa5
CODEGEN_SUMMARY=V004/support/aicore-section-summary.txt
CANDIDATE_AICORE_BYTES=44544
PARENT_AICORE_BYTES=44248
AICORE_SECTION_BYTEWISE_DIFFERENT=YES
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
DEVICE_ID=NONE
FREE_HBM=NOT_COLLECTED; compile-only task
DEVICE_LOAD=NOT_COLLECTED; no device program started
OTHER_PROCESS_PRESENT=NOT_APPLICABLE
RESULT=Compile PASS after one environment-only retry; no device program was started.
GIT_COMMIT=RECORDED_IN_GIT_LOG
NEXT_ACTION=V004 source and compile evidence committed; Correctness and Local remain NOT_RUN/NONE.
