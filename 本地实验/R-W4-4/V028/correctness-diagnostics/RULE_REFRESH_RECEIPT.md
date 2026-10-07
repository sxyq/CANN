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
