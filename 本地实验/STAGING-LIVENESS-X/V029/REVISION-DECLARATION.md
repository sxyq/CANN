ROUTE: STAGING-LIVENESS-X
REVISION: V029
DIRECT_PARENT: STAGING-LIVENESS-X/V028
PARENT_SOURCE_SHA: f60d71c702c1801282fd2db1728a24b1398c95a8f4b83f8a8803cfa00ff3cdab
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the first-tile MTE3_V release wait from after SyncMTE2ToV to after residual DMA and before the value-tile handle assignment.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V028 places both waits after SyncMTE2ToV. V029 changes only the first-tile wait to the post-residual, pre-handle position.
V029_EDIT_TIMESTAMP: 2026-10-06T20:54:03.431977954Z
ELAPSED_FROM_V028_COMPILE_PASS: 185.305505755s (V028 PASS 2026-10-06T20:50:58.126472199Z); CHILD_SLA_FAIL=YES (deadline exceeded by 5.305505755s).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
