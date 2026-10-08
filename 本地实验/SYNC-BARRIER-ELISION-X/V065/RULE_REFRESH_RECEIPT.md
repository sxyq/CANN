# RULE_REFRESH_RECEIPT - SYNC-BARRIER-ELISION-X V065

- RULE_REFRESH_UTC: `2026-10-08T15:21:39.727576225Z`.
- Owner/worktree/branch: same SYNC-BARRIER-ELISION-X Route context; `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`; `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V065: `c97bd4e307b2b78d9c6512b51369a979d3922415`, V064 result commit. Worktree was clean; V065 is unused and is a fresh sibling.
- Required rules reread: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- Route state from committed Route-local evidence: V064 commit `c97bd4e3`, Compile PASS, Correctness PASS 7/7, 62 Local pairs; paired-median score `-1.474359%` and separate pooled median-latency speedup `+0.645161%`; `LOCAL_REJECTED_NOISY`. V063 also remains rejected/noisy. Neither is promoted.
- Current Local Best and V065 Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V064 Candidate is not inherited.
- Route-local duplication audit: reviewed V001-V064 declarations and available diff patches for the exact row-wise fallback FP16 `PIPE_V` between `Mul(outputRow, outputRow, gammaLocal, width)` and `Add(outputRow, outputRow, biasLocal, width)` in `ProcessSmallLowPrecisionContiguousBatched`. Status: `NO_MATCH_IN_SCOPED_ROUTE_DECLARATIONS_OR_AVAILABLE_DIFFS_FOR_EXACT_FALLBACK_SITE`. V002/V015/V043 are in other functions/branches; V057 modifies `ApplyFp16GammaBiasBatch`, not this fallback. No other Route or shared record was read.
- Active-path basis: for FP16 `128x256` with 8 blocks, row width 256 is 16-element aligned and within `kSmallLowPrecisionContiguousMaxWidth=2048`; 128 rows yield multiple rows per block and dispatch to `ProcessSmallLowPrecisionContiguousBatched`. `kFp16RepeatMaxWidth=192`, so width 256 selects the row-wise fallback branch containing the target barrier.
- V065 single factor: delete only the `AscendC::PipeBarrier<PIPE_V>()` after fallback row gamma `Mul` and before fallback row bias `Add`. Preserve both arithmetic operations, the barrier after Add, all other operations/synchronization, and all other paths.
- Test case: FP16 `128x256` for correctness, same-binary qualification, and two 31-pair interleaved Local blocks. Runner support changes only labels/case dimensions; no additional kernel change.
- Device: V064's device-3 assignment was explicitly released after its result capture. No V065 device assignment has been received. Do not run any V065 NPU runtime until a fresh exclusive device-3 assignment and resource snapshot are available. Compile is host/build work and follows immediately after the Candidate edit.
- No shared-record or lease writes, Online, SSH, cross-route reads, push, reset, clean, rebase, history rewrite, or Local Best change.

This receipt is complete before the V065 Candidate performance edit. Compile is the immediate next experimental action after that one edit.
