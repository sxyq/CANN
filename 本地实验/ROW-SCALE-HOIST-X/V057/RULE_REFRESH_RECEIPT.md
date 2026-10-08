# RULE_REFRESH_RECEIPT

- Route: `ROW-SCALE-HOIST-X`
- Revision: `V057`
- Refreshed at: `2026-10-08T15:56:20Z`
- Worktree / branch / HEAD: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4` / `exp/row-scale-hoist-x-w4` / `d553af9b8a7220d7b8b9a680a751bfa3e48100cb`
- V057 was absent. The existing untracked V001/V002 build scratch was observed and left untouched.

## Required rules and project records

The current versions below were read/refreshed before any V057 Candidate edit.

| File | SHA-256 |
|---|---|
| `AGENTS.md` | `1232b8fbc6d82691ef590344ce4b532917d460a5bdb3fa78a8f6b3833cd9434d` |
| `项目规则/实验总则.md` | `5e3f5de523e36f2415c4a44591b70f8b1c45d1a500d29adcc33051fcdb315d34` |
| `项目规则/执行约定.md` | `7e7eb531146b93d66c8acba4bfff0266e7e04454ee206258a8ddbebe606a6` |
| `项目规则/本地性能测试规范.md` | `cecf4753c8e7a1e3ee7a1564e3c7fb20bc767bbcd26310d8790b59607bae5bfc` |
| `项目规则/服务器实验规范.md` | `100605a8a9861d500f9b8113106a9f8e0cc51cdf1eb10902c5ee78de176bb301` |
| `项目规则/线上提交规范.md` | `2c457057d0b48febca0ef87c03b53e1925b1e0a67ed4da4613c97a2f05bab12f` |
| `项目规则/Git工作流程.md` | `b02f29f84dca2a510218669115e2fcb4c46fcfaf9cf34f3bbcd58c7e25b2f8a1` |
| `技术路线/技术路线总表.md` | `21e20edd5a6fe39ec82935a2c0f952641d45bad0b61a2b5d149b6e4a4b919f8b` |
| `技术路线/技术路线图.md` | `d2eddfc7b6bbe003a090e55ad82356183937cc5bf6beecbf26c588e6ae1423a5` |
| `技术路线/路线成绩表.tsv` | `292659636fbbe29d743648658755174cfd4923c12c2d4508f2aa15b564c4f574` |
| `技术路线/全版本记录.tsv` | `4ce799e1a48ea108328f87493d43e022755d0b73311bc8329d585f563306d451` |
| `.agents/skills/cann-mainline/SKILL.md` | `8188d5122baa0333e2c38de480b9cad3b036c78f7ca4083625a78f48af6e21c2` |
| `.agents/skills/cann-route-executor/SKILL.md` | `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/SKILL.md` | `09fdb58767c34b608925af637d5c7f2e9fe74f2f2998decadfbd72e79417c82f` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/references/float_compute.md` | `2dd3af46857f86d86fa7379a1923e2ede1d5e1c7c0ae73f71aaf7955b9c01000` |
| `/home/data4t2/lelinfeng/.agents/skills/ascendc-env-check/SKILL.md` | `441c92e99763aaf5433316fdc480a5943a796d47ae3f8453586d4c46c8c0e1fc` |
| `/home/data4t2/lelinfeng/.agents/skills/ascendc-env-check/references/npu_commands.md` | `b1bdf44b904ed10a37c7742fb2306ac96f70dc8fb584df9cfe96609702cff514` |

The shared route TSVs and route map contain no `ROW-SCALE-HOIST-X` row in this branch. No shared record will be edited; the route-local committed evidence is authoritative for this continuation.

## Route checkpoint and execution constraints

- V056 is closed at HEAD above. V026 remains `CURRENT_LOCAL_BEST` and the exact parent source SHA-256 is `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- V056 numeric result is descriptive only and `MEASUREMENT_BLOCKED`; its device-0 release is recorded. No V057 device assignment is present in this context.
- The loop is one Candidate change, immediate Compile, then Correctness, Local, numeric result, and a scoped route-local commit. Preserve failures/noisy samples. Local measurement uses the canonical 45 warmups, 32 timed samples per invocation, Parent stability, and four interleaved P/C blocks.
- Do not run Correctness or Local without a current non-conflicting device assignment and a fresh live device/process snapshot. Leave existing processes untouched. No shared-record writes, Online, push, branch/worktree creation, or modification of V001/V002 scratch.
- FP32 correctness uses `atol=2^-16`, `rtol=2^-10`, matched ratio `>=0.99`, and max absolute error `<=1e-2` under the project mixed-tolerance standard.

## Route-local duplicate audit and V057 selection

Read and compared the exact V026 declaration/source plus route-local declarations/results for V024, V027, V028, V039, V040, V050, and V056; the complete V001-V056 declaration inventory was searched for row-scale placement/order and gamma-scratch usage.

- V040 targets the same `ProcessSmallFp32FullTileBatched` FP32 `[128,4096]` dispatch, but only moves value-row scaling after gamma multiplication. It does not scale a gamma scratch.
- V050 applies gamma-scratch scaling in `ProcessSmallFp32Batched` at `[128,3072]` and its Candidate failed correctness; it is a different function/subcase and is not a parent.
- V039 applies gamma-scratch scaling in `ProcessFp32FullRowOutputPipelined`; V045/V046 use `ProcessNarrowMidOverlap`. These do not touch the selected consumer.
- V024 already tested the pre-conversion FP16 full-tile placement and is excluded; V057 is not that probe.
- No recorded V026 sibling applies the existing FP32 row scale to a reusable gamma scratch in `ProcessSmallFp32FullTileBatched`.

Selected one-factor V057: on exact V026, in `ProcessSmallFp32FullTileBatched`, reuse the dead-after-reduction `xBuf_` row as scratch for `gammaLocal * invRmsValues[batchRow]`, then multiply the retained `valueRow` by that scaled gamma. Preserve the cached gamma, bias, arithmetic precision/count, reductions, dispatch, buffers, events, stores, and every other consumer. Target FP32 `[128,4096]`; direct parent V026. The next experimental action after editing Candidate is Compile.
