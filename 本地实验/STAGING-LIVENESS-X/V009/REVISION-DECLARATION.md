ROUTE: STAGING-LIVENESS-X
REVISION: V009
DIRECT_PARENT: STAGING-LIVENESS-X/V008
PARENT_SOURCE_SHA: 87e06505c775108275a82f2197fbd1b45652a921a249b6947e6736e53b4e317b
PARENT_SCORE: NOT_RUN (W4 compile-first)
SINGLE_HYPOTHESIS: In ProcessFp32FullRowOutputPipelined, issue each tile's x MTE2 transfer before its matching MTE3_V release wait, then issue the residual transfer after that wait.
CONTEXT_CLASS: W4-COMPILE-SWEEP
WHY_NOT_DUPLICATE: V008 drained before either current-tile input transfer. V009 moves the wait between the x and residual transfers, leaving shared value staging untouched until the later vector Add.
V009_EDIT_TIMESTAMP: 2026-10-06T19:37:53.170275265Z
ELAPSED_FROM_V008_COMPILE_PASS: 193.502396s (V008 PASS 2026-10-06T19:34:39.667878857Z); CHILD_SLA_FAIL=YES (180s deadline exceeded by 13.502396s).
COMPILE_REQUIRED: YES — local CMake compile immediately after source edit.
LOCAL_GATE: SUSPENDED_FOR_W4
ONLINE: FORBIDDEN
