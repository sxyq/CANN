ROUTE: STAGING-LIVENESS-X
REVISION: V031
DIRECT_PARENT: STAGING-LIVENESS-X/V030
PARENT_SOURCE_SHA: 2aada56d0a943266e99cecbf5d154a29c2bb38e906917e9d102b617aeb2d4662
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the first-tile MTE3_V release wait from before the value-tile handle assignment to after that assignment and before SyncMTE2ToV.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V030 places both waits before the value-tile handle. V031 changes only the first-tile wait to the post-handle position.
V031_EDIT_TIMESTAMP: 2026-10-06T21:00:51.316929467Z
ELAPSED_FROM_V030_COMPILE_PASS: 165.051438208s (V030 PASS 2026-10-06T20:58:06.265491259Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
