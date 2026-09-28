# TRACK-B RESEARCH BRIEF — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/REDUCE-HIER-X
BRANCH=exp/main2-r2-reduce-hier
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
PRIORITY=2

## Goal

Study ONLY the RMS square-sum reduction topology. Target is lower RMS
reduction latency and UB traffic while keeping mathematical semantics
identical (same FP32 intermediate semantics, same result tolerance).

## Allowed research axes

- FP32 partial accumulation
- hierarchical reduction
- vector reduction organization
- partial-sum lifetime
- reduction workspace organization

## Forbidden

- dtype specialization
- row scheduling changes
- multi-row DMA
- wide specialization
- sync-removal hypothesis
- gamma/bias caching
- epilogue arithmetic changes
- mode selection / rows-per-block

## Old-route policy

REDUCE-INVSCALE-X is EVIDENCE DONOR ONLY.
Its V002 must NOT be used as this route's parent.
This route starts from frozen R31B-V011 and gets its own declaration.
Do not restore old worktrees.

## Frozen seed

phase4/workspaces/REDUCE-HIER-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

Study sites in the frozen seed:
- reduceFp32Buf_ / ReduceSum usage
- square-sum accumulation across tiles
- GetValue / scalar handoff of the RMS denominator
- partial-sum lifetime across the row

Idea-pool donor notes (research only, not parent code):
- R006 chunked/hierarchical reduce
- R011 manual vector reduce
- R019 invRms divide order (careful; this route owns reduction, not invRms math sequence)

## Track-B output required

Write `phase4/research/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md` with 3–5
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
- Mathematical semantics must stay identical
- ONE FACTOR AT A TIME after approval
