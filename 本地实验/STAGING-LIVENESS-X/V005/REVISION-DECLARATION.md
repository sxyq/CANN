ROUTE: STAGING-LIVENESS-X
REVISION: V005
DIRECT_PARENT: STAGING-LIVENESS-X/V004
PARENT_SOURCE_SHA: 320fd709b248900475d53e63a4ea1e1993415dbc2c1a365bb41442049d9b2962
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessWideFp32FullCacheRows output staging, move the two outstanding MTE3_V release waits from before current-tile normalization to immediately after current-tile Muls and before gamma/bias updates. The preceding tile's output occupies a disjoint value range, so this tests only a later release-drain point.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V004 moved the drain across gamma/bias prefetch; V005 moves the drain across current-tile normalization, a separate lifetime boundary. It changes no buffer layout, tile width, core count, arithmetic, or store order.
INHERITED_SLA_STATUS: CHILD_SLA_FAIL — V003 Compile PASS at 2026-10-06T18:29:39Z to V004 NEXT_EDIT_TIMESTAMP 2026-10-06T18:37:21Z (462 seconds, over 180 seconds).
COMPILE_REQUIRED: YES — compile immediately after this source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_SLA: 180 seconds from known Compile PASS.
