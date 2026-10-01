# MAIN-2 Long-Run Bootstrap Receipt

DATE: 2026-10-01
ROLE: MAIN-2 long-run control
SOURCE: current canonical files plus the supplied MAIN-2 long-run directive

## Mandatory read and canonical anchors

```text
AGENTS=READ
CANN_MAINLINE_SKILL=READ
PROJECT_RULES=READ
TECHNICAL_ROUTE_HISTORY=READ
WAVE1_MAIN2_RECORDS=READ
WAVE2_HANDOFFS=READ
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN=02482b46c2ee1fdd5bab1f88a474c7e70426f661
CANONICAL_STATUS=CLEAN
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

The four older inter-pass/cross-row/parameter-residency/row-occupancy routes
remain read-only evidence under the supplied Planning lifecycle directive.
This receipt does not independently make a PARK, CLOSE, MERGE, REPLACE, or
slot decision.

## Active portfolio and context isolation

| Lane | Route | Agent | Branch | Worktree | Mode |
|---|---|---|---|---|---|
| M2-1 | UB-LIFETIME-SAFE-CHAMPION-X | `01a0f97d-9396-7b00-81cf-c68a64db2f32` | `w2/m2/ub-lifetime-safe` | `/home/data4t2/lelinfeng/cann-w2-m2-ub` | Track-A recovery for Planning-selected UBX-H1 |
| M2-2 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `01a0f97d-97e8-72f1-9fed-d8246e743cb7` | `w2/m2/hotloop-addr` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr` | Track-B, address expression axis |
| M2-3 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `01a0f97f-014e-7b71-85e7-9b8c1dccc2df` | `w2/m2/hotloop-branch` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-branch` | Track-B, one branch-kind axis |
| M2-4 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `01a0f97f-fe62-7f72-b64b-dd3a00b48fb4` | `w2/m2/tilecount-unroll` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/tilecount-unroll` | Track-B, one tile-count axis |
| M2-5 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `01a0f97f-93bf-74c2-afa9-d788d434d39d` | `w2/m2/reduce-finalize` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize` | Track-B, scalar handoff-edge axis |

Each Agent received only its own worktree and route scope. No sibling private
context is shared. The four research Agents are forbidden from source edits,
Revision creation, build, correctness, timing, profiling, server3, and Online.

## Current UB gate

```text
ROUTE=UB-LIFETIME-SAFE-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B V011 / 45.16
SELECTED_HYPOTHESIS=UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT
CANDIDATE_SOURCE_SHA256=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
BUILD=PASS
LINK=PASS
EXECUTABLE_IDENTITY=PASS
CORRECTNESS=CORRECTNESS_FAILED; 7/8 passed; fp32-wide-16384 failed
PERFORMANCE=NOT_ELIGIBLE
ONLINE=NOT_AUTHORIZED
```

The failed V001 source and raw correctness evidence remain retained. The UB
Agent is handling the required independent evidence commit and explicit
restore/revert audit. No V002 or second performance mechanism is authorized.

## Control gates

```text
MAIN_SELECTED_FOR_RESEARCH_LANES=NONE
REVISION_CREATED_BY_THIS_BOOTSTRAP=0
KERNEL_FILES_CHANGED_BY_MAIN=0
SERVER_JOBS_STARTED_BY_THIS_BOOTSTRAP=0
PERFORMANCE_RUNS=0
ONLINE_SUBMISSIONS=0
CANONICAL_SHARED_LEDGER_CHANGES=0
NEW_ROUTES_OUTSIDE_APPROVED_10=0
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
DASHBOARD=研究/主代理/MAIN-2-W2/DASHBOARD.md
```

This receipt is a control checkpoint. Route-local facts must be committed in
the owning branch before the Dashboard is updated. The long-run remains active
until the UB gate and all four research lanes reach a Planning-reviewable state,
or an explicit system/Planning stop occurs.

## Superseded prior snapshot retained

The parent version of this file recorded the earlier continuation snapshot and
remains available in Git history. Its material facts are retained here rather
than silently discarded:

```text
PRIOR_CANONICAL_HEAD=ed860e392d7604694ac6664da60aff1fc1f4c04f
PRIOR_REMOTE_FRESHNESS=UNVERIFIED
PRIOR_DASHBOARD_PATH=UNRESOLVED
PRIOR_SERVER3=UNREACHABLE_FROM_THIS_RUNTIME
PRIOR_SERVER_JOBS=0
PRIOR_CORRECTNESS_RUNS=0_AT_THAT_CHECKPOINT
PRIOR_PERFORMANCE_RUNS=0
PRIOR_ONLINE_SUBMISSIONS=0
PRIOR_CANONICAL_SHARED_LEDGER_CHANGES=0
PRIOR_ACTIVE_PORTFOLIO=UB plus HOTLOOP-ADDR, HOTLOOP-BRANCH, TILECOUNT, REDUCE-FINALIZE
PRIOR_AGENT_HANDLES=previous handles were checked and returned not_found; no duplicate replacement was retained
```

That snapshot predates the later exact-source UB correctness run and the current
fetch to `02482b46`; it is historical evidence, not the current gate.
