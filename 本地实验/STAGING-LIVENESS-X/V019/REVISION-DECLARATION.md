ROUTE: STAGING-LIVENESS-X
REVISION: V019
DIRECT_PARENT: STAGING-LIVENESS-X/V018
PARENT_SOURCE_SHA: 75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait from after SyncMTE2ToV to before the value-tile handle assignment, retaining the second-tile post-Sync wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V018 places both waits after SyncMTE2ToV. V019 changes only the first-tile wait to the pre-handle position.
V019_EDIT_TIMESTAMP: 2026-10-06T20:18:59.772690375Z
ELAPSED_FROM_V018_COMPILE_PASS: 110.592346051s (V018 PASS 2026-10-06T20:17:09.180944324Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
