# ROUTE ASYNC-TRIPLE-X

ROUTE: ASYNC-TRIPLE-X
REVISION: V001
CONTEXT_CLASS: HISTORICAL_DERIVED
WORKTREE: /Users/sunyiyang/Desktop/Project/cann-next6/ASYNC-TRIPLE-X
BRANCH: exp/next6-async-triple-x
DIRECT_PARENT: none (fresh route seed from canonical ae46d7c)
PARENT_SOURCE_SHA: n/a
PARENT_SCORE: n/a
SINGLE_HYPOTHESIS: Add MTE3 store overlap so MTE2 prefetch N+1, Vector compute N, and MTE3 store N-1 form true triple overlap.

## Donor
R013 DOUBLE-BUFFER PIPELINE (MTE2 prefetch N+1 + Vector compute N).

## V001 scope
ONLY add MTE3 overlap. Do NOT change reduction, parameter policy, dtype strategy, or core mapping.

## WHY_NOT_DUPLICATE
R013 proved two-stage overlap only. Full MTE2/V/MTE3 triple overlap is still unexplored (idea-pool R013 wide triple).

## Loop
Code -> server3 compile/link -> targeted NPU correctness -> 2-4 paired local probes -> Child Handoff -> Main Review.
Child conclusions: LOCAL_REJECTED | NEEDS_ONE_MORE_LOCAL | ONLINE_CANDIDATE.
Do NOT submit to CANNJudge. Do NOT write into other routes. Same agent owns V001-V003.

