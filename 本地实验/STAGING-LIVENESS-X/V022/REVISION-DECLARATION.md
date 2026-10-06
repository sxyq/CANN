ROUTE: STAGING-LIVENESS-X
REVISION: V022
DIRECT_PARENT: STAGING-LIVENESS-X/V021
PARENT_SOURCE_SHA: 47950f827b615cf67754c492201563beef39feeb48bf95d1664cb86088c5de87
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the second-tile MTE3_V release wait from after residual DMA to between the x and residual input transfers.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V021 places the first wait between x and residual DMA and the second after residual DMA. V022 changes only the second-tile wait to the inter-input position.
V022_EDIT_TIMESTAMP: 2026-10-06T20:28:32.059526401Z
ELAPSED_FROM_V021_COMPILE_PASS: 167.264435236s (V021 PASS 2026-10-06T20:25:44.795091112Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
