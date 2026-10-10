# N03 V002 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V002
PARENT=4e9c8e182ab592745949625df1d07972cbfe1dd2
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16

## Source identity

SOURCE_PATH=../V001/submission.asc
SOURCE_SHA256=e97152453e04037e2d638c4a3c4d59b03793660f0b37deb1aea36d76de9b8f97
SOURCE_CHANGE=NONE

## Single change

CHANGE=Reuse the existing R31B V016 probe build entry with its expected CMakeLists.txt name and recorded CANN host environment.
EXISTING_CMAKE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/probe-src/CMakeLists.txt
EXISTING_CMAKE_SHA256=8ce59cfe81c7db66642cc8801aa3d0174c76d2b266f0251c296a6879b93c0259
EXPECTED_KERNEL_FILE=R31B-V016-WIDE-TILE-SEED_kernel.asc
LINK_ENTRY=tiling_api register platform unified_dlog graph_base with system libstdc++.so.6, dl, and m
ENV_ENTRY=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled; use Ascend 8.5.0.alpha002 compiler and the existing CPLUS_INCLUDE_PATH

No Candidate arithmetic, memory access, synchronization, tiling, or compute amount changed. No Runner was created or started.

## Compile

REMOTE_HOST=hwnput3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V002
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
COMPILE_COMMAND=cmake -S probe-src -B build-probe-existing -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng && cmake --build build-probe-existing --target probe_v016 probe_v011 --parallel 1
COMPILE=PASS; CONFIGURE_RC=0; PROBE_TARGETS_RC=0
PROBE_V016=PASS; Candidate SHA was compiled into probe_v016
PROBE_V011=PASS; V011 Parent probe linked

## Runtime status

CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
DEVICE_ID=NONE
RESOURCE_NOTE=NPU2 is assigned to N01 and NPU3 is assigned to N02; other NPUs have user processes. No device program was started and no reset operation was attempted.

## Preserved failures

FAILURE_1=probe-src lacked CMakeLists.txt when only the existing CMakeLists.probe.txt file was copied; configure did not start.
FAILURE_2=latest compiler selection caused the CCE plugin to report vector not found.
FAILURE_3=alpha002 compiler selection without the recorded CPLUS_INCLUDE_PATH caused the same vector header failure.
RESULT=Existing probe entry reused successfully after restoring the expected filename and environment initialization; both probe targets built and linked.
GIT_COMMIT=TO_BE_RECORDED
NEXT_ACTION=Wait for an explicitly released NPU, then run the existing safe validation path without resetting another user's process; until then keep Correctness=NOT_RUN and Local=NONE.
