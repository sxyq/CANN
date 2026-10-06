ROUTE: STAGING-LIVENESS-X
REVISION: V010
DIRECT_PARENT: STAGING-LIVENESS-X/V009
PARENT_SOURCE_SHA: 58ae271ab2edbf6d0bd65c959d44ec583c98dfa0f34fbc0aed07d7ae3020c131
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move the second-tile MTE3_V release wait from before the residual DMA to immediately after that DMA and before value-tile use.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V009 places the wait between x and residual DMA for both tiles. V010 keeps the first-tile placement and moves only the second-tile wait after residual issue.
V010_EDIT_TIMESTAMP: 2026-10-06T19:43:20.824197778Z
ELAPSED_FROM_V009_COMPILE_PASS: 252.0s (V009 PASS 2026-10-06T19:39:08.848986132Z); CHILD_SLA_FAIL=YES.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
