# RULE_REFRESH_RECEIPT — R-W4-4 V028

- ISSUED_UTC: 2026-10-06T23:10:55Z
- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V028
- WORKTREE: `/home/data4t2/lelinfeng/cann-r-w4-4`
- BRANCH: `routes/r-w4-4-mode-dispatch-cutoff-x`
- HEAD: `0b22ce0e0b5b1fcbd5970d297ad9d1a805e96c67`
- ROLE: replacement sole owner; correctness diagnosis only

## Rules and skills reread

- `AGENTS.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/本地性能测试规范.md`
- `项目规则/服务器实验规范.md`
- `项目规则/线上提交规范.md`
- `项目规则/Git工作流程.md`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `技术路线/路线成绩表.tsv`
- `技术路线/全版本记录.tsv`
- `调度/当前任务.tsv` (read-only lookup; no matching row for this Route/branch)
- `.agents/skills/cann-mainline/SKILL.md`
- `.agents/skills/cann-route-executor/SKILL.md`
- `.agents/skills/cann-main-orchestrator/SKILL.md`
- `.agents/skills/cann-record-owner/SKILL.md`
- `.agents/skills/cann-support-research/SKILL.md`
- `.agents/skills/cann-online-owner/SKILL.md`
- `ascendc-docs-search`, `ascendc-api-best-practices`, `ascendc-precision-debug`, `ops-precision-standard`
- Route research lookup under `研究/` found no `R-W4-4` or `MODE-DISPATCH-CUTOFF-X` record.

## Enforced constraints

- Keep V028 as one change: `kSmallFp32BatchMaxWidth` 512 -> 256.
- Do not run Local while exact Parent or Candidate correctness fails.
- Do not create V029 or add a performance change.
- Do not modify shared records, another Route, or Online state.
- Preserve all prior correctness/build evidence.

## Current evidence status

- Exact Direct Parent source: V027 SHA `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- Patched Candidate source: `9fc8ded08c6a9f0dc1392dcc46bbe6572f0c66fd65df81f57a2c18700a295c37`.
- Candidate patch is one correctness-only `SyncVToMTE2()` after each wide-FP32 output tile, before reusing `xBuf_`/`residualBuf_` for the next parameter DMA.
- Existing Parent/Candidate C15 failures reproduce on device 2 and device 7; the width sweep passes D=8192 and fails D=8193, D=16384, and D=32768.
- Three repeated exact Parent D=32768 runs fail with varying bad counts, so the current evidence supports an existing wide-FP32 synchronization/lifetime defect, not a V028 cutoff regression.

## Post-fix update (2026-10-06T23:10:55Z)

- Patched Candidate now passes the route-bound C01-C16 matrix (16/16) on device 2; D=32768 also passes two repeated direct correctness probes. Full details: `V028-CORRECTNESS-FIX-RESULT.md`.
- Exact V027 Parent still fails the same route-bound C15 on device 2 (`rc=3`, `bad=30306`, `max_abs=1.2031`); Parent evidence remains in `PARENT-direct-device2-20261006T2315Z/`.
- Final gate classification is Candidate correctness repaired; Local remains blocked by the exact Parent failure. No Local or Online was run.

## Runner/input identity finding

- The shared generic `runner_main.inc` is absent from this V028 stage and is therefore a genuine `TOOLING_BLOCKER` for that runner path.
- The available direct route-bound executable is `clx_ref_candidate_probe`, built from `runner_ref_candidate.asc` -> `runner_ref.inc` with `SRX_SUBMISSION "submission.asc"`; the Parent twin binds `parent.asc`.
- The runner generates deterministic inputs in `runner_ref.inc`: `InputValue(index,37,11)` for x, `InputValue(index,17,3)` for residual, gamma `0.75 + ((i*13)%100)/200`, and bias `((i*7)%100)/400 - 0.125`, with dtype conversion before H2D. These are harness inputs, not official Judge input bytes.
- Existing probe dependencies bind to this worktree's staged source paths; no other Route or shared source was used.

## Disposition

`PARENT_SHARED_FAILURE` remains an active hard blocker. The route-specific harness is usable for correctness-only diagnosis, but its Parent result is still failing; therefore Local, V029, performance edits, shared-record edits, and Online remain prohibited.

## Refresh receipt — 2026-10-07T02:41:11Z

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- WORKTREE: `/home/data4t2/lelinfeng/cann-r-w4-4`
- BRANCH: `routes/r-w4-4-mode-dispatch-cutoff-x`
- HEAD_AT_REFRESH: `6508cbdf` (`docs(R-W4-4): close V028 parent failure diagnosis`)
- CANONICAL_ROUTE_SKILL: `.agents/skills/cann-route-executor/SKILL.md` read from `origin/main` at `d8c3b380276df5549f057dee19031e04fdd874a9`; local and canonical SHA256 both `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8`.
- REQUIRED_RULES_READ: `AGENTS.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`; `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`.
- ROUTE_LOOKUP: no matching Route row in `调度/当前任务.tsv`; route-specific research lookup returned no record under `研究/`.
- CURRENT_STAGE: route-local V028 correctness diagnostic evidence, pre-commit.
- ACTIVE_COMMAND: `NONE`.
- NEXT_ACTION: commit only the V028 route-local correctness diagnostics and this receipt; then continue exact canonical Parent correctness reproducibility.

### Control-copy finding, not a gate change

- The canonical V027 Parent SHA `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033` still fails C15 on the route-bound device-2 probe (`rc=3`, `bad=30306`, `max_abs=1.2031`).
- A separate diagnostic copy SHA `c3007e431d75f3123b8f9d45cbbabaf6440b23596bd26b665a79d12f6ce5dde8` differs only by one inserted `SyncVToMTE2()` call (plus explanatory comments). Its 19-case matrix passed with all `rc=0` and `bad=0`; raw output is in `PARENT-REPAIR-SYNC-20261007T021547Z/`.
- The first standalone launch of that diagnostic copy returned `127` because `libgraph.so` was not on the loader path; after loading the CANN environment, the C15 retry returned `0`. The launch failure is not counted as a kernel correctness result.
- The repaired copy is not the exact Parent, not an official artifact, and does not clear `PARENT_SHARED_FAILURE`. No Local, Online, V029 performance edit, or shared-record edit is authorized by this result.

## Replacement-owner refresh — 2026-10-07T06:03:40Z

- ROLE: replacement sole owner for the existing route, explicitly assigned by the user; no new branch or worktree.
- WORKTREE / BRANCH: `/home/data4t2/lelinfeng/cann-r-w4-4` / `routes/r-w4-4-mode-dispatch-cutoff-x`.
- HEAD_BEFORE_RUN: `4515b5c8bb5016c54730e0b6cd413df4da0787b9` (prior owner's exact Parent C15 diagnostic).
- RULES_REREAD: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md` from `origin/main`; `项目规则/实验总则.md`; `执行约定.md`; `服务器实验规范.md`; `本地性能测试规范.md`; `线上提交规范.md`; `Git工作流程.md`; technical route tables/map and scheduler tables. Canonical Route Executor SHA256 was `da53e318fbef98813e638ae8a5aa7b2a367f18bde84dd284e5b6d717ee17f666`; its active-portfolio text conflicts with this user's explicit replacement assignment, which is followed without widening scope.
- ROUTE_FACTS: V028 single change remains `kSmallFp32BatchMaxWidth 512 -> 256`; exact V027 Parent SHA256 remains `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`; prior exact C15 attempts failed. Repaired Parent SHA is forbidden and was not used.
- HOST_TARGET: current local host `hwnput3`, as clarified by the user. After that clarification, no SSH/external host query was used. A local read-only 8-device snapshot was captured.
- NEXT_ACTION / RESULT: one exact Parent-only C15 FP32 `1x32768` probe on local device 4; `FREE_HBM` conservative lower bound `5898 MB` from 65536 MB capacity and reported 90% usage (+1 percentage-point safety allowance); AICore `0%`, AIVector `0%`, existing process PID `2999855` unchanged. Result: `rc=3`, `bad=28174`, `max_abs=1.2031`.
- EVIDENCE: `PARENT-C15-RETRY-20261007T055757Z/` contains exact source/runner/wrapper/executable hashes, command, runtime environment, raw sample/stats/stdout/stderr, launch and post HBM/load/process snapshots, full per-device pre snapshots, process tables, timestamps and disk snapshots.
- DISPOSITION: genuine exact-Parent correctness-baseline blocker. Stop this V028 experiment loop. No Candidate correctness, Local performance, repaired Parent, V029, Online, Candidate edit, or shared-record write. Existing untracked V029/build outputs were preserved.

## Planning control refresh — 2026-10-07

- CONTROL: latest Planning instruction supersedes the inherited 180-second CHILD_SLA termination rule. Keep this same Route owner, context, branch, and worktree; do not spawn a duplicate writer. Never close the owner due to elapsed time, heartbeat/network delay, long commands, result documentation, or a Git commit.
- ROLE / SCOPE: Route-local work only. Main is coordination-only and performs no edits, Git, compile, correctness, Local, NPU, or shared-record actions. Online is paused. Use the current local host only; no SSH, DNS, or host-key work.
- KNOWN BASELINE: `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact V027 Parent SHA `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`, C15 FP32 `1x32768`. Do not repeat that probe; classify it as a Parent defect, not a Candidate regression.
- RESULT BOUNDARY: `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`. Any route-local ranking is restricted to cases where the exact direct Parent and Candidate both pass. A partial result is never an Official candidate.
- IN-FLIGHT REVISION: existing V029 is preserved, not duplicated or overwritten. Its recorded single factor is `kSmallFp32BatchMaxWidth 256 -> 128`, Parent SHA is the original V028 `06384465fe0e3831048fc15903807ec3aeae55cdcb1852f5d85d60fbb9154ffb`, Compile is PASS, and Correctness/Local are not run. The current repaired V028 Candidate has a different SHA; V029 identity and parent binding must be respected during any route-local continuation.
- NEXT ACTION: inspect existing route-local case identity and Parent/Candidate pass evidence, then continue only eligible width/rows/dtype/shape cutoff ranking. Preserve current Candidate, V029, and every failed/noisy result. No shared records, Online, push, or lifecycle decision.
