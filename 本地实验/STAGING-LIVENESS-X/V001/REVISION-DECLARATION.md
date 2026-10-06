ROUTE: STAGING-LIVENESS-X
REVISION: V001
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE: 45.16 (Official; saved result status PROMOTE)
SINGLE_HYPOTHESIS: In the pass-1 wide-FP32 RMS staging loop, issue the next-tile prefetch immediately after the current tile's ready wait, before current-tile Add, instead of after Add. This moves exactly one prefetch point and changes only pass staging/prefetch timing.
CONTEXT_CLASS: W4-COMPILE-SWEEP / compile-first
WHY_NOT_DUPLICATE: This is an isolated prefetch-issue-time probe against the exact R31B-V011 official source. It does not change V011's tile/bank layout, core count, cross-row pipeline, arithmetic, or output pass.
COMPILE_REQUIRED: YES — compile immediately after the source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
NEXT_EDIT_SLA: 180 seconds from known Compile PASS.
