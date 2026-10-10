---
name: cann-mainline
description: CANN AddRmsNormBias 工作流路由器。把任务转到已有的主题 Skill。
---

# CANN 工作流路由器

本文件只提供入口映射，不新增执行器、脚本或状态系统。

| 主题 | Skill |
|---|---|
| Main / Coordinator | .agents/skills/cann-main-orchestrator/SKILL.md |
| Route Agent | .agents/skills/cann-route-executor/SKILL.md |
| Support Agent | .agents/skills/cann-support-research/SKILL.md |
| Record Owner | .agents/skills/cann-record-owner/SKILL.md |
| Online / Judge Owner | .agents/skills/cann-online-owner/SKILL.md |

Ascend C API、架构、Tiling、精度、调试和 profiling 使用各自已有入口。成绩、路线状态和历史资料以项目正式记录为准。

## 资源安全

目标 NPU FREE_HBM >= 100 MB 时，Compile、Correctness、Local 和 Profile 可以按当前任务直接执行。AICore、VLLM、其他进程、设备负载、lease 和独占状态只记录为上下文，不形成额外等待或人工确认。
