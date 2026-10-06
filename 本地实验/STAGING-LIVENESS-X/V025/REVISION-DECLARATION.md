ROUTE: STAGING-LIVENESS-X
REVISION: V025
DIRECT_PARENT: STAGING-LIVENESS-X/V024
PARENT_SOURCE_SHA: 3424f2b95c62ca37f57712b5d095ce31587ded25f176cee99577ea2aa32f5734
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the first-tile MTE3_V release wait from after residual DMA to after the value-tile handle assignment.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V024 places both waits after residual DMA and before the value-tile handle. V025 changes only the first-tile wait to follow the handle assignment.
V025_EDIT_TIMESTAMP: 2026-10-06T20:39:12.065856638Z
ELAPSED_FROM_V024_COMPILE_PASS: 187.088177911s (V024 PASS 2026-10-06T20:36:04.977678751Z); CHILD_SLA_FAIL=YES (deadline exceeded by 7.088177911s).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
