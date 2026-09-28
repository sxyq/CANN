# TRACK-B RESEARCH BRIEF — EPILOGUE-FUSE-X

ROUTE=EPILOGUE-FUSE-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/EPILOGUE-FUSE-X
BRANCH=exp/main2-r2-epilogue-fuse
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_EPILOGUE_DATAFLOW
PRIORITY=3

## Goal

Optimize the dataflow AFTER the RMS value is obtained:

invRms
→ normalize
→ gamma
→ bias
→ output conversion / store

This is an arithmetic / dataflow route. It is NOT the MIX-A sync route.

## Allowed

- arithmetic fusion
- UB intermediate elimination
- instruction fusion
- reducing unnecessary UB round trips

## Forbidden

- changing reduction
- changing row scheduling
- changing mode
- changing dtype-specific paths
- using sync/fence removal as the core performance variable
- wide-path redesign
- multi-row DMA

## Scope warning (collision with DTYPE-SPECIAL-X)

MAIN-1 DTYPE-SPECIAL-X owns dtype-specific arithmetic/conversion paths.
EPILOGUE-FUSE-X must keep unified FP32 intermediate semantics and must not
create per-dtype specialized epilogues. If a hypothesis requires a
dtype-special path, reject it as CONCEPT_COLLISION_WITH_MAIN1.

## Frozen seed

phase4/workspaces/EPILOGUE-FUSE-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

Study sites in the frozen seed:
- invRms computation and use
- normalize / scale application
- gamma/bias apply
- output conversion and store
- UB buffers that only carry epilogue intermediates

Idea-pool donor note (research only):
- R010 + R027 software-mask / fused epilogue after one u load

## Track-B output required

Write `phase4/research/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES.md` with 3–5
candidate hypotheses. Each hypothesis must include:

MECHANISM
EXPECTED_BOTTLENECK
FILES/FUNCTIONS TO TOUCH
WHY_ORTHOGONAL_TO_MAIN1
WHY_NOT_DUPLICATE_EXISTING_MAIN2
EXPECTED_WIN_SHAPES
EXPECTED_RISK_SHAPES
CORRECTNESS_RISK
MEASUREMENT_PLAN

Then STOP. Do not edit kernel until Main approves exactly one hypothesis.

## Hard constraints

- MAIN-1 worktrees under /Users/sunyiyang/Desktop/Project/cann-sixlane/ are READ/WRITE FORBIDDEN
- Do not modify shared control under canonical cann/
- Do not submit to CANNJudge
- Keep mathematical semantics identical within tolerance
- ONE FACTOR AT A TIME after approval
