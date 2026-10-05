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

## ACTIVE PORTFOLIO W3

当前正式路线池为 `ACTIVE PORTFOLIO W3`，仅包含以下五条路线：

- `R1 UB-BANK-LAYOUT-CHAMPION-X`
- `R2 ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X`
- `R3 TINY-MINIMAL-KERNEL-CHAMPION-X`
- `R4 MULTIROW-PANEL-RMS-CHAMPION-X`
- `R5 CROSSROW-FULL-PIPELINE-CHAMPION-X`

`OLD W2 HISTORICAL ONLY`：`SYNC`、`CASE47`、`CASE14`、`SELECTIVE`、`TINY-FIXED-OVERHEAD` 只属于历史 W2，不能用于当前状态、路线选择或执行判断。

当前已知 W3 事实为 Local facts only，不能写成 Official：R1 `V001-V010` 无稳定 Local Best；R2 至 `V016`，Best `V015`，target `-18.0207%`，control `-19.7413%`；R3 `V001-V010` 无稳定 Best；R4 至 `V013`，Best `V009`，FP16 `128x12288`，`-0.4954%`；R5 至 `V016`，Best `V012`，`10.040 us`，`-2.994%`。

## 角色表

| 角色 | 负责 | 不负责 |
|---|---|---|
| Planning / Review Layer | Route 生命周期、路线组合、是否 Online | — |
| Main | C2C 协调、并发协调、状态汇总、向 Planning 报告 | 读写 Candidate、Route 私有 worktree、实验目录、Dashboard、共享记录；运行 Git、Compile、Correctness、Local、NPU 或身份计算 |
| Route Agent | 一个 Route 的 Candidate、Compile、Correctness、Local、实验 Git | 其他 Route、共享记录、Route 生命周期、正式 Online |
| Support Agent | 跨 Route 研究、群聊和公开资料取证、硬件/API 分析 | Candidate、Route ownership、共享记录、正式 Online |
| Record Owner | 共享 TSV、路线记录和 Dashboard 的唯一写入 | Candidate、实验、Route 决策、执行控制 |
| Online Owner | Planning 批准后的唯一正式 Judge submitter | Candidate、Route 决策、共享成绩写入 |

Main 的项目状态只能来自 Child C2C receipt、Record Owner receipt、Online Owner receipt、权威共享状态和 Planning 指令。

## RULE REFRESH REQUIRED / STATE REFRESH REQUIRED

每次 Main 在用户状态、执行或 Planning 回复前，必须只读刷新权威规则和共享状态。权威规则至少包括本文件、对应角色 Skill、`项目规则/` 下相关文件；共享状态至少刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取 `技术路线/技术路线图.md`。Main 优先采用最新 Child receipt 与 Record receipt；来源冲突时必须报告 `STATE_SYNC_GAP`，不得自行猜测或写入共享记录。

Main 只读上述权威规则和共享状态，不读取 Candidate、Route 私有 worktree、实验目录或归档材料，不执行 Git、Compile、Correctness、Local、NPU 或身份计算。

已批准 Route 内的普通下一 Revision 标记为 `NO MAIN APPROVAL REQUIRED`。Route 应长期复用同一 owner、context、branch 和 worktree；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

## PERSIST EXECUTION DISCIPLINE

Route 执行必须闭合为：

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

正常完成必须满足 `LOCAL SCORE REQUIRED`，并报告 numeric Local score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best。Compile 或 Correctness 失败也必须保留证据、Git commit/status，并发送 `VERSION_RECORD_EVENT REQUIRED`。`NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS`。Record Owner 必须记录每个 `VERSION_RECORD_EVENT`，包括负结果、失败结果和非 Local Best，不得发明缺失数据。

Route Agent 在每个新 Revision 以及 resume、reconnect、context restore 或 long interruption 后，必须在 Candidate edit 前重新读取本文件、对应 Skill、`项目规则/实验总则.md`、`项目规则/执行约定.md`、`项目规则/服务器实验规范.md`、`项目规则/本地性能测试规范.md`，先发送 `RULE_REFRESH_RECEIPT`；无 receipt 不得 edit。

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

## server3 资源准入

对 Compile、Correctness、Local、Profile 使用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → 允许立即执行
```

以下都不是执行 Gate，只记录不阻塞：AICore utilization 非零、Vector/Core busy、VLLM 驻留、其他用户进程、device 非 idle、load 较高、没有 exclusive lease、没有 exclusive authorization、旧 lease、未知 lease owner、不存在 clean window。

lease 只是 coordination / bookkeeping metadata，不是执行权限；不得为了运行实验删除、覆盖、伪造或重写他人的 lease。不得因上述事实要求等待、暂停 Route、拒绝 Correctness/Local、要求独占 NPU 或要求资源负责人批准。

唯一有效的设备资源停止条件：所有可用 NPU FREE_HBM < 100 MB；真实执行出现 OOM / allocation failure / runtime resource failure；启动自己的任务会实际破坏或终止其他用户任务；server3 不可连接。

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

版本记录：人类可读版本树和 Markdown 表唯一位于 `技术路线/技术路线图.md`；结构化账本唯一位于 `技术路线/全版本记录.tsv`。Route Agent → `VERSION_RECORD_EVENT` → Record Owner 异步同步；Main 不读写这两份 shared files。

历史证据、旧字段和归档控制文件保持原样；它们不重新成为当前规则入口。群聊提取细节只在 `工具/提取/README.md` 维护，本文不重复操作说明。
