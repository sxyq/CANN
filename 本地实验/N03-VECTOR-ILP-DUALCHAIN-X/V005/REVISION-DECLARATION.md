# N03 V005 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V005
PARENT=24e543ca7cf836c661fcd9d60240554c5b53f085
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V005/submission.asc
SOURCE_PARENT_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V004/submission.asc
SOURCE_SHA256_BEFORE=815bdb2b150470c0152ca966a11894891449c9d6f80993b8750cbee67634f9bf
SOURCE_SHA256_AFTER=13a0d610cad47fa920d704b10b4f66efa9eabe0e7b242ec3aab3f612c62d22f1

## Single change

Function: `ProcessWideLowPrecision`, BF16 Pass 2, `batchRows == 2`.

V005 replaces the previous grouped two-row Vector order with a direct three-stage
dual-chain microkernel. The startup stage issues both independent `Muls` operations;
the steady stage completes row zero with `Mul` then `Add`; the drain stage completes
row one with `Mul` then `Add`. The six arithmetic Vector operations and their element
counts are unchanged.

The two value tiles remain `valueFp32[col]` and `valueFp32[rowWidth + col]`. The
outer tile loop, `col`, `valid`, parameter conversion, GM `Load`/`Store`, Scalar row
and tile derivation, event IDs, event calls, barriers, output order, arithmetic, and
memory traffic are unchanged. V003 traversal/index changes are not present.

## Mechanism boundary

N03 studies Vector instruction-level parallelism only. N05 Scalar address generation
and tile traversal are excluded. No Runner or CMake input was created.

## Validation

COMPILE=PASS
REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V005
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
EXISTING_CMAKE=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V005/probe-src/CMakeLists.txt
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_SOURCE_SHA256=13a0d610cad47fa920d704b10b4f66efa9eabe0e7b242ec3aab3f612c62d22f1
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng && cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
CONFIGURE_RC=0
PROBE_TARGETS_RC=0
PROBE_V016=PASS; Candidate object and executable built
PROBE_V011=PASS; V011 Parent object and executable built
COMPILE_LOG=V005/support/compile-probe-v005.log
CANDIDATE_OBJECT_SHA256=d3d901818e1c826eff47d36973be1f4ad599eb6706996e90427da164eddd1ee7
PARENT_OBJECT_SHA256=ee911ef3908a977f9c781c67cbfea365fa00f03bdc3ea24fbaccac814ce076c4
CANDIDATE_ELF_SHA256=b74a2eab74e0033d918b93b45f622e092021aba78dd83c95b860b7d68d4f6de2
PARENT_ELF_SHA256=f9f564dbf3cc68d86fc1f9d091005959c05ba3b0078c11fbd2a784c5d429eaa5
CODEGEN_SUMMARY=V005/support/aicore-section-summary.txt
CANDIDATE_AICORE_SHA256=c6d12ee51ac64f982ef5411bfbe85fc519dc195027442d927bfb8e08fb2f6320
PARENT_AICORE_SHA256=930caebea319960182c5a392c131103519c8d57acd8f7a7b1297ee3a92cfc2a5
CANDIDATE_AICORE_BYTES=44544
PARENT_AICORE_BYTES=44248
AICORE_SECTION_BYTEWISE_DIFFERENT=YES
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
DEVICE_ID=NONE
FREE_HBM=NOT_COLLECTED; compile-only task
DEVICE_LOAD=NOT_COLLECTED; no device program started
OTHER_PROCESS_PRESENT=NOT_APPLICABLE
RESULT=Compile PASS; no device program was started.
GIT_COMMIT=RECORDED_IN_GIT_LOG
NEXT_ACTION=V005 source and Compile evidence ready for local commit; Correctness and Local remain NOT_RUN/NONE.
