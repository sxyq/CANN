# CANN AddRmsNormBias

本仓库服务于 2026 CANN 挑战赛西南赛区 `AddRmsNormBias`。正式主线在仓库根目录；`归档/` 只保存历史资料，不承担当前执行入口。

## 规则优先级

1. 当前会话与用户明确指令；
2. 本文件与当前角色 Skill；
3. `项目规则/` 中对应主题的唯一说明；
4. `技术路线/`、`调度/` 中的正式事实记录；
5. `归档/` 中的历史材料只作证据参考。

当前主题入口：

- Main：`.agents/skills/cann-main-orchestrator/SKILL.md`
- Route：`.agents/skills/cann-route-executor/SKILL.md`
- Support：`.agents/skills/cann-support-research/SKILL.md`
- Record：`.agents/skills/cann-record-owner/SKILL.md`
- Online：`.agents/skills/cann-online-owner/SKILL.md`
- 兼容路由：`.agents/skills/cann-mainline/SKILL.md`

## 角色表

| 角色 | 负责 | 不负责 |
|---|---|---|
| Planning / Review Layer | Route 生命周期、路线组合、是否 Online | — |
| Main | C2C 协调、并发协调、状态汇总、向 Planning 报告 | 直接读写项目文件、Candidate、Dashboard、TSV、Git、Compile、Correctness、Local、NPU、身份计算 |
| Route Agent | 一个 Route 的 Candidate、Compile、Correctness、Local、实验 Git | 其他 Route、共享记录、Route 生命周期、正式 Online |
| Support Agent | 跨 Route 研究、群聊和公开资料取证、硬件/API 分析 | Candidate、Route ownership、共享记录、正式 Online |
| Record Owner | 共享 TSV、路线记录和 Dashboard 的唯一写入 | Candidate、实验、Route 决策、执行控制 |
| Online Owner | Planning 批准后的唯一正式 Judge submitter | Candidate、Route 决策、共享成绩写入 |

Main 的项目状态只能来自 Child C2C receipt、Record Owner receipt、Online Owner receipt 和 Planning 指令。Main 不直接打开或读取项目文件。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

Route Agent 不读取其他 Route worktree；同一 Route 只有一个 Candidate writer。Support 可跨 Route 读证据，但不进入 Route worktree 写入。

## 执行主链

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → NEXT CHANGE
```

单 Revision 只表达一个小变化。失败和负结果保留。记录异步写入，不能成为实验前置条件。

## 绝对安全规则

- 禁止 force push、reset、clean、历史改写和删除失败证据；
- 不杀、不暂停、不迁移其他用户进程、服务、数据或设备任务；
- 不让多个写入者同时修改同一 Route；
- 未经 Planning 批准，不创建正式 Route、不改变 Route 生命周期；
- Route Agent 不正式提交 Online；
- Dashboard 只展示状态，不控制实验；
- 未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。

## 正式事实位置

- Official Score：`技术路线/路线成绩表.tsv` 与 `线上结果/`；
- Route / Revision：`技术路线/全版本记录.tsv`；
- 当前调度：`调度/当前任务.tsv`；
- Local ↔ Official：`调度/本地线上校准.tsv`。

历史证据、旧字段和归档控制文件保持原样；它们不重新成为当前规则入口。群聊提取细节只在 `工具/提取/README.md` 维护，本文不重复操作说明。
