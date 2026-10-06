ROUTE: STAGING-LIVENESS-X
REVISION: V032
DIRECT_PARENT: STAGING-LIVENESS-X/V031
PARENT_SOURCE_SHA: 487c954d8cc9cf90f82e53def5f67dc39aa5e0b85651adbfb186a15da5641822
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the second-tile MTE3_V release wait from before the value-tile handle assignment to after that assignment and before SyncMTE2ToV.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V031 places the first wait after the value-tile handle and the second before it. V032 changes only the second-tile wait to the post-handle position.
V032_EDIT_TIMESTAMP: 2026-10-06T21:04:28.888900759Z
ELAPSED_FROM_V031_COMPILE_PASS: 156.634508245s (V031 PASS 2026-10-06T21:01:52.254392514Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
