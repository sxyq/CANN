---
name: cann-mainline
description: CANN AddRmsNormBias 工作流兼容路由器。识别当前角色并转到对应的角色 Skill。
---

# CANN workflow router

本文件不定义执行流程。先识别当前角色，再读取唯一对应 Skill：

| 角色 | Skill |
|---|---|
| Main / Coordinator | `.agents/skills/cann-main-orchestrator/SKILL.md` |
| Route Agent | `.agents/skills/cann-route-executor/SKILL.md` |
| Support Agent | `.agents/skills/cann-support-research/SKILL.md` |
| Record Owner | `.agents/skills/cann-record-owner/SKILL.md` |
| Online / Judge Owner | `.agents/skills/cann-online-owner/SKILL.md` |

技术 Skill（Ascend C API、架构、Tiling、精度、调试、profiling）保持各自入口。成绩、Route 状态和历史证据以项目正式记录为准；本路由器不复制这些数据。
