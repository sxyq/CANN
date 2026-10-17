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

Compile、Correctness、Local 和 Profile 按当前任务直接调用已有入口，不做固定 FREE_HBM 数值预检，也不等待人工确认、独占窗口或 lease。记录 FREE_HBM、AICore、VLLM、其他进程和设备负载；只有命令实际出现资源错误或会影响其他用户任务时，才停止该设备操作并记录，源码工作仍可继续。
