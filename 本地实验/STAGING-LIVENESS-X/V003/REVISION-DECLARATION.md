ROUTE: STAGING-LIVENESS-X
REVISION: V003
DIRECT_PARENT: STAGING-LIVENESS-X/V002
PARENT_SOURCE_SHA: 5ed93e66e9c8cd67209f7c9f1be4f36e31fd38aba1833bfab66b97aeb576d2dc
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: Move only the final outstanding pass-1 staging release waits from before the row reduction tail to after inverse-RMS calculation, immediately before event-ID release/return.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V003 changes the terminal lifetime of the two pass-1 staging slots only. It does not change the V001 prefetch placement, V002 ready-wait placement, buffer layout, tile width, core count, cross-row pipeline, arithmetic, or output pass.
COMPILE_REQUIRED: YES — compile immediately after this source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_SLA: 180 seconds from known Compile PASS.
