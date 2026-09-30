# MAIN-1 WAVE-2 CAMPAIGN STATUS

## Bootstrap

- PHASE: WAVE-2 OFFICIAL-AWARE EXPLORATION
- ROLE: MAIN-1
- CANONICAL_HEAD_AT_BOOTSTRAP: `ed860e392d7604694ac6664da60aff1fc1f4c04f`
- OVERALL_OFFICIAL_CHAMPION: `R31B V011 / 45.16`
- PLANNING_DECISIONS_CHANGED: `0`
- NEW_ROUTES_OUTSIDE_APPROVED_5: `0`
- SOURCE_KERNEL_WRITTEN_BY_MAIN: `0`
- SHARED_LEDGER_WRITTEN_BY_MAIN: `0`

Wave-1 Local-positive candidates remain separate from the current Official anchor. The Wave-2 lanes start from the Official-backed parents recorded below; Local-positive history is donor evidence only until a new Official result proves otherwise.

## Fresh child isolation

| Lane | Route | Branch | Worktree | Agent | Track-B status |
|---|---|---|---|---|---|
| M1-1 | SYNC-TOPOLOGY-CHAMPION-X | `w2/m1/sync-topology` | `/Users/sunyiyang/.codex/worktrees/w2-m1-sync/cann` | `01a0f00c-ce63-7c33-b0ed-3b22136bb65e` | STARTED |
| M1-2 | STORE-EPILOGUE-W2-X | `w2/m1/store-epilogue` | `/Users/sunyiyang/.codex/worktrees/w2-m1-store/cann` | `01a0f00d-a9b6-7a80-a620-c7ba276b4655` | STARTED |
| M1-3 | EPI-ARITH-CHAMPION-W2-X | `w2/m1/epi-arith` | `/Users/sunyiyang/.codex/worktrees/w2-m1-epi/cann` | `01a0f00d-aa4b-7b30-8588-943dcb45b9a6` | STARTED |
| M1-4 | SELECTIVE-FASTPATH-CHAMPION-X | `w2/m1/selective-fastpath` | `/Users/sunyiyang/.codex/worktrees/w2-m1-fastpath/cann` | `01a0f00d-aab7-7de3-8cc4-fe16db4e5d14` | STARTED |
| M1-5 | SMALLMID-DATAFLOW-CHAMPION-X | `w2/m1/smallmid-dataflow` | `/Users/sunyiyang/.codex/worktrees/w2-m1-smallmid/cann` | `01a0f00d-ab3a-74c1-a181-f1b26936c6c3` | STARTED |

Each child has one branch, one writable worktree, and one fresh context. The managed worktree paths above are the actual paths returned by the worktree manager; no second checkout was created.

## Parent map

| Route | Direct parent | Official anchor | Scope |
|---|---|---:|---|
| SYNC-TOPOLOGY-CHAMPION-X | R31B V011 | 45.16 | barrier / flag / event dependency topology |
| STORE-EPILOGUE-W2-X | STORE-EPILOGUE-X V002 | 45.07 | store issue / drain / writeback dependency |
| EPI-ARITH-CHAMPION-W2-X | R31B V011 | 45.16 | post-invRms arithmetic before store |
| SELECTIVE-FASTPATH-CHAMPION-X | R31B V011 | 45.16 | one-donor dispatch gates with V011 fallback |
| SMALLMID-DATAFLOW-CHAMPION-X | R31B V011 | 45.16 | small/mid fixed-cost dataflow |

## Main selection rule

Each child must return 3–5 Track-B hypotheses with the required mechanism, bottleneck, shape/dtype scope, risk, duplicate audit, and minimal experiment fields. Main will select at most one hypothesis per route with `MAIN_SELECTED=YES`. Until that message is sent, no child may create a performance Revision or alter Candidate source.

## Online order

Default order if multiple lanes qualify together: SELECTIVE-FASTPATH, SYNC-TOPOLOGY, EPI-ARITH, SMALLMID-DATAFLOW, STORE-W2. Main may change this order only with an evidence-based reason recorded in a handoff.

## Current lifecycle state

No route lifecycle decision was made in this bootstrap. Any `LANE_NEEDS_PLANNING_REVIEW` remains a report to Planning / Review Layer, not a Main decision.
