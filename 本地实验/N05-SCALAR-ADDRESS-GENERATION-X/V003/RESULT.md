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
STANDALONE_RUNTIME_PROBE_BUILD=PASS
PAIRED_RUNNER_BUILD=PASS_AFTER_RETRY; FIRST_ATTEMPT=FAIL_<vector>_NOT_FOUND
RUNTIME_EXECUTION=NOT_RUN
CORRECTNESS=NONE
LOCAL_SCORE=NONE
PUSH=NO

The existing V002 CMake entry compiled `device`, `submission`, and `full_link`: `CONFIGURE_RC=0`, `BUILD_RC=0`, `FINAL_RC=0`, `SSH_RC=0`. Full raw output, including pre/post device and process snapshots, is in `compile-attempt-01.log`; artifact details are in `COMPILE-RESULT.md` and `build-artifacts.txt`.

The existing V016 probe CMake entry built standalone V002 and V003 probe targets successfully. The first paired-runner build failed with `fatal error: 'vector' file not found`; the one retry with the existing include and system-first library paths built the paired executable successfully. The initial probe linker failures and configure failure are preserved in `runtime-compile-v002-v003.log` and `runtime-compile-probe-entry.log`.

No runtime executable was started. Correctness against V002 and R31B V011 and paired Local therefore remain `NONE`. NPU3 had no running process in the saved pre-run snapshot. The V002 first-pass input cursor is retained. Tile order, arithmetic, DMA calls, event sequence, and row/core ownership are unchanged.
