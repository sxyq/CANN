# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V056

- Date: 2026-10-08 UTC; refreshed after V055 commit `fa3e518e` and before the V056 Candidate edit.
- Owner/context: same sole SYNC-BARRIER-ELISION-X Route owner/context; no duplicate Owner or context.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V056: `fa3e518e`, V055 `LOCAL_REJECTED_NOISY`; exact R31B-V011 remains Local Best.
- Read current rules in this worktree: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; and `项目规则/线上提交规范.md`.
- Route-local duplication audit: inspected V001-V055 declarations and actual diff hunks. No prior Route diff deletes the FP16 `AscendC::PipeBarrier<PIPE_V>()` between `Add(xLocal, xLocal, residualLocal, totalElems)` and `ToFloat(valueLocal, xLocal, totalElems)` in the FP16 branch of `ProcessSmallLowPrecisionContiguousBatched`. V041 deletes an analogous barrier in generic `Process()`; V048/V052/V055 delete distinct barriers later in batched paths.
- Baseline: V056 `parent.asc` and pre-edit `submission.asc` are copied from the frozen V055 Parent snapshot, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, exact R31B-V011. No rejected Candidate is inherited.
- Single factor: delete only the FP16 batched-path `PIPE_V` barrier after in-place `Add` and before `ToFloat`.
- Required order: apply this one Candidate edit, then Compile immediately. After Compile PASS, Correctness may run only after a fresh snapshot shows a safe eligible device/context without interfering with another active device operation. Local requires a fresh exclusive device assignment after Correctness PASS; retain all raw samples, throughput, latency, jitter, and load evidence.
- Device state: V055's device-3 assignment was explicitly released at `2026-10-08T03:23:28Z`. No V056 Local assignment is active; request a fresh exclusive allocation before timing. Do not disturb existing processes.
- Local method: FP16 128x128 same-binary qualification and two 31-pair interleaved device-event blocks, with wall latency, throughput, jitter, and load snapshots. Numeric single-shape Local is not comparable to Official `45.16`.
- Scope: existing local host `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`; no SSH, Online, push, shared ledger/dashboard/device-TSV writes, other-Route access, new branch/worktree, reset, clean, rebase, or history rewrite.
- Stage at receipt: V056 scaffolded from exact R31B-V011; Candidate source edit not yet applied. Compile must be the immediate next experimental action after that one-line edit.
