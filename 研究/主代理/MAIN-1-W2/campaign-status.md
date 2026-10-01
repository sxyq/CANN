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
- CHILD_SPAWN_RETRIES: `initial 10 attempts failed with runtime 429; one EPI child later exited on concurrency limit and was replaced`

Wave-1 Local-positive candidates remain separate from the current Official anchor. The Wave-2 lanes start from the Official-backed parents recorded below; Local-positive history is donor evidence only until a new Official result proves otherwise.

## Fresh child isolation

| Lane | Route | Branch | Worktree | Agent | Track-B status |
|---|---|---|---|---|---|
| M1-1 | SYNC-TOPOLOGY-CHAMPION-X | `w2/m1/sync-topology` | `/Users/sunyiyang/.codex/worktrees/w2-m1-sync/cann` | `01a0f038-7eae-7e13-9130-2dd37ca68948` | HANDOFF COMPLETE; Agent closed |
| M1-2 | STORE-EPILOGUE-W2-X | `w2/m1/store-epilogue` | `/Users/sunyiyang/.codex/worktrees/w2-m1-store/cann` | `01a0f039-1cbb-79d1-bb77-9f63f830b5f3` | HANDOFF COMPLETE; Agent closed |
| M1-3 | EPI-ARITH-CHAMPION-W2-X | `w2/m1/epi-arith` | `/Users/sunyiyang/.codex/worktrees/w2-m1-epi/cann` | `01a0f05a-3dd0-7fa1-80da-dd5b51ba0784` | HANDOFF COMPLETE; Agent closed; one distinct candidate, pool exhausted |
| M1-4 | SELECTIVE-FASTPATH-CHAMPION-X | `w2/m1/selective-fastpath` | `/Users/sunyiyang/.codex/worktrees/w2-m1-fastpath/cann` | `01a0f039-295d-7900-8099-8097e2e68b78` | HANDOFF COMPLETE; Agent closed |
| M1-5 | SMALLMID-DATAFLOW-CHAMPION-X | `w2/m1/smallmid-dataflow` | `/Users/sunyiyang/.codex/worktrees/w2-m1-smallmid/cann` | `01a0f039-2fb0-7d52-aacb-f8e668ba426c` | HANDOFF COMPLETE; Agent closed |

Agent history, runtime checks, and committed handoff references are recorded in `agent-state-reconciliation.md`. All five follow-up requests are handled by their existing Agent IDs; no duplicate Route Agent was created.

Each child has one branch, one writable worktree, and one fresh context. The managed worktree paths above are the actual paths returned by the worktree manager; no second checkout was created.

## Parent map

| Route | Direct parent | Official anchor | Scope |
|---|---|---:|---|
| SYNC-TOPOLOGY-CHAMPION-X | R31B V011 | 45.16 | barrier / flag / event dependency topology |
| STORE-EPILOGUE-W2-X | STORE-EPILOGUE-X V002 | 45.07 | store issue / drain / writeback dependency |
| EPI-ARITH-CHAMPION-W2-X | R31B V011 | 45.16 | post-invRms arithmetic before store |
| SELECTIVE-FASTPATH-CHAMPION-X | R31B V011 | 45.16 | one-donor dispatch conditions with V011 fallback |
| SMALLMID-DATAFLOW-CHAMPION-X | R31B V011 | 45.16 | small/mid fixed-cost dataflow |

## Main selection rule

Each child must return 3–5 Track-B hypotheses with the required mechanism, bottleneck, shape/dtype scope, risk, duplicate audit, and minimal experiment fields. This round is review-only: all five routes remain `MAIN_SELECTED=NONE` pending Planning / Review. No child may create a performance Revision or alter Candidate source.

## Main review

All five committed handoffs have a verified Direct Parent, source SHA, route boundary, one-factor proposal, probe outline and history/cross-route comparison. The route-level review is recorded in `handoff-review.md`. EPI reports only one distinct hypothesis after duplicate rejection and requests Planning review of its candidate-pool exhaustion; this does not change route lifecycle. No hypothesis is selected.

## Online order

Default order if multiple lanes qualify together: SELECTIVE-FASTPATH, SYNC-TOPOLOGY, EPI-ARITH, SMALLMID-DATAFLOW, STORE-W2. Main may change this order only with an evidence-based reason recorded in a handoff.

## Current lifecycle state

No route lifecycle decision was made in this bootstrap. Any `LANE_NEEDS_PLANNING_REVIEW` remains a report to Planning / Review Layer, not a Main decision.

## RECOVERY_RECEIPT / BOOTSTRAP_RECEIPT (2026-10-02)

- `CONTEXT_EPOCH`: 2026-10-02; recovery followed a loaded summary.
- `HEAD_AT_RECOVERY`: `d37d67559416d4523c6b2e161d476af63b45b6f1` on `main1/champion-exploit`; worktree clean and equal to its same-name `origin` branch before this receipt was recorded.
- `BOOTSTRAP_RECEIPT_COMMIT`: `52cfcbe22e6a2a66a87b1836c75187ca25941a09`, pushed to `origin/main1/champion-exploit`.
- `ORIGIN_MAIN` / `CANONICAL_HEAD`: `ed860e392d7604694ac6664da60aff1fc1f4c04f`; fetched and verified equal.
- `MAIN_BRANCH`: `main1/champion-exploit`.
- `MAIN1_WORKTREE`: `/Users/sunyiyang/Desktop/Project/cann/worktrees/main/main1`.
- `WORKTREES`: 22 linked worktrees under `/Users/sunyiyang/Desktop/Project/cann/worktrees/`; all have sparse-checkout, clean file status, and HEAD equal to the same-name `origin/<branch>` reference.
- `OVERALL_CHAMPION` / `CURRENT_OFFICIAL_SCORE`: `R31B V011`, `45.16`, `15/15`; source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `RULES_READ`: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/{实验总则,执行约定,本地性能测试规范,服务器实验规范,Git工作流程,线上提交规范}.md`; `技术路线/{技术路线总表,技术路线图}.md`; `技术路线/{路线成绩表,全版本记录}.tsv`; `调度/{当前任务,线上候选,本地线上校准}.tsv`; `调度/主代理分工.md`; Main-1 W2 campaign, agent reconciliation, handoff review, and all five committed route handoffs.
- `SKILL_READ`: `.agents/skills/cann-mainline/SKILL.md` read in full.
- `CURRENT_ROUTE`: none active; all five Track-B handoffs are complete.
- `CURRENT_AGENT`: MAIN-1; child agents: none active (all five recorded as completed and closed).
- `CURRENT_WORKTREE` / `CURRENT_BRANCH`: `/Users/sunyiyang/Desktop/Project/cann/worktrees/main/main1` / `main1/champion-exploit`.
- `ACTIVE_REVISION`: none; `MAIN_SELECTED=NONE` for all five routes.
- `LAST_LOCAL_SCORE`: no Wave-2 Candidate has a local score. Existing donor results remain tied to their recorded revisions.
- `LAST_OFFICIAL_SCORE`: SYNC `45.16` (R31B V011); STORE `45.07` (STORE-EPILOGUE-X V002); EPI `45.16` (R31B V011); SELECTIVE-FASTPATH `45.16` (R31B V011); SMALLMID `45.16` (R31B V011).
- `LAST_DASHBOARD_UPDATE`: unavailable; no project Dashboard entry point was found.

### Current Wave-2 worktrees

The earlier tables retain the paths used when the agents launched. Current paths below were read from `git worktree list`; each branch HEAD equals both its same-name remote reference and its recorded last commit.

| ROUTE | BRANCH | CURRENT_WORKTREE | LAST_LOCAL_COMMIT = LAST_PUSHED_COMMIT | DIRECT_PARENT / OFFICIAL_ANCHOR |
|---|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `w2/m1/sync-topology` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/sync-topology` | `9071292b916d88a9aed0304db64d30435a5f51c4` | R31B V011 / 45.16 |
| STORE-EPILOGUE-W2-X | `w2/m1/store-epilogue` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/store-epilogue` | `415a24295525f88babd2abdb300bfcfc6a04074e` | STORE-EPILOGUE-X V002 / 45.07 |
| EPI-ARITH-CHAMPION-W2-X | `w2/m1/epi-arith` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/epi-arith` | `0ce5441e0e94b06d92f3db596631f35c38870d6c` | R31B V011 / 45.16 |
| SELECTIVE-FASTPATH-CHAMPION-X | `w2/m1/selective-fastpath` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/selective-fastpath` | `4d8e073e0279b6860c371d7a13d4c06fdc6f1fec` | R31B V011 / 45.16 |
| SMALLMID-DATAFLOW-CHAMPION-X | `w2/m1/smallmid-dataflow` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/smallmid-dataflow` | `7f51d2506693b5c6c91fef2f0b867c6bb41aacc9` | R31B V011 / 45.16 |

### Dashboard discovery

This section records the initial recovery-time search and is superseded by the 2026-10-02 discovery in `MAIN1_LONGRUN_STATUS` below.

- `DASHBOARD_PATH_UNRESOLVED`
- `DASHBOARD_DATA_PATH_UNRESOLVED`
- `DASHBOARD_REFRESH_COMMAND_UNRESOLVED`
- Searched project rules, project docs, scripts, and hidden project files. The only match for “看板” refers to GitCode issue state, not this project. No replacement Dashboard was created. Planning review is needed to identify the official path and refresh command.

### Acknowledged operating rules

- Track-B and duplicate review precede implementation; a Planning selection must be recorded before any performance Revision.
- One performance Revision changes one technical factor.
- For a rejected Candidate, preserve the source and evidence commits, then make an explicit revert or restore commit; never invent a historical revert.
- No hidden local-only chain; each route retains its own context, branch, and worktree.
- Keep Dashboard data current once its official path is supplied.
- Commit each independent fact; push at the scheduled interval and before handoff.
- Online submission remains Main/Judge-Owner only, with exact-source identity verified before and after submission.
- No Candidate source, Revision, server run, performance measurement, or Online submission was created in this recovery.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES_OUTSIDE_APPROVED_5=0`.

At that recovery point, the next step was to request Planning / Review selections and identify the Dashboard. The later C2C CONTROL and current Dashboard reference are recorded below.

## MAIN1_LONGRUN_STATUS (2026-10-02)

### BOOTSTRAP_RECEIPT

- `HEAD`: `b78bb071c916700c64ca345450dfd74db4eb2371` before this receipt.
- `ORIGIN_MAIN`: `ed860e392d7604694ac6664da60aff1fc1f4c04f`; fetched from `origin` and unchanged.
- `MAIN_BRANCH`: `main1/champion-exploit`; `origin/main1/champion-exploit` matched the recorded Main-1 HEAD before this receipt.
- `WORKTREES`: 22 linked worktrees under `/Users/sunyiyang/Desktop/Project/cann/worktrees/`, plus the canonical checkout. Paths and branches came from `git worktree list --porcelain`.
- `OVERALL_CHAMPION`: `R31B V011`, Official `45.16`, `15/15`; source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `RULES_READ`: root `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; all six current `项目规则/` files; `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/全版本记录.tsv`; `技术路线/路线成绩表.tsv`; `调度/当前任务.tsv`; `调度/线上候选.tsv`; `调度/本地线上校准.tsv`; `调度/服务器设备使用.tsv`; `调度/主代理分工.md`; Main-1 and Main-2 campaign status; Main-1 Wave-2 review; five Main-1 and five Main-2 committed Track-B handoffs.
- `SKILL_READ`: `.agents/skills/cann-mainline/SKILL.md`, complete.
- `DASHBOARD_PATH`: `归档/任务看板/index.html`.
- `DASHBOARD_DATA_PATH`: the embedded JSON snapshot in `归档/任务看板/index.html`, generated from project ledgers, result records, Main campaign status, and registered worktrees.
- `DASHBOARD_REFRESH_COMMAND`: `node 归档/任务看板/refresh.mjs`.
- `LAST_ONLINE_SUBMIT_AT`: EPILOGUE-ARITH-CHAMPION-X V002, submission `6abb8840694b590c3c9b6db3`, Official `44.96`, result timestamp `2026-09-29T09:43:28.310Z`.
- `CURRENT_ROUTE`: none yet executing; existing child Agent IDs were resumed for this campaign and are awaiting task input.
- `ACTIVE_REVISION`: none. No Wave-2 Candidate source, build, correctness run, local timing, or Online submission has been created.
- `PLANNING_DECISIONS_CHANGED`: `0`.
- `NEW_ROUTES`: `0`.

### Current lane instructions and Main review

The attached C2C CONTROL supplies these current Planning selections. Main review confirms the proposed single-factor boundaries against the committed handoffs. Each Route Agent must record its Revision declaration and commit it before touching Candidate source.

| Route | Current instruction | Main review / first action | Revision state |
|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | H2, low-precision parameter prefetch ordering | Ready for a single-factor declaration; preserve event ownership and buffer depth. | None |
| STORE-EPILOGUE-W2-X | H1, move the existing full-row Store issue point | Ready for a single-factor declaration from STORE V002; Store count, address, chunk shape, events, and arithmetic stay fixed. | None |
| EPI-ARITH-CHAMPION-W2-X | H3, group independent row Mul/Add operations | Ready for a single-factor declaration. The selected SYNC H2 is limited to the low-precision path, so it does not share EPI H3's wide-FP32 arithmetic loop. | None |
| SELECTIVE-FASTPATH-CHAMPION-X | H1, BF16 D32768 full V017 donor qualification | Qualification only first: recover exact M values, prove dispatch reachability, then compare exact V011/V017 sources. No dispatch Revision unless the qualification evidence supports it. | None |
| SMALLMID-DATAFLOW-CHAMPION-X | SMD-H6, reuse BF16 parameter conversion for existing rows | Ready for a single-factor declaration after confirming `localRows>1` and buffer lifetime. This is D<=4096 FP32 conversion reuse; Main-2 PARAM-RESIDENCY is D>8192 GM cache policy, so the committed scopes do not overlap. | None |

Route worktrees and branches are the current locations returned by Git, not the older paths in handoff launch tables:

| Route | Branch | Current worktree | Last handoff commit |
|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `w2/m1/sync-topology` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/sync-topology` | `9071292b916d88a9aed0304db64d30435a5f51c4` |
| STORE-EPILOGUE-W2-X | `w2/m1/store-epilogue` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/store-epilogue` | `415a24295525f88babd2abdb300bfcfc6a04074e` |
| EPI-ARITH-CHAMPION-W2-X | `w2/m1/epi-arith` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/epi-arith` | `0ce5441e0e94b06d92f3db596631f35c38870d6c` |
| SELECTIVE-FASTPATH-CHAMPION-X | `w2/m1/selective-fastpath` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/selective-fastpath` | `4d8e073e0279b6860c371d7a13d4c06fdc6f1fec` |
| SMALLMID-DATAFLOW-CHAMPION-X | `w2/m1/smallmid-dataflow` | `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/smallmid-dataflow` | `7f51d2506693b5c6c91fef2f0b867c6bb41aacc9` |

### RULE_CONFLICT

- `ONLINE_OWNER`: the attachment assigns submission ownership and automatic cadence to Main-1. Current `线上提交规范.md` and `调度/主代理分工.md` reserve submission for one unified Judge Owner after Planning approval and explicit confirmation. Main-1 will prepare exact-source packages and queue entries only; formal Judge submission awaits Planning's written resolution and the designated owner.
- `SERVER_CONCURRENCY`: the attachment allows up to five NPU-associated jobs per device; current `服务器实验规范.md` allows up to eight distinct versions total, one per device. The planned five lanes fit the shared allowance when assigned to distinct devices. No second NPU-associated job will be placed on a device; broader concurrency needs Planning to update the project rule.
- `ONLINE_CADENCE`: automatic submission every 18–22 minutes is not compatible with the current approval-and-owner sequence. No timer-driven submission will run while this remains unresolved.

These conflicts affect formal Online submissions and concurrency above one version per device; they do not prevent the four independent Local candidates or the Fastpath qualification from proceeding under the current project rules.

### Server snapshot and scheduler state

- Read-only server snapshot: `hwnput3`, 8 x 910B3, 2026-10-02. Free HBM by device 0–7: `5313, 1409, 5319, 5318, 6346, 1530, 5377, 32332 MB`. Project disk availability: `707 GB`.
- VLLM and Python processes were present on multiple devices and left untouched. No Main-1 job was started by this bootstrap.
- The latest-row lease view has one unreleased entry for device 6: owner `MAIN-2`, route `STORE-EPILOGUE-X`, lease `R2-STORE-V002-TIMING`, started `2026-09-28T00:00:00Z`. Main-1 will avoid device 6 for timing until its owner resolves the lease. No other device has an unreleased lease in the local schedule log.
- `SERVER_JOBS`: zero Main-1 jobs started.
- `NEW_REVERTS`: `0`; historical records remain unchanged.
- `PROCESS_VIOLATIONS`: none observed in this bootstrap.

### Next action

Monitor the five resumed Route Agent IDs, review their committed declarations and qualification report, then assign server jobs by device. Agents may work only in their own worktree. No route may alter another route's files, shared schedule, or lifecycle state. Rebuild the Dashboard after each committed event.

### Dashboard export

```dashboard-json
{
  "handoffs": "5/5",
  "selected": "4 implementation lanes; 1 qualification lane",
  "activeAgents": 0,
  "pendingAgents": 5,
  "activeRevision": "Declarations not yet committed",
  "stage": "ROUTE_CONTEXTS_ASSIGNED",
  "state": "Five existing Route Agent tasks are queued; Main-1 is waiting for declaration commits and the Fastpath qualification report",
  "nextStep": "Review declaration commits; then allocate distinct server devices for build and correctness",
  "blocker": "Formal Online submission owner and cadence require Planning resolution",
  "planningDecisionsChanged": 0,
  "newRoutes": 0
}
```
