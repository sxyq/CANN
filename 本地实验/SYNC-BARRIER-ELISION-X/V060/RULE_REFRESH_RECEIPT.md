# RULE_REFRESH_RECEIPT - SYNC-BARRIER-ELISION-X V060

- Date: 2026-10-08 UTC.
- Ownership: resume the same sole Route context, existing worktree, and existing branch; no replacement owner or duplicate worktree.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V060: `8b86bf47d9d86800a57c2329fca24ae670d2dae2`, V059 result commit. The only pre-existing untracked path was this partial V060 scaffold. A targeted process check found no active V060 compile, correctness, or Local command.
- V059: committed and preserved; numeric score `+1.792574%`, verdict `LOCAL_REJECTED_NOISY`; exact `R31B-V011` remains Local Best. No V059 evidence is modified.
- Rules reread: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; and `项目规则/线上提交规范.md`.
- Route evidence reread: V059 declaration, result, receipt, and source identities; V011 Local Best declaration; and V001-V059 Route-local one-operation diff patches/declarations for synchronization-site duplication. No other Route worktree or shared record was inspected for experiment selection.
- Parent identity: V060 `parent.asc` and pre-edit `submission.asc` were byte-identical, both SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, the recorded exact `R31B-V011` source. V059 parent identity records the same hash.
- Duplication audit: no V001-V059 diff deletes the exact generic `Process()` `SyncSToV()` immediately after `meanSquare` calculation and before `Duplicate(xFp32, meanSquare, 1)`. Nearby distinct tests: V028 deletes the handoff in `ProcessNarrowMidOverlap`; V054 deletes the corresponding handoff in `ProcessSmallLowPrecisionContiguousBatched`; V030 deletes a later `ProcessNarrowMidOverlap` handoff; V037 deletes a later generic-`Process()` handoff after extracting `invRms`; V053 deletes a later batched handoff. These are different call sites.
- Single factor: delete only the generic `Process()` `SyncSToV()` after scalar `meanSquare` and before vector `Duplicate`. Preserve the reduction, scalar computation, `Duplicate`/`Sqrt`, later `PipeBarrier`/`SyncVToS`, output path, and all other source behavior.
- Shape: FP16 8x128 qualification and Local. V037 records width 128 as dispatching to generic `Process()`; the seven-case Correctness runner retains its existing cases, including 8x128 and 128x128.
- Required sequence: finish this scaffold, apply the one Candidate deletion, then Compile as the immediate next experimental action. Do not use an NPU before Compile PASS. After PASS, take a fresh device-3 HBM/process snapshot and use the assigned device only if `FREE_HBM >= 100 MB`; leave all existing processes untouched.
- Local protocol: same-binary qualification followed by two 31-pair interleaved Parent/Candidate device-event blocks, with every raw sample, throughput, wall latency, jitter, and load snapshot retained. No outlier filtering. This single-shape Local is route-local and not comparable to Official 45.16.
- No shared writes, Official comparison, Online, push, other-Route access, new branch/worktree, reset, clean, rebase, or history rewrite.

This receipt precedes the V060 Candidate performance edit.
