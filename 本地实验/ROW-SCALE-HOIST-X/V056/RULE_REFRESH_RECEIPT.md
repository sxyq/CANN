# RULE_REFRESH_RECEIPT

- Route: `ROW-SCALE-HOIST-X`
- Revision: `V056`
- Refreshed at: `2026-10-08T15:13:54Z`
- Worktree / branch / HEAD: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4` / `exp/row-scale-hoist-x-w4` / `b47a09e09b71f31f611258594c6a68232390b914`
- Existing untracked V001/V002 scratch was observed and preserved. V056 was absent before this receipt.

## Required project rules and records

The following files were read in this worktree before any V056 Candidate edit:

| File | SHA-256 |
|---|---|
| `AGENTS.md` | `1232b8fbc6d82691ef590344ce4b532917d460a5bdb3fa78a8f6b3833cd9434d` |
| `项目规则/实验总则.md` | `5e3f5de523e36f2415c4a44591b70f8b1c45d1a500d29adcc33051fcdb315d34` |
| `项目规则/执行约定.md` | `7e7eb531146b93d66c8acba4bfff0266e7e04493654ee206258a8ddbebe606a6` |
| `项目规则/本地性能测试规范.md` | `cecf4753c8e7a1e3ee7a1564e3c7fb20bc767bbcd26310d8790b59607bae5bfc` |
| `项目规则/服务器实验规范.md` | `100605a8a9861d500f9b8113106a9f8e0cc51cdf1eb10902c5ee78de176bb301` |
| `项目规则/线上提交规范.md` | `2c457057d0b48febca0ef87c03b53e1925b1e0a67ed4da4613c97a2f05bab12f` |
| `项目规则/Git工作流程.md` | `b02f29f84dca2a510218669115e2fcb4c46fcfaf9cf34f3bbcd58c7e25b2f8a1` |
| `技术路线/技术路线总表.md` | `21e20edd5a6fe39ec82935a2c0f952641d45bad0b61a2b5d149b6e4a4b919f8b` |
| `技术路线/技术路线图.md` | `d2eddfc7b6bbe003a090e55ad82356183937cc5bf6beecbf26c588e6ae1423a5` |
| `技术路线/路线成绩表.tsv` | `292659636fbbe29d743648658755174cfd4923c12c2d4508f2aa15b564c4f574` |
| `技术路线/全版本记录.tsv` | `4ce799e1a48ea108328f87493d43e022755d0b73311bc8329d585f563306d451` |

## Skills and precision / device guidance

| File | SHA-256 |
|---|---|
| `.agents/skills/cann-mainline/SKILL.md` | `8188d5122baa0333e2c38de480b9cad3b036c78f7ca4083625a78f48af6e21c2` |
| `.agents/skills/cann-route-executor/SKILL.md` | `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/SKILL.md` | `09fdb58767c34b608925af637d5c7f2e9fe74f2f2998decadfbd72e79417c82f` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/references/float_compute.md` | `2dd3af46857f86d86fa7379a1923e2ede1d5e1c7c0ae73f71aaf7955b9c01000` |
| `/home/data4t2/lelinfeng/.agents/skills/ascendc-env-check/SKILL.md` | `441c92e99763aaf5433316fdc480a5943a796d47ae3f8453586d4c46c8c0e1fc` |
| `/home/data4t2/lelinfeng/.agents/skills/ascendc-env-check/references/npu_commands.md` | `b1bdf44b904ed10a37c7742fb2306ac96f70dc8fb584df9cfe96609702cff514` |

## Route-local basis

| File | SHA-256 |
|---|---|
| `本地实验/ROW-SCALE-HOIST-X/V026/REVISION-DECLARATION.md` | `ce2bb13cd47c8066696c7bcb474fbaad7a201491e47c90df0a43829364368493b` |
| `本地实验/ROW-SCALE-HOIST-X/V026/compile/src/submission.asc` | `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9` |
| `本地实验/ROW-SCALE-HOIST-X/V023/REVISION-DECLARATION.md` | `55727c1bb747c558c0640a04f1d43aed24957f974977bffa55f0eced12f09a39` |
| `本地实验/ROW-SCALE-HOIST-X/V053/REVISION-DECLARATION.md` | `27b60b490cc600886ab34eb4e73050d31fbdfed9684e7354b83c68e1ab735c04` |
| `本地实验/ROW-SCALE-HOIST-X/V055/REVISION-DECLARATION.md` | `04549df5288fd6fb0a873bef22caf9777256c00610f874f3deabae4804e76bcf` |
| `本地实验/ROW-SCALE-HOIST-X/V055/RESULT.md` | `ca1aa3de2e314bafaa0f25826c3b9172c0dca3c20fcebc58f226cb760604993b` |

## Refreshed constraints and checkpoint

- Route Agent owns only this route's Candidate and evidence. Shared records remain Record Owner-owned; Online is not authorized.
- The loop is one change, then Compile immediately, Correctness, Local, result, and a scoped commit. Failed/noisy evidence is retained; later revisions start from the current Local Best.
- V055 is committed at the HEAD above with `MEASUREMENT_BLOCKED`; it is not the Local Best. Current Local Best and exact V056 parent are V026, source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Device 0 is allocated to V056 by the current user directive. Use only after Compile PASS; take fresh HBM/process snapshots before Correctness and before Local, leave existing processes untouched, and explicitly release after numeric result capture.
- BF16 correctness uses mixed tolerance `atol=rtol=1/64`, matched ratio `>=0.99`, and max absolute error `<=1.0`.
- Local protocol: 45 warmups, 32 timed samples per invocation, 32-sample Parent stability, and four interleaved Parent/Candidate blocks. Preserve every raw sample and report descriptive numeric results separately from qualification/promotion.

## V056 single-factor selection

- Target: BF16 `[128,4096]`, `ProcessBf16FullTileBatchedOutputPipelined`, direct sibling of V026.
- Change: after row reductions, use the dead-after-reduction `xFp32Buf_` tile as scratch for `gammaFp32 * invRmsValues[batchRow]`; then multiply the retained FP32 value row by that scaled gamma scratch before the unchanged bias and BF16 conversion.
- This changes only the row-scale operand placement in this consumer. It adds no operation, buffer, allocation, synchronization, dispatch, or store change.
- Distinctness check: V023 changes the scale/gamma order on the value operand in this full-tile consumer; V055 applies gamma-scratch scaling in the separate BF16 full-row consumer; V053 applies it in the separate small contiguous-batched consumer. V056 tests the gamma-scratch placement only in the full-tile batched consumer, from exact V026.
- No Candidate edit had occurred when this receipt was written.
