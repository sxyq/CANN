# V050 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: V049 result commit `9cf0f165e681219c042ec63af739253c57d6bfff`.
- Rule sources refreshed: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`; `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`.
- Route-local evidence refreshed: V026 exact parent source; V021/V022; V027-V029; V032-V049 declarations, focused diffs, and available results. V049 is committed and classified `MEASUREMENT_BLOCKED`; current Local Best remains V026.
- V050 hypothesis: in FP32 `ProcessSmallFp32Batched`, apply each row's existing `invRms` to a reusable FP32 gamma scratch before multiplying the retained value row. Target FP32 `[128,3072]`, 40 blocks, 3-4 rows/core.
- Distinctness: V027 covers value-scale/gamma ordering in this consumer; V039 and V045 cover gamma-operand placement in different consumers. V050 changes only the operand placement in `ProcessSmallFp32Batched`.
- Direct parent: exact V026 source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Candidate SHA-256: `5d769538274d3b0744a478f641c852769a97d070b5b4ed316786dc67be45ad93`; compile source copy matches.
- Compile: PASS for `device` and `submission` at `/tmp/cann-row-scale-hoist-x-v050-compile.20261008T041237Z` using the existing toolkit package path. Configure emitted an unused `ASC_DIR` warning; both requested targets built successfully.
- Device: V049's device-0 assignment was explicitly released after result capture at `2026-10-08T03:49:06Z`. No V050 device assignment has been received. Do not run Correctness or Local until a fresh exclusive assignment is provided; do not use device 0 meanwhile.
- Execution: one Candidate change -> Compile PASS -> await fresh exclusive device assignment -> Parent/Candidate Correctness -> Local only if both pass -> result -> scoped commit. No shared-record or Online work.
