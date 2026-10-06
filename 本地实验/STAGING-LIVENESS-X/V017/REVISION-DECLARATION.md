ROUTE: STAGING-LIVENESS-X
REVISION: V017
DIRECT_PARENT: STAGING-LIVENESS-X/V016
PARENT_SOURCE_SHA: 3424f2b95c62ca37f57712b5d095ce31587ded25f176cee99577ea2aa32f5734
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait from before SyncMTE2ToV to immediately after it, retaining the first-tile pre-Sync wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V016 places both waits before SyncMTE2ToV. V017 changes only the second-tile wait to follow that synchronization.
V017_EDIT_TIMESTAMP: 2026-10-06T20:13:19.542331428Z
ELAPSED_FROM_V016_COMPILE_PASS: 154.949337794s (V016 PASS 2026-10-06T20:10:44.643049634Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
