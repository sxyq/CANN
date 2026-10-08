# V055 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Revision: `V055`
- Refreshed at: `2026-10-08T13:17:34Z`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- HEAD at refresh: `e61f5696d27d3ec221c69b30d063f4c5a6b218c0`
- Current Local Best: `V026`
- Direct parent: `V026`, source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`
- V054: committed at `e61f5696`; `MEASUREMENT_BLOCKED`, not Local Best.
- Existing untracked V001/V002 scratch is preserved and not touched.

## Required project instructions

| File | SHA-256 |
|---|---|
| `AGENTS.md` | `1232b8fbc6d82691ef590344ce4b532917d460a5bdb3fa78a8f6b3833cd9434d` |
| `.agents/skills/cann-mainline/SKILL.md` | `8188d5122baa0333e2c38de480b9cad3b036c78f7ca4083625a78f48af6e21c2` |
| `.agents/skills/cann-route-executor/SKILL.md` | `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8` |
| `项目规则/实验总则.md` | `5e3f5de523e36f2415c4a44591b70f8b1c45d1a500d29adcc33051fcdb315d34` |
| `项目规则/执行约定.md` | `7e7eb531146b93d66c8acba4bfff0266e7e04493654ee206258a8ddbebe606a6` |
| `项目规则/本地性能测试规范.md` | `cecf4753c8e7a1e3ee7a1564e3c7fb20bc767bbcd26310d8790b59607bae5bfc` |
| `项目规则/服务器实验规范.md` | `100605a8a9861d500f9b8113106a9f8e0cc51cdf1eb10902c5ee78de176bb301` |
| `项目规则/线上提交规范.md` | `2c457057d0b48febca0ef87c03b53e1925b1e0a67ed4da4613c97a2f05bab12f` |
| `项目规则/Git工作流程.md` | `b02f29f84dca2a510218669115e2fcb4c46fcfaf9cf34f3bbcd58c7e25b2f8a1` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/SKILL.md` | `09fdb58767c34b608925af637d5c7f2e9fe74f2f2998decadfbd72e79417c82f` |
| `/home/data4t2/lelinfeng/.agents/skills/ops-precision-standard/references/float_compute.md` | `2dd3af46857f86d86fa7379a1923e2ede1d5e1c7c0ae73f71aaf7955b9c01000` |
| `/home/data4t2/lelinfeng/.agents/skills/ascendc-env-check/SKILL.md` | `441c92e99763aaf5433316fdc480a5943a796d47ae3f8453586d4c46c8c0e1fc` |

## Applied requirements

- One Route-local performance change per revision; exact loop is edit -> Compile -> Correctness -> Local -> result -> scoped commit.
- The next experimental action after the Candidate edit is Compile; no research or documentation step will intervene.
- Preserve failed, noisy, and negative evidence. Local is descriptive, not Official; V026 remains Local Best unless a valid improvement is established.
- Do not modify shared records, other Routes, other worktrees, Online state, or V001/V002 scratch. No push is authorized by this task.
- BF16 mixed tolerance: `atol=1/64`, `rtol=1/64`, required matched ratio `>=0.99`, max absolute error `<=1.0`.
- Local protocol: 45 warmups per invocation, 32 timed samples per invocation, 32-sample Parent stability run, and four interleaved Parent/Candidate blocks. Preserve every raw sample and before/after device snapshot.
- Device 0 is assigned to this Route for V055. Take the required live runtime snapshot at the prescribed device stage, leave existing processes untouched, and explicitly record release after result capture.

## Route evidence used

- V026 declaration and source: `本地实验/ROW-SCALE-HOIST-X/V026/REVISION-DECLARATION.md`, `本地实验/ROW-SCALE-HOIST-X/V026/submission.asc`.
- V025 declaration identifies its full-row BF16 change as scale placement on the value operand; V055 instead scales the gamma operand in FP32 scratch.
- V053 declaration applies gamma-scratch scaling in `ProcessSmallLowPrecisionContiguousBatched`, not the V055 full-row consumer.
- V054 result establishes the current closeout and confirms V026 remains Local Best.
