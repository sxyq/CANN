ROUTE: STAGING-LIVENESS-X
REVISION: V018
DIRECT_PARENT: STAGING-LIVENESS-X/V017
PARENT_SOURCE_SHA: b04dd2f736abf0fafb614c7b42f392f507b868488de79865043439247f3d0b1d
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait from before SyncMTE2ToV to immediately after it, retaining the second-tile post-Sync wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V017 places the first wait before SyncMTE2ToV and the second after it. V018 changes only the first wait to follow synchronization.
V018_EDIT_TIMESTAMP: 2026-10-06T20:16:12.671541584Z
ELAPSED_FROM_V017_COMPILE_PASS: 122.318150460s (V017 PASS 2026-10-06T20:14:10.353391124Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
