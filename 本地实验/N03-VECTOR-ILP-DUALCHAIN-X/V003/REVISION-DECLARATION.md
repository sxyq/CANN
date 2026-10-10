# N03 V003 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V003
STATUS=REJECTED_OVERLAP
PARENT=945d5a29012a25ecbff01a9bf33a229b82f0a5ba
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V003/rejected-index-order.asc
SOURCE_SHA256=d7e3e96e63a3acbfe5d4c0a8b88276e06c76471c945bc9b236f6fb4f79e01909

## Boundary decision

The attempted change made the Pass 1 stream tile-major for multi-row BF16 batches. It
changed the `(batchRow, tile)` traversal and recomputed row/tile indices with modulo
and division. That changes the tile/row address derivation and overlaps the N05
Scalar address-generation route. V003 is therefore retained as rejected evidence and
is not promoted as an N03 result.

N03 is limited to source-order changes among independent Vector operations. The
existing row-major traversal, `col`, `rowOffset`, GM load addresses, output addresses,
events, barriers, reduction slots, and compute amount must remain unchanged.

## Compile evidence

COMPILE=PASS; the existing V016 probe entry compiled the V003 Candidate and V011
Parent kernel targets before this boundary decision.
REMOTE_EVIDENCE=/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X/V003
COMPILE_LOG=V003/support/compile-probe-v003.log
CODEGEN_EVIDENCE=V003/support/rejected-probe-v016-object.objdump.txt;V003/support/rejected-codegen-object.diff
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
RESULT=Rejected for route overlap; no device program was started.
NEXT_ACTION=Use a BF16 Vector-operation-order-only change with the original row-major address derivation.
