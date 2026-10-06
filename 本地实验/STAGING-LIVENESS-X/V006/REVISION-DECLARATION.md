ROUTE: STAGING-LIVENESS-X
REVISION: V006
DIRECT_PARENT: STAGING-LIVENESS-X/V005
PARENT_SOURCE_SHA: be0e8f794c135b5d5a0544be1b574dd874446fe9067da35c82e2435454211366
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, defer each prior-row MTE3_V release wait until after the corresponding current-row input tile is loaded and synchronized, immediately before Add overwrites that shared value staging tile.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V005 drains both releases at row entry. V006 moves each drain to its matching tile's last safe overwrite point, allowing input DMA to proceed first; buffer layout, tile width, core count, arithmetic, and store order are unchanged.
INHERITED_SLA_STATUS: CHILD_SLA_FAIL=YES — V003 Compile PASS at 2026-10-06T18:29:39Z to V004 NEXT_EDIT_TIMESTAMP 2026-10-06T18:37:21Z (462 seconds, over 180 seconds).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_SLA: 180 seconds from known Compile PASS.
