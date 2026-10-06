ROUTE: STAGING-LIVENESS-X
REVISION: V033
DIRECT_PARENT: STAGING-LIVENESS-X/V032
PARENT_SOURCE_SHA: 83011795621da15118e2e09d20683e2aa1b1d4e6264bef0e056d9a29bf03ce34
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait from before SyncMTE2ToV to immediately after SyncMTE2ToV and before the value-tile Add.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V032 waits for the second tile release before SyncMTE2ToV and the first tile release before that same synchronization. V033 changes only the second-tile wait position; math, buffers, tile sizes, and all other paths are unchanged.
V033_EDIT_TIMESTAMP: 2026-10-06T21:14:48.680279132Z
ELAPSED_FROM_V032_COMPILE_PASS: 558.588132352s (V032 PASS 2026-10-06T21:05:30.094769513Z); CHILD_SLA_FAIL=YES (180s deadline 2026-10-06T21:08:30.094769513Z).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
