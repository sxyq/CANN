# N05 V003 Result

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V003
PARENT=V002
RESOURCE_COMMAND=ssh cann-server3 'npu-smi info'
RESOURCE_RC=0
RESOURCE_LOG=resource-snapshot-20261010T1430Z.log
RESOURCE_UPDATE=N01 finished; NPU2 released; NPU3 has no process
SOURCE_REVIEW=PASS
CHANGE=Pass-2 output GM address cursor only
COMPILE=PASS
CORRECTNESS=NONE
LOCAL_SCORE=NONE
PUSH=NO

The existing V002 CMake entry compiled `device`, `submission`, and `full_link`: `CONFIGURE_RC=0`, `BUILD_RC=0`, `FINAL_RC=0`, `SSH_RC=0`. Full raw output, including pre/post device and process snapshots, is in `compile-attempt-01.log`; artifact details are in `COMPILE-RESULT.md` and `build-artifacts.txt`.

Correctness and Local remain `NONE` as instructed. The V002 first-pass input cursor is retained. Tile order, arithmetic, DMA calls, event sequence, and row/core ownership are unchanged. No new Runner or runtime program was started.
