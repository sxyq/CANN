ROUTE: STAGING-LIVENESS-X
REVISION: V013
DIRECT_PARENT: STAGING-LIVENESS-X/V012
PARENT_SOURCE_SHA: df877eaa112ad5387c66ea28b1dbcef561a6fb92d235afe31d8f97dc399a289b
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, move only the first-tile MTE3_V release wait across the local value-tile handle assignment while retaining the second-tile placement from V012.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V012 moved only the second-tile wait after handle assignment. V013 applies the same one-factor placement to the first-tile wait.
V013_EDIT_TIMESTAMP: 2026-10-06T19:57:11.502577311Z
ELAPSED_FROM_V012_COMPILE_PASS: 96.383685471s (V012 PASS 2026-10-06T19:55:35.118891840Z); CHILD_SLA_FAIL=NO.
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
