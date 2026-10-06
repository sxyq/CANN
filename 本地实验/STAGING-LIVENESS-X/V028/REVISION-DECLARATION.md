ROUTE: STAGING-LIVENESS-X
REVISION: V028
DIRECT_PARENT: STAGING-LIVENESS-X/V027
PARENT_SOURCE_SHA: a7271a7f31b4caddb50795d96b2afdfee8b65add44e21728ad67ad53e343f828
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows, move only the second-tile MTE3_V release wait from before SyncMTE2ToV to immediately after synchronization.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V027 places the first wait after SyncMTE2ToV and the second before it. V028 changes only the second-tile wait to the post-Sync position.
V028_EDIT_TIMESTAMP: 2026-10-06T20:49:58.158038760Z
ELAPSED_FROM_V027_COMPILE_PASS: 172.819878558s (V027 PASS 2026-10-06T20:47:05.338159602Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
