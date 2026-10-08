# ROW-SCALE-HOIST-X V053 Rule Refresh

- Refreshed: 2026-10-08.
- Worktree/branch: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`, `exp/row-scale-hoist-x-w4`.
- Route checkpoint: HEAD `3392f8a47f81561a2fffae25a180f4b693cf37d6`, V052 negative-result commit. Existing untracked V001/V002 scratch is preserved.
- Current Local Best and V053 direct parent: V026, source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- V052: `LOCAL_REJECTED`; no promotion. Its candidate was slower by pooled median and mean and all four paired block medians; device 0 was released after capture.
- Read in full: `AGENTS.md`; `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`.
- Rule source SHA-256 values:
  - `AGENTS.md`: `1232b8fbc6d82691ef590344ce4b532917d460a5bdb3fa78a8f6b3833cd9434d`
  - `.agents/skills/cann-route-executor/SKILL.md`: `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8`
  - `项目规则/实验总则.md`: `5e3f5de523e36f2415c4a44591b70f8b1c45d1a500d29adcc33051fcdb315d34`
  - `项目规则/执行约定.md`: `7e7eb531146b93d66c8acba4bfff0266e7e04493654ee206258a8ddbebe606a6`
  - `项目规则/本地性能测试规范.md`: `cecf4753c8e7a1e3ee7a1564e3c7fb20bc767bbcd26310d8790b59607bae5bfc`
  - `项目规则/服务器实验规范.md`: `100605a8a9861d500f9b8113106a9f8e0cc51cdf1eb10902c5ee78de176bb301`
  - `项目规则/线上提交规范.md`: `2c457057d0b48febca0ef87c03b53e1925b1e0a67ed4da4613c97a2f05bab12f`
  - `项目规则/Git工作流程.md`: `b02f29f84dca2a510218669115e2fcb4c46fcfaf9cf34f3bbcd58c7e25b2f8a1`
- Revision inventory: V053 did not exist at refresh. The V026-to-V052 source diff changes only the BF16 narrow (`width <= 192`) subcase of `ProcessSmallLowPrecisionContiguousBatched`; its wide (`width > 192`) subcase remains V026-identical. V051 targets FP32 contiguous-batched, and V043/V046/V047 target `ProcessNarrowMidOverlap`. The V053 wide BF16 consumer/subcase point is distinct from those recorded probes.
- Authorized order: one Candidate change from exact V026, then immediate Compile. Device 0 remains assigned to V053 but must not be used before Compile PASS; take a fresh device snapshot and require FREE_HBM >= 100 MB before correctness. Run Local only after both Parent and Candidate correctness pass; retain samples/load/jitter and release explicitly after numeric capture.
- Scope: route-local artifacts only; no shared-record writes, Online action, push, worktree/branch creation, or edits to V001/V002 scratch.
