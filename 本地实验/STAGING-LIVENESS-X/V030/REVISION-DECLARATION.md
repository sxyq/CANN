ROUTE: STAGING-LIVENESS-X
REVISION: V030
DIRECT_PARENT: STAGING-LIVENESS-X/V029
PARENT_SOURCE_SHA: b04dd2f736abf0fafb614c7b42f392f507b868488de79865043439247f3d0b1d
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the second-tile MTE3_V release wait from after SyncMTE2ToV to after residual DMA and before the value-tile handle assignment.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V029 places the first wait after residual DMA and the second after SyncMTE2ToV. V030 changes only the second-tile wait to the post-residual, pre-handle position.
V030_EDIT_TIMESTAMP: 2026-10-06T20:57:10.981943434Z
ELAPSED_FROM_V029_COMPILE_PASS: 132.592921804s (V029 PASS 2026-10-06T20:54:58.389021630Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
