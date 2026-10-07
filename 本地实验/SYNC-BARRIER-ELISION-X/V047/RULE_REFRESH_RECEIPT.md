# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V047

- Date: 2026-10-07 UTC.
- Owner/context: per Main handoff, the prior owner is no longer a writer; this context is the sole writer for the existing Route.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD at refresh: `233996afd4057b7662751661acb016eb25755ccf` (V046 result commit).
- Read in this worktree: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`; and `.agents/skills/ops-profiling/SKILL.md`.
- Role/scope: edit and record only this Route's Candidate and V047 evidence. Do not access another Route worktree, modify shared ledgers or dashboards, decide Route lifecycle, or submit Online.
- Route loop: one Candidate synchronization change, immediate Compile, Correctness, Local, result, and scoped commit. Compile failure permits only compile fixes; correctness failure permits only correctness fixes. Preserve failed and negative evidence.
- Exact baseline: V047 `parent.asc` and pre-edit `submission.asc` both have SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, matching R31B-V011. Do not inherit V046 or any rejected candidate. R31B-V011 remains Local Best; V046's numeric `-4.676489%` result is noisy and rejected.
- Single factor: delete only the `SyncMTE2ToV()` immediately after the batched x/residual `Load`s in `ProcessSmallLowPrecisionContiguousBatched`.
- Route-local duplication audit: inspected the V001-V046 revision declarations and actual `diff.patch` files. No previous diff deletes this exact operation at this function point. V034 and V044 target generic `Process()`; V045 targets parameter loads in `ProcessNarrowMidOverlap`; V046 removes a separate V-to-MTE2 handoff after reductions.
- Compile/correctness: local host `hwnput3`, Ascend910B3 / `dav-2201`, installed CANN `8.5.0.alpha002`; Compile must immediately follow the Candidate edit. Correctness may proceed after Compile and a fresh read-only safety check, without waiting for a Local performance lease.
- Local: use the Route's same-binary qualification and two 31-pair interleaved FP16 128x128 device-event blocks; preserve every raw latency, throughput, wall diagnostic, device/process/load snapshot, jitter, and outlier. No timing until Main renews an exclusive device assignment after Correctness. Numeric single-shape Local is not comparable to Official 45.16; noisy results are not promoted.
- Process safety: do not signal, stop, restart, migrate, or modify other processes. PID 2277907 was absent in the read-only snapshot; do not act on it. Do not overlap Local timing with another Route.
- Git/network: preserve all existing evidence; no new worktree/branch, reset, clean, rebase, history rewrite, push, SSH, Online, or shared-record/dashboard write. Commit only explicit V047 Route paths after result closeout.
- Stage at issuance: V047 scaffold and exact baseline identity verified; one Candidate edit remains; next experiment action after that edit is Compile.
