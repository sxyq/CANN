# ROUTE REDUCE-INVSCALE-X

ROUTE: REDUCE-INVSCALE-X
REVISION: V001
CONTEXT_CLASS: HISTORICAL_DERIVED
WORKTREE: /Users/sunyiyang/Desktop/Project/cann-next6/REDUCE-INVSCALE-X
BRANCH: exp/next6-reduce-invscale-x
DIRECT_PARENT: legal R006-derived reduction-architecture parent
PARENT_SOURCE_SHA: record before coding
PARENT_SCORE: LOCAL_PARENT or formal if known
SINGLE_HYPOTHESIS: R019 scalar reciprocal + vector Muls normalization is orthogonal to R006 reduction organization.

## Donors
R006 REDUCTION ARCHITECTURE + R019 SCALAR INVSCALE.

## V001 scope
From R006-derived parent, ONLY replace post-reduction normalization with R019-style invscale. Do NOT change tile, pipeline, row scheduling, or parameter residency.

## Loop
Local-first; no CANNJudge; one conceptual change.

