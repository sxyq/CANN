# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V048

- Date: 2026-10-07 UTC.
- Owner/context: same sole Route owner; prior owner is no longer a writer.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD at refresh: V047 result commit `523f8c5de6079616ccd78e62a8129fb73a12e1e5`.
- Refreshed from this worktree: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`; `.agents/skills/ops-profiling/SKILL.md`; and V001-V047 Route revision declarations and actual diff patches for point-level duplication.
- Ownership/scope: modify only this Route's Candidate and V048 evidence. Do not edit shared records/dashboard, another Route, or submit Online.
- Authoritative order: one Candidate synchronization change, immediate Compile, Correctness, then Local only after correctness and renewed exclusive device assignment, followed by result and scoped commit. Do not insert documentation or another experimental action after the edit and before Compile.
- Baseline: fresh sibling from exact R31B-V011. Both V048 `parent.asc` and pre-edit `submission.asc` SHA256 are `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V047 is not inherited. R31B-V011 remains Local Best after V047 Correctness failure.
- Single factor: remove only `AscendC::PipeBarrier<PIPE_V>()` immediately after `AscendC::Mul(xFp32, valueLocal, valueLocal, totalElems)` and before the row `ReduceSum` loop in `ProcessSmallLowPrecisionContiguousBatched`.
- Duplication audit: V001-V047 actual Route diff hunks contain no deletion at this exact function/instruction point. V007 targets the corresponding but distinct `ProcessNarrowMidOverlap` site; V046 deletes the later `SyncVToMTE2()` after reduction. V047 deletes the earlier input-load MTE2-to-V handoff.
- Compile/correctness environment: local host `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`; source the installed `aarch64-linux/script/set_env.sh` and set the recorded C++ include path. Correctness follows Compile and may run on a safe device after a read-only process/device check.
- Local method: device-event latency, paired Parent/Candidate qualification, and two 31-pair interleaved FP16 128x128 blocks; keep all raw latency, throughput, wall diagnostic, load, jitter and process context. Wait for Main's explicit exclusive device assignment after Correctness. Noisy results get numeric scores and are not promoted; single-shape Local is not comparable to Official 45.16.
- Preservation/safety: do not signal, stop, restart, migrate or modify other processes. Preserve all evidence. No new worktree/branch, reset, clean, rebase, history rewrite, push, SSH, Online, or shared record/dashboard write.
- Stage at refresh: V048 scaffolded from R31B-V011 with versioned build and correctness labels; candidate factor not yet edited. Next edit is the single declared barrier deletion; immediate next experiment action is Compile.
