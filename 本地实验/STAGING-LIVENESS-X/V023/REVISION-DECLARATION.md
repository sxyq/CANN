ROUTE: STAGING-LIVENESS-X
REVISION: V023
DIRECT_PARENT: STAGING-LIVENESS-X/V022
PARENT_SOURCE_SHA: 58ae271ab2edbf6d0bd65c959d44ec583c98dfa0f34fbc0aed07d7ae3020c131
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the first-tile MTE3_V release wait from between x and residual input transfers to after the residual transfer.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V022 places both waits between x and residual DMA. V023 changes only the first-tile wait to the post-residual position.
V023_EDIT_TIMESTAMP: 2026-10-06T20:31:30.950830507Z
ELAPSED_FROM_V022_COMPILE_PASS: 125.645818249s (V022 PASS 2026-10-06T20:29:25.305012258Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
