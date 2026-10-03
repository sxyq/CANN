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
- `CURRENT_ROUTE`: four existing implementation lanes are in Main-reviewed V001 build/correctness preparation; SELECTIVE-FASTPATH remains qualification-only.
- `ACTIVE_REVISION`: SYNC V001, STORE V001, EPI V001, SMALLMID V001. Candidate source commits exist; build, correctness, performance, and Online have not run.
- `PLANNING_DECISIONS_CHANGED`: `0`.
- `NEW_ROUTES`: `0`.

### Current lane instructions and Main review

The attached C2C CONTROL supplies these current Planning selections. Each Route Agent declared its Revision before the source commit. Main's V001 single-factor review is recorded below. Build and correctness work is approved; SYNC local timing is approved only for its declared primary shape after harness and device-lease revalidation.

| Route | Current instruction | Main review / first action | Revision state |
|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | H2, low-precision parameter prefetch ordering | `SINGLE_CHANGE_AUDIT=PASS`; only the existing MTE2_V wait moved, and it remains before current-slot reads. | V001; source SHA `27c853e1...`; Build and Correctness PASS on primary/control. Timing harness is being prepared; d4 lease released until a fresh lease is granted. |
| STORE-EPILOGUE-W2-X | H1, move the existing full-row Store issue point | `SINGLE_CHANGE_AUDIT=PASS`; the same Store body is issued after the final tile barrier; address, count, geometry, and event sequence are unchanged. | V001; source SHA `48b9428d...`; run-004 evidence committed and pushed; Parent/Candidate outputs both vary across repeats, all four calls RC=3; no timing. |
| EPI-ARITH-CHAMPION-W2-X | H3, group independent row Mul/Add operations | `SINGLE_CHANGE_AUDIT=PASS`; per-element Mul -> Add order is preserved, with independent rows grouped and only the intended barriers moved. | V001; source SHA `9a28f5cd...`; full matrix failed (2 PASS, 10 FAIL, 2 MISSING). D8193 retry also returned ACL 507035 for both sides; no timing. |
| SELECTIVE-FASTPATH-CHAMPION-X | H1, BF16 D32768 full V017 donor qualification | Qualification only: exact M and direct V011/V017 same-binary/paired evidence are still missing. | `QUALIFICATION_INCOMPLETE`; committed evidence audit `85aff92e`; V016 M=2 runner is not evidence of V017's M. No device job assigned. |
| SMALLMID-DATAFLOW-CHAMPION-X | SMD-H6, reuse BF16 parameter conversion for existing rows | `SINGLE_CHANGE_AUDIT=PASS`; FP32 parameter buffers are separately allocated and reused only for BF16 `localRows>1`, within declared D<=4096 scope. | V001; source SHA `a689e5ab...`; Correctness failed at BF16 D=2049; D=3073 and D=4095 were not run. No timing. |

#### Main Review facts

| Revision | Direct parent SHA | Candidate SHA | Main review | Local state |
|---|---|---|---|---|
| SYNC V001 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` | `PASS`; only the declared wait ordering changed; event ownership and buffer depth remain intact. | `BUILD=PASS; CORRECTNESS=PASS (2 shapes); PERFORMANCE=NOT_RUN; LOCAL_VERDICT=NOT_COMPLETE` |
| STORE V001 | `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839` | `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88` | `PASS`; existing Store body moved to the final-tile point; fallback and drain remain unchanged. | `BUILD=PASS; CORRECTNESS=UNRESOLVED (Parent/Candidate both vary on repeated FP32 outputs); PERFORMANCE=NOT_RUN; LOCAL_VERDICT=NOT_COMPLETE` |
| EPI V001 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59` | `PASS`; only same-kind row operations are grouped; each row retains Mul before Add. | `BUILD=PASS; CORRECTNESS=FAILED (2 PASS, 10 FAIL, 2 MISSING); LOCAL_VERDICT=CORRECTNESS_FAILED; PERFORMANCE=NOT_RUN` |
| SMALLMID V001 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a` | `PASS`; separately allocated FP32 parameter buffers cover selected BF16 mid widths and remain live through their final use. | `BUILD=PASS; CORRECTNESS=FAILED at BF16 D=2049; D=3073/D=4095 not run; PERFORMANCE=NOT_RUN` |

Every source SHA above was recomputed from the committed `submission.asc`; parent SHAs were recomputed from their declared parent files. SYNC, STORE, and EPI metadata have the required parent/source fields; SMALLMID metadata and sidecar agree. The original Candidate source pushes and server-side fast-forwards were verified. Subsequent evidence changes are local commits unless a push is separately confirmed. SYNC timing is authorized only after its timing harness is ready and a fresh device lease is recorded; STORE, EPI, and SMALLMID have no timing eligibility in their current correctness state.

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

- Live read-only snapshot: `hwnput3`, 8 x 910B3, Toolkit `8.5.0.alpha002` available. Free HBM by device 0–7: `5313, 5262, 5319, 5318, 6346, 5356, 5357, 13428 MB`; project disk availability `698 GB`; host RAM available `711 GiB`, Swap full but no swap-in/out during the sample, CPU idle 90%, no measured I/O wait.
- Existing VLLM / user Python activity remains untouched. No AddRmsNormBias probe or project build process was present during the process query. The historical `phase4-review-repro/R31B-V016` directory is absent from server3; new build outputs must use unique paths under `/home/data4t2/lelinfeng/cann/`.
- The latest-row lease view has one unreleased entry for device 6: owner `MAIN-2`, route `STORE-EPILOGUE-X`, lease `R2-STORE-V002-TIMING`, started `2026-09-28T00:00:00Z`. Main-1 will avoid device 6 for timing until its owner resolves the lease. No other device has an unreleased lease in the local schedule log.
- `SERVER_JOB_BOARD`: SYNC V001 → device 4; STORE V001 → device 5; EPI V001 → device 7; SMALLMID V001 → device 1. Assigned for build/correctness only, one Candidate per device. Each Route Agent must repeat live device/HBM/process preflight immediately before its run. No performance lease is active for these assignments.
- `SERVER_JOBS`: four assignments staged; source branches are pushed and corresponding server mirrors are fast-forwarded. Harness preparation is pending; no build or correctness job has started.
- `SOURCE_TRANSFER_STATE` (2026-10-02): for each row, local `submission.asc` SHA256, local `submission.sha256` value, server `submission.asc` SHA256, and server sidecar value matched exactly.

| Route | server mirror HEAD | exact source SHA256 |
|---|---|---|
| SYNC V001 | `6ef9657013bf56a08ae2c6b85bf30b1ea7c84a1e` | `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` |
| STORE V001 | `62809fd4c4ff0de516fe307465b1aeb221b349d4` | `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88` |
| EPI V001 | `d5585f0427897c583c61e73dd5a9311496b427d3` | `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59` |
| SMALLMID V001 | `47f72c55d72c01ca6abbb917bd3a1ff4b456bc78` | `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a` |

- `NEW_REVERTS`: `0`; historical records remain unchanged.
- `PROCESS_VIOLATIONS`: none observed in this bootstrap.

### Next action

Have each existing Route Agent prepare and commit its reproducible route-local build/correctness harness, then perform one central fetch/fast-forward before running the assigned matrices. Keep timing and Online closed until those facts are complete. Agents may edit only their own Route worktree; no route may change another route, shared lifecycle state, or Planning decisions. Refresh the Dashboard after the committed campaign update.

### Dashboard export

```dashboard-json
{
  "handoffs": "CASE47, CASE14, FASTPATH, and TINY Track-B handoffs complete; no Official input map; SYNC V001 timing retry preparation active",
  "selected": "5 Planning-approved routes; 0/5 performance hypotheses MAIN_SELECTED",
  "activeAgents": 1,
  "pendingAgents": 0,
  "activeRevision": "SYNC V001 only",
  "stage": "EVENT_DRIVEN_SYNC_PARENT_SAME_BINARY",
  "state": "SYNC V001 60-warmup timing executable Build PASS; fresh Main d4 lease recorded; Parent same-binary is the only allowed timing stage under this lease. Recorded score champion R31B V011/45.16 lacks remote SHA; highest recorded exact triplet is EPI V002/44.96",
  "nextStep": "Record Parent same-binary raw/jitter and release this lease; decide any later qualification stage from its result",
  "blocker": "Official testcase input map unavailable; R31B V011 remote source identity absent; no Candidate timing or Local score",
  "planningDecisionsChanged": 0,
  "newRoutes": 3,
  "lanes": [
    {"route":"SYNC-TOPOLOGY-CHAMPION-X","instruction":"Continue approved V001 Local closure","firstAction":"Run Parent same-binary only under lease M1-SYNC-V001-D4-SAMEBINARY-W60-20261003T021619Z","revisionState":"60-warmup timing executable Build PASS; SHA 7815e6b6…494c6ca; Candidate SHA 27c853e1…a9117ec; new d4 lease active; no Parent window, Candidate P/C, or Local score yet"},
    {"route":"CASE47-SMALL-CLUSTER-CHAMPION-X","instruction":"Track-B NEEDS_MORE_EVIDENCE; no MAIN_SELECTED","firstAction":"Obtain official input map for case4/case7","revisionState":"Research commit 8dfbc642; no Revision; shape/dtype and dispatch remain unknown"},
    {"route":"CASE14-INTRAROW-PARALLELISM-CHAMPION-X","instruction":"Track-B NEEDS_MORE_EVIDENCE; no MAIN_SELECTED","firstAction":"Obtain official input map and source-bound case14 profile","revisionState":"Research commits d9dbd733, 17540c05; D-slice applicability unproven; no Revision"},
    {"route":"SELECTIVE-FASTPATH-CHAMPION-X","instruction":"Research cycle complete; NEEDS_MORE_EVIDENCE; no MAIN_SELECTED","firstAction":"Obtain Official input mapping; no implementation until Main selects a hypothesis","revisionState":"Latest commit d0df90a7; branch ahead of remote by 2; donor local evidence does not prove V011 gain or Official coverage"},
    {"route":"TINY-FIXED-OVERHEAD-CHAMPION-X","instruction":"Track-B NEEDS_MORE_EVIDENCE; no MAIN_SELECTED","firstAction":"Obtain Official input map for case1/3/5","revisionState":"Research commit 200cf036; no Revision; shape/dtype and dispatch remain unknown"}
  ]
}
```

### MAIN-1 branch publication

- `LOCAL_HEAD`: `fa32b2d43ea61732f54239171087ad6a18f8a613`.
- `REMOTE_HEAD_AT_ATTEMPT`: `de46001b80c68118313af2befc25924d00f1ed9d`.
- `PUSH_ATTEMPT`: HTTPS push timed out; a later `git ls-remote` still reported the remote at `de46001b80c68118313af2befc25924d00f1ed9d`.
- `RETRY_RESULT`: ordinary push succeeded.
- `LOCAL_HEAD_AFTER_RETRY`: `dc1cb8a20fcb1360dd7306df7f573dbd7e208fb9`.
- `REMOTE_HEAD_AFTER_RETRY`: `dc1cb8a20fcb1360dd7306df7f573dbd7e208fb9`.
- `PUSH_PENDING`: `NO`.

### Current branch publication state

- `PREVIOUS_PUSH_ATTEMPT`: no result was captured; an origin query later failed to connect to GitHub port 443 after 75 seconds.
- `RETRY_RESULT`: ordinary push succeeded from `f455e54c` through `bc34b8c7f8ed4638595dc4f526dc749aa201c212`.
- `LOCAL_HEAD`: `bc34b8c7f8ed4638595dc4f526dc749aa201c212`.
- `REMOTE_HEAD`: `bc34b8c7f8ed4638595dc4f526dc749aa201c212`.
- `PUSH_PENDING`: `NO`.

## MAIN1_W2_HARNESS_DELIVERY (2026-10-02)

- `MAIN1_HEAD_BEFORE_UPDATE`: `c818012a11557a0ec9d1f364bcd835f8589ec08f`; matched `origin/main1/champion-exploit` after fetch.
- Four implementation lanes now have committed build/correctness harness changes. The SELECTIVE-FASTPATH lane remains qualification-only.

| Route | Harness commit | Server route mirror | Assigned device | Dedicated output path |
|---|---|---|---:|---|
| SYNC V001 | `c0063657a04f4f1c9b746a8377bd820cb26469b4` | `/home/data4t2/lelinfeng/cann-w2-m1-sync` at the same commit | 4 | `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/` |
| STORE V001 | `5b632c4da540cb3d711d72c1c2b9843bae759eba` | `/home/data4t2/lelinfeng/cann-w2-m1-store` at the same commit | 5 | `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/` |
| EPI V001 | `728465c2fc8cd83b1f7fdd67e46d956f1a9305d6` | `/home/data4t2/lelinfeng/cann-w2-m1-epi` at the same commit | 7 | `/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/` |
| SMALLMID V001 | `fefa4ea3321c065e307aae1bcedd8d5e1f5adec8` | `/home/data4t2/lelinfeng/cann-w2-m1-smallmid` at the same commit | 1 | `/home/data4t2/lelinfeng/cann/local-experiments/SMALLMID-DATAFLOW-CHAMPION-X/V001/a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a/` |

The exact Candidate source, `submission.sha256`, server route-tree source, and server sidecar values matched for all four lanes:

| Route | SOURCE_SHA256 |
|---|---|
| SYNC V001 | `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` |
| STORE V001 | `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88` |
| EPI V001 | `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59` |
| SMALLMID V001 | `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a` |

- All four dedicated output paths were absent before staging; no existing server result was replaced. The server canonical checkout remains on `main`, one commit behind its origin, and was not changed.
- Live server snapshot on 2026-10-02: `hwnput3`, free HBM d1/d4/d5/d7 = 5262/6346/5336/11446 MB; project disk available = 698 GB. No active scheduler lease was found on the four assigned devices. VLLM was present on d0-d6; d7 also showed Python/Ray processes. These processes remain untouched. No lane-specific build/correctness process was present.
- `BUILD=NOT_STARTED`; `CORRECTNESS=NOT_STARTED`; `PERFORMANCE=NOT_RUN`; `ONLINE=NOT_RUN`.
- Next action: each existing Route Agent repeats the device/resource snapshot immediately before its assigned exact-source build and correctness run. No timing or Online action is authorized in this stage.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`.

## MAIN1_PORTFOLIO_CORRECTION (2026-10-03)

- Planning portfolio: SYNC continues; STORE and EPI are parked and their worktrees are to close; SMALLMID V001 is correctness-rejected and its worktree is to close; SELECTIVE-FASTPATH gets one research-only cycle. The three approved replacement routes are CASE47-SMALL-CLUSTER, CASE14-INTRAROW-PARALLELISM, and TINY-FIXED-OVERHEAD. No sixth lane is opened.
- Current active portfolio: SYNC, CASE47, CASE14, SELECTIVE-FASTPATH, TINY. Planning decisions are followed as received; this Main changes none of them. Three approved route slots are being populated, with zero routes outside the approved five.
- Champion recomputation from fetched `origin/main`: `OVERALL_CHAMPION=R31B V011`, Official `45.16`, `15/15`. The canonical `result.json`, `source-meta.json`, `submission.sha256`, and retained `submission.asc` all identify source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- STORE V001 closure class: `PARKED_BY_BASELINE_OR_ENVIRONMENT_BLOCK`. Run-004 has Parent and Candidate output variation; all four calls returned RC=3. No Candidate-only correctness failure is established. Preserve branch and V001 evidence.
- EPI V001 closure class: `PARKED_BY_BASELINE_OR_ENVIRONMENT_BLOCK`. The matrix is 2 PASS, 10 FAIL, 2 MISSING; at D8193 both Parent and Candidate returned ACL 507035. The shared wide-FP32 failures do not establish that H3 alone caused them. Preserve branch and V001 evidence.
- SMALLMID V001 closure class: `CORRECTNESS_REJECTED`. BF16 D2049 failed; D3073 and D4095 remain unrun. Preserve branch and complete V001 evidence.
- The STORE, EPI, and SMALLMID worktrees are clean with no ignored or untracked files. Their branches remain present; all existing evidence commits remain reachable. No matching route process or active device lease was found. Before removing those linked worktrees, add an explicit current-source restoration record to each branch while keeping its V001 submission package unchanged.
- SYNC V001 remains the only active Revision. Its first Build/Correctness run passed, but the later pointer-cast Build attempt failed. The current Route Agent is validating the committed fix; no timing process is running. Main will add a fresh d4 lease only after the latest Build and Correctness both pass.
- Support-A (`01a0fe0f-8958-78d2-a7c7-d4eb803a2210`) is mapping Official case families from recorded sources. Support-B (`01a0fe0f-8a32-75c2-930a-4bd3bfab9fb6`) is evaluating existing pipeline and profiler evidence. Both are read-only and may not start device jobs.
- Branch/worktree names for the approved replacements: `w2/m1/case47-small-cluster`, `w2/m1/case14-intrarow-parallelism`, and `w2/m1/tiny-fixed-overhead`, under `worktrees/w2/m1/`. Each will use a minimal sparse checkout and a fresh Route Agent context. All three Direct Parents are the exact Official-backed R31B V011 source above.
- Event order: each Route Agent's committed source triggers its own Build immediately; Build PASS triggers Correctness; Correctness PASS triggers Formal Local Performance once Main records that lane's device lease. Score recording and dashboard refresh happen per result; no lane waits for unrelated routes or pushes.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES_OUTSIDE_APPROVED_5=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_PORTFOLIO_REBALANCE_COMPLETED (2026-10-03)

### Champion and Official Case Evidence

- Recomputed from fetched `origin/main`: current verified anchor remains R31B V011, Official `45.16`, `15/15`. Retained source, sidecar, and result source field all match SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SUPPORT-A found no retained Official testcase→shape/dtype/rows map. `result.json` records IDs, time, and score only. Cases 1/3/5 are each below 10us; cases 4/7 are within the 10–70us time band. Time bands do not prove shared dispatch or a structural cluster. V011 source comments describe intended paths but do not prove the actual Official case inputs.
- SUPPORT-B concluded `PIPELINE_ONLY_EXPLANATION=INSUFFICIENT` for case14's approximately 4.40x Official time ratio. Historical 2.16x full-overlap ceiling and profiler summaries lack original in-repo exports for re-computation; exact case14 rows/D/dtype and utilization remain unknown.

### CASE14 Main Review

- Route Agent commit `5fda6e90` records four Track-B hypotheses at `研究/CASE14-INTRAROW-PARALLELISM-CHAMPION-X/Track-B.md`. Direct Parent SHA matches R31B V011.
- Main review: `NEEDS_MORE_EVIDENCE`; no hypothesis selected. H1's same-row D-slice reduction overlaps historical R008/C001; H2–H4 overlap prior tile, pipeline, and multirow axes. Need Official case metadata, case14 raw profile, C001 exact source/TLE log, and direct-invoke workspace ABI facts before reconsideration.

### Closed Worktrees

| Route | Planning classification | Current source pointer | Restore commit |
|---|---|---|---|
| STORE-EPILOGUE-W2-X V001 | `PARKED_BY_BASELINE_OR_ENVIRONMENT_BLOCK` | STORE-EPILOGUE-X V002 / `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839` | `53d9f04b` |
| EPI-ARITH-CHAMPION-W2-X V001 | `PARKED_BY_BASELINE_OR_ENVIRONMENT_BLOCK` | R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `d9c08342` |
| SMALLMID-DATAFLOW-CHAMPION-X V001 | `CORRECTNESS_REJECTED` | R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `351be0cd` |

The three local linked worktrees were removed after confirming clean status, no ignored/untracked files, no active route process, and no lease. Branches and all source/build/correctness evidence remain. Each V001 `submission.asc` remained unchanged; restoration is recorded separately in `CURRENT_SOURCE.json`.

### Active Main-1 Portfolio

| Route | Branch | Worktree | Agent | Direct Parent | Current stage |
|---|---|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `w2/m1/sync-topology` | `worktrees/w2/m1/sync-topology` | `01a0fd8f-8a90-75a2-b10d-9e4eda3e00dd` | R31B V011 | Direct-source harness commit `84c9cd14` reviewed and ASCPLUGIN registration issue cleared. Latest RUN_ID `...214024Z`: Configure PASS; Parent/Candidate host wrapper compiles failed because `aclrtStream` was undeclared in force-included shim. No new executable, Correctness, or timing; Candidate SHA unchanged. |
| CASE47-SMALL-CLUSTER-CHAMPION-X | `w2/m1/case47-small-cluster` | `worktrees/w2/m1/case47-small-cluster` | `01a0fe2e-76e0-7201-b1d2-6c9487eb1823` | R31B V011 | Track-B; no Revision or `MAIN_SELECTED`. |
| CASE14-INTRAROW-PARALLELISM-CHAMPION-X | `w2/m1/case14-intrarow-parallelism` | `worktrees/w2/m1/case14-intrarow-parallelism` | `01a0fe2e-7772-7e83-825d-e90811e0db2c` | R31B V011 | Track-B reviewed; `NEEDS_MORE_EVIDENCE`; no direction selected. |
| SELECTIVE-FASTPATH-CHAMPION-X | `w2/m1/selective-fastpath` | `worktrees/w2/m1/selective-fastpath` | `01a0fe2e-799a-70e0-8b87-6c81ff9a3292` | R31B V011 | Fresh context on existing branch/worktree; one research-only cycle. |
| TINY-FIXED-OVERHEAD-CHAMPION-X | `w2/m1/tiny-fixed-overhead` | `worktrees/w2/m1/tiny-fixed-overhead` | `01a0fe2e-785a-7332-995f-0e9be2faf3a5` | R31B V011 | Track-B; no Revision or `MAIN_SELECTED`. |

The three new worktrees are sparse, approximately 7.5 MB each, and were created under `cann/worktrees/w2/m1/`. Their working trees are clean. All three use exact Official-backed R31B V011 as Direct Parent; no Local-positive revision is used as a parent.

- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES_WITHIN_APPROVED_PORTFOLIO=3`; `NEW_ROUTES_OUTSIDE_APPROVED_5=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.
- Main-1 and canonical changes are committed locally. No external push was made under the repository push policy; user changes in the canonical worktree remain untouched.

## MAIN1_TRACK_B_HANDOFF_REVIEW (2026-10-03)

### CASE47

- Route handoff commit `2404d4e4` records five hypotheses against R31B V011 source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Main review: `NEEDS_MORE_EVIDENCE`; no hypothesis selected. H2/H5 duplicate recorded row-ownership or transfer-order mechanisms; H3 requires cross-block reduction support unavailable in the current call interface; H4 overlaps prior wide-tile work. H1 is conditionally distinct, but needs the actual case4/case7 shape and dtype plus a minimal API-feasibility prototype before reconsideration.
- Judge testcase IDs and times do not establish input shape, dtype, rows, D, dispatch, or core ownership. No Revision, Build, correctness run, device use, or performance measurement was started.

### SELECTIVE-FASTPATH

- Route handoff commit `5f669c30` studies the complete STORE-EPILOGUE-X V003 donor, source SHA `0cdef265459d4683a1813593a881a5cf25ae75246aab49279d121896b71184ca`, against the R31B V011 fallback. The candidate probe shapes are FP32 `(M,D)=(8,16384),(1,32768),(1,16384)`; none is mapped to an Official testcase.
- The donor's Official score is `44.38`, below V011 `45.16`; its reported Local gains compare against STORE V002 and do not establish a V011 gain. Comparison against visible CASE47 and TINY hypotheses found distinct source-level mechanisms for short-row epilogue, wide-row output chunking, core count, row ownership, generic loop control, buffer footprint, and event-ID lifetime. Actual Official workload overlap remains unknown without the input manifest.
- Main review: `NEEDS_MORE_EVIDENCE`; no hypothesis selected. Obtain Judge input mapping before any implementation discussion. No Revision, Build, correctness run, device use, or performance measurement was started.

### TINY

- Route handoff commit `990a6a37` records five hypotheses against R31B V011 source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; case1/3/5 input shape and dtype are not retained in Judge results.
- Main review: `NEEDS_MORE_EVIDENCE`; no hypothesis selected. H1 changes active core count and may alter `localRows` or select another existing specialization; H2 depends on `rowCount == blockCount`; H3 only applies to a confirmed generic single-tile path; H4 needs generated UB layout/resource evidence; H5 needs API confirmation for conditional event-ID allocation and release.
- The five mechanisms are separate at source level from FASTPATH's wide FP32 store donor, but Official case-to-path mapping is absent, so workload overlap cannot be ruled out. No Revision, Build, correctness run, device use, or performance measurement was started.

- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_CONTINUATION_RECEIPT (2026-10-03)

### Official identity recomputation

- `RECORDED_OVERALL_CHAMPION=R31B V011 / 45.16 / 15-of-15` remains the score recorded in the canonical route ledger.
- Strict source audit requires the retained source bytes, `submission.sha256`, a recorded remote SHA, a matching `result.json` source SHA, a 15/15 result, and `formal_result_eligible=true` where that field exists. `CURRENT_VERIFIED_CHAMPION=EPILOGUE-ARITH-CHAMPION-X V002 / 44.96 / 15-of-15` is the highest result meeting those recorded conditions.
- R31B V011 retained source, sidecar, and `result.json` source SHA agree, but its `source-meta.json` says the result digest came from the submit client, `judge_native_source_hash=MISSING`, and no remote SHA is recorded. Therefore `RECORDED_OVERALL_CHAMPION` and `CURRENT_VERIFIED_CHAMPION` remain separate until Judge-native or remote identity evidence is available.
- The Planning-approved Direct Parent for the five Main-1 lanes remains the exact R31B V011 source. This audit does not change Parent selection, Official score history, or Planning decisions.
- Read-only CANNJudge problem lookup returned the 15 testcase IDs and public timing references only. The submission lookup for R31B V011 returned HTTP 403. No retained source mapped testcase IDs to rows, D, dtype, dispatch, or core ownership.

### Route handoffs and local execution

| Route | Latest recorded state | Candidate / Revision | Next action |
|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | V001 Build and Correctness PASS; prior Parent same-binary blocked at 0.292906 block drift; new 60-warmup timing executable Build PASS | V001; Candidate source SHA unchanged | Run Parent same-binary only under the newly recorded d4 lease, then release it and record the stage result |
| CASE47-SMALL-CLUSTER-CHAMPION-X | Track-B `NEEDS_MORE_EVIDENCE`; no performance hypothesis selected | Research commit `8dfbc642`; no Revision | Obtain case4/case7 input and dispatch map; keep `MAIN_SELECTED=NONE` |
| CASE14-INTRAROW-PARALLELISM-CHAMPION-X | Track-B `NEEDS_MORE_EVIDENCE`; D-slice applicability unproven | Research commits `d9dbd733`, `17540c05`; no Revision | Obtain case14 rows, D, dtype, dispatch, and a source-bound profile; keep `MAIN_SELECTED=NONE` |
| SELECTIVE-FASTPATH-CHAMPION-X | One research cycle complete; mechanism differs from visible CASE47/TINY proposals, Official coverage unknown | Latest commit `d0df90a7`; branch ahead of remote by 2; no Revision | Await Official input mapping; no performance implementation |
| TINY-FIXED-OVERHEAD-CHAMPION-X | Track-B `NEEDS_MORE_EVIDENCE`; no performance hypothesis selected | Research commit `200cf036`; no Revision | Obtain case1/3/5 input and dispatch map; keep `MAIN_SELECTED=NONE` |

- CASE47, CASE14, FASTPATH, and TINY use their existing isolated Route worktrees and branches. No Candidate kernel was changed, no new Revision was opened, and no Official submission was started.
- STORE-W2, EPI-W2, and SMALLMID worktrees remain closed. Their branches and all evidence remain present. Current-source restoration commits are STORE `53d9f04b`, EPI `d9c08342`, and SMALLMID `351be0cd`; classifications remain STORE/EPI `PARKED_BY_BASELINE_OR_ENVIRONMENT_BLOCK` and SMALLMID `CORRECTNESS_REJECTED`.
- The three old evidence branches are ahead of their recorded remote refs by 1, 2, and 13 commits respectively. CASE47, CASE14, and TINY have no matching remote refs; FASTPATH is ahead by 2. SYNC support commits `c0b14f9b` and `32741065` were pushed, bringing its route branch to `origin`.
- The canonical d4 lease was committed as `48baa17f` and pushed to `origin/main`. Latest preflight `2026-10-03T02:16:19Z`: d4 HBM `59190/65536 MB`, AICore `0%`, VLLM EngineCore PID `2999855`, project disk free `607G`; no SYNC runner or active d4 lease existed before this reservation.
- New lease `M1-SYNC-V001-D4-SAMEBINARY-W60-20261003T021619Z` is active for Parent same-binary only. Candidate SHA, server source, local/remote runner source, and timing executable identity matched before the lease. Parent window and Candidate P/C have not run.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES_OUTSIDE_APPROVED_5=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_TIMING_HARNESS_BUILD_ATTEMPT (2026-10-03)

- Main reviewed Route commit `6f0e9046`; it changes timing-harness module registration only. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Direct Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Route-local failure record is preserved in commit `03f595f5` at `本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/BUILD-FAIL-SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6f0e9046-20261002T210858Z.md`; it indexes, without copying over, the server's original logs.
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6f0e9046-20261002T210858Z`. Source, sidecar, Parent and all nine staged harness-file SHA values matched before Configure. CANN `8.5.0.alpha002`, SoC `Ascend 910B3`; d4 free HBM before Build was 2494 MB and project disk available was 617 GB. Existing Python/VLLM processes were left untouched.
- Configure: PASS. Build/Link: FAIL, exit 2. The Candidate module linked, but ASCPLUGIN reported `Unknown kernelInfo` for Parent `add_rms_norm_bias_custom`; the Parent module link then reported three unresolved `__origin__add_rms_norm_bias_custom<T>` symbols. Neither timing nor correctness executable was produced; no Correctness or timing ran.
- Server logs and before/after resource snapshots are under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6f0e9046-20261002T210858Z/`.
- Main's next action is to review a harness-only change that registers Parent and Candidate from their exact ASC source paths and renames only their host `run_kernel` wrappers, following the in-repository WIDE-X-FRESH4 CMake pattern. After commit review, Build/Link proceeds immediately; PASS triggers Correctness immediately. Timing still requires a separate Main lease.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_TIMING_HARNESS_BUILD_ATTEMPT_2 (2026-10-03)

- Harness commit `84c9cd14` registers the exact Parent and Candidate ASC source files directly and gives their host `run_kernel` wrappers distinct names. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-84c9cd14-20261002T214024Z`. Source and seven harness file identities matched. CANN `8.5.0.alpha002`; d4 free HBM before Build was 2488 MB and project disk available was 616 GB.
- Configure: PASS. Build/Link: FAIL, return code `2`; Parent and Candidate host wrapper compilation both report `unknown type name 'aclrtStream'` from `submission.asc:3479`. The force-included `local_abi_shim.h` lacks the ACL declaration. No new executable was produced; Correctness and timing did not run.
- Direct-source module registration cleared the previous ASCPLUGIN `Unknown kernelInfo` failure. Main requested that the next harness-only change add the ACL header to the shim, preserve `submission.asc`, index this failure attempt separately, and submit the shim change for review before another Build.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_EVENT_DRIVEN_EXECUTION (2026-10-02)

- The four earlier Route Agent IDs returned `not_found` from the current runtime. No route process or lane output directory existed at the next server snapshot; no test stage was running then.
- Replacement execution contexts use the same four existing local branches and worktrees. No source change, Revision declaration, route change, or Planning decision was made while replacing the unavailable contexts.

| Route | Agent ID at latest assignment | Existing branch | Existing worktree | Candidate SHA256 | Stage |
|---|---|---|---|---|---|
| SYNC V001 | `01a0fd8f-8a90-75a2-b10d-9e4eda3e00dd` | `w2/m1/sync-topology` | `worktrees/w2/m1/sync-topology` | `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` | BUILD PASS; CORRECTNESS PASS; adding route-local timing harness; no server timing process |
| STORE V001 | `01a0fd8f-8a18-7640-9d7d-7098151510fc` | `w2/m1/store-epilogue` | `worktrees/w2/m1/store-epilogue` | `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88` | run-004 recorded in commit `a1ae33c27513583db2111cdedb5daf87ee5ff02f`; pushed; no timing |
| EPI V001 | `01a0fd8f-8b02-72b2-964d-7b2fed1ba3ad` | `w2/m1/epi-arith` | `worktrees/w2/m1/epi-arith` | `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59` | D8193 retry complete; both calls ACL 507035; evidence commit `4571c82e` local only, not pushed; no timing |
| SMALLMID V001 | `01a0fd14-5474-7140-83fd-054dd4f35187` | `w2/m1/smallmid-dataflow` | `worktrees/w2/m1/smallmid-dataflow` | `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a` | BUILD PASS; CORRECTNESS FAILED at BF16 D=2049; no timing; closed |

- Server snapshot: free HBM d1/d4/d5/d7 = 1412/5121/1494/40757 MB; project disk available = 630 GB. Existing VLLM and Python work remains untouched. No lane-specific compile or runner process was present at this snapshot.
- Per-route flow: fixed local source commit → build → correctness → immediate formal local performance, subject only to that route's own result and the existing per-device timing protocol. GitHub push and unrelated route progress are not prerequisites.
- Each stage produces its own route-local evidence commit. Main updates the route summary, dashboard and Online Queue as each score arrives; no batch closeout is required to advance another lane.
- Online submission remains governed by the current repository approval and unified Judge Owner process. The attachment's different Main-1 ownership/cadence instruction remains a recorded policy conflict.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`.

### Stage results (2026-10-02)

| Route | Build | Correctness | Current action |
|---|---|---|---|
| SYNC V001 | PASS | PASS: `[2,12288] FP16` and `[2,8192] FP16`; matched ratio 1.0 for both | Timing has not run: no device-event harness or active process existed. Idle d4 lease released; prepare harness, then request a fresh lease. |
| STORE V001 | PASS | run-004: Parent repeats differ in 94,690 bytes; Candidate repeats differ in 79,925 bytes; all four golden comparisons return RC=3 | Both variants are unstable on repeated identical inputs; correctness remains unresolved and timing is not eligible. Evidence recorded in route commit `a1ae33c2`. |
| EPI V001 | PASS | Matrix result: 2 PASS, 10 FAIL, 2 MISSING; D8193 retry again returned ACL 507035 for both sides | Correctness failure evidence is complete; no timing eligibility and no further runner assigned. |
| SMALLMID V001 | PASS; executable SHA `159532deffb1ddaf33792c0b7c296c3bcf7cf2f0cc2ea68fbc0e8de7b68e5208`; source SHA matches | FAIL at BF16 D=2049; matched ratio `0.56540386`, max abs `4.26492296`, RC=3; D=3073 and D=4095 not run | Preserve failure evidence; no timing or new Revision |

- The former SMALLMID Agent ID returned `not_found`; replacement `01a0fd14-5474-7140-83fd-054dd4f35187` uses the same branch and worktree.
- SYNC's idle lease `M1-SYNC-V001-D4-PERF-20261002T161619Z` was released at `2026-10-02T17:14:57Z`; no timing runner or samples existed. d4's existing Python and VLLM EngineCore processes were left unchanged. Re-lease only after the harness is ready.
- EPI primary and D8193 retry evidence are at `/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T163852Z/` and `/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T171334Z/`. Both retry calls returned ACL 507035; no TSV was written. No further run or Local performance is assigned.
- STORE run-004 is at `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-004.log` and its sibling output directory. Parent and Candidate both vary across repeated identical inputs; do not infer a Candidate-only regression or run timing.
- No new Local score was produced by SYNC, STORE, EPI, or SMALLMID. None was added to the Online Queue; no Official result or score was changed.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`.

### RECOVERY_UPDATE (2026-10-03)

- SYNC V001: Build and Correctness PASS; Local timing has not run. No timing harness or SYNC process was present on d4, so the idle lease was released. The existing Route Agent is preparing the exact-shape timing harness; a new lease is required before any device measurement.
- STORE V001: run-004 finished on `1x32768 FP32`. Parent repeats differ in 94,690 bytes and Candidate repeats differ in 79,925 bytes. All four invocations return RC=3. The Parent is also nondeterministic, so evidence does not isolate a Candidate-only defect. No performance data exists.
- EPI V001: matrix result is 2 PASS, 10 FAIL, 2 MISSING. D8193 Parent/Candidate retry again returned ACL `507035` and produced no TSV; both sides also fail the runner golden at D12288, D16384, D18416, D18417, and D32768. Local performance was not authorized or run; no more runner is assigned.
- SMALLMID V001: Correctness failure at BF16 D2049 remains; D3073 and D4095 are unrun. No follow-up performance work is assigned.
- No Local result or Official submission was produced. No Online Queue row was added. Main decisions remain unchanged; `PLANNING_DECISIONS_CHANGED=0`, `NEW_ROUTES=0`.

### STORE correctness detail

- Run 001 remains preserved as `INCOMPLETE` because both executables failed to load `libgraph.so`.
- Run 002 resolved runtime loading and completed all 25 cases: 23 PASS; `1x16384 FP32` and `1x32768 FP32` each returned RC=3 for Parent and Candidate, and their output bytes differ. This does not establish correctness against an independent golden on those shapes.
- Run-002 evidence: local commit `34f5063e`; Candidate source SHA remains `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`; server logs are under `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002/`.
- Source review confirms deterministic `x`, residual, gamma, and bias generation shared by Parent and Candidate. Run-003 output variation therefore remains unexplained; no Candidate-specific correctness conclusion is recorded. A four-invocation run-004 on `1x32768 FP32` is assigned, with existing executable identities and a new output directory. No measurement lease or Local timing has started.

### SMALLMID ownership and stage update

- The previous Agent ID `01a0fcf8-0053-7800-a592-897178653e7d` reported work in the same V001 context while replacement `01a0fd14-5474-7140-83fd-054dd4f35187` owned that worktree. Record `OWNERSHIP_CONFLICT=YES`; no Candidate source change is observed.
- Main confirmed the previous Agent is not active, the replacement reports no running command, and server3 has no active SMALLMID runner/build process. Main assigns the replacement as the sole owner of the existing branch/worktree; no second worktree or Revision was created.
- Fixed Candidate SHA: `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a`. Existing Build run `20261002T151744Z-device1` passed and produced executable SHA `159532deffb1ddaf33792c0b7c296c3bcf7cf2f0cc2ea68fbc0e8de7b68e5208`.
- The first launch stopped before a test case because the executable could not load `libgraph.so`; the next run reached the first BF16 case and failed at D=2049 with matched ratio `0.56540386`, max abs `4.26492296`, RC=3. D=3073 and D=4095 were not run. This is `CORRECTNESS_FAILED`; no timing lease was added.
- Evidence is committed at local HEAD `52a54ce6` on `w2/m1/smallmid-dataflow`. Both former owners report completion and are closed; no command is running and no follow-up implementation is authorized in this turn.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`.

## MAIN1_SYNC_TIMING_HARNESS_BUILD_ATTEMPT_3 (2026-10-03)

- Route HEAD at the start of this run was `b7c93ec61771f782ce92478cfb967e0623102fad`; it adds only the prior attempt's numeric Build/Link return code. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Direct Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-e2d3d8c3-20261002T220501Z`. Candidate, sidecar, source metadata, Parent, and all seven support-file identities matched. The remote sidecar verification passed.
- Server: `hwnput3`, Ascend 910B3, `npu-smi 25.0.rc1.1`, CANN `8.5.0.alpha002`. Before and after Build, d4 HBM was `59190/65536 MB` used, AICore `0%`; VLLM EngineCore PID `2999855` remained unchanged. Project disk availability stayed at `616 GB`.
- Configure: PASS. Build/Link: FAIL, return code `2`. Parent and Candidate ASC compilation both report `unknown type name 'TensorGroupInfo'` at `submission.asc:3479`. The prior `aclrtStream` error is cleared; the remaining type is not visible to the ASC source parser through the current host-only force-include options.
- No new timing or correctness executable was produced; Correctness and timing were not run. A pre-existing Candidate shared library had timestamp `2026-10-02 21:14:19Z`, before this run, and is not accepted as this attempt's output.
- Configure, Build/Link, identity, and before/after resource files remain under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-e2d3d8c3-20261002T220501Z/`.
- Next: Route Agent submits a support-only type-visibility change. Main reviews it before another Build; Correctness follows immediately only if both targets Build/Link successfully. Timing still requires a fresh Main lease. No score or Online Queue entry was added.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_TIMING_HARNESS_BUILD_ATTEMPT_4 (2026-10-03)

- Harness/documentation HEAD at run start: `7f73e93d7232ab3bbcc2ac2974efa5f888ffdbf9`. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Direct Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-7f73e93d-20261002T223305Z`. Candidate, sidecar, source metadata, Parent, and all eight support-file identities matched; remote sidecar verification passed.
- Server: `hwnput3`, Ascend 910B3, CANN `8.5.0.alpha002`, `npu-smi 25.0.rc1.1`. Before and after Build, d4 HBM was `59190/65536 MB` used, AICore `0%`; VLLM EngineCore PID `2999855` remained unchanged. Project disk availability stayed at `616 GB`.
- Configure: PASS. Build/Link: FAIL, return code `2`. Parent and Candidate still report `TensorGroupInfo`, `TensorInfo`, and `aclrtStream` undeclared. The logged command placed `local_tensor_metadata.h` inside the AICore option segment; the host segment contains the shim path without the `-include` option.
- No new timing or correctness executable was produced; Correctness and timing were not run. The existing Candidate shared library still had the earlier timestamp `2026-10-02 21:14:19Z` and was not treated as output from this run.
- Configure, Build/Link, identity, and before/after resource files remain under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-7f73e93d-20261002T223305Z/`.
- Next: Route Agent fixes support-only compiler argument routing and updates its reproducibility notes. Main reviews the commits before another Build; Correctness follows immediately only if both targets Build/Link successfully. No score or Online Queue entry was added.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_TIMING_HARNESS_BUILD_ATTEMPT_5 (2026-10-03)

- Harness commit: `9ed799384956d1a747a632ecbd8321730c6882c3`. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Direct Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-9ed79938-20261002T230002Z`. Candidate, sidecar, source metadata, Parent, and all eight support-file identities matched; remote sidecar verification passed.
- Server: `hwnput3`, Ascend 910B3, CANN `8.5.0.alpha002`, `npu-smi 25.0.rc1.1`. Before and after Build, d4 HBM was `59190/65536 MB` used, AICore `0%`; VLLM EngineCore PID `2999855` remained unchanged. Project disk availability stayed at `616 GB`.
- Configure: PASS. Build/Link: FAIL, return code `2`. The generated command now includes both metadata and shim `-include` arguments in the intended compiler sections. Errors for `TensorGroupInfo` and `TensorInfo` are cleared; Parent and Candidate both still report `aclrtStream` undeclared.
- No new timing or correctness executable was produced; Correctness and timing were not run. The existing Candidate shared library timestamp predates this run and was not treated as its output.
- Configure, Build/Link, identity, and before/after resource files remain under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-9ed79938-20261002T230002Z/`.
- Next: Route Agent records this failed attempt, then makes a support-only declaration of the exact `aclrtStream` ABI type for ASC parsing and updates the build notes. Main reviews before another Build; Correctness follows immediately only after both Build targets pass. No score or Online Queue entry was added.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_BUILD_CORRECTNESS_AND_LOCAL_QUALIFICATION (2026-10-03)

- Route commit `6e650cf5224ca0750d9d4886f5ccc428a9edde19` adds the `aclrtStream` ABI alias to the support metadata header. Candidate source SHA remains `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`; Direct Parent R31B V011 SHA remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Build RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`. Configure and both targets passed. Timing executable SHA `3628f9156ab7171e28c0d88a13f6e528f2cf85f138bb412eeab47d35bb24de2f`; correctness executable SHA `7a8335eec7156cf695cf75281a576a2b403e7df5da24de7fd7c1d8759abc97a7`; Parent DSO `017ab02a4fe0da2c2692ba18c17c89ad27cd928ef66a14bf8035d2c4731ec4ec`; Candidate DSO `1a7172946ceab8786920f0e386d2df11307d58e22d86a8f6da90d11cce3844f4`. Build log and identities remain in the RUN_ID directory.
- Correctness ran immediately after Build. `[2,12288] FP16` and `[2,8192] FP16` both PASS, matched ratio `1.0`, max absolute error `0.00048828125`, return code `0`. No performance result was inferred from this stage.
- Timing lease `M1-SYNC-V001-D4-PERF-20261002T232615Z` was added and released; Main-1 lease commits are `2ea6348a` and `18dd2485`, both pushed. First same-binary launch omitted `set_env.sh`, failed to load `libruntime.so` with RC `127`, and produced no samples. Retry sourced the CANN environment and used `SAME-BINARY-SETENV`.
- Parent same-binary retry completed 45 warmups and 62 device-event samples. Parent correctness PASS; overall MAD/median `0.062929`, block medians `10.98 us` and `8.42 us`, block drift `0.292906` (`29.29%`). This exceeds the `0.25` protocol-block threshold: `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`. No Parent window qualification or Candidate P/C ran; no Local delta/score exists. This is not `LOCAL_REJECTED` and does not establish a Candidate defect.
- Parent raw samples, jitter, runner output, and pre/post resource snapshots remain under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z/`. d4 HBM stayed `59190/65536 MB`, AICore `0%`, VLLM EngineCore PID `2999855` unchanged, disk `616 GB`; no experiment process remained after release.
- Current Local state: `BUILD=PASS; CORRECTNESS=PASS; PERFORMANCE=MEASUREMENT_BLOCKED; LOCAL_VERDICT=MEASUREMENT_BLOCKED`. Preserve the Local evidence and source identity. Any fresh measurement requires a new lease after the measurement stability issue is reviewed; no further device work is assigned here.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_SYNC_WARMUP60_AND_WINDOW_RETRY (2026-10-03)

- `CONSOLIDATION_OWNER=CODEX_MAIN`; scope: this SYNC V001 stage summary and its canonical campaign/scheduling/ledger rows. Main-2 is not editing canonical records during this update.
- The retry kept SYNC V001 source SHA `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` unchanged. The timing runner uses 60 synchronized warmups; its rebuilt executable SHA is `7815e6b6c49f1662b245c10e24ee6f0eca8724d455e81b8755c29d331494c6ca`, with runner SHA `ae8d573c62bc952a454ff59178ba9c666fdbc76bee6bfbd31d33ce2b7248912e`.
- Parent same-binary on d4, shape `[2,12288]` FP16: 62 device-event samples, median `8.320000 us`, MAD/median `0.024038`, block drift `0.019231`; `PASS`. The source, Parent and Candidate modules, runner, and timing executable identities are recorded in lease `M1-SYNC-V001-D4-SAMEBINARY-W60-20261003T021619Z` and remote run `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-WARMUP60-20261003T020523Z`.
- The next Parent window lease `M1-SYNC-V001-D4-WINDOW-PC-W60-20261003T023748Z` was released after SSH timed out before the stage began. No Parent window or Candidate P/C samples were produced. A read-only SSH retry from this session also timed out; no remote process was started.
- Current SYNC V001 state remains `BUILD=PASS; CORRECTNESS=PASS; SAME_BINARY=PASS; PARENT_WINDOW=NOT_RUN; CANDIDATE_PC=NOT_RUN; LOCAL_SCORE=NONE`. The W60 raw and jitter files remain on server3 and have not yet been copied into this route's local evidence directory, so this receipt records the measured summary and remote evidence path only.
- Next action: after direct SSH access returns, retrieve and verify the existing W60 raw/jitter evidence, then take a fresh live device snapshot and lease before retrying Parent window. Run Candidate P/C only if Parent window passes. No new Revision, score, Online Queue entry, or Planning decision was added.
- `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES_OUTSIDE_APPROVED_5=0`; `PERFORMANCE_HYPOTHESES_MAIN_SELECTED=0`.

## MAIN1_TRACKB_AND_SYNC_CONTINUATION (2026-10-03)

- Main-1 review branch is pushed through `32c619e5819eb0c1c3aa7de8568e957fa43cc091`. No Candidate source, Revision, score, Online submission, or Planning lifecycle decision changed in this follow-up.
- Support-A's read-only Judge query returned 15 testcase IDs and public `tbest` values only. The R31B V011 submission-detail query returned HTTP 403. Per-case shape, dtype, `availableCoreNum`, dispatch and ownership remain unknown; time values are not used to infer them. The retained C001 result records case1 TLE and cases2-15 skipped, with no case14 execution evidence.
- CASE47 follow-up commit `d2af2ff29fddf9bb1384f38341a8dcba2b83bd99`: H1 D-split is infeasible through the current call interface; H2-H4 duplicate existing R016/SCHED ownership and task-granularity work; H5 needs dispatch reachability. case4/7 inputs and runtime core count remain unknown.
- SELECTIVE-FASTPATH audit commit `f8e28d86f268e07cc49de558fe7e51aa9a70c8d7`: STORE V003 source, V002 parent, V011 fallback and saved Judge identity agree. Local deltas compare V003 with V002; full-donor Official 44.38 is below V011 45.16. CASE47 tile-width/active-core and TINY active-core changes can interact with its dispatch; Official input coverage remains unproven.
- TINY supplement commit `f39f0fb309eed9a4d8a17447f4ebe9ee834e252f`: case1/3/5 share the V011 Host and template entry, but their dtype specialization and device paths are not known. `ACTIVE_CORE_COUNT` is duplicate to R016/SCHED/CASE47-H2; other proposals remain conditional on an exact input manifest and path replay.
- SYNC V001 W60 same-binary remains PASS (median `8.320000 us`, MAD/median `0.024038`, drift `0.019231`). The retrieval Agent, SSH alias, and direct Mac connection to `10.11.32.3:22` all timed out; raw/jitter remain remote. Parent window and Candidate P/C remain NOT_RUN; its lease remains released.
- CASE14 API supplement commit `3ed15bd4fb713b2c0273d8932e2936b84ee4d7d0`: C001 compiles for the recorded CANN/DAV_C220 Vector target and its host wrapper allocates workspace; this confirms expressibility only. Official case1 is TLE and case14 is skipped. Support-B's `PIPELINE_ONLY_EXPLANATION=INSUFFICIENT` uses the 4.3963x case14 time ratio, while the cited 2.1598x pipeline ceiling comes from a historical summary without raw exports; case14 utilization and path remain unknown.
- CASE47, FASTPATH and TINY remain `MAIN_SELECTED=NONE`; CASE14 retains `NEEDS_MORE_EVIDENCE` and no selection. SYNC V001 continues its already approved Local measurement closure. Main-2 remains unchanged. `PLANNING_DECISIONS_CHANGED=0`; `NEW_ROUTES=0`.

## SUPPORT_BOOTSTRAP_RECEIPT / SUPPORT-2 HARDWARE EVIDENCE (2026-10-03)

### Bootstrap receipt

- Fresh context read: root `AGENTS.md`, `.agents/skills/cann-mainline/SKILL.md`, experiment and execution rules, server and local measurement rules, route overview/map/score and revision records, server device ledger, Main-1 W2 campaign, and current evidence for all five Main-1 lanes.
- Canonical route ledger records `R31B V011 / 45.16 / 15-of-15`. The campaign's source-identity review separately records `EPILOGUE-ARITH-CHAMPION-X V002 / 44.96` as the highest result currently meeting its stricter remote/Judge identity evidence. These two recorded fields remain separate.
- `SYNC-TOPOLOGY-CHAMPION-X`: V001 Build and Correctness PASS; W60 Parent same-binary PASS at `[2,12288] FP16` (median 8.32 us, MAD/median 0.024038, block drift 0.019231). The next Parent window stopped before runner start after SSH timeout; its lease is released. No Parent window samples, Candidate P/C, or Local score exist. W60 raw/jitter remain on server3.
- `CASE47-SMALL-CLUSTER-CHAMPION-X`: Track-B needs input shape/dtype and dispatch evidence. H1 D-split is infeasible through the current call interface; H2-H4 duplicate prior ownership/tile directions; H5 still needs dispatch reachability. No Revision or device run.
- `CASE14-INTRAROW-PARALLELISM-CHAMPION-X`: Track-B needs the Official input map and a case-bound profile. Rows, D, dtype, dispatch and active-core count remain unknown. No Revision, build, correctness run, or timing.
- `SELECTIVE-FASTPATH-CHAMPION-X`: STORE V003 Local deltas compare against STORE V002; its Official 44.38 is below R31B V011 45.16. Official case coverage remains unknown. No Revision or device run.
- `TINY-FIXED-OVERHEAD-CHAMPION-X`: no shape/dtype/dispatch map for cases 1/3/5 and no selected hypothesis. `ACTIVE_CORE_COUNT` overlaps R016/SCHED work. No Revision or device run.
- Device ledger last-row evaluation leaves `R2-STORE-V002-TIMING` on d6 under MAIN-2 as `LEASED` from 2026-09-28, with no later row for that lease. The latest SYNC d4 leases are `RELEASED`. This Support context ran no NPU probe and received no lease; Main should resolve the d6 ledger entry before any later d6 reservation.
- Canonical worktree has unrelated user edits. The Dashboard HTML is modified and its template/refresh script are untracked; the diff was read. No Dashboard write or refresh was performed. The campaign file was clean before this additive entry.
- C2C was unavailable in this runtime: `tools.multi_agent_v1__send_input` was not exposed, and local `codex agents --help` only provides session browsing. No Agent ID was returned here, so no C2C message was sent. This report records the handoff facts without using a thread/session API.

### Hardware evidence inventory

The server rules and recorded experiments identify server3 as Ascend 910B3; the architecture reference maps 910B3 to DAV_2201. Numeric examples for other DAV_2201 SKUs are not treated as 910B3 measurements. No NPU command or build was run, and no Candidate file was changed in this review.

| Area | Existing evidence | Limit |
|---|---|---|
| Pipeline activity | `CASE14-SEGMENTED-TIMING.md` reports 535 Parent task samples. In the `>=13 us` duration bucket, V/S/MTE2/MTE3 busy ratios are 0.463/0.157/0.443/0.170; sum 1.233, with a stated ideal-overlap ceiling near 2.16x. Eight-to-six tile round trips saved 0.16 us, about 0.08 us per removed tile including synchronization. | The local probes are `2x/8x/16x32768` and `2x/8x16384` FP32, not mapped to Official case14. Original `op_summary` exports are absent from the retained V002 evidence directory, so the profile summary cannot be independently recomputed here. The report's roughly 110 sync/barrier calls per row at D=32768 is source enumeration, not a per-call latency measurement. |
| `PipeBarrier` | R31A V028 removed two redundant PipeBarriers before `SetFlag` in the D=24576 batch-affine path. Five correctness runs passed. Eight clean paired blocks across d4/d6 favored V028; median delta was -1.09%, while same-code control showed +0.59% slot bias. The handoff estimates roughly 0.008-0.012 us per dynamic barrier for that path. | This is a path-level estimate from a narrow single-change comparison, not a universal primitive latency. The correctness evidence applies to those V-pipe dependencies only. |
| MTE2/V/MTE3 overlap | R31B V017 deferred the MTE3 completion wait. Two attempts measured BF16 D=32768 at -2.00 us (-14.3%), with 16/16 and 20/20 clean pairs; reported overlap scaling is 0.11-0.15 us per tile. R31A V021's analogous FP32 deferred-wait probe remained mixed at D=24576 and had no qualified D=32768 P/C. | These are shape- and path-specific wait-placement results. They do not give isolated `SetFlag`/`WaitFlag` latency or a portable MTE bandwidth value. |
| Load-wait diagnostic | BATCH-RESIDENT V001's follow-up records `second_set_run=false`; its first polling log has `chosen=null` and empty device data for all 16 attempts. | No MTE2 transfer or event-wait duration was measured by this log. |
| DataCopy / bandwidth | API notes state that aligned `DataCopyPad` and `DataCopy` performance is similar and the two `DataCopyPad` parameter forms use the same MTE instruction. | ALIGN-TAIL V001 Candidate timing was withheld after Parent window qualification failed twice; there is no retained direct-copy versus pad-copy paired result or 910B3 bandwidth measurement. |
| Cast and scalar handoff | Existing hypotheses describe per-core parameter casts and V-to-scalar `GetValue` paths. ASYNC-TRIPLE's 0.5-2 us tail estimate is explicitly assumption-based. VECTOR-MATH V002 changed `Muls` to `Duplicate+Mul`, but correctness was 53/54 and short-shape same-binary qualification failed. | No isolated Cast throughput, V/S handoff latency, or `GetValue` latency measurement is retained. V002's timings cannot identify those costs separately. |
| Kernel fixed cost | The short-kernel note records repeated same-binary MAD/median failures for 5-6 us shapes across d4/d5/d6, while 12 us qualified. | This measures repeatability and noise, not an empty-kernel or launch/init fixed-cost floor. No blank-kernel result was found. |
| Active cores / row utilization | V011 host code chooses `blockCount=min(availableCoreNum,rowCount)`. TINY and CASE47 records identify ownership/core-count overlap with prior R016/SCHED work. SCHED V002's 33x100 probe showed five clean pairs favoring the Candidate at -4.4%, while Candidate same-binary drift was 0.258 and the route verdict remained NEEDS_ONE_MORE_LOCAL. | No active-core utilization trace is linked to an Official testcase. Actual row ownership and specialization for CASE14, CASE47 and TINY remain unknown without the input map and a case-bound profile. |

### Dashboard update recommendation

The user's current `index.html` snapshot predates the W60 result and the released Parent-window attempt. When the owner can safely refresh their edited Dashboard, update only `snapshot.main1.state`, `nextStep`, `blocker`, the SYNC lane `revisionState`, and the CASE47, SELECTIVE, and TINY lane states: W60 Parent same-binary PASS; Parent window not started after SSH timeout; no Candidate P/C or Local score; raw/jitter still remote; d4 lease released; CASE47 H1, SELECTIVE H3, and TINY H2 selected for the proxy inputs recorded below, with no result for those experiments. Leave the user's other Dashboard fields untouched until reviewed.

### Current stop point

No kernel-level IR/assembly export, isolated DataCopy/Cast/scalar/kernel-floor measurement, or Official per-case shape/dispatch mapping was found in the searched retained project evidence. Main has selected CASE47 H1, SELECTIVE H3, and TINY H2 as recorded below. Further hardware conclusions require those existing artifacts or an explicitly assigned Main lease; this Support context performed no device probe and made no route selection.

### Main selection update (2026-10-03)

- Main selected `SELECTIVE-FASTPATH-CHAMPION-X` H3, `H3-FP32-WIDE-STORE-V003`, for one conditional donor experiment. The donor is STORE V003; the exact fallback is R31B V011.
- The local proxy labels are `FP32-8x16384`, `FP32-1x32768`, and `FP32-1x16384`, matching the H3 handoff's explicit target shapes. They remain local proxies; no Official case mapping is inferred.
- Current result state for this selected V011-versus-V003 experiment: `CURRENT_LOCAL_RESULT=NONE`, `CURRENT_OFFICIAL_RESULT=NONE`. The existing STORE V003 Official 44.38 and its Local deltas against STORE V002 are historical donor results, not results for this selected comparison.
- The SELECTIVE worktree contains no result package for this H3 comparison at this receipt. No NPU work will start before Main assigns a lease. No shared route/score row was changed by Support-2.

### Main selection update — TINY H2 (2026-10-03)

- Main selected `TINY-FIXED-OVERHEAD-CHAMPION-X` H2, `TINY-H2-ROW-OWNERSHIP-FASTFORM`, for one V001 OFAT from R31B V011.
- The change is limited to per-block row ownership arithmetic when `rowCount==blockCount`; `blockCount`, core count, and dataflow remain unchanged.
- Inputs are local proxies only. The Main update did not include exact proxy labels; the route hypothesis still records M, D, and dtype as unknown. No Official case mapping is inferred.
- The selected experiment has no result package in the searched TINY worktree: `CURRENT_LOCAL_RESULT=NONE_FOUND`, `CURRENT_OFFICIAL_RESULT=NONE_FOR_SELECTED_EXPERIMENT`. Existing R31B V011 results remain the parent record, not results of this experiment.
- No shared route-score or device-ledger row was edited. No NPU work will start before Main assigns a lease.

### Main selection update — CASE47, TINY, and SELECTIVE V001 status (2026-10-03)

- **CASE47-SMALL-CLUSTER-CHAMPION-X H1:** Main selected V001 from R31B V011 for a proxy-only FP32 `D=257`, `M=2*A` experiment, where `A` is read from the runtime vector-core count. The selected H1 is the current fresh-context `ProcessNarrowMidOverlap` hypothesis: group scalar handoff when `localRows>=2`, while preserving each row's math. These inputs do not map to Official case4 or case7. The older Track-B note's H1 label referred to a different hypothesis set; its D-split infeasibility statement does not describe this selected experiment. No Candidate commit, Build, Correctness, Local result, or Official result exists for this selection.
- **TINY-FIXED-OVERHEAD-CHAMPION-X H2:** Main selected `TINY-H2-ROW-OWNERSHIP-FASTFORM` V001 from R31B V011. The fastform applies only under `rowCount==blockCount`; all other inputs retain the exact Parent ownership formula. Proxy is FP32 `D=256`, `M=max(2,floor(A/2))`, with runtime `availableCoreNum=A`; same-shape fallback control uses `B<M`. Numeric `A` and `B` await live runtime reads. Consequently, the fastform condition for the primary proxy is not yet confirmed. No Official mapping is known. No Candidate commit, Build, Correctness, Local result, or Official result exists for this selection.
- **SELECTIVE-FASTPATH-CHAMPION-X H3:** Selection remains the one conditional STORE V003 donor experiment with exact R31B V011 fallback. Its proxies are FP32 `8x16384`, `1x32768`, and `1x16384`; none maps to an Official case. Main's C2C described two untracked V001 review files: `本地实验/SELECTIVE-FASTPATH-CHAMPION-X/V001/REVISION-DECLARATION.md` and `submission.asc`. A read-only status check of the registered Route-4 worktree (`worktrees/w2/m1/selective-fastpath`, branch `w2/m1/selective-fastpath`) now finds both files tracked in HEAD commit `1a43be725337232a7c7102ca6e884127435c2541`; the worktree is clean and the branch is one commit ahead of origin. This differs from the supplied untracked status. Replacement Route Agent `01a100c0-de75-7341-9dda-128a2a8e5de5` is the sole writer reviewing V001. Prior Agent `01a10081-319c-7843-b67f-827666a20b0c` failed with 502 and is closed. The source commit exists; Build, Correctness, Local result, and Official result remain NONE.
- **Support-A handoff:** Prior read-only Agent `01a10081-332f-7030-bc91-518a317070d0` failed with 502 and is closed. Replacement `01a100c0-df25-7723-8046-fd333277dc8f` is read-only. This handoff does not provide an Official input-shape mapping.
- The CASE47 and TINY Route agents have no Candidate commits or experiment results yet. Across all three selections, Local and Official results for the selected experiments remain NONE; the historical parent/donor results above are not results for these selections. No shared Route score or device-ledger row was edited. No NPU probe was run or reserved by Support-2; device work still requires a Main-assigned lease.

### Main selection and Support-A update (2026-10-03)

- **CASE47 H1 / V001:** Main approved the proxy experiment from R31B V011 with FP32 `D=257` and `M=2*A`; `A` is read at runtime from `ACL_DEV_ATTR_VECTOR_CORE_NUM`. All test shapes are proxies; no Official case mapping is known. The selected mechanism groups scalar handoff in `ProcessNarrowMidOverlap` when `localRows>=2` and preserves row math. Candidate commit and Build/Correctness/Local/Official results remain NONE.
- **TINY H2 / V001:** Main approved the ownership formula fastform only when `rowCount==blockCount`, with exact R31B V011 behavior otherwise. Proxy is FP32 `D=256`, `M=max(2,floor(A/2))`, with primary `availableCoreNum=A`; the control keeps the same shape and caps the core count at `B<M`. Live numeric `A`, `M`, and `B` are pending. No Official mapping is known. Candidate commit and Build/Correctness/Local/Official results remain NONE.
- **SELECTIVE H3 / V001:** Source commit is `1a43be725337232a7c7102ca6e884127435c2541`; Candidate SHA-256 is `b7d04e2e5ec3f93503bf2ff7b60291e68009b8856c7a07fe66a222915e50c368`. Main review found that a runtime bool was added to the original kernel entry, weakening exact R31B V011 fallback behavior. Route-4 Agent was asked to preserve this commit and submit a separate implementation fix that restores exact fallback before Build. Build and Correctness have not run; this V001 is not ready for execution or result collection. Local and Official results remain NONE.
- **Agent replacements after 502:** Route-4 old `01a10081-319c-7843-b67f-827666a20b0c` was replaced by `01a100c0-de75-7341-9dda-128a2a8e5de5`; Route-2 old `01a10081-302e-7b32-a670-f051d0054129` was replaced by `01a100cc-5f4d-7522-9512-4244619dcaf3`; Support-A old `01a10081-332f-7030-bc91-518a317070d0` was replaced by read-only `01a100c0-df25-7723-8046-fd333277dc8f`. Each prior Agent failed with 502. These records do not change Route ownership or lifecycle.
- **Support-A group-file findings:** The original `总报告.md` was missing from the searched project and attachment paths; an existing summary was not used as a substitute. The group file contains a negative cube+vector anecdote; FlashRMS was a joke with no experiment; kernel fusion was stated as intent only; case14 `157 us` appeared alongside a timeout and is invalid; the case4/7 cluster claim is an unsupported community hypothesis. Support-A edited no files.
- **Shared status mismatch:** `调度/当前任务.tsv` and `技术路线/路线成绩表.tsv` still show `MAIN_SELECTED=NONE` for the selected Routes. Route-row ownership and cross-Main write responsibility remain unresolved. Support-2 did not edit either TSV; confirm the authorized consolidation owner, with Planning/owner resolution as needed, before changing shared rows.
