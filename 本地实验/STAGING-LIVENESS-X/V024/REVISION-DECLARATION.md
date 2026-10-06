ROUTE: STAGING-LIVENESS-X
REVISION: V024
DIRECT_PARENT: STAGING-LIVENESS-X/V023
PARENT_SOURCE_SHA: d37b6dcf5216c08ee0d0d28f2eff81b69a6465bf598270ffba0e9a2854d02fad
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the second-tile MTE3_V release wait from between x and residual input transfers to after the residual transfer.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V023 places the first wait after residual DMA and the second between x and residual DMA. V024 changes only the second-tile wait to the post-residual position.
V024_EDIT_TIMESTAMP: 2026-10-06T20:35:13.702427662Z
ELAPSED_FROM_V023_COMPILE_PASS: 110.493238152s (V023 PASS 2026-10-06T20:33:23.209189510Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
