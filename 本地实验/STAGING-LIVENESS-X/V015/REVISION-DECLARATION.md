ROUTE: STAGING-LIVENESS-X
REVISION: V015
DIRECT_PARENT: STAGING-LIVENESS-X/V014
PARENT_SOURCE_SHA: 75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait from after SyncMTE2ToV to before the local value-tile handle assignment, retaining the first-tile wait after SyncMTE2ToV.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V014 places both waits after SyncMTE2ToV. V015 changes only the second-tile wait to the earlier pre-handle position.
V015_EDIT_TIMESTAMP: 2026-10-06T20:07:07.435619517Z
ELAPSED_FROM_V014_COMPILE_PASS: 149.302247550s (V014 PASS 2026-10-06T20:04:38.133371967Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
