ROUTE: STAGING-LIVENESS-X
REVISION: V012
DIRECT_PARENT: STAGING-LIVENESS-X/V011
PARENT_SOURCE_SHA: 2aada56d0a943266e99cecbf5d154a29c2bb38e906917e9d102b617aeb2d4662
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait across the local value-tile handle assignment while retaining its position after residual DMA.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V011 places both waits after residual DMA and before value-tile handle creation. V012 changes only the second-tile wait to follow that handle assignment.
V012_EDIT_TIMESTAMP: 2026-10-06T19:54:48.009106051Z
ELAPSED_FROM_V011_COMPILE_PASS: 153.425788339s (V011 PASS 2026-10-06T19:52:14.583317712Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
