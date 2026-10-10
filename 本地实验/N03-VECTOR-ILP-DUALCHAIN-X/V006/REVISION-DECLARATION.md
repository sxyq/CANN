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

COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
DEVICE_ID=NONE
FREE_HBM=NOT_COLLECTED; compile-only task
DEVICE_LOAD=NOT_COLLECTED; no device program started
AICORE_LOAD=NOT_COLLECTED; no device program started
OTHER_PROCESS_PRESENT=NOT_APPLICABLE
REMOTE_HOST=cann-server3
REMOTE_DIR=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V006
SOC_VERSION=Ascend910B3
NPU_ARCH=dav-2201
EXISTING_CMAKE=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V006/probe-src/CMakeLists.txt
COMPILE_SOURCE_FILE=probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_LOG=V006/support/compile-probe-v006.log
RESULT=Compile not run in this turn; the existing probe entry was not started.
NEXT_ACTION=Run the existing V016 and V011 probe targets in a later explicitly requested compile turn; no device execution is authorized by this record.
