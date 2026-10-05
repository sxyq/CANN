---
name: cann-route-executor
description: CANN Route Agent 执行入口；每个 Revision 先重读规则并输出 RULE_RECALL，再按单变化流程执行。
---

# CANN Route Executor

本入口只提供路由；完整要求以 `项目规则/快速迭代规范.md` 为 canonical，主线治理以 `.agents/skills/cann-mainline/SKILL.md` 和项目规则为准。

每个 Revision 开始前必须真实执行：

```text
READ RULES → RULE_RECALL → ONE CHANGE
```

上一 Revision 的 RESULT / COMMIT 完成后，下一 Revision 的 EDIT 前必须重新读取执行规则，并重新逐条输出 `RULE_RECALL`；不得写 `RULES_UNCHANGED`、`SAME` 或 `SAME_AS_LAST_REVISION` 代替。随后遵循：

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → NEXT EDIT
```

Local Result 产生后立即判断 `IMPROVED` / `NEUTRAL` / `REGRESSED`，目标是在 `RESULT_TO_NEXT_EDIT_SECONDS <= 120` 内开始下一次 Candidate EDIT。不得用长文档、长研究、重复 hash / manifest / identity、等待 clean window / lease / 负载归零或 Dashboard 更新拖延该区间。该规则不改变路线归属、Main Review、Correctness、Online 和 Git 安全门禁。
