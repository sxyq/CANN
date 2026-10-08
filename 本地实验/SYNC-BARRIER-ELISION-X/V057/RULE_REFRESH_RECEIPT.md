# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V057

- Date: 2026-10-08 UTC; refreshed after V056 commit `31ccb598` and before the V057 Candidate source edit.
- Owner/context: same sole SYNC-BARRIER-ELISION-X Route owner/context; no duplicate Owner or context.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V057: `31ccb598`, V056 `LOCAL_REJECTED_NOISY`; exact R31B-V011 remains Local Best.
- V056 closeout preserved: 62 interleaved pairs; pooled ratio-of-medians `+31.590414%`, block scores `+33.553719%` and `-0.660502%`, pooled device-event CV about 47-49%; no promotion. Device 3 assignment released at `2026-10-08T04:21:58Z`.
- Read current entry and role rules: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`.
- Read current formal route context: `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`; current V056 `RESULT.md`, `REVISION-DECLARATION.md`, `source-meta.json`, source pair, and qualification/correctness/Local evidence. No route-specific `研究/SYNC-BARRIER-ELISION-X` directory is present in this worktree.
- Formal lookup result: the four shared technical-route files contain no `SYNC-BARRIER-ELISION-X` row. The score table's `OVERALL` Champion remains R31B V011 Official `45.16`; V056 route-local records keep `R31B-V011` as this Route's Local Best. No shared file was modified.
- Duplication audit: read this Route's V001-V056 revision declarations and mechanically compared all 56 available route-local `parent.asc`/`submission.asc` pairs. None deletes the `PIPE_V` barrier in `ApplyFp16GammaBiasBatch` between batched `Mul(outputLocal[col], ..., gammaLocal[col], ...)` and `Add(outputLocal[col], ..., biasLocal[col], ...)`. Similar Mul-to-Add barriers in V002/V015/V043 are in different functions/paths; V049 targets the pre-epilogue barrier after `FromFloat`. Actual V029/V031/V036 `SyncVToS()` deletion hunks are at distinct generic or narrow/mid points.
- Baseline: V057 `parent.asc` and unedited Candidate scaffold are to be copied from V056 `parent.asc`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, the exact R31B-V011 source. No rejected Candidate is inherited.
- Single factor: delete only the `PIPE_V` barrier immediately after the batched FP16 gamma `Mul` and before the batched bias `Add` in `ApplyFp16GammaBiasBatch`.
- Required order: make this one Candidate edit, then Compile as the immediate next experimental action. After Compile PASS, Correctness may use a fresh safe eligible device/context without disturbing other active operations. After Correctness PASS, obtain a separate exclusive device assignment before same-binary qualification and Local timing; retain all raw samples, throughput, latency, jitter, and load snapshots.
- Local method: FP16 128x128 same-binary qualification followed by two 31-pair interleaved device-event blocks; wall latency and throughput retained. Numeric single-shape Local is not comparable to Official `45.16`.
- Device state: V056's device-3 assignment was released after capture. No V057 device assignment is active; obtain a fresh exclusive allocation before V057 Local. Do not disturb any existing process.
- Scope: current local host `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`; no SSH, Online, shared ledger/dashboard/device-TSV writes, other-Route access, new branch/worktree, reset, clean, rebase, history rewrite, or push absent a qualifying improvement.
- Revision identifier: V057 is unused in this Route worktree; V056 is committed at HEAD as `31ccb598`.
