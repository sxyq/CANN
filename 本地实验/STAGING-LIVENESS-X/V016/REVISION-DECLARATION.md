ROUTE: STAGING-LIVENESS-X
REVISION: V016
DIRECT_PARENT: STAGING-LIVENESS-X/V015
PARENT_SOURCE_SHA: a8c026c3858b0a0e5f536b03b99549f163ac58fecd832dc89dc0fec1647dad29
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait from after SyncMTE2ToV to before the value-tile handle assignment, retaining the second-tile pre-handle placement.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V015 places the first wait after SyncMTE2ToV and the second before handle assignment. V016 changes only the first wait to the second wait's earlier position.
V016_EDIT_TIMESTAMP: 2026-10-06T20:09:31.112689617Z
ELAPSED_FROM_V015_COMPILE_PASS: 94.453864997s (V015 PASS 2026-10-06T20:07:56.658824620Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
