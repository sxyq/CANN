ROUTE: STAGING-LIVENESS-X
REVISION: V034
DIRECT_PARENT: STAGING-LIVENESS-X/V033
PARENT_SOURCE_SHA: 0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait from before SyncMTE2ToV to immediately after SyncMTE2ToV and before the second-tile release wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V033 waits for the first and second tile releases after the value-tile handle, with the first wait before SyncMTE2ToV. V034 changes only the first-tile wait position; math, buffers, tile sizes, and all other paths are unchanged.
V034_EDIT_TIMESTAMP: 2026-10-06T21:18:39.749736159Z
ELAPSED_FROM_V033_COMPILE_PASS: 165.596635648s (V033 PASS 2026-10-06T21:15:54.160460275Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
