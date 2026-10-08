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
- Device assignment: exclusive device 0 received for V050 Parent/Candidate Correctness and Local only if both passed. Pre-correctness snapshot at `2026-10-08T04:34:17Z` showed 9137/65536 MB used (56399 MB free), so the assigned >=100 MB gate passed; unrelated processes were left undisturbed.
- Runner setup: the first configure attempt failed because `ASC_DIR` was not supplied; that output is retained in `runners-configure.log`. Retried with the proven route-local package path `/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/tikcpp/ascendc_kernel_cmake`; all four Parent/Candidate correctness/local runner targets built at `/tmp/cann-row-scale-hoist-x-v050-runners.rPvBxa`.
- Correctness: Parent PASS; Candidate FAIL on FP32 `[128,3072]`, dispatch `ProcessSmallFp32Batched`, with candidate delta executed. Runner process exit codes were 0 and 2 respectively. The outer pipeline exited 1 only because tee used `本地実験` instead of `本地实验`; complete stdout and exit receipts were restored under canonical V050 paths. No runner was relaunched.
- Local: NOT_RUN because Candidate correctness failed.
- Release: device 0 explicitly released after correctness capture at `2026-10-08T04:41:56Z`; see `device-release.log` (5226/65536 MB used). V050 result commit: `7e2ecac6c25f3ca561166d712a7b0b298938e04b`.
- Execution: one Candidate change -> Compile PASS -> Parent PASS/Candidate FAIL -> Local NOT_RUN -> result -> scoped commit. Current Local Best remains V026. No shared-record or Online work.
