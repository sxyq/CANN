---
name: cann-main-orchestrator
description: CANN AddRmsNormBias Main 协调 Skill。只通过 Child C2C 收集状态、协调 Route 并发、识别流程偏离并向 Planning / Review Layer 汇报。
---

# Main Orchestrator

完整有效要求清单唯一入口为 [项目规则/执行约定.md](../../../项目规则/执行约定.md#main-完整刷新与复述)。本 Skill 规定 Main 的协调边界；每次刷新、派发和答复都必须同时按该清单逐项执行。摘要、context compaction、模型切换或新指令不能解除未完成授权、读写边界和安全要求；新指令只按明确范围替换旧要求。

## ACTIVE PORTFOLIO W4

Main 当前协调 `ACTIVE PORTFOLIO W4` 的 15 条授权路线和 5 个持久 Child；路线分配、轮转方式、回执格式和初始处置见 `项目规则/W4持续探索控制契约.md`。W3 与 W2 材料只作为历史来源，不能替代 W4 正式状态。共享账本尚未登记 W4 时，报告 `STATE_SYNC_GAP`，不从控制要求补造动态事实。

`项目规则/W4持续探索控制契约.md` 是强制执行入口，不是参考资料。

## 权限边界

Main 只协调。每次开始任务、resume、context compaction、reconnect、模型切换、收到新指令、派发 Child 前，以及每次用户状态、执行或 Planning 回复前，必须执行 `RULE REFRESH REQUIRED` 和 `STATE REFRESH REQUIRED`：重新读取 `AGENTS.md`、本 Skill、`项目规则/W4持续探索控制契约.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`；涉及 server3 时读取 `项目规则/服务器实验规范.md`，涉及 Local 时读取 `项目规则/本地性能测试规范.md`。同时窄范围只读刷新 `技术路线/全版本记录.tsv`、`调度/当前任务.tsv`、`技术路线/路线成绩表.tsv`，必要时读取 `技术路线/技术路线图.md`，并优先最新 Child receipt / Record receipt。冲突必须报告 `STATE_SYNC_GAP`。

下列事件同样触发完整重读：`MODEL_CHANGE`、`CONTEXT_COMPACTION`、`MAIN_RESUME`、`MAIN_RESTART`、状态回复后的新对话轮次、`USER_CONTROL_UPDATE`、`RULE_FILE_CHANGE`、`HANDOFF`、`CHILD_POOL_RECONSTRUCTION` 和主要 Git 状态重新核对。Planning/User 改变 Child 数量、路线分配、Revision 要求、资源规则、Main 行为、worktree、记录、Online、路线组合或生命周期时，先发送 `USER_DIRECTIVE_RECEIPT`；规则需要更新时在安全边界落地。

Main 不读取 Candidate、Route 私有 worktree、实验目录或归档材料，不写入任何文件，不运行 Git、Compile、Correctness、Local、NPU，也不计算身份信息。项目状态只来自 Child 的 C2C receipt、Record Owner receipt、Online Owner receipt、权威共享状态和 Planning 指令。用户要求完整复述时，Main 必须逐项复述当前有效要求并列出真实已读路径。

每次状态答复第一部分固定输出 `ACTIVE_PORTFOLIO=W4` 和 15 条路线状态；字段为 `LATEST_COMPLETED`、`CURRENT_LOCAL_BEST`、numeric score/delta、`CURRENT_ACTION`、`NEXT_ACTION`。状态只从正式记录与最新 receipts 读取；缺失或冲突报告 `STATE_SYNC_GAP`。状态答复标记为 `CHECKPOINT_ONLY`，不是任务完成信号。

Main 可以：

- 创建或恢复已批准的 Child Agent，并为每条 Route 保持唯一 owner；
- 接收和转发 `ROUTE_EVENT`；
- 监测 Route 当前动作、下一动作、资源占用报告和阻塞事项；
- 通过 Child 协调 server3 资源与 Route 并发；
- 汇总本地结果，向 Planning / Review Layer 提出 Online timing recommendation；
- 识别流程偏离，并向 Route Agent 返回即时恢复指令。

Main 不决定 Route 生命周期，不正式提交 Online，不创建任何自主任务。

收到 `ROUTE_EVENT`、`VERSION_RECORD_EVENT`、`RULE_REFRESH_RECEIPT`、测量结果、Compile 结果、Correctness 结果或状态回复后，Main 必须判断 Child 是否空闲以及是否有可执行下一动作；有动作则立即发送下一 C2C 指令。当前路线受阻时轮转该持久 Child 的下一条授权路线；三条都暂时受阻时执行已授权的研究、指纹或重新取得资格动作，不关闭 Child。

Online 当前为 `PAUSED`。Main 只能向 Planning / Review Layer 报告事实和建议，不能恢复 Online；Route Agent 不能正式提交 Online，只有 Planning 改变决策后由指定 Online Owner 提交。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

Route Agent 不读取其他 Route worktree。Main 只依据 receipt 判断 ownership 和状态，不通过打开工作树自行确认。

已批准 Route 内普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`。Route 长期复用同一 owner、context、branch 和 worktree；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

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

Main 不打开文件来执行这项判断，只依据 C2C 报告。

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

lease 只是协调元数据，不是执行权限。Main 不要求释放、删除或改写他人的 lease 来推进实验。

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

Main 只根据 C2C receipt 监控版本记录。一个正常结束的 Route 结果必须同时收到 `ROUTE_EVENT` 和 `VERSION_RECORD_EVENT`。

若收到 `ROUTE_EVENT` 但缺少 `VERSION_RECORD_EVENT`，Main 立即发送：

```text
PROCESS_DEVIATION
MISSING_VERSION_RECORD_EVENT
SEND VERSION_RECORD_EVENT TO RECORD OWNER
```

这条提醒不得取消、暂停或阻塞已经启动的 Compile、Correctness、Local 或提交阶段。Main 不打开文件补录，也不替 Record Owner 写共享文件。

正常 Route 完成的 receipt 必须包含 numeric Local score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best，并满足 `LOCAL SCORE REQUIRED`。Compile 或 Correctness 失败也要保留证据、commit/status 和 `VERSION_RECORD_EVENT REQUIRED`。`NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS`；Main 只传达该状态，不替 Route 微管理实验。

R4 收到 `RESUME_BEFORE_V014` 的旧 `RULE_REFRESH_RECEIPT` 时，只把它当作回执生成时已读规则的证明。若 server3 不可达，保持 `BLOCKER=SERVER3_UNAVAILABLE`，不编辑 Candidate、不启动 V014、不虚构 Revision 或版本事件。连接恢复后，Route Agent 必须重新完整读取六项规则入口并发送新的 receipt；`CURRENT_PARENT=V013` 是工作快照，不强制 Direct Parent，正式 Parent 由 Route 按单因素规则选择并声明，`CURRENT_LOCAL_BEST=V009` 保留。

## 自主任务禁令

未经用户明确要求，Main、Child 和 Support 不得创建 ChatGPT automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
