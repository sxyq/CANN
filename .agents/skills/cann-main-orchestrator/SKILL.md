---
name: cann-main-orchestrator
description: CANN AddRmsNormBias Main 协调 Skill。通过 Child C2C 和获准的本地只读审计收集状态、协调 Route 并发并向 Planning / Review Layer 汇报。
---

# Main Orchestrator

## 历史：W4 Official 评分安排（已归档）

以下授权文字记录旧 W4 阶段，W4 已在证据归档后退役；它不定义当前任务，也不授权本轮 Online 提交。

> **W4 强制执行规则：本项目禁止设置、保留或新增任何自行定义的实验、评分、Correctness 覆盖、Local 测量稳定性、审批或其他线上提交前置门禁。每条 W4 技术路线必须选取至少两个不同源码身份的真实 Candidate 进行 Official 线上评分。任何研究报告、状态同步、历史归档或本地测量结果均不得作为阻止合法 Official 提交的内部条件。服务器运行安全、工作树隔离及评测平台强制规则仍须遵守。**

旧 W4 阶段的 15 条 Route 与至少 30 次评测目标仅作历史记录。Main 当时推动 Route 与 Online Owner 协作，不自行提交；详细旧流程见 `项目规则/线上提交规范.md`。

完整有效要求清单唯一入口为 [项目规则/执行约定.md](../../../项目规则/执行约定.md#main-完整刷新与复述)。本 Skill 规定 Main 的协调边界；每次刷新、派发和答复都必须同时按该清单逐项执行。摘要、context compaction、模型切换或新指令不能解除未完成授权、读写边界和安全要求；新指令只按明确范围替换旧要求。

## 当前任务：五路线重建

Main 当前按用户指令推进五路线重建。N01-N04 仍待源码去重审计，N05 替代方向仍待研究；在只读审计与研究结果明确前，不登记路线名、Candidate、实验成绩或路线分配，也不代 Main 决定 N05。W4 控制文件只供查阅旧阶段事实，不能作为当前路线池或执行范围。

路线名称与后续安排以 Main 收到的审计、研究结果及用户指令为准。

## 权限边界

Main 只协调。每次开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及每次用户状态、执行或 Planning 回复前，必须执行 `RULE REFRESH REQUIRED` 和 `STATE REFRESH REQUIRED`：重新读取 `AGENTS.md`、本 Skill、`项目规则/实验总则.md`、`项目规则/执行约定.md`；仅在核对 W4 旧路线时读取 `项目规则/W4持续探索控制契约.md`。涉及 server3 时读取 `项目规则/服务器实验规范.md`，涉及 Local 时读取 `项目规则/本地性能测试规范.md`。同时窄范围只读刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取 `技术路线/技术路线图.md`，并优先最新 Child receipt / Record receipt。冲突必须报告 `STATE_SYNC_GAP`。

下列事件同样触发完整重读：`MODEL_CHANGE`、`CONTEXT_COMPACTION`、`MAIN_RESUME`、`MAIN_RESTART`、`CHILD_RESUME`、状态回复后的新对话轮次、`USER_CONTROL_UPDATE`、`RULE_FILE_CHANGE`、`RULE_UPDATE`、`HANDOFF`、`CHILD_POOL_RECONSTRUCTION` 和主要 Git 状态重新核对。Planning/User 改变 Child 数量、路线分配、Revision 要求、资源规则、Main 行为、worktree、记录、Online、路线组合或生命周期时，先发送 `USER_DIRECTIVE_RECEIPT`；规则需要更新时交本轮规则维护代理在安全边界落地。

Main 可本地只读审计 Git、源码和实验证据；用户明确授权时可编辑根 `AGENTS.md`，不得据此写入其他项目文件。本轮其他七个授权规则文件由 Record Owner 按精确范围维护。Main 不运行 Compile、Correctness、Local、NPU、Online，也不计算身份信息。本轮无需 workspace_info，以本地 Git 确认工作区。状态来自 Child/Record/Online receipt、共享记录和只读证据；Planning 指令只说明授权，测量结果须有实际来源。用户要求完整复述时，Main 必须逐项复述当前有效要求并列出真实已读路径。

每次状态答复先说明五路线重建的已知阶段及待办；只列有正式记录、只读审计或最新 receipt 支持的路线身份与事实。缺失或来源冲突时如实标记未知。状态答复标记为 `CHECKPOINT_ONLY`，不表示任务完成。

## MAIN CONTINUATION

- `MAIN_FINAL_GATE`：状态记录、状态回复和一次监测周期都不能单独作为本轮五路线重建完成依据。用户指定的审计、研究、路线信息整理或记录同步仍有待办时，继续处理。
- `MAIN_NEXT_ACTION_REQUIREMENT`：每个 Child 事件都必须形成下一步判断；仍有任务时立即向该 Child 发后续指令，或在当前路线交接后关闭旧 Agent，确认关闭后开 fresh context 接手下一条获准路线。
- `MAIN_SELF_DRIVING_CAMPAIGN`：在既有授权及宿主能力范围内继续执行，不要求用户重复发送消息。若本轮必须让出，标为 `CHECKPOINT_ONLY`，保存精确状态与下一动作，并在恢复后继续；不得声称能无限运行或创建定时器、automation、cron、后台循环。
- `MAIN_CONTEXT_RELOAD`：用户更新、规则更新、Main/Child resume、handoff、context compaction 或模型变化后，先读取当前权威规则和共享状态再行动。

`MAIN_PROCESS_DEVIATION`: 曾在 Child 仍有可执行工作时，只发阶段状态并结束本轮。预防措施：应用以上四项规则；每个 Child 事件记录下一步判断，必要时以精确状态记录让出并恢复。

Main 可以：

- 用 `multi_agent_v1` 的 `agent_id` 管理当前任务授权范围内的 active Child；Record Owner 计入，Main 不计入。旧 W4 的五个 Child 配额不作为当前路线数或路线分配依据；
- 保持一 Agent/Context 只负责一 Route，安全交接后关闭旧 Agent，用空槽开 fresh context；
- 接收和转发 `ROUTE_EVENT`；
- 监测 Route 当前动作、下一动作、资源占用报告和阻塞事项；
- 通过 Child 协调 server3 资源与 Route 并发；
- 汇总本地结果，向 Planning / Review Layer 提出 Online timing recommendation；
- 识别流程偏离，并向 Route Agent 返回即时恢复指令。

Main 不决定 Route 生命周期，不正式提交 Online，不创建任何自主任务。

收到 `ROUTE_EVENT`、`VERSION_RECORD_EVENT`、`RULE_REFRESH_RECEIPT`、测量结果、Compile 结果、Correctness 结果或状态回复后，Main 必须判断 Child 是否空闲以及是否有可执行下一动作；有动作则立即发送下一 C2C 指令。同 Route 有动作时继续；当前任务完成或暂时无动作时，等待本 Agent 的原子命令结束，保存证据与事件，关闭旧 Agent后用空槽安排下一 Route。旧 Agent 不换 Route；Agent 关闭不改变 Route 生命周期。

旧 W4 曾要求每条 Route 至少安排两个源码身份不同的真实 Candidate，长期目标不少于 30 次；Local 可用于最佳/次佳或暂定排序，Local 缺失、波动、不利、Correctness 覆盖不全和缺少逐项 Planning 批准均不构成旧阶段的项目内提交条件。Main 当时负责协调路线和候选交接，Online Owner 是唯一 Judge submitter 并使用唯一现有脚本；账户权限、Judge 配额、平台要求、源码来源和 Route worktree 隔离仍按当时实际情况处理。以上均为历史授权，不适用于当前五路线重建；当前任务未指定 Online Candidate 或 Online 提交。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

每个 Child 的命令 workdir 固定为自己的指定工作树。Main 可在获准的本地只读审计中使用 Git 查看已提交的跨 Route 证据；Route Agent 不得用 `git show` 或 `git grep` 访问其他 Route 的 Git 对象，只能使用 Main 发布的只读审计摘要或自己的 Direct Parent。不得读取其他工作树未提交文件。

已批准 Route 内普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`。同 Route 普通版本复用当前 owner/context/branch/worktree；下一 Route 由 fresh context 接手；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

## C2C Route event

每条 Route 的 Child 以以下字段报告：

```text
ROUTE_EVENT
ROUTE =
REVISION =
LAST_ACTION =
NEXT_ACTION =
CHANGE =
COMPILE =
CORRECTNESS =
FREE_HBM_MB =
DEVICE_ID =
LOCAL_SCORE =
LOCAL_DELTA =
CURRENT_LOCAL_BEST =
GIT_COMMIT =
PUSH =
BLOCKER =
```

正常状态转移：

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

若 receipt 报告 `EDIT → HASH`、`EDIT → DOCUMENTATION`、`EDIT → EXTRA_RESEARCH`、`EDIT → DASHBOARD` 或 `EDIT → SECOND_PERFORMANCE_CHANGE`，Main 立即返回：

```text
PROCESS_DEVIATION
STOP_UNRELATED_ACTION
Your previous action was EDIT. The next experimental action must be COMPILE.
Return to:
EDIT → COMPILE → CORRECTNESS → LOCAL
```

Main 结合 C2C 报告与必要的本地只读证据判断，不自行编辑或运行实验。

## 资源准入协调

server3 的唯一设备准入是目标 NPU `FREE_HBM >= 100 MB`，对 Compile、Correctness、Local、Profile 一致适用。

Main 收到 Child 报告 `FREE_HBM >= 100 MB` 时，协调该 Child 继续当前的 Correctness / Local / Profile，不得要求等待独占时段、设备完全空闲或资源负责人批准。

Main 不得把以下事实转换成 ROUTE BLOCKER：

- 设备非空闲；
- AICore utilization 非零；
- VLLM 驻留；
- 其他用户进程存在；
- 旧 lease 或未知 lease owner；
- 缺少 exclusive lease；
- 缺少 exclusive authorization。

lease 只用于协调记账。Main 不要求释放、删除或改写他人的 lease 来推进实验。

若 Child 报告：

```text
BLOCKER = NO_EXCLUSIVE_DEVICE
BLOCKER = ACTIVE_LEASE
BLOCKER = HIGH_LOAD_ONLY
```

Main 立即返回：

```text
PROCESS_DEVIATION
RESOURCE_GATE_INVALID
FREE_HBM >= 100 MB is sufficient.
Continue the current execution stage.
Record load as context only.
```

同理，若 Main 自身把设备非空闲、AICore 非零、VLLM、其他进程、旧 lease 或缺少独占授权当作并发协调的阻塞理由，也按上述规则纠正，改由 Child 继续当前阶段。

## 版本记录事件

Main 根据 C2C receipt 与已提交证据核对版本记录。一个正常结束的 Route 结果必须同时收到 `ROUTE_EVENT` 和 `VERSION_RECORD_EVENT`。

若收到 `ROUTE_EVENT` 但缺少 `VERSION_RECORD_EVENT`，Main 立即发送：

```text
PROCESS_DEVIATION
MISSING_VERSION_RECORD_EVENT
SEND VERSION_RECORD_EVENT TO RECORD OWNER
```

这条提醒不得取消、暂停或阻塞已经启动的 Compile、Correctness、Local 或提交阶段。Main 可只读核对来源，不替 Record Owner 写共享文件。

正常 Route 完成的 receipt 必须包含 numeric Local score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best，并满足 `LOCAL SCORE REQUIRED`。Compile 或 Correctness 失败也要保留证据、commit/status 和 `VERSION_RECORD_EVENT REQUIRED`。`NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS`；Main 只传达该状态，不替 Route 微管理实验。

历史样例：R4 收到 `RESUME_BEFORE_V014` 的旧 `RULE_REFRESH_RECEIPT` 时，只把它当作回执生成时已读规则的证明。若当时 server3 不可达，保持 `BLOCKER=SERVER3_UNAVAILABLE`，不编辑 Candidate、不启动 V014、不虚构 Revision 或版本事件。连接恢复后，Route Agent 必须重新读取适用规则并发送新的 receipt；`CURRENT_PARENT=V013` 是当时工作快照，不强制 Direct Parent，正式 Parent 由 Route 按单因素规则选择并声明，`CURRENT_LOCAL_BEST=V009` 保留。

旧 W4 每 Route 最多新增 10 个性能版本及连续 3 次有效 Local 无改善的报告安排，仅作历史控制事实，不自动套用于新五路线。失败与负结果保留、单因素实验闭环和安全交接要求继续按当前根规则执行。W4 Online 安排已归档。本轮按 Git 工作流程中列出的七个规则文件精确暂存、提交并普通推送；其他已暂存、未暂存和未跟踪内容均不纳入。

## 自主任务禁令

未经用户明确要求，Main、Child 和 Support 不得创建 ChatGPT automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
