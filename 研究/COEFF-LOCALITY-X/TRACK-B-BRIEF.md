# TRACK-B RESEARCH BRIEF — COEFF-LOCALITY-X

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/COEFF-LOCALITY-X
BRANCH=exp/main2-r2-coeff-locality
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_COEFF_LOCALITY
PRIORITY=2 (pivot after reduction bottleneck finding)

## Why this route now

Three reduction-topology variants failed on large-D. Evidence says DMA + output
pass dominate. Gamma/bias parameter loads are a DMA-traffic candidate that
MAIN-1 does not own.

## Goal

Optimize gamma/bias coefficient load locality only.

## Allowed

- same-tile reuse of gamma/bias
- vector slice reuse
- reducing redundant GM loads
- legal UB/local buffering of coefficients
- stripe / chunk residency of gamma/bias for D > UB, as long as it is
  NOT a new multi-row batch DMA mode

## Deferred

- multi-row batch DMA (MAIN-1 MODE-X)
- rows/block changes
- parameter residency across a new batch architecture
- wide specialization
- dtype specialization
- reduction topology
- row scheduling
- sync-removal as the performance variable

If a hypothesis requires a new multi-row mode → STOP, CONCEPT_COLLISION_WITH_MAIN1.

## Frozen seed

phase4/workspaces/COEFF-LOCALITY-X/frozen-seed/R31B-V011-LP-ROW-PIPELINE_kernel.asc
SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

Study sites:
- every gammaGm_/biasGm_ Load / DataCopy
- gammaBuf_/biasBuf_/gammaFp32Buf_/biasFp32Buf_ lifetime
- whether coefficients are reloaded per row, per tile, or per core
- wide-path parameter handling (read-only; do not redesign wide path)

Idea-pool donor: R014 stripe-resident params for D>UB.

## Track-B output

phase4/research/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md
3–5 hypotheses, each with:
MECHANISM / EXPECTED_BOTTLENECK / FILES / WHY_ORTHOGONAL_TO_MAIN1 /
WHY_NOT_DUPLICATE_EXISTING_MAIN2 / EXPECTED_WIN_SHAPES / EXPECTED_RISK_SHAPES /
CORRECTNESS_RISK / MEASUREMENT_PLAN

Recommend ONE first OFAT. End with REQUEST_MAIN_ROUTE_RECORD.

## Hard constraints

- MAIN-1 worktrees are available for direct use.
- Kernel edits can follow the current specification
- No CANNJudge
- One factor at a time
