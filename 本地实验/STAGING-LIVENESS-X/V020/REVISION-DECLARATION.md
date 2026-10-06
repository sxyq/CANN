ROUTE: STAGING-LIVENESS-X
REVISION: V020
DIRECT_PARENT: STAGING-LIVENESS-X/V019
PARENT_SOURCE_SHA: b04dd2f736abf0fafb614c7b42f392f507b868488de79865043439247f3d0b1d
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait from after SyncMTE2ToV to before the value-tile handle assignment, retaining the first-tile pre-handle wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V019 places the first wait before handle assignment and the second after SyncMTE2ToV. V020 changes only the second wait to the pre-handle position.
V020_EDIT_TIMESTAMP: 2026-10-06T20:22:21.338706348Z
ELAPSED_FROM_V019_COMPILE_PASS: 146.705871049s (V019 PASS 2026-10-06T20:19:54.632835299Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
