---
name: cann-mainline
description: CANN AddRmsNormBias 工作流路由器。把任务转到已有的主题 Skill。
---

# CANN 工作流路由器

本文件只提供入口映射，不新增执行器、脚本或状态系统。比赛已结束，当前默认任务是源码与结果归档；不启动 Route Agent、设备实验或 Judge。此前连续优化要求仅为历史参考。

| 主题 | Skill |
|---|---|
| Main / Coordinator | .agents/skills/cann-main-orchestrator/SKILL.md |
| Route Agent | .agents/skills/cann-route-executor/SKILL.md |
| Support Agent | .agents/skills/cann-support-research/SKILL.md |
| Record Owner | .agents/skills/cann-record-owner/SKILL.md |
| Online / Judge Owner | .agents/skills/cann-online-owner/SKILL.md |

Ascend C API、架构、Tiling、精度、调试和 profiling 使用各自已有入口。成绩、路线状态和历史资料以项目正式记录为准。

## 资源安全

归档不调用设备。未来若用户明确授权新的实验，仍沿用已有入口、100 MB HBM 历史资源要求和服务器安全约束，不影响其他用户任务；资源与质量分别记录，不以 HBM 可用证明计时合格。

全版本源码、压缩原始记录和恢复位置见 `归档/比赛结束-20261017/README.md`；当前分类以根 README、AGENTS 和项目结构为准。
