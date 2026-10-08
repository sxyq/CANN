# V051 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: `a739b217`; V050 correctness result commit `7e2ecac6` and corrected assignment receipt commit `a739b217` are preserved.
- Rule sources refreshed: `AGENTS.md`; `.agents/skills/cann-route-executor/SKILL.md`; `.agents/skills/cann-mainline/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`; and the required route overview, route map, score table, and revision ledger.
- Route-local evidence refreshed: V026 exact source SHA; V050 compile and correctness result; V039/V045 consumer boundaries; the pre-directive V051 order-swap source SHA `ccf35a52e8e53a87b08740f18d1c2e558bf5b2e3b358bc9af1d54c3c7ff8774b` and its compile log. V050 Candidate correctness failed and Local was not run; V026 remains Local Best.
- Current authorized V051 factor: per-row `invRms` is applied to a gamma scratch only in the narrow FP32 `ProcessSmallFp32ContiguousBatched` subcase. Direct parent source is copied byte-for-byte into this attempt before the single edit.
- Preservation: no prior V050/V051 source, compile log, result, or raw evidence was modified or removed. The old V051 files are retained as a superseded compile-only attempt, not used for correctness.
- Device 0 assignment is held for V051, but no device call is permitted until the current Candidate and freshly built executable are tied to the exact candidate SHA. Take a fresh HBM/process snapshot after Compile PASS; proceed at `FREE_HBM >= 100 MB` without a load gate, and release after result capture.
- Current stage: source edit pending; after that edit the immediate next experimental action is a fresh Compile to a distinct build/log path. No shared-record or Online work.
