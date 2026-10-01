# MAIN-2 Wave-2 Track-B Campaign Status

DATE: 2026-09-30
BASELINE: origin/main `ed860e392d7604694ac6664da60aff1fc1f4c04f`
ROLE: MAIN-2 control-only record
SCOPE: staged Agent launch and Track-B handoff audit; no kernel or shared-ledger changes

## Route bootstrap

All five specified M2 worktrees were created from `origin/main` and are clean. Their
local branches did not have remote branch counterparts at bootstrap; each remains an
isolated route branch and must be pushed only after its own handoff is committed.

| Route | Branch | Worktree | Direct parent | Initial state |
|---|---|---|---|---|
| INTERPASS-PIPELINE-CHAMPION-X | `w2/m2/interpass-pipeline` | `/home/data4t2/lelinfeng/cann-w2-m2-interpass` | R31B V011 / 45.16 | fresh bootstrap |
| CROSSROW-PIPELINE-CHAMPION-X | `w2/m2/crossrow-pipeline` | `/home/data4t2/lelinfeng/cann-w2-m2-crossrow` | R31B V011 / 45.16 | fresh bootstrap |
| UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` | `/home/data4t2/lelinfeng/cann-w2-m2-ub` | R31B V011 / 45.16 | fresh bootstrap |
| PARAM-RESIDENCY-CHAMPION-X | `w2/m2/param-residency` | `/home/data4t2/lelinfeng/cann-w2-m2-param` | R31B V011 / 45.16 | fresh bootstrap |
| ROW-OCCUPANCY-CHAMPION-X | `w2/m2/row-occupancy` | `/home/data4t2/lelinfeng/cann-w2-m2-occupancy` | R31B V011 / 45.16 | fresh bootstrap |

## Staged launch log

| Batch | Routes | Agent IDs | State at launch | 429 |
|---|---|---|---|---|
| Batch 1 | INTERPASS, CROSSROW | `01a0f0ad-2b5b-7372-b10f-848f8447ff5d`, `01a0f0ad-3276-7f33-ae76-3c43f9f35a12` | CREATED; Track-B work in progress | NO immediate 429 |
| Batch 2 | not started | NONE | gated on Batch 1 | NONE |
| Batch 3 | not started | NONE | gated on Batch 2 | NONE |

## Hard gates

- `MAIN_SELECTED=NONE` for every route.
- No Revision, Candidate Kernel, build, correctness, timing, profiling, server3, Online, Judge, or shared-ledger change is permitted in this campaign.
- Child writes are limited to its own `研究/<ROUTE>/...` handoff and evidence references.

## Final reconciliation

| Route | Current Agent | Handoff | Commit | Local clean | Remote sync | Review state |
|---|---|---|---|---|---|---|
| INTERPASS-PIPELINE-CHAMPION-X | closed after completion (`01a0f0ad-2b5b-7372-b10f-848f8447ff5d`) | `研究/INTERPASS-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` | `d926de88e7be4d3d60f79181fa988f295df3d448` | YES | NO; push hung and was terminated once | handoff complete; MAIN_SELECTED=NONE |
| CROSSROW-PIPELINE-CHAMPION-X | closed after completion (`01a0f0ad-3276-7f33-ae76-3c43f9f35a12`) | `研究/CROSSROW-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` | `25a14dc8d3ccb58180b7801c75d3a61cd21e7694` | YES | NO; GitHub username unavailable | handoff complete; MAIN_SELECTED=NONE |
| UB-LIFETIME-SAFE-CHAMPION-X | replacement closed after completion (`01a0f0fe-a99e-7713-a739-8905b5092fc2`) | `研究/UB-LIFETIME-SAFE-CHAMPION-X/TRACK-B-HANDOFF.md` | `33f6a98441c0f62be4fc7e353b06ed5f050507ec` | YES | NO; GitHub username unavailable | handoff complete; MAIN_SELECTED=NONE |
| PARAM-RESIDENCY-CHAMPION-X | closed after completion (`01a0f0d9-e841-72d3-a60c-2f65bec54efc`) | `研究/PARAM-RESIDENCY-CHAMPION-X/TRACK-B-HANDOFF.md` | `62b57cb0325cf515ca68eea8e8c2fe4f9187086b` | YES | NO; push timed out once | handoff complete; MAIN_SELECTED=NONE |
| ROW-OCCUPANCY-CHAMPION-X | no current Agent; attempts `01a0f0ef-fa00-7563-a2ba-1802981543ee`, `01a0f10d-a884-7bd0-9deb-ee64d870fab4`, `01a0f121-45f5-71b3-a6c1-5ea0b4026a88` stalled and were closed | NONE | NONE | YES at baseline | N/A | `OCCUPANCY_RUNTIME_STALLED`; no hypothesis written by Main |

## Final counters

```text
STAGED_AGENT_LAUNCH = PASS (Batch 1: 2; Batch 2: 2; Batch 3: 1; no 429)
M2_TRACK_B_HANDOFFS = 4/5
TOTAL_M2_HANDOFF_COMMITS = 4
MAIN_SELECTED_COUNT = 0
REVISION_CREATED = 0
KERNEL_FILES_CHANGED = 0
SERVER_RUNS = 0
PERFORMANCE_RUNS = 0
ONLINE_SUBMISSIONS = 0
CANONICAL_SHARED_LEDGER_CHANGES = 0
PLANNING_DECISIONS_CHANGED = 0
NEW_ROUTES_OUTSIDE_APPROVED_5 = 0
BLOCKER = OCCUPANCY_RUNTIME_STALLED
```

The four completed handoffs were reviewed for required parent identity, route boundary,
3-5 hypotheses, duplicate audit, and explicit no-implementation state. The fifth route
is intentionally left without a handoff because Main is forbidden to author its research
hypotheses. GitHub pushes for the new local branches and control branch are blocked by
missing HTTPS write credentials; no force push or repeated retry was used.

## Long-run continuation bootstrap (2026-10-01)

This continuation uses the new MAIN-2 portfolio instruction supplied on 2026-10-01.
It does not rewrite the historical reconciliation above.

### Canonical and champion receipt

```text
CANONICAL_BRANCH=main
CANONICAL_HEAD=ed860e392d7604694ac6664da60aff1fc1f4c04f
LOCAL_ORIGIN_MAIN=ed860e392d7604694ac6664da60aff1fc1f4c04f
CANONICAL_STATUS=CLEAN
REMOTE_FRESHNESS=UNVERIFIED; git fetch timed out connecting to github.com:443; bounded ls-remote timed out
MAIN2_CONTROL_BRANCH=w2/main2/control
MAIN2_CONTROL_HEAD=b56d6769b539154480b83e02fb6a1e675d922251
OVERALL_OFFICIAL_CHAMPION=R31B V011 / 45.16
CHAMPION_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CHAMPION_EVIDENCE=技术路线/冠军/champions.tsv; 线上结果/R31B/V011/result.json; local retained source SHA verified
```

The repository's local `origin/main` ref is the base used below, but it cannot be
represented as freshly fetched from GitHub in this session.

### Current portfolio and owners

| Lane | Route | Agent | Branch / worktree | State |
|---|---|---|---|---|
| M2-1 | UB-LIFETIME-SAFE-CHAMPION-X | `01a0f89c-3266-7c43-84e6-90a2c9897408` | `w2/m2/ub-lifetime-safe` / `/home/data4t2/lelinfeng/cann-w2-m2-ub` | Planning-selected UBX-H1; lifetime proof required before V001/source edit |
| M2-2 | HOTLOOP-ADDR-HOIST-CHAMPION-X | `01a0f89c-2fbc-72b0-a373-9b823f2d6c32` | `w2/m2/hotloop-addr` / `worktrees/w2/m2/hotloop-addr` | Track-B only; `MAIN_SELECTED=NONE` |
| M2-3 | HOTLOOP-BRANCH-HOIST-CHAMPION-X | `01a0f89d-06d2-78a1-98eb-f30de790e0fe` | `w2/m2/hotloop-branch` / `worktrees/w2/m2/hotloop-branch` | Track-B only; `MAIN_SELECTED=NONE` |
| M2-4 | TILECOUNT-STATIC-UNROLL-CHAMPION-X | `01a0f89d-0976-78e1-aaf8-c91e389fbd8a` | `w2/m2/tilecount-unroll` / `worktrees/w2/m2/tilecount-unroll` | Track-B only; `MAIN_SELECTED=NONE` |
| M2-5 | REDUCE-FINALIZE-HANDOFF-CHAMPION-X | `01a0f89d-e115-7190-8a4b-b8a166463ef3` | `w2/m2/reduce-finalize` / `worktrees/w2/m2/reduce-finalize` | Track-B only; `MAIN_SELECTED=NONE` |

All four fresh research worktrees are clean at local `origin/main` commit
`ed860e392d7604694ac6664da60aff1fc1f4c04f`. Their branches were absent from the
local branch/worktree inventory before creation. The in-repository research worktrees
are excluded locally through `.git/info/exclude`; this is not a tracked canonical edit.

The latest supplied Planning instruction says the older INTERPASS, CROSSROW,
PARAM-RESIDENCY and ROW-OCCUPANCY lanes are PARKed and retained as read-only evidence.
Their branches/worktrees and handoffs remain untouched. This Main did not edit the
canonical lifecycle/shared ledgers; the four Planning directives are recorded here only.

### Dashboard, server, and online gates

```text
DASHBOARD_PATH=DASHBOARD_PATH_UNRESOLVED
DASHBOARD_DATA_PATH=UNRESOLVED
DASHBOARD_REFRESH_COMMAND=UNRESOLVED
REASON=No canonical reference found in AGENTS.md, Skill, project rules, formal project docs, or local project files; no substitute dashboard created.
SERVER3_STATUS=UNREACHABLE_FROM_THIS_RUNTIME
REASON=ssh alias cann-server3 did not resolve; direct endpoint stopped at host-key verification; no device/lease freshness claim is made.
ONLINE_QUEUE=NO_NEW_ELIGIBLE_CANDIDATE
ONLINE_SUBMISSION=NONE
```

The last shared device-lease entries are historical (2026-09-27); without live server
access no compile/correctness/timing job will be assigned. Existing unrelated Judge-ready
packages are not treated as candidates from this new portfolio and will not be resubmitted.

### Counters at continuation start

```text
ACTIVE_ROUTE_AGENTS=5
TRACK_B_HANDOFFS_NEW=0
REVISION_CREATED=0
KERNEL_FILES_CHANGED_BY_MAIN=0
SERVER_JOBS_STARTED=0
PERFORMANCE_RUNS=0
ONLINE_SUBMISSIONS=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_MADE_BY_MAIN=0
NEW_SLOTS_OUTSIDE_MAIN2_PORTFOLIO=0
BLOCKERS=DASHBOARD_PATH_UNRESOLVED; GITHUB_REMOTE_UNREACHABLE; SERVER3_UNREACHABLE_FROM_RUNTIME
```

### Long-run checkpoint (2026-10-01, after committed-handoff review)

This checkpoint is based on control-branch HEAD `6fc4d09a74bdd61b42a6f5c15a04bd69805c38dd`.
It preserves the continuation-start snapshot above and records only new, verified
events. The repository canonical HEAD remains `ed860e392d7604694ac6664da60aff1fc1f4c04f`;
the ADDR and BRANCH route agents each completed a bounded fetch confirming that
`origin/main` resolves to this same SHA.

| Lane | Latest durable fact | Git / gate state |
|---|---|---|
| UB-LIFETIME-SAFE | Static proof `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md` | Commit `3f015003cdba8e9188b9345bc45c9a2e404d22ab`; Main static-lifetime gate PASS for Planning-selected UBX-H1. Proof now explicitly guards the old unconditional `outputBuf_.Get<T>()`; no Kernel edit observed. Formal V001 declaration is still uncommitted; implementation/build/correctness/performance remain pending. |
| HOTLOOP-ADDR-HOIST | Track-B handoff `研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md` | Commit `3c65f7e053f79bf8b4d87a1b2d8de43fba4d7889`; 3 surviving hypotheses; `MAIN_SELECTED=NONE`; worktree clean at last check. Cross-route duplicate-audit follow-up requested. |
| HOTLOOP-BRANCH-HOIST | Track-B handoff `研究/HOTLOOP-BRANCH-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md` | Commit `7e46bd771aa6fa3dd0216d643ebbed1f31e7f332`; 3 surviving hypotheses; `MAIN_SELECTED=NONE`; worktree clean at last check. Cross-route duplicate-audit follow-up requested. |
| TILECOUNT-STATIC-UNROLL | No new committed handoff observed | No Revision or source change observed; handoff/runtime completion not yet received. |
| REDUCE-FINALIZE-HANDOFF | No new committed handoff observed | No Revision or source change observed; handoff/runtime completion not yet received. |

Remote and execution gates:

```text
REMOTE_READ_FRESHNESS=VERIFIED_AT_ed860e392d7604694ac6664da60aff1fc1f4c04f
REMOTE_WRITE=BLOCKED; normal HTTPS pushes failed because GitHub write credentials are unavailable
FORCE_PUSH_OR_ALTERNATE_REMOTE=NOT_USED
DASHBOARD_PATH=DASHBOARD_PATH_UNRESOLVED
SERVER3=UNREACHABLE_FROM_THIS_RUNTIME; no live lease/HBM snapshot
SERVER_JOBS=0; CORRECTNESS_RUNS=0; PERFORMANCE_RUNS=0; ONLINE_SUBMISSIONS=0
CANONICAL_SHARED_LEDGER_CHANGES=0
PLANNING_DECISIONS_MADE_BY_MAIN=0
NEW_SLOTS_OUTSIDE_MAIN2_PORTFOLIO=0
```

Both completed Track-B handoffs remain research-only. Their cross-route audit
supplements and the remaining two route handoffs are still open; do not treat the
local commits as remote-synced or the overall MAIN-2 long-run as complete.
