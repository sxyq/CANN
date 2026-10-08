# RULE_REFRESH_RECEIPT - SYNC-BARRIER-ELISION-X V061

- RULE_REFRESH_UTC: `2026-10-08T11:50:47Z`.
- Ownership: same sole Route context, worktree, and branch; no replacement owner or worktree.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- V060: result commit `5cf24bc7`, `LOCAL_REJECTED_NOISY`, numeric route-local score `+8.373702%`, paired median delta `-1.2100 us`, 62 raw pairs retained; device 3 released after capture.
- Current Local Best and V061 Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V061 will not inherit rejected V060 Candidate changes.
- Rules reread: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`; and current formal Route/Revision records.
- Route-local evidence reviewed: V060 result/declaration and available V001-V060 Route-local diffs/declarations. No other Route worktree or shared record was inspected for operation selection. `研究/SYNC-BARRIER-ELISION-X` is absent in this worktree.
- Exact-site duplicate check: `NO_MATCH_IN_SCOPED_DIFFS` for the exact `AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(outputRelease)` call guarded by the alternating output-buffer release state at the selected low-precision batched output site. The scoped `rg` exit 1 means no textual match; it is not treated as a tooling failure. This is a source-site uniqueness check, not a claim that removing the wait is behaviorally safe.
- Single factor: delete only that one `WaitFlag<MTE3_V>(outputRelease)` call after `Muls(valueLocal, valueLocal, invRmsValues[batchRow], valid)` and before `FromFloat(outputLocal, valueLocal, valid)`. Preserve the conditional, release-state updates, all SetFlag calls, and every other synchronization and operation.
- Local shape: FP16 128x128, to exercise the low-precision batched path. Correctness retains the existing seven FP16 cases.
- Device 3 assignment: reserved for V061 Correctness/Local after Compile PASS. Do not use device 3 before Compile PASS; then take a fresh HBM/process snapshot and proceed at `FREE_HBM >= 100 MB` without idle gating. Leave existing processes untouched.
- No device timing before the V061 Compile PASS; no shared writes, Official comparison, Online, push, reset, clean, rebase, or history rewrite.
- SLA deviation: V060 `RESULT.md` mtime is `2026-10-08T11:38:07.626311577Z`; V060 result commit time is `11:40:13Z`; the 180-second result-to-next-edit target is already exceeded. At this receipt timestamp, the gap from durable RESULT evidence is at least `12m39.4s`; record the actual V061 edit/Compile timestamps in the next Route event without backdating.

This receipt is complete before the V061 Candidate performance edit. The immediate next experimental action after that edit is Compile.
