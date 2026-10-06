ROUTE: STAGING-LIVENESS-X
REVISION: V014
DIRECT_PARENT: STAGING-LIVENESS-X/V013
PARENT_SOURCE_SHA: 6602a86c7a9a19b804747d1453a25137a34a2c36f53ea8e906532f0a4ed90f5e
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait from before SyncMTE2ToV to immediately after it, retaining the second-tile wait position.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V013 places both waits before SyncMTE2ToV. V014 changes only the first-tile wait to follow that synchronization.
V014_EDIT_TIMESTAMP: 2026-10-06T20:03:55.742126917Z
ELAPSED_FROM_V013_COMPILE_PASS: 120.620065501s (V013 PASS 2026-10-06T20:01:55.122061416Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
