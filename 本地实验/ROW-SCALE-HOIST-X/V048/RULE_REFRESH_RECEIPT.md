# V048 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: `5353ab2f` (V047 result and wording correction committed)
- Rule sources refreshed: `AGENTS.md`; `.agents/skills/cann-route-executor/SKILL.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- Route-local current evidence: V047 `RESULT.md`, `local-result.json`, declarations and raw blocks; revision declaration inventory; V035, V042, V043, and V044 declarations/results.
- Current Local Best: V026. V047 is committed with `MEASUREMENT_BLOCKED`; no shared records or Online activity.
- Planning scope: continue one-factor row-scale placement/order probes as siblings from V026.
- V048 hypothesis: in FP16 one-row `ProcessNarrowMidOverlap`, issue the existing half `invRms` Muls on the output before waiting for the already-started gamma/bias MTE2 transfer, then multiply by gamma after the parameter-ready wait. This tests overlap of the row-scale vector operation with parameter DMA.
- Duplicate boundary: V035 changes scale/gamma order in the resident multi-row FP16 branch; V042 moves the wait boundary in FP32 one-row; V044 scales the freshly loaded FP16 gamma after the wait. V048 targets only the FP16 one-row output operand before the parameter wait.
- Parent: exact V026 source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Target: FP16 `[40,3072]`, 40 vector cores/blocks, one row per core, `ProcessNarrowMidOverlap`, `residentParams=false`.
- Device assignment: Planning assigned device 0 exclusively to V048 Correctness and, if Parent and Candidate pass, Local through numeric result capture. Snapshot after Compile PASS at `2026-10-08T02:34:15Z` showed device 0 healthy, AICore 0%, HBM 3432/65536 MB (62104 MB free), and no NPU 0 process. A second pre-correctness snapshot at `02:38:53Z` and pre-Local snapshot at `02:40:49Z` are retained.
- Execution: one Candidate change -> Compile immediately -> Correctness -> Local only if Parent and Candidate pass -> result -> commit.
- Current execution: Compile PASS; Parent/Candidate Correctness PASS; Local numeric result captured as `MEASUREMENT_BLOCKED` (score 119.552054493, delta -16.3544278481%). Device 0 explicitly released at `2026-10-08T02:46:17Z` after numeric capture; see `device-release.log`.
