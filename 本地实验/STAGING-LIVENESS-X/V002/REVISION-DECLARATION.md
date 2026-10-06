ROUTE: STAGING-LIVENESS-X
REVISION: V002
DIRECT_PARENT: STAGING-LIVENESS-X/V001
PARENT_SOURCE_SHA: c11cd5b27a89ca1d30aee180a09ac7796aa1aa83e8ff2ead512f2ea82fe87e49
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: Move exactly one pass-1 wide-FP32 current-tile MTE2_V ready wait below the next-tile prefetch block, changing only staging wait/prefetch ordering.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V002 starts from the latest successful V001 compile and changes only the ready-wait placement; it does not change buffer layout, tile width, core count, cross-row pipeline, arithmetic, or output staging.
COMPILE_REQUIRED: YES — compile immediately after this source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_SLA: 180 seconds from known Compile PASS.
