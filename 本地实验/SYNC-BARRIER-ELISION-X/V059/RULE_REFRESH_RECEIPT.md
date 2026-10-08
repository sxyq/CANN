# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V059

- Date: 2026-10-08 UTC.
- Ownership: continuing the same sole SYNC-BARRIER-ELISION-X Route context; no replacement owner/worktree/branch. Read-only process inspection found no active V058/V059 runner or Local command.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`.
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V059: `6b5bbc8ce7e5e813917e33abc7b47a5a354a58ce`, V058 result commit. Existing V001–V058 evidence remains untouched.
- V058 closeout: `LOCAL_REJECTED_NOISY`; +4.654011% from 62 retained interleaved pairs, high event jitter and unstable independent medians. Local Best remains exact `R31B-V011`; V058 is not inherited.
- Rules reread: `AGENTS.md`; worktree `.agents/skills/cann-mainline/SKILL.md`; canonical worktree `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `执行约定.md`; `本地性能测试规范.md`; `服务器实验规范.md`; `Git工作流程.md`; and `线上提交规范.md`.
- Route-local evidence reread: V011 declaration and Local record; V058 declaration, prior rule receipt, result, source metadata, exact Parent source, CMake configuration, correctness/Local runner, and Compile log. No separate `研究/SYNC-BARRIER-ELISION-X/` directory exists in this worktree.
- Duplication audit: inspected the actual one-operation deletion in each available V001–V058 `diff.patch` and cross-checked revision declarations. No prior Route revision deletes the exact common `PIPE_V` barrier after the dtype-specific input-combine branch and before `Mul(xFp32, valueLocal, valueLocal, totalElems)` in `ProcessSmallLowPrecisionContiguousBatched`. Nearby but distinct tested points include V025’s narrow/mid pre-square barrier, V040’s generic-path conversion-to-square barrier, V048’s batched square-to-ReduceSum barrier, and V056’s batched Add-to-ToFloat barrier. The exact V059 site is on the FP16 128x128 batched path used by the assigned Local shape.
- Baseline: V059 will be a fresh sibling of exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, verified against the V058 Parent. V059 was absent at audit time; no revision number is reused.
- Single factor: delete only that one common `PIPE_V` barrier before the batched square `Mul`. Preserve the input combine, square `Mul`, following barrier/reduction, every other operation, and all other source behavior.
- Required sequence: after the one Candidate performance edit, Compile is the immediate next experimental action. On Compile PASS, take a fresh device-3 HBM/process snapshot; proceed with NPU work only at `FREE_HBM >= 100 MB`, leaving all processes untouched.
- Device assignment: device 3 is exclusively assigned to V059 Parent/Candidate Correctness and, only after correctness PASS, same-binary qualification and interleaved Local through raw result capture. Preserve every raw event sample, throughput, wall latency, jitter, and load snapshot; explicitly release after capture.
- Local method: FP16 128x128, device-event primary and wall-clock diagnostics, two 31-pair interleaved blocks with no outlier filtering; numeric route-local score is not comparable to Official 45.16. No Official comparison, shared-record/lease writes, Online, push, other-Route access, new worktree/branch, reset, clean, rebase, or history rewrite.

This receipt precedes the V059 Candidate edit.
