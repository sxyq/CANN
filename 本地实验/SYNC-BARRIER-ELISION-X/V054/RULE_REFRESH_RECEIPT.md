# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V054

- Date: 2026-10-08 UTC, refreshed before the V054 Candidate edit.
- Owner/context: same sole Route owner/context; no duplicate Owner or worktree. Previous owner remains non-writing per handoff.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V054: `18ba122d`, completed V053 noisy-result commit. Worktree was clean before scaffolding.
- Current Local Best: exact R31B-V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; V053 is `LOCAL_REJECTED_NOISY` and is not inherited.
- Read current rules: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; and `项目规则/线上提交规范.md`. Rechecked the Route's current result in V053 and the exact source call site.
- Duplication audit: searched all V001-V053 Route-local `diff.patch` files for removed `SyncSToV()` calls and reviewed relevant declarations. The selected call after `meanSquares` extraction and before scalar-slot `Duplicate` in `ProcessSmallLowPrecisionContiguousBatched` has not been deleted. V028 targets the analogous `ProcessNarrowMidOverlap` handoff; V030/V037 target different scalar handoffs; V053 targets the later `invRmsValues` handoff.
- Baseline: V054 `parent.asc` and pre-edit `submission.asc` both hash to `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; this is a fresh sibling from exact R31B-V011.
- Single factor: delete only the `SyncSToV()` after the scalar `meanSquares` extraction loop and before `Duplicate(scalarSlot, meanSquares[batchRow], 1)`.
- Required order: apply that one Candidate edit, then Compile immediately; Correctness only after Compile PASS, Local only after Correctness PASS. Preserve failures and all raw measurements.
- Device: V053's device-3 assignment is released. No V054 device assignment has been issued. Request and receive a fresh exclusive allocation before Correctness or Local; do not use device 3 or another device for those actions before assignment.
- Method: retain the V053 route-local 128x128 FP16 runner, same-binary qualification, two 31-pair interleaved device-event blocks, raw throughput/wall/device timing, jitter and load snapshots. Single-shape Local is not comparable to Official `45.16`.
- Scope: existing local host only; no SSH, Online, push, shared ledger/dashboard/device-TSV writes, other-Route access, new branch/worktree, reset, clean, rebase, or history rewrite.
- Stage at receipt: V054 scaffolded from exact R31B-V011; Candidate edit not yet applied. Compile is the immediate next experimental action after the one-line edit.
