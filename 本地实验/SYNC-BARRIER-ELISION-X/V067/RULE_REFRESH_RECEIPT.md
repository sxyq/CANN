# RULE_REFRESH_RECEIPT - SYNC-BARRIER-ELISION-X V067

- RULE_REFRESH_UTC: `2026-10-08T16:36:26.825197490Z`.
- Owner/worktree/branch: same sole SYNC-BARRIER-ELISION-X Route context; `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`; `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- HEAD before V067: `01d52d2eb1259925a2483a8d8ef167e736370a74`, the scoped V066 result commit. V067 did not exist.
- Required current-worktree rules reread: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- V066 result from committed Route-local evidence: Compile PASS, 8/8 FP16 bitwise Correctness PASS, 62 FP16 `128x256` Local pairs, primary paired-median score `+2.347418%`, pooled median-latency ratio `+9.154930%`, mean-latency ratio `-7.362204%`; noisy Candidate outlier and qualification. Verdict `LOCAL_REJECTED_NOISY`; no promotion.
- Current Local Best and V067 Direct Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V067 `parent.asc` and initial `submission.asc` are byte-identical to that source. V066 is not inherited.
- Route-local duplicate audit: checked the exact `Add(valueRow, valueRow, biasFp32, width)` followed by `PipeBarrier<PIPE_V>` context in all 66 available Route-local `submission.asc` files V001-V066; all 66 retain it. No existing Route declaration/result identifies this BF16 row-wise post-Add point as a prior deletion. No other Route worktree or shared record was read.
- Active-path basis: the kernel dispatch sends aligned multi-row FP16/BF16 width 256 to `ProcessSmallLowPrecisionContiguousBatched`; for BF16 the `width > kFp32RepeatMaxWidth` (192) branch applies row-wise FP32-staged gamma/bias. The single selected point is the `PIPE_V` immediately after `Add(valueRow, valueRow, biasFp32, width)` in that BF16 fallback.
- V067 one-factor declaration: delete only that post-Add `PIPE_V` barrier in the BF16 row-wise fallback. Preserve the gamma `Mul`, pre-Add barrier, `Add`, conversion, all other barriers/events, and all other paths.
- Support runner changes only: encode BF16 inputs/parameters (`dtype=2`), add BF16 `128x256` bitwise Parent/Candidate Correctness, and label same-binary and Local evidence as V067/BF16. The Kernel hypothesis remains one barrier deletion.
- Device coordination: device 2 is excluded while MODE-DISPATCH V093 occupancy remains unresolved. After V067 Compile PASS, take a fresh live device/process snapshot and use only a non-conflicting device with at least 100 MB free HBM; prefer device 3 only if its snapshot remains eligible. Do not disturb existing processes. No formal lease API or shared TSV write is required. No device use before Compile PASS.
- No Online, SSH, push, shared-record write, cross-route read, history rewrite, reset, clean, rebase, or Local Best change.

This receipt and the exact Route-local duplicate audit were completed before the V067 Candidate performance edit.
