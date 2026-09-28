# ROUTE SCHED-ROWGROUP-X

ROUTE: SCHED-ROWGROUP-X
REVISION: V001
CONTEXT_CLASS: HISTORICAL_DERIVED
WORKTREE: /Users/sunyiyang/Desktop/Project/cann-next6/SCHED-ROWGROUP-X
BRANCH: exp/next6-sched-rowgroup-x
DIRECT_PARENT: scheduling-centric baseline from R016 SHAPE-AWARE SCHEDULING evidence
PARENT_SOURCE_SHA: record before coding
PARENT_SCORE: LOCAL_PARENT or formal if known
SINGLE_HYPOTHESIS: shape-aware rows/task plus 32-byte-safe row-group ownership yields complementary gain over scheduler alone.

## Donors
R016 SHAPE-AWARE SCHEDULING + R012 32B ROW-GROUP.

## V001 scope
From scheduling-centric baseline, ONLY add 32-byte-safe row-group ownership. No batch compute, parameter cache, pipeline, or new reduction.

## Loop
Local-first; no CANNJudge; one conceptual change.

