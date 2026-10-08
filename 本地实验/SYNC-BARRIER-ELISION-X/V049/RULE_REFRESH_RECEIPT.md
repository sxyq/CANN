# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V049

- Date: 2026-10-07 UTC.
- Owner/context: same sole Route owner; the prior owner is no longer a writer.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD at refresh: V048 result commit `479e9cf0a2948349b80f3af7f6aa0bfe28c2641f`.
- Read in this worktree: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`; and V001-V048 Route declarations and actual diff patches for point-level duplication.
- Scope: edit and record only this Route's Candidate and V049 evidence. Do not modify shared ledgers/dashboard, another Route, or submit Online.
- Required loop: one Candidate synchronization edit, immediate Compile, Correctness, Local only after correctness and renewed exclusive device assignment, Result, then scoped commit. Compile failure permits only compile fixes; correctness failure permits only correctness fixes. Preserve negative and tool-failure evidence.
- Baseline: V049 is a fresh sibling from exact R31B-V011; pre-edit `parent.asc` and `submission.asc` must both hash to `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V048 is not inherited. R31B-V011 remains Local Best; V048's numeric `+1.505017%` is rejected noisy, with opposite block scores.
- Single factor: delete only the `AscendC::PipeBarrier<PIPE_V>()` immediately after `FromFloat(outputLocal, valueLocal, totalElems)` and before FP16 gamma/bias application in `ProcessSmallLowPrecisionContiguousBatched`.
- Duplication audit: V001-V048 actual Route diff hunks contain no deletion at this exact operation point. V003/V009 test conversion-to-gamma barriers in `ProcessNarrowMidOverlap`, and V014 tests a pre-conversion barrier in a different generic tiled path. Recent V046-V048 hunks target distinct points earlier in this function.
- Compile/correctness host: local `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`; source installed `aarch64-linux/script/set_env.sh` and set the recorded C++ include path. Correctness follows Compile after a fresh read-only device/process check; no timing lease is assumed for correctness.
- Local: two Parent and two Candidate same-binary 31-pair qualification runs, then two 31-pair interleaved FP16 128x128 device-event blocks, 60 warmups per invocation. Preserve all raw latency, throughput, wall diagnostic, process/load snapshots, jitter and run direction. No timing until Main renews exclusive device assignment after Correctness. Numeric single-shape Local is not comparable to Official 45.16; noisy results are not promoted.
- Safety/network/Git: preserve all evidence and processes; no new worktree/branch, reset, clean, rebase, history rewrite, push, SSH, Online, or shared record/dashboard write. The V048 device-3 assignment ended at result closeout and does not carry into V049.
- Stage at refresh: V049 is scaffolded from R31B-V011 and its build/runner labels are V049; Candidate factor is not yet edited. The next Candidate edit is the declared single barrier deletion; immediate next experimental action is Compile.
