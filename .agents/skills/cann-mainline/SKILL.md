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

## ACTIVE PORTFOLIO W3

当前入口只服务 `ACTIVE PORTFOLIO W3`：`R1 UB-BANK-LAYOUT-CHAMPION-X`、`R2 ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X`、`R3 TINY-MINIMAL-KERNEL-CHAMPION-X`、`R4 MULTIROW-PANEL-RMS-CHAMPION-X`、`R5 CROSSROW-FULL-PIPELINE-CHAMPION-X`。`OLD W2 HISTORICAL ONLY`：`SYNC`、`CASE47`、`CASE14`、`SELECTIVE`、`TINY-FIXED-OVERHEAD` 不进入当前状态和执行判断。

## Main refresh boundary

每次 Main 在用户状态、执行或 Planning 回复前必须执行 `RULE REFRESH REQUIRED` 与 `STATE REFRESH REQUIRED`：只读刷新权威规则和共享状态，至少读取 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取路线图，并优先最新 Child receipt / Record receipt。冲突必须写 `STATE_SYNC_GAP`。

Main 不读取 Candidate、Route 私有 worktree 或实验目录，不执行 Git、Compile、Correctness、Local、NPU 或身份计算。已批准 Route 内普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

## server3 资源准入

任何角色入口都共用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → Compile / Correctness / Local / Profile 均允许立即执行
```

AICore utilization、VLLM、其他用户进程、device 非 idle、load 高、lease 存在、缺少 exclusive lease、缺少 exclusive authorization 都不是执行 Gate，只记录不阻塞。lease 是协调元数据，不是执行权限。详细规则见 `项目规则/服务器实验规范.md`。
