---
name: cann-mainline
description: CANN AddRmsNormBias 工作流兼容路由器。识别当前角色并转到对应的角色 Skill。
---

# CANN workflow router

## W4 Official Scoring Override

> **W4 强制执行规则：本项目禁止设置、保留或新增任何自行定义的实验、评分、Correctness 覆盖、Local 测量稳定性、审批或其他线上提交前置门禁。每条 W4 技术路线必须选取至少两个不同源码身份的真实 Candidate 进行 Official 线上评分。任何研究报告、状态同步、历史归档或本地测量结果均不得作为阻止合法 Official 提交的内部条件。服务器运行安全、工作树隔离及评测平台强制规则仍须遵守。**

本规则覆盖本文其他 Online 次数、候选资格及批准流程表述；当前授权不少于每路线两个不同源码身份的真实 Candidate。各角色边界及唯一脚本见 `项目规则/线上提交规范.md`。

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

## ACTIVE PORTFOLIO W4

当前入口服务 W4-R01 至 W4-R15；W3/W2 只作为历史证据，不替代当前状态。Route ID、任务和成绩见 W4 当前共享记录。

## Main refresh boundary

每次 Main 在开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及每次用户状态、执行或 Planning 回复前，必须执行 `RULE REFRESH REQUIRED` 与 `STATE REFRESH REQUIRED`：重新读取 `AGENTS.md`、`.agents/skills/cann-main-orchestrator/SKILL.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`；涉及 server3 时读取 `项目规则/服务器实验规范.md`，涉及 Local 时读取 `项目规则/本地性能测试规范.md`。同时只读刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取路线图，并优先最新 Child receipt / Record receipt。冲突必须写 `STATE_SYNC_GAP`。

Main 按角色 Skill 协调，不进入 Route worktree、不编辑 Candidate 或提交 Official；Online Owner 是唯一正式提交者。用户明确授权的多版本 Official 评测不需要逐次批准。Main 的其他本地操作边界见 Main Skill。已批准 Route 内普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。用户要求完整复述时，Main 必须逐项复述当前有效要求，并列出真实已读路径。

每次状态回复第一部分固定为 `ACTIVE_PORTFOLIO=W4` 与 W4-R01 至 W4-R15 状态；字段按 Main Skill 和共享表当前定义读取。Local 缺失时如实写 `NONE`，不得据此停止 Official 工作。

## server3 资源准入

任何角色入口都共用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → Compile / Correctness / Local / Profile 均允许立即执行
```

AICore utilization、VLLM、其他用户进程、device 非 idle、load 高、lease 存在、缺少 exclusive lease、缺少 exclusive authorization 都不是执行 Gate，只记录不阻塞。lease 是协调元数据，不是执行权限。详细规则见 `项目规则/服务器实验规范.md`。
