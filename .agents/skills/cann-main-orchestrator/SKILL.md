---
name: cann-main-orchestrator
description: CANN AddRmsNormBias Main 协调 Skill。通过 Child C2C 和获准的本地只读审计收集状态、协调 Route 并发并向 Planning / Review Layer 汇报。
---

# Main Orchestrator

完整有效要求清单唯一入口为 [项目规则/执行约定.md](../../../项目规则/执行约定.md#main-完整刷新与复述)。本 Skill 规定 Main 的协调边界；每次刷新、派发和答复都必须同时按该清单逐项执行。摘要、context compaction、模型切换或新指令不能解除未完成授权、读写边界和安全要求；新指令只按明确范围替换旧要求。

## ACTIVE PORTFOLIO W4

Main 当前协调 `ACTIVE PORTFOLIO W4` 的 15 条授权路线和最多 5 个 active Child（含 SLOT-3 Record Owner）；单 Route fresh context、队列、回执和版本额度见 `项目规则/W4持续探索控制契约.md`。W3 与 W2 材料只作为历史来源，不能替代 W4 正式状态。共享账本尚未登记 W4 时，报告 `STATE_SYNC_GAP`，不从控制要求补造动态事实。

`项目规则/W4持续探索控制契约.md` 是强制执行入口。

## 权限边界

Main 只协调。每次开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及每次用户状态、执行或 Planning 回复前，必须执行 `RULE REFRESH REQUIRED` 和 `STATE REFRESH REQUIRED`：重新读取 `AGENTS.md`、本 Skill、`项目规则/W4持续探索控制契约.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`；涉及 server3 时读取 `项目规则/服务器实验规范.md`，涉及 Local 时读取 `项目规则/本地性能测试规范.md`。同时窄范围只读刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取 `技术路线/技术路线图.md`，并优先最新 Child receipt / Record receipt。冲突必须报告 `STATE_SYNC_GAP`。

下列事件同样触发完整重读：`MODEL_CHANGE`、`CONTEXT_COMPACTION`、`MAIN_RESUME`、`MAIN_RESTART`、`CHILD_RESUME`、状态回复后的新对话轮次、`USER_CONTROL_UPDATE`、`RULE_FILE_CHANGE`、`RULE_UPDATE`、`HANDOFF`、`CHILD_POOL_RECONSTRUCTION` 和主要 Git 状态重新核对。Planning/User 改变 Child 数量、路线分配、Revision 要求、资源规则、Main 行为、worktree、记录、Online、路线组合或生命周期时，先发送 `USER_DIRECTIVE_RECEIPT`；规则需要更新时交本轮规则维护代理在安全边界落地。

Main 已获准本地只读 Git、源码和实验证据审计；不写任何文件，不运行 Compile、Correctness、Local、NPU、Online，也不计算身份信息。本轮无需 workspace_info，以本地 Git 确认工作区。状态来自 Child/Record/Online receipt、共享记录和只读证据；Planning 指令只说明授权，测量结果须有实际来源。用户要求完整复述时，Main 必须逐项复述当前有效要求并列出真实已读路径。

每次状态答复第一部分固定输出 `ACTIVE_PORTFOLIO=W4` 和 15 条路线状态；字段为 `LATEST_COMPLETED`、`CURRENT_LOCAL_BEST`、numeric score/delta、`CURRENT_ACTION`、`NEXT_ACTION`。状态只从正式记录与最新 receipts 读取；缺失或冲突报告 `STATE_SYNC_GAP`。状态答复标记为 `CHECKPOINT_ONLY`，不表示任务完成。

## MAIN CONTINUATION

- `MAIN_FINAL_GATE`：状态记录、状态回复和一次监测周期都不能作为 W4 完成依据。Child、可执行动作、证据、研究、测量资格、记录同步或路线组合复核仍有待办时，不报告 W4 已完成。
- `MAIN_NEXT_ACTION_REQUIREMENT`：每个 Child 事件都必须形成下一步判断；仍有任务时立即向该 Child 发后续指令，或在当前路线交接后关闭旧 Agent，确认关闭后开 fresh context 接手下一条获准路线。
- `MAIN_SELF_DRIVING_CAMPAIGN`：在既有授权及宿主能力范围内继续执行，不要求用户重复发送消息。若本轮必须让出，标为 `CHECKPOINT_ONLY`，保存精确状态与下一动作，并在恢复后继续；不得声称能无限运行或创建定时器、automation、cron、后台循环。
- `MAIN_CONTEXT_RELOAD`：用户更新、规则更新、Main/Child resume、handoff、context compaction 或模型变化后，先读取当前权威规则和共享状态再行动。

`MAIN_PROCESS_DEVIATION`: 曾在 Child 仍有可执行工作时，只发阶段状态并结束本轮。预防措施：应用以上四项规则；每个 Child 事件记录下一步判断，必要时以精确状态记录让出并恢复。

Main 可以：

- 用 `multi_agent_v1` 的 `agent_id` 管理最多 5 个 active Child，Record Owner 计入，Main 不计入；
- 保持一 Agent/Context 只负责一 Route，安全交接后关闭旧 Agent，用空槽开 fresh context；
- 接收和转发 `ROUTE_EVENT`；
- 监测 Route 当前动作、下一动作、资源占用报告和阻塞事项；
- 通过 Child 协调 server3 资源与 Route 并发；
- 汇总本地结果，向 Planning / Review Layer 提出 Online timing recommendation；
- 识别流程偏离，并向 Route Agent 返回即时恢复指令。

Main 不决定 Route 生命周期，不正式提交 Online，不创建任何自主任务。

收到 `ROUTE_EVENT`、`VERSION_RECORD_EVENT`、`RULE_REFRESH_RECEIPT`、测量结果、Compile 结果、Correctness 结果或状态回复后，Main 必须判断 Child 是否空闲以及是否有可执行下一动作；有动作则立即发送下一 C2C 指令。同 Route 有动作时继续；当前任务完成或暂时无动作时，等待本 Agent 的原子命令结束，保存证据与事件，关闭旧 Agent后用空槽安排下一 Route。旧 Agent 不换 Route；Agent 关闭不改变 Route 生命周期。

当前授权范围为 `ONLINE_AUTHORIZATION_SCOPE=W4_BEST_OF_ROUTE_VALIDATED_CANDIDATES_ONLY`：每条现有 W4 Route 最多提交一个符合全部资格的路线内最佳 Candidate，无需另等逐 Route Planning 批准。Main 可向 Online Owner 提供带来源的 Route 结果与资格材料，但不得正式提交；Route Agent 同样不得提交。每 Route 资格、源码/提交对应关系、Judge 规则与额度要求见 `项目规则/线上提交规范.md`。不得用不同 shape 的跨 Route Local 百分比排序，也不得在规则中预列具体合格 Route；不合格 Route 标为 `NO_ELIGIBLE_SUBMISSION`。授权范围外事项须另获明确授权。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

每个 Child 的命令 workdir 固定为自己的指定工作树；跨 Route 已提交证据从本工作树通过 `git show` 读取，不读取其他工作树未提交文件。Main 可以本地只读 Git/源码/实验审计核对 ownership 和阶段。

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

R4 收到 `RESUME_BEFORE_V014` 的旧 `RULE_REFRESH_RECEIPT` 时，只把它当作回执生成时已读规则的证明。若 server3 不可达，保持 `BLOCKER=SERVER3_UNAVAILABLE`，不编辑 Candidate、不启动 V014、不虚构 Revision 或版本事件。连接恢复后，Route Agent 必须重新完整读取六项规则入口并发送新的 receipt；`CURRENT_PARENT=V013` 是工作快照，不强制 Direct Parent，正式 Parent 由 Route 按单因素规则选择并声明，`CURRENT_LOCAL_BEST=V009` 保留。

每 Route 本轮新增性能版上限 10，连续 3 个有效 numeric Local 无改善报告 `STAGNATION_3`。研究、重复和旧结果恢复不新增性能版；无效测量及构建/精度失败不计入有效 Local 或连续无改善次数，真实失败版本仍须记录。达到上限或报告后完成安全交接，再安排 fresh context；不自行关闭 Route。W4 线上提交仅按当前授权范围及资格规则执行，本轮 `PUSH=NO`。

## 自主任务禁令

未经用户明确要求，Main、Child 和 Support 不得创建 ChatGPT automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
