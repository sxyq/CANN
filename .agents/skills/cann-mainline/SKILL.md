---
name: cann-mainline
description: CANN AddRmsNormBias 工作流兼容路由器。识别当前角色并转到对应的角色 Skill。
---

# CANN workflow router

## W4 Official Scoring Override

各角色边界及唯一脚本见 `项目规则/线上提交规范.md`。

识别角色后按需读取对应 Skill 与 [项目规则/执行约定.md](../../../项目规则/执行约定.md)。

本文件不定义执行流程。先识别当前角色，再读取唯一对应 Skill：

| 角色 | Skill |
|---|---|
| Main / Coordinator | `.agents/skills/cann-main-orchestrator/SKILL.md` |
| Route Agent | `.agents/skills/cann-route-executor/SKILL.md` |
| Support Agent | `.agents/skills/cann-support-research/SKILL.md` |
| Record Owner | `.agents/skills/cann-record-owner/SKILL.md` |
| Online / Judge Owner | `.agents/skills/cann-online-owner/SKILL.md` |

技术 Skill（Ascend C API、架构、Tiling、精度、调试、profiling）保持各自入口。成绩、Route 状态和历史证据以项目正式记录为准；本路由器不复制这些数据。

## ACTIVE PORTFOLIO W4

当前入口服务 W4-R01 至 W4-R15；W3/W2 只作为历史证据，不替代当前状态。Route ID、任务和成绩见 W4 当前共享记录。

## Main refresh boundary

Main 开始任务、恢复上下文或收到新指令时，按需读取 `AGENTS.md`、对应 Skill、项目规则和共享状态；涉及 server3 或 Local 时读取对应说明。冲突记录为 `STATE_SYNC_GAP`。

Main 按角色 Skill 协调，不进入 Route worktree、不编辑 Candidate 或提交 Official；Online Owner 是唯一正式提交者。Main 的其他本地操作边界见 Main Skill。Route 内普通下一 Revision 沿用当前上下文；Main 不逐项微管理 Revision。

每次状态回复第一部分固定为 `ACTIVE_PORTFOLIO=W4` 与 W4-R01 至 W4-R15 状态；字段按 Main Skill 和共享表当前定义读取。Local 缺失时如实写 `NONE`，不得据此停止 Official 工作。

## server3 资源安全

任何角色入口都共用同一个资源条件：

```text
目标 NPU FREE_HBM >= 100 MB → Compile / Correctness / Local / Profile 均允许立即执行
```

AICore utilization、VLLM、其他用户进程、device 非 idle、load 高、lease 存在、缺少 exclusive lease、缺少 exclusive authorization 只记录为上下文，不改变执行。lease 是协调元数据，不是执行权限。详细规则见 `项目规则/服务器实验规范.md`。
