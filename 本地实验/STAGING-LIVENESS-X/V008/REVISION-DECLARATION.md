ROUTE: STAGING-LIVENESS-X
REVISION: V008
DIRECT_PARENT: STAGING-LIVENESS-X/V007
PARENT_SOURCE_SHA: 95001d61cfd807899d5a0d067388ac7523fc270cc7bfd7364c9c79704b2ae070
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move each tile-specific prior-row MTE3_V release wait to before the matching current-row input DMA issue, testing the earliest safe drain point.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V007 drains after issuing input DMA and before SyncMTE2ToV; V008 moves that drain before input DMA. Buffer layout, tile width, core count, arithmetic, and store order remain unchanged.
V008_EDIT_TIMESTAMP: 2026-10-06T19:33:30.216548464Z
ELAPSED_FROM_V007_COMPILE_PASS: 296.091805s (V007 PASS 2026-10-06T19:28:34.124743281Z); CHILD_SLA_FAIL=YES (180s deadline exceeded).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
