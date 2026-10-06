ROUTE: STAGING-LIVENESS-X
REVISION: V011
DIRECT_PARENT: STAGING-LIVENESS-X/V010
PARENT_SOURCE_SHA: 47950f827b615cf67754c492201563beef39feeb48bf95d1664cb86088c5de87
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move the first-tile MTE3_V release wait from between x and residual DMA to immediately after residual DMA, matching the second-tile placement.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V010 moved only the second-tile wait after residual issue. V011 changes only the first-tile wait, keeping the second-tile placement and all buffer, arithmetic, tile, and store behavior unchanged.
V011_EDIT_TIMESTAMP: 2026-10-06T19:51:16.206607408Z
ELAPSED_FROM_V010_COMPILE_PASS: 144.337903776s (V010 PASS 2026-10-06T19:48:51.868703632Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
