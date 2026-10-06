---
name: cann-mainline
description: CANN AddRmsNormBias 工作流兼容路由器。识别当前角色并转到对应的角色 Skill。
---

# CANN workflow router

Main 的完整有效要求清单见 [项目规则/执行约定.md](../../../项目规则/执行约定.md#main-完整刷新与复述)。识别角色后必须读取对应 Skill 与该清单；摘要、context compaction、模型切换或新指令不能解除未完成授权和边界，新指令只按明确范围替换旧要求。

本文件不定义执行流程。先识别当前角色，再读取唯一对应 Skill：

| 角色 | Skill |
|---|---|
| Main / Coordinator | `.agents/skills/cann-main-orchestrator/SKILL.md` |
| Route Agent | `.agents/skills/cann-route-executor/SKILL.md` |
| Support Agent | `.agents/skills/cann-support-research/SKILL.md` |
| Record Owner | `.agents/skills/cann-record-owner/SKILL.md` |
| Online / Judge Owner | `.agents/skills/cann-online-owner/SKILL.md` |

技术 Skill（Ascend C API、架构、Tiling、精度、调试、profiling）保持各自入口。成绩、Route 状态和历史证据以项目正式记录为准；本路由器不复制这些数据。

## ACTIVE PORTFOLIO W3

当前入口只服务 `ACTIVE PORTFOLIO W3`：`R1 UB-BANK-LAYOUT-CHAMPION-X`、`R2 ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X`、`R3 TINY-MINIMAL-KERNEL-CHAMPION-X`、`R4 MULTIROW-PANEL-RMS-CHAMPION-X`、`R5 CROSSROW-FULL-PIPELINE-CHAMPION-X`。`OLD W2 HISTORICAL ONLY`：`SYNC`、`CASE47`、`CASE14`、`SELECTIVE`、`TINY-FIXED-OVERHEAD` 不进入当前状态和执行判断。

## Main refresh boundary

每次 Main 在开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及每次用户状态、执行或 Planning 回复前，必须执行 `RULE REFRESH REQUIRED` 与 `STATE REFRESH REQUIRED`：重新读取 `AGENTS.md`、`.agents/skills/cann-main-orchestrator/SKILL.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`；涉及 server3 时读取 `项目规则/服务器实验规范.md`，涉及 Local 时读取 `项目规则/本地性能测试规范.md`。同时只读刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取路线图，并优先最新 Child receipt / Record receipt。冲突必须写 `STATE_SYNC_GAP`。

Main 不读取 Candidate、Route 私有 worktree 或实验目录，不写入任何文件，不执行 Git、Compile、Correctness、Local、NPU 或身份计算。已批准 Route 内普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。用户要求完整复述时，Main 必须逐项复述当前有效要求，并列出真实已读路径。

每次状态回复第一部分固定为 `ACTIVE_PORTFOLIO=W3` 与 R1-R5 五行状态表，字段为 `LATEST_COMPLETED`、`CURRENT_LOCAL_BEST`、numeric score/delta、`CURRENT_ACTION`、`NEXT_ACTION`；数据只从三份共享记录和最新 receipts 核对，缺失或冲突报告 `STATE_SYNC_GAP`。

## server3 资源准入

任何角色入口都共用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → Compile / Correctness / Local / Profile 均允许立即执行
```

AICore utilization、VLLM、其他用户进程、device 非 idle、load 高、lease 存在、缺少 exclusive lease、缺少 exclusive authorization 都不是执行 Gate，只记录不阻塞。lease 是协调元数据，不是执行权限。详细规则见 `项目规则/服务器实验规范.md`。
