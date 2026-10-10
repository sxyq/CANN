# N04 V007 Result

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V007
PARENT=V006 / commit 8103d6fc1a5cf53448b62864cf0a24182e5d0339
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Result

CANDIDATE=NONE
CHANGE=NONE; the installed Ascend C API has no untested SetL2CacheHint choice with distinct 2201 runtime semantics.
AUDIT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V007/capability-audit-v007.log
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
DEVICE_ID=3
DEVICE_RUN=NONE
NEW_RUNNER=NO
PUSH=NO

## Evidence

The installed API declares four CacheMode values. On the active 2201 path, DISABLE is the only mode that changes the GM address; NORMAL, LAST, and PERSISTENT return the original address. The legal READ/WRITE/RW CacheRwMode values do not change that result because the implementation does not branch on the template value. V003 and V005 used DISABLE in their tested inputs, and V006 used NORMAL in both inputs, so there is no real untested hint factor to send through the V016 compile and no-reset probe.

V005's rows=2,width=32768 Candidate/Parent shared probe mismatch remains retained in V005 and is not treated as cache evidence. V003's rows=2,width=8192 Candidate/Parent correctness PASS remains the last verified runnable shape.

STATUS=STOPPED_NO_NEW_HINT
