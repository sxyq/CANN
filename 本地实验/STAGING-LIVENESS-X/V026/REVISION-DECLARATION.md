ROUTE: STAGING-LIVENESS-X
REVISION: V026
DIRECT_PARENT: STAGING-LIVENESS-X/V025
PARENT_SOURCE_SHA: 487c954d8cc9cf90f82e53def5f67dc39aa5e0b85651adbfb186a15da5641822
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the second-tile MTE3_V release wait from before the value-tile handle assignment to immediately after it.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V025 places the first wait after the value-tile handle and the second before it. V026 changes only the second-tile wait to the post-handle position.
V026_EDIT_TIMESTAMP: 2026-10-06T20:43:33.541305100Z
ELAPSED_FROM_V025_COMPILE_PASS: 204.786040313s (V025 PASS 2026-10-06T20:40:08.755264787Z); CHILD_SLA_FAIL=YES.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
