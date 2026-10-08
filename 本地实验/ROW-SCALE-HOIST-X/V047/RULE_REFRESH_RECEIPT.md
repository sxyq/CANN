# V047 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: `c6487228` (`V046` result commit)
- Rule sources reread: `AGENTS.md`; `.agents/skills/cann-route-executor/SKILL.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- Route-local current evidence reread: V046 `RESULT.md`, `local-result.json`, compile record, raw block logs, and V047 scaffold/source.
- Current route result: V046 score `95.0260266050`, Candidate `+5.2343274498%` slower by pooled medians, verdict `MEASUREMENT_BLOCKED`; Local Best remains V026. The three paired-block deltas are `-6.305765%`, `+6.662222%`, and `+56.741573%`.
- Planning authorization: continue this Route's existing row-scale placement/order axis with one OFAT sibling, V047, parented directly to V026. No Online or shared-record edits.
- V047 parent source SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- V047 hypothesis: in FP16 `ProcessNarrowMidOverlap` only when parameters are resident on a multi-row core, form a native-half scratch copy of `gammaLocal * half(invRms)` and use it for the output multiply. Preserve the one-row path and all other dtypes/paths.
- Target: FP16 `[128,2050]`, 40 vector cores/blocks, 3-4 rows per core; this width bypasses the aligned low-precision contiguous path and selects `ProcessNarrowMidOverlap`.
- Device assignment: device 0 is exclusive to V047 correctness and, only after both Parent and Candidate pass, Local through numeric result capture. Do not use it until Compile passes and a fresh HBM/process snapshot is saved; explicitly release after result capture.
- Execution order: one Candidate change -> Compile -> Correctness -> Local if both pass -> result -> commit. Preserve all existing route evidence and untracked scratch.
