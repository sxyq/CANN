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

## ACTIVE PORTFOLIO W4

当前 Planning 控制范围为 `ACTIVE PORTFOLIO W4`，包含 15 条授权路线；最多 5 个 active Child，Record Owner 占一个槽，Main 不计入。单 Route fresh context、队列、版本额度、回执和停止条件见 `项目规则/W4持续探索控制契约.md`。共享账本尚未完成 W4 登记时，必须标记 `STATE_SYNC_GAP`，不得从控制要求补造版本、分数或路线状态。

`ACTIVE PORTFOLIO W3` 保留为历史记录来源，仅包含以下五条路线：

- `R1 UB-BANK-LAYOUT-CHAMPION-X`
- `R2 ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X`
- `R3 TINY-MINIMAL-KERNEL-CHAMPION-X`
- `R4 MULTIROW-PANEL-RMS-CHAMPION-X`
- `R5 CROSSROW-FULL-PIPELINE-CHAMPION-X`

`OLD W2 HISTORICAL ONLY`：`SYNC`、`CASE47`、`CASE14`、`SELECTIVE`、`TINY-FIXED-OVERHEAD` 只属于历史 W2，不能用于当前状态、路线选择或执行判断。

历史 W3 事实仍为 Local facts only，不能写成 Official：R1 `V001-V010` 无稳定 Local Best；R2 至 `V016`，Best `V015`，target `-18.0207%`，control `-19.7413%`；R3 `V001-V010` 无稳定 Best；R4 至 `V013`，Best `V009`，FP16 `128x12288`，`-0.4954%`；R5 至 `V016`，Best `V012`，`10.040 us`，`-2.994%`。

## W4 DURABLE CONTROL

`项目规则/W4持续探索控制契约.md` 是当前 W4 持续探索的强制执行入口。Main、Route、Record Owner 在控制重读、Child 恢复、路线切换、规则文件变化和主要 Git 状态重新核对后，按该文件及本文件、对应 Skill、实验总则、执行约定、服务器规范、本地性能规范重新读取并发送相应 receipt。每个 Route 任务结束当前安全闭环后关闭旧 Agent，确认关闭后用空槽开 fresh context 接手下一 Route，禁止同 Agent 换 Route。Record Owner 占 SLOT-3，全部 Child 合计最多 5 个；Main 不计入，Online 保持 `PAUSED`。本轮每 Route 新增性能版上限 10，连续 3 个有效 numeric Local 无改善报告 `STAGNATION_3`；研究、旧结果恢复和重复不新增性能版；无效测量及构建/精度失败不计入有效 Local 或连续无改善次数。

## 角色表

| 角色 | 负责 | 不负责 |
|---|---|---|
| Planning / Review Layer | Route 生命周期、路线组合、是否 Online | — |
| Main | C2C 协调、Child 管理、状态汇总；本地只读 Git、源码和实验证据审计 | 写入任何文件；运行 Compile、Correctness、Local、NPU、Online 或身份计算 |
| Route Agent | 一个 Route 的 Candidate、Compile、Correctness、Local、实验 Git | 其他 Route、共享记录、Route 生命周期、正式 Online |
| Support Agent | 跨 Route 研究、群聊和公开资料取证、硬件/API 分析 | Candidate、Route ownership、共享记录、正式 Online |
| Record Owner | SLOT-3；主工作树内获准的规则及四份共享记录 | Candidate、实验、Route ownership/决策、Dashboard、归档及其他工作树 |
| Online Owner | Planning 批准后的唯一正式 Judge submitter | Candidate、Route 决策、共享成绩写入 |

Main 的项目状态来自 Child/Record/Online receipt、权威共享状态及获准的本地只读审计；Planning 指令规定授权，测量数字仍须真实结果支持。

Online 当前为 `PAUSED`；Main 与 Route Agent 不得擅自恢复，只有 Planning 改变决策后由指定 Online Owner 正式提交。

Main 完整有效要求清单见 `项目规则/执行约定.md` 的“Main 完整刷新与复述”及相邻章节；W4 路线控制细则以 `项目规则/W4持续探索控制契约.md` 为强制补充。Main 在规则指定的触发点读取全部适用入口；摘要、context compaction、模型切换或新指令不能解除未完成授权和边界，新指令只按明确范围替换旧要求。

## RULE REFRESH REQUIRED / STATE REFRESH REQUIRED

每次 Main 在开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及用户状态、执行或 Planning 回复前，必须只读刷新权威规则和共享状态。权威规则至少包括本文件、对应角色 Skill、`项目规则/W4持续探索控制契约.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`；涉及 server3 或 Local 时读取对应规范。共享状态至少刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取 `技术路线/技术路线图.md`。Main 优先采用最新 Route receipt、Record receipt 和正式记录；来源冲突时必须报告 `STATE_SYNC_GAP`，不得自行猜测或写入共享记录。

完整重读也由 `MODEL_CHANGE`、`CONTEXT_COMPACTION`、`MAIN_RESUME`、`MAIN_RESTART`、`CHILD_RESUME`、状态回复后的新对话轮次、`USER_CONTROL_UPDATE`、`RULE_FILE_CHANGE`、`RULE_UPDATE`、`HANDOFF`、`CHILD_POOL_RECONSTRUCTION` 和主要 Git 状态重新核对触发。Planning/User 改变 Child 数量、路线分配、Revision 要求、资源规则、Main 行为、worktree、记录、Online、路线组合或生命周期时，Main 先发 `USER_DIRECTIVE_RECEIPT`；若需改规则，由本轮规则维护代理在安全边界更新 W4 控制文件及受影响入口。正在运行的原子性能命令可以完成，规则维护不要求其他有效路线整体暂停。

Main 可本地只读核对 Git、源码、实验证据和共享状态；不写任何文件，不运行 Compile、Correctness、Local、NPU、Online 或身份计算。本轮无需 workspace_info，工作区由本地 Git 确认。

已批准 Route 内的普通下一 Revision 标记为 `NO MAIN APPROVAL REQUIRED`。同 Route 的普通版本复用当前 owner/context/branch/worktree；结束本次 Route 任务后按安全交接关闭旧 Agent，下一 Route 使用 fresh context；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

## PERSIST EXECUTION DISCIPLINE

Route 执行必须闭合为：

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

正常完成必须满足 `LOCAL SCORE REQUIRED`，并报告 numeric Local score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best。Compile 或 Correctness 失败也必须保留证据、Git commit/status，并发送 `VERSION_RECORD_EVENT REQUIRED`。`NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS`。Record Owner 必须记录每个 `VERSION_RECORD_EVENT`，包括负结果、失败结果和非 Local Best，不得发明缺失数据。

Route Agent 在每个新 Revision、resume、Route switch、Child restart、reconnect、context restore 或 long interruption 后，必须在 Candidate edit 前重新读取本文件、对应 Skill、`项目规则/W4持续探索控制契约.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`、`项目规则/服务器实验规范.md`、`项目规则/本地性能测试规范.md`，先发送 `RULE_REFRESH_RECEIPT`；无 receipt 不得 edit。

### MAIN_CONTINUATION_RULES

- `MAIN_FINAL_GATE`：状态记录、状态回复或一次监测周期不代表 W4 完成。只要仍有 Child、下一动作、证据收集、研究、重新取得测量资格、记录同步或路线组合复核未完，不得报告 W4 已完成。
- `MAIN_NEXT_ACTION_REQUIREMENT`：每个 Child 事件都必须形成明确的下一步判断；仍有工作时，Main 向对应 Child 发出后续指令，不能只确认收到或汇总。
- `MAIN_CONTEXT_RELOAD`：收到用户更新、规则更新、Main/Child 恢复、handoff、context compaction 或模型变化后，重新读取当前权威规则和共享状态，再继续执行。
- `MAIN_SELF_DRIVING_CAMPAIGN`：Main 可在既有授权和宿主工具能力范围内继续 W4 工作，不要求用户重复授权。若平台要求让出本轮，输出 `CHECKPOINT_ONLY`，保存精确路线、动作、阶段、回执和待办，并在下一轮从该状态继续。不得创建 timer、automation、cron 或后台循环，也不得宣称宿主能无限运行。

一次状态回复只是 `CHECKPOINT_ONLY`。只要仍有活动 Child、可执行路线、待研究、待重新取得资格、待指纹、待记录或待路线组合复核，Main 必须继续轮转、处理事件并发送下一动作，不能把一次状态回复当作任务结束。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

每个 Child 只在自己的指定工作树运行命令；跨 Route 已提交证据通过本工作树的 `git show` 读取，不读取其他工作树的未提交文件。Record Owner 的唯一工作树是 `/Users/sunyiyang/Desktop/Project/cann`、分支 main；Route 不写主目录。Main 已获准本地只读审计。

## 执行主链

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

单 Revision 只表达一个小变化。失败和负结果保留。记录异步写入，不能成为实验前置条件。

## server3 资源准入

对 Compile、Correctness、Local、Profile 使用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → 允许立即执行
```

以下只记录为负载上下文，不影响执行：AICore utilization 非零、Vector/Core busy、VLLM 驻留、其他用户进程、device 非 idle、load 较高、没有 exclusive lease、没有 exclusive authorization、旧 lease、未知 lease owner、不存在 clean window。

lease 只用于 coordination / bookkeeping；不得为了运行实验删除、覆盖、伪造或重写他人的 lease。不得因上述事实要求等待、暂停 Route、拒绝 Correctness/Local、要求独占 NPU 或要求资源负责人批准。

唯一有效的设备资源停止条件：所有可用 NPU FREE_HBM < 100 MB；真实执行出现 OOM / allocation failure / runtime resource failure；启动自己的任务会实际破坏或终止其他用户任务；server3 不可连接。

## 绝对安全规则

- 禁止 force push、reset、clean、历史改写和删除失败证据；
- 不杀、不暂停、不迁移其他用户进程、服务、数据或设备任务；
- 不让多个写入者同时修改同一 Route；
- 未经 Planning 批准，不创建正式 Route、不改变 Route 生命周期；
- Route Agent 不正式提交 Online；
- Dashboard 只展示状态，不控制实验；本轮不修改；
- 本轮 `PUSH=NO`；规则与共享记录分开提交，精确暂存获准内容，保留既有未提交改动；
- 未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。

## 正式事实位置

- Official Score：`技术路线/路线成绩表.tsv` 与 `线上结果/`；
- Route / Revision：`技术路线/全版本记录.tsv`；
- 当前调度：`调度/当前任务.tsv`；
- Local ↔ Official：`调度/本地线上校准.tsv`。

版本记录：人类可读版本树和 Markdown 表唯一位于 `技术路线/技术路线图.md`；结构化账本唯一位于 `技术路线/全版本记录.tsv`。Route Agent → `VERSION_RECORD_EVENT` → Record Owner 异步同步；Main 只读允许的共享状态，不写入这两份 shared files。

历史证据、旧字段和归档控制文件保持原样；它们不重新成为当前规则入口。群聊提取细节只在 `工具/提取/README.md` 维护，本文不重复操作说明。
