# MAIN-2 Wave-2 Long-Run Bootstrap Receipt

DATE: 2026-10-01
ROLE: MAIN-2 control-only record
SCOPE: continuation bootstrap, route ownership, evidence gates, and runtime blockers

## Canonical anchor

```text
CANONICAL_BRANCH=main
CANONICAL_HEAD=ed860e392d7604694ac6664da60aff1fc1f4c04f
LOCAL_ORIGIN_MAIN=ed860e392d7604694ac6664da60aff1fc1f4c04f
CANONICAL_STATUS=CLEAN
REMOTE_FRESHNESS=UNVERIFIED_FOR_THIS_RECEIPT
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

The local `origin/main` ref is the reproducible base for this continuation. A
fresh GitHub read/write confirmation is not claimed because the normal HTTPS
remote is currently unavailable from this runtime. No force push or alternate
remote was used.

## Current portfolio and ownership

| Lane | Route | Branch | Worktree | Current runtime fact |
|---|---|---|---|---|
| M2-1 | UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` | `/home/data4t2/lelinfeng/cann-w2-m2-ub` | V001 evidence exists; correctness gate pending |
| M2-2 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `w2/m2/hotloop-addr` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr` | Batch 1 fresh Track-B context created |
| M2-3 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `w2/m2/hotloop-branch` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-branch` | Batch 1 fresh Track-B context created |
| M2-4 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `w2/m2/tilecount-unroll` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/tilecount-unroll` | Track-B only; no current runtime handle verified |
| M2-5 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `w2/m2/reduce-finalize` | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize` | Track-B only; no current runtime handle verified |

Previous handles for the five continuation lanes were checked once and all
returned `not_found`; no same-route replacement was created before that check.
The two Batch 1 contexts were created with separate route ownership and no
immediate 429. Their work is limited to committed research in their own
worktrees.

## UB V001 gate receipt

```text
UB_ROUTE=UB-LIFETIME-SAFE-CHAMPION-X
UB_REVISION=V001
MAIN_SELECTED=YES_FOR_UBX-H1_ONLY
PARENT=R31B V011 / 45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
SOURCE_IDENTITY=PASS
STATIC_LIFETIME_PROOF=PASS_STATIC_LIFETIME_PROOF
BUILD_STATUS=PASS
LINK_STATUS=PASS
EXECUTABLE_IDENTITY=PASS
CORRECTNESS_STATUS=NOT_RUN
PERFORMANCE_STATUS=NOT_ELIGIBLE
ONLINE=NOT_AUTHORIZED
```

The source identity was recomputed from the retained `submission.asc` and
matches `submission.sha256` and `source-meta.json`. The untracked
`V001/support/correctness/` and `V001/support/correctness-build/` directories
are retained historical support evidence; they are not deleted or implicitly
staged. Correctness is not claimed until a reachable server3 and an exact
source run are available.

## Execution gates

```text
DASHBOARD_PATH=DASHBOARD_PATH_UNRESOLVED
SERVER3=UNREACHABLE_FROM_THIS_RUNTIME
SERVER3_REASON=ssh cann-server3 hostname resolution failed
SERVER_JOBS_STARTED=0
CORRECTNESS_RUNS_STARTED=0
PERFORMANCE_RUNS_STARTED=0
ONLINE_SUBMISSIONS=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_MADE_BY_MAIN=0
```

No device lease or HBM state is inferred while server3 is unreachable. The
next legal UB action is exact-source correctness after server reachability and
the required device gate are confirmed. The Track-B lanes remain research-only
until Planning selection.

## Control invariants

```text
MAIN_SELECTED_FOR_TRACK_B_LANES=NONE
NEW_ROUTE_SLOTS_CREATED=0
KERNEL_FILES_CHANGED_BY_MAIN=0
ROUTE_CONTEXT_SHARING=FORBIDDEN
REMOTE_WRITE=BLOCKED_BY_NORMAL_HTTPS_CREDENTIAL/CONNECTIVITY_FAILURE
```
