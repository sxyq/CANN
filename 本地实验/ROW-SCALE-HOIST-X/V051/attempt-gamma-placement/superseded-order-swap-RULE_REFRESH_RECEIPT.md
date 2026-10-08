# V051 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: `a739b217` (V050 receipt correction; V050 result commit `7e2ecac6` is preserved).
- Rule sources refreshed: `AGENTS.md`; `.agents/skills/cann-route-executor/SKILL.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- Route-local evidence refreshed: exact V026 source and SHA; V027, V028 and V029 changed-path evidence; V032-V050 declarations and available focused diffs/results. V050 Candidate correctness failed in `ProcessSmallFp32Batched`; Local was not run. Current Local Best remains V026.
- V051 hypothesis: in FP32 `ProcessSmallFp32ContiguousBatched`, narrow helper subcase (`rowWidth <= kFp32RepeatMaxWidth`), move the existing row-scale `Muls` from before gamma multiplication to after gamma and before bias.
- Duplicate audit: V028 changes the wider (`rowWidth > kFp32RepeatMaxWidth`) direct-loop subcase of the same consumer at width 2048 and is correctness tooling-blocked. V027 targets `ProcessSmallFp32Batched` at width 3072. V029 has no edit in `ProcessSmallFp32ContiguousBatched`. No prior V026 sibling covers the narrow helper subcase.
- Direct parent SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Candidate source is staged from exact V026 pending the single declared edit. No V051 correctness or Local device assignment has been received; do not use any device for correctness/Local until a fresh exclusive assignment is received.
- Execution: one source edit -> immediate Compile -> await fresh device assignment -> Parent/Candidate Correctness -> Local only if both pass -> result -> scoped commit. No shared-record or Online work.
