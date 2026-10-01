# MAIN-2 Local Control Dashboard

UPDATED_UTC: 2026-10-01T22:20:43Z
SCOPE: MAIN-2 local control only
CANONICAL_SHARED_LEDGER: UNMODIFIED

## Latest closure receipt

The task-scoped Track-B reconciliation for the five originally approved M2
routes is recorded in
`研究/主代理/MAIN-2-W2/TRACK-B-HANDOFF-CLOSURE-20261001.md`. It confirms
five local committed handoffs, `MAIN_SELECTED=NONE` throughout, no new
implementation or experiment, and unresolved remote write synchronization.
The later portfolio notes in this dashboard remain historical control records;
this receipt does not make a lifecycle decision or change the shared ledgers.

## Latest long-run checkpoint

BOOTSTRAP_RECEIPT: `BOOTSTRAP-RECEIPT-20261001.md`
CANONICAL_HEAD: `02482b46c2ee1fdd5bab1f88a474c7e70426f661`
OVERALL_OFFICIAL_CHAMPION: `R31B V011 / 45.16`

| Route | Latest local fact | Revision / selection | Worktree |
|---|---|---|---|
| UB-LIFETIME-SAFE-CHAMPION-X | V001 build/link/identity PASS; correctness FAILED (7/8, FP32-wide-16384); evidence `c44e1a89`; restore trail `6b6972a0`; status update `12cfb16` | V001 failed and restored to legal parent evidence; no V002 | CLEAN |
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V005 committed `ee5f3b73`; four refined items; duplicate audit and shape qualification complete | `MAIN_SELECTED=NONE`; Track-B only | CLEAN |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | V003 committed `4a144c78`; HBH-09..HBH-12 | `MAIN_SELECTED=NONE`; Track-B only | CLEAN |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X | pool audit committed `526c181a`; TCSU-H1..H3 only | `MAIN_SELECTED=NONE`; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` | CLEAN |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X | boundary audit committed `6ac8774b`; RFH-1 only | `MAIN_SELECTED=NONE`; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` | CLEAN |

All route-local contexts are closed. No research lane created a Revision or
changed kernel/build/runner files. No formal timing, Online, or Judge action
is eligible from this checkpoint.

## Resource and online snapshot

```text
SNAPSHOT_UTC=2026-10-01T22:12:02Z
HOST=hwnput3
FREE_HBM_MB=0:5313,1:5263,2:5319,3:5318,4:6346,5:5356,6:5357,7:12508
AICORE_PERCENT=0:18,1:20,2:33,3:33,4:0,5:0,6:0,7:1
EXISTING_USERS=VLLM_WORKERS_ON_0_TO_6; VLLM/RAY/PYTHON_ON_7
PROCESS_ACTIONS=NONE; no user process stopped, paused, migrated, or preempted
SERVER3_ALIAS=cann-server3; DNS_UNRESOLVED_FROM_THIS_RUNTIME
SERVER_JOBS_STARTED_BY_THIS_CHECKPOINT=0
FORMAL_PERFORMANCE_RUNS=0
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
ONLINE_SUBMISSIONS=0
```

The active-branch aggregate push and its bounded `ls-remote` verification each
timed out (`exit 124`). Remote synchronization is therefore unconfirmed; no
force push or alternate remote was used.

## Planning gate

```text
UB_REVIEWABLE=YES; V001_CORRECTNESS_FAILED_AND_RESTORE_TRAIL_RECORDED
ADDR_TRACK_B_REVIEWABLE=YES
BRANCH_TRACK_B_REVIEWABLE=YES
TILECOUNT_TRACK_B_REVIEWABLE=YES; POOL_EXHAUSTED
REDUCE_TRACK_B_REVIEWABLE=YES; POOL_EXHAUSTED
MAIN_SELECTED_FOR_RESEARCH=0
REVISION_CREATED_BY_THIS_CHECKPOINT=0
KERNEL_FILES_CHANGED_BY_MAIN=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_CHANGED=0
READY_FOR_PLANNING_REVIEW=YES_LOCAL_ONLY
```

This file is the explicit Dashboard for the current MAIN-2 long-run because
the repository has no pre-existing Dashboard path. It is not a replacement
for `调度/当前任务.tsv`, `技术路线/路线成绩表.tsv`, or any other canonical
shared ledger.

## Anchors

```text
CANONICAL_HEAD=02482b46c2ee1fdd5bab1f88a474c7e70426f661
ORIGIN_MAIN=02482b46c2ee1fdd5bab1f88a474c7e70426f661
CANONICAL_STATUS=CLEAN
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

`origin/main` was fetched and canonical `main` was fast-forwarded. The
remote commit only registers MAIN-1 Wave-2 schedule rows; no Candidate source
or score changed.

## Active lanes

| Lane | Route | Branch / worktree | State | Current gate |
|---|---|---|---|---|
| M2-1 | UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` / `/home/data4t2/lelinfeng/cann-w2-m2-ub` | Track-A | correctness harness repair and exact-source correctness |
| M2-2 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `w2/m2/hotloop-addr` / `worktrees/w2/m2/hotloop-addr` | Track-B | next 3 hypotheses / codegen gate |
| M2-3 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `w2/m2/hotloop-branch` / `worktrees/w2/m2/hotloop-branch` | Track-B | next 3 hypotheses / invariant branch proof |
| M2-4 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `w2/m2/tilecount-unroll` / `worktrees/w2/m2/tilecount-unroll` | Track-B | parent codegen gate; no implementation |
| M2-5 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `w2/m2/reduce-finalize` / `worktrees/w2/m2/reduce-finalize` | Track-B | RFH-1 Planning decision; pool exhausted |

The older INTERPASS, CROSSROW, PARAM-RESIDENCY, and ROW-OCCUPANCY routes
remain Planning-owned read-only evidence and are not active in this dashboard.

## Current candidate and queue

```text
UB_V001=BUILD_PASS; EXECUTABLE_IDENTITY_PASS; CORRECTNESS=NOT_RUN
UB_SOURCE_SHA256=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
TRACK_B_MAIN_SELECTED_COUNT=0
NEW_REVISION_CREATED_BY_MAIN=0
LOCAL_BEST_CHANGES=0
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
ONLINE_SUBMISSIONS=0
LAST_OFFICIAL_RESULT=NONE_THIS_LONGRUN
```

No Track-B route may create a Candidate. UB cannot enter same-binary timing or
Online until correctness passes and Main/Planning gates are recorded.

## Live host resource snapshot

```text
LOCAL_HOSTNAME=hwnput3
SSH_ALIAS=cann-server3 (DNS_UNRESOLVED_FROM_THIS_RUNTIME)
NPU_COUNT=8
FREE_HBM_MB_BY_DEVICE=0:5313,1:1408,2:5319,3:5318,4:6346,5:5356,6:5357,7:13310
AICORE_PERCENT_BY_DEVICE=0:32,1:39,2:31,3:30,4:0,5:0,6:0,7:32
CORRECTNESS_DEVICE_PLAN=4 (non-formal; recheck immediately before run)
FORMAL_PERFORMANCE_LEASES_CREATED=0
```

HBM and load values are observational. Existing vLLM and other processes are
not stopped, paused, migrated, or treated as experiment failures. Device 4 is
only a correctness candidate and must be rechecked immediately before use.

## Agent events

```text
UB_AGENT=01a0f956-8bbc-7193-a7da-73d463dcd6b8 (fresh; Track-A correctness gate)
ADDR_AGENT=01a0f957-7963-70a3-afe9-84683e1c6227 (fresh; Track-B)
BRANCH_AGENT=01a0f957-7c60-7c92-ac0a-8217713e493d (fresh; Track-B)
TILECOUNT_AGENT=NOT_STARTED_THIS_BATCH
REDUCE_AGENT=NOT_STARTED_THIS_BATCH
429_OCCURRED=NO
CONTEXT_SHARING=NO
```

The three active contexts each own one route and one worktree. Their current
work is bounded and must end in a committed route-local evidence file or an
explicit pool-exhausted finding.

## Event log

| UTC | Event | Evidence / consequence |
|---|---|---|
| 2026-10-01T21:09Z | bounded `git fetch origin main` succeeded | remote `origin/main` advanced to `02482b46` |
| 2026-10-01T21:10Z | canonical fast-forward | local `main == origin/main`, clean |
| 2026-10-01T21:17Z | UB harness audit | `support/correctness/correctness_runner.asc` referenced missing `../submission.asc`; Candidate source itself unchanged |
| 2026-10-01T21:18Z | UB correctness context created | support-only binding fix and exact-source correctness are the next gate |
| 2026-10-01T21:19Z | ADDR/BRANCH contexts created | Track-B only; no Revision or device work |

## Blockers and invariants

```text
DASHBOARD_PATH=研究/主代理/MAIN-2-W2/DASHBOARD.md
SERVER3_REMOTE_ACCESS=BLOCKED_BY_DNS_ALIAS
LOCAL_SERVER_HOST=AVAILABLE_FOR_NONFORMAL_CHECKS
GITHUB_WRITE=NOT_CONFIRMED; no repeated push attempted
CANONICAL_SHARED_LEDGER_CHANGES=0
KERNEL_FILES_CHANGED_BY_CURRENT_LONGRUN=0
PERFORMANCE_RUNS=0
FORCE_PUSH=0; RESET=0; CLEAN=0
```
