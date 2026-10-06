ROUTE: STAGING-LIVENESS-X
REVISION: V007
DIRECT_PARENT: STAGING-LIVENESS-X/V006
PARENT_SOURCE_SHA: 627c0fdf94412e1b1546c1bbd6f3cb370f8040b7a1f0c3207e41511605e1cac3
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move the two tile-specific MTE3_V release waits from after input MTE2 completion to immediately before SyncMTE2ToV, allowing the pending input DMA to overlap the release drain.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V006 waited after SyncMTE2ToV. V007 places the same safe overwrite waits before that synchronization; buffer layout, tile width, core count, arithmetic, and store order are unchanged.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
