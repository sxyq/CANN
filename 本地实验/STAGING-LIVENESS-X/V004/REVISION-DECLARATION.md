ROUTE: STAGING-LIVENESS-X
REVISION: V004
DIRECT_PARENT: STAGING-LIVENESS-X/V003
PARENT_SOURCE_SHA: 73b44de058c758ad385089f32223c2291184b51a7acaab1140783775b007475c
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: Move exactly one output-pass MTE3_V release-drain point from before parameter prefetch to immediately after parameter prefetch and before value reuse.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V004 changes only the output staging release-drain placement in the latest V003 source. It does not change input staging, buffer layout, tile width, core count, cross-row pipeline, arithmetic, or output store order.
COMPILE_REQUIRED: YES — compile immediately after this source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_TIMESTAMP: 2026-10-06T18:37:21Z
