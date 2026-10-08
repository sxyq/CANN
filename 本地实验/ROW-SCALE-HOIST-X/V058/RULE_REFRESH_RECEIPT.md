# ROW-SCALE-HOIST-X V058 Rule Refresh Receipt

REFRESHED_AT_UTC=2026-10-08T16:25:17Z
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
HEAD=dbfce9f07d851bddb915fbbee332e0e869b27ee1
CURRENT_LOCAL_BEST=V026
V057_STATE=COMMITTED; COMPILE_PASS; PARENT_CORRECTNESS_PASS; CANDIDATE_CORRECTNESS_FAIL; LOCAL_NOT_RUN
V057_PARENT=V026
V026_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9

## Files reread and SHA-256

- `AGENTS.md`: `1232b8fbc6d82691ef590344ce4b532917d460a5bdb3fa78a8f6b3833cd9434d`
- `.agents/skills/cann-route-executor/SKILL.md`: `dada5ac4d536ef31f5593949876b4eabf180b381dec5f089515c6dfe3a27c3b8`
- `.agents/skills/cann-mainline/SKILL.md`: `8188d5122baa0333e2c38de480b9cad3b036c78f7ca4083625a78f48af6e21c2`
- `项目规则/实验总则.md`: `5e3f5de523e36f2415c4a44591b70f8b1c45d1a500d29adcc33051fcdb315d34`
- `项目规则/执行约定.md`: `7e7eb531146b93d66c8acba4bfff0266e7e04493654ee206258a8ddbebe606a6`
- `项目规则/服务器实验规范.md`: `100605a8a9861d500f9b8113106a9f8e0cc51cdf1eb10902c5ee78de176bb301`
- `项目规则/本地性能测试规范.md`: `cecf4753c8e7a1e3ee7a1564e3c7fb20bc767bbcd26310d8790b59607bae5bfc`

## Execution constraints acknowledged

- Route ownership stays in this existing worktree and branch. Do not touch other Routes, shared records, Online, or push.
- Keep V057's failed correctness evidence unchanged. The next sibling's direct parent is exact V026, not V057; `CURRENT_LOCAL_BEST` remains V026 unless a later accepted Local result proves otherwise.
- One performance change only. Immediately after the Candidate performance edit, the next experimental action is Compile. Then Correctness; run Local only after Parent and Candidate both pass.
- Preserve all raw and failed evidence. Record the numeric Local result independently from quality/promotion, commit only scoped Route evidence, then send `ROUTE_EVENT + VERSION_RECORD_EVENT`.

## Worktree snapshot

At refresh, `HEAD` is the V057 result commit above. The only untracked paths observed are pre-existing V001/V002 build scratch; they are preserved and out of scope. No V058 directory existed before this receipt.
