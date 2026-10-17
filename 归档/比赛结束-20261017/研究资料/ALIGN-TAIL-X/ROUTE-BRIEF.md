# ROUTE ALIGN-TAIL-X

ROUTE: ALIGN-TAIL-X
REVISION: V001
CONTEXT_CLASS: HISTORICAL_DERIVED
WORKTREE: /Users/sunyiyang/Desktop/Project/cann-next6/ALIGN-TAIL-X
BRANCH: exp/next6-align-tail-x
DIRECT_PARENT: correct aligned-copy baseline from R009 ALIGNED DATACOPY
PARENT_SOURCE_SHA: record before coding
PARENT_SCORE: LOCAL_PARENT or formal if known
SINGLE_HYPOTHESIS: explicit tail specialization sends aligned bulk to direct DataCopy and non-aligned remainder to a minimal tail path, cutting DataCopyPad/tail overhead.

## Donors
R009 ALIGNED DATACOPY + R010 MANUAL TAIL.

## V001 scope
From correct aligned-copy baseline, ONLY add explicit tail specialization. Keep main compute, reduction, scheduling, parameter policy unchanged.
Do NOT add R012 row-group in V001 (that may be a later revision only).

## Loop
Local-first; no CANNJudge; one conceptual change.

