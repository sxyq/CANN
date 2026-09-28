# TRACK-B RESEARCH BRIEF — SCHED-CHAMPION-X

ROUTE=SCHED-CHAMPION-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/SCHED-CHAMPION-X
BRANCH=exp/main2-r2-sched-champion
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
PRIORITY=1

## Goal

Apply the already Local + Official validated mechanism:

32B-safe row-group ownership / core scheduling

onto the frozen strong baseline. The mechanism was proven on a weak parent
(SCHED-ROWGROUP-X V001, parent 17.14 → Official 22.27, 15/15).

## Single-hypothesis boundary

ONLY allowed to change:
- row ownership
- core assignment
- row-group scheduling

FORBIDDEN to change:
- mode selection
- rows/block
- DMA segmentation
- wide path
- dtype path
- reduction algorithm
- UB lifetime
- epilogue arithmetic

## Non-duplication audit (Main, 2026-09-27)

Frozen R31B-V011 `Process()` partitions rows only via
`blockIdx / baseRows / extraRows`. It does NOT contain
`rowGroup = 32/gcd(rowBytes,32)` whole-group ownership.
NON_DUPLICATION_AUDIT=PASS.

## Donor reference (read-only, do not copy wholesale)

Mechanism description:
/Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/SCHED-ROWGROUP-X/WHY_NOT_DUPLICATE.md
Donor source header:
/Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/SCHED-ROWGROUP-X/SCHED-ROWGROUP-X-V001-submission.asc

Donor core rule (from donor comments):
- rowGroup = 32 / gcd(rowBytes, 32)
- each complete group owned wholly by one core
- tasks rounded up to a multiple of rowGroup
- remove parent "if row not 32B-aligned then one core"

## Frozen seed

phase4/workspaces/SCHED-CHAMPION-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

Entry points to study in the frozen seed:
- Process() row partition at ~lines 171–176
- widePath_ early return and its own row handling
- mode dispatch after beginRow/localRows are computed

## Track-B output required

Write `phase4/research/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md` with 3–5
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
- Do not read R31B current worktree or V016 as seed
- Do not modify shared control under canonical cann/
- Do not submit to CANNJudge
- Do not invent a second performance variable
- If you find frozen champion already has an equivalent mechanism, write
  DO_NOT_IMPLEMENT_DUPLICATE and stop

## After Main approval (later, not this turn)

REVISION declaration fields required before code:
ROUTE / REVISION / DIRECT_PARENT / PARENT_SOURCE_SHA / OFFICIAL_ANCHOR /
SINGLE_HYPOTHESIS / CONTEXT_CLASS / WHY_NOT_DUPLICATE_MAIN1 /
WHY_NOT_DUPLICATE_MAIN2
