# MAIN-2 Wave-2 Long-Run Status

DATE: 2026-10-01
ROLE: MAIN-2 control-only status
CONTROL_PARENT_HEAD: `da8c79acae40f23bf47fc5706a9e54fd1a25d1e6`

This checkpoint records only committed research and gate facts. Track-B audit
results are not Local performance verdicts and do not select a Revision.

## Canonical and champion

```text
CANONICAL_HEAD=ed860e392d7604694ac6664da60aff1fc1f4c04f
LOCAL_ORIGIN_MAIN=ed860e392d7604694ac6664da60aff1fc1f4c04f
CANONICAL_STATUS=CLEAN
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

The local origin ref is the reproducible base. GitHub freshness and write
sync remain unverified because the normal HTTPS remote cannot authenticate or
complete from this runtime. No force push or alternate remote was used.

## Agent launch and closure

| Batch | Route | Agent | Result |
|---|---|---|---|
| 1 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `01a0f937-7571-7b50-a0e3-05ca160a2c04` | closed after commit `515fb545`; no 429 |
| 1 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `01a0f937-78a7-77b0-ad3b-7df241aefc54` | closed after commit `29cbda4c`; no 429 |
| 2 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `01a0f93c-6dd7-75d0-94c8-6f0a66f3c65d` | closed after commit `0d062c51`; no 429 |
| 2 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `01a0f93c-70c3-7122-9527-3e982afb0d90` | closed after commit `581efdde`; no 429 |
| 3 | UB-LIFETIME-SAFE-CHAMPION-X | `01a0f93d-a74a-7901-b434-be05755c6077` | closed after commit `3f71c824`; no 429 |

All five contexts used separate route worktrees and were closed after their
bounded tasks. Earlier stale handles were checked once and returned
`not_found`; no same-route duplicate replacement was created. No Kernel,
build, server, correctness, timing, profiling, or Online work was delegated
to these audits.

## Route status

### UB-LIFETIME-SAFE-CHAMPION-X

```text
BRANCH=w2/m2/ub-lifetime-safe
WORKTREE=/home/data4t2/lelinfeng/cann-w2-m2-ub
DIRECT_PARENT=R31B V011 / 45.16
REVISION_COUNT=1 (V001 already declared before this checkpoint)
SELECTED_HYPOTHESIS=UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT
MAIN_SELECTED=YES_FOR_THIS_HYPOTHESIS_ONLY
STATIC_LIFETIME_PROOF=PASS_STATIC_LIFETIME_PROOF
SOURCE_IDENTITY=PASS; candidate SHA=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
BUILD=PASS; LINK=PASS; EXECUTABLE_IDENTITY=PASS
CORRECTNESS=NOT_RUN
LOCAL_BEST=UNCHANGED; SERVER_BEST=NONE
ONLINE_COUNT=0; OFFICIAL_BEST=NONE_FOR_THIS_ROUTE
LANE_STATE=BUILD_COMPLETE_CORRECTNESS_BLOCKED
```

The exact source and sidecar match. The retained `support/correctness/` and
`support/correctness-build/` directories are historical untracked evidence
and remain untouched. The next legal action is exact-source correctness after
server3 reachability and device eligibility are confirmed; timing and Online
are not unlocked by the static proof or build alone.

### HOTLOOP-ADDR-HOIST-CHAMPION-X

```text
BRANCH=w2/m2/hotloop-addr
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr
DIRECT_PARENT=R31B V011 / 45.16
REVISION_COUNT=0
HYPOTHESES=ADDR-H1 row-base; ADDR-H2 rolling input cursor; ADDR-H3 stateful counters
AUDIT_RESULT=ADDR-H1/H2 statically feasible; H3 needs codegen evidence
DUPLICATE_RESULT=distinct from HBH-01/02/03; HBH-03 is surface-adjacent only
MAIN_SELECTED=NONE; LOCAL_VERDICT=NONE; SERVER_BEST=NONE; ONLINE_COUNT=0
NEXT=inspect parent codegen before any selection or source edit
LANE_STATE=TRACK_B_AUDIT_COMPLETE_RESEARCH_ONLY
```

### HOTLOOP-BRANCH-HOIST-CHAMPION-X

```text
BRANCH=w2/m2/hotloop-branch
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-branch
DIRECT_PARENT=R31B V011 / 45.16
REVISION_COUNT=0
HYPOTHESES=HBH-01 coefficient-mode; HBH-02 terminal-row prefetch; HBH-03 final-tile valid length
AUDIT_RESULT=no mechanism duplicate with ADDR-H1/H2/H3; HBH-03 shares code surface but freezes address/index
MAIN_SELECTED=NONE; LOCAL_VERDICT=NONE; SERVER_BEST=NONE; ONLINE_COUNT=0
NEXT=codegen/branch-predicate evidence with address/index surfaces frozen
LANE_STATE=TRACK_B_AUDIT_COMPLETE_RESEARCH_ONLY
```

### TILECOUNT-STATIC-UNROLL-CHAMPION-X

```text
BRANCH=w2/m2/tilecount-unroll
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/tilecount-unroll
DIRECT_PARENT=R31B V011 / 45.16
REVISION_COUNT=0
HYPOTHESES=TCSU-H1/H2/H3; fixed N=2 parameter, first-pass, and output-loop unroll
AUDIT_RESULT=source-level N=2 proof; parent codegen gate UNVERIFIED for all three
DUPLICATE_RESULT=distinct from tile geometry, DMA, reduction topology, and branch/address lanes
MAIN_SELECTED=NONE; LOCAL_VERDICT=NONE; SERVER_BEST=NONE; ONLINE_COUNT=0
NEXT=parent disassembly/codegen; reject any source unroll already emitted by compiler
LANE_STATE=TRACK_B_AUDIT_COMPLETE_RESEARCH_ONLY
```

### REDUCE-FINALIZE-HANDOFF-CHAMPION-X

```text
BRANCH=w2/m2/reduce-finalize
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize
DIRECT_PARENT=R31B V011 / 45.16
REVISION_COUNT=0
HYPOTHESES=RFH-1 surviving terminal V->MTE2 release-wait edge; RFH-2..4 duplicate-rejected
AUDIT_RESULT=RFH-1 remains one independent research lead; static feasibility only
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES
MAIN_SELECTED=NONE; IMPLEMENTATION_APPROVED=NO; LOCAL_VERDICT=NONE; ONLINE_COUNT=0
NEXT=Planning decision; no Revision may be created by Main
LANE_STATE=TRACK_B_EXHAUSTED_PENDING_PLANNING
```

For all four Track-B lanes, “audit complete” means the proposed mechanisms
were compared against history and current lanes. It does not mean codegen,
correctness, local performance, or Official performance was tested.

## Server, Dashboard, and Online

```text
SERVER3_STATUS=UNREACHABLE_FROM_THIS_RUNTIME
SERVER3_REASON=ssh cann-server3 hostname resolution failed
DASHBOARD_PATH=DASHBOARD_PATH_UNRESOLVED
SERVER_JOBS_STARTED=0
MAX_COMPILE_CONCURRENCY_OBSERVED=NONE
POLICY_CAPACITY=up to 8 cards; at most 5 non-formal jobs/card
FORMAL_PERFORMANCE_RUNS=0; formal rule remains one timing task/card
HBM_BLOCKS=UNKNOWN; no live device/lease snapshot available
LOCAL_BEST_CHANGES=0
SERVER_BEST=NONE
ONLINE_SUBMISSIONS=0
```

No existing vLLM or unrelated process was stopped, paused, migrated, or
counted as a server experiment.

## Git and governance

```text
CONTROL_COMMIT=da8c79acae40f23bf47fc5706a9e54fd1a25d1e6
ROUTE_COMMITS=515fb545,29cbda4c,0d062c51,581efdde,3f71c824
PUSHES_THIS_CHECKPOINT=0; REMOTE_SYNC=NOT_CONFIRMED
REVERTS=0
FORCE_PUSH=0; RESET=0; CLEAN=0
PROCESS_VIOLATIONS=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_CHANGED_BY_MAIN=0
NEW_SLOTS_CREATED_BY_MAIN=0
```

The normal HTTPS push blocker is carried forward from the prior control
receipt; no repeated push attempt was made. All route worktrees are clean
except for the two retained UB support directories. No shared TSV, route
score, lifecycle, or calibration file was edited.

## Calibration and final gates

```text
NEW_OFFICIAL_RESULTS=0
TP=0; FP=0; TN=0; FN=0
CALIBRATION=NO_NEW_ONLINE_DATA
EVALUATOR_FINDINGS=NONE
MAIN_SELECTED_COUNT_FOR_TRACK_B=0
REVISION_CREATED_BY_THIS_LONGRUN_AUDIT=0
KERNEL_FILES_CHANGED_BY_THIS_LONGRUN_AUDIT=0
READY_FOR_PLANNING_REVIEW=YES_FOR_TRACK_B_FACTS; NO_FOR_UB_CORRECTNESS
```

The only formal candidate in the current portfolio is UB V001, which remains
blocked before correctness. The four Track-B lanes provide committed facts
and open questions for Planning; none has been promoted to implementation.
