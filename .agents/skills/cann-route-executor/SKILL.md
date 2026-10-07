---
name: cann-route-executor
description: CANN AddRmsNormBias Route Agent 执行 Skill。负责单 Route 的一修改循环、server3 Compile、Correctness、Local measurement、结果提交和版本 Git。
---

# Route Executor

## ACTIVE PORTFOLIO W4

Route Agent 只执行 W4 控制文件列出的授权路线，并在同一持久 Child 内按固定顺序轮转；一条 Route 同时只能有一个 Candidate 写入者。W3 与 W2 材料只作为历史来源，不能替代 W4 路线状态。完整路线池、5 个 Child 分配、轮转和初始处置见 `项目规则/W4持续探索控制契约.md`。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

只读写自己的 Route worktree，不读取其他 Route worktree，不改共享调度、共享成绩或 Dashboard，不决定 Route 生命周期，不正式提交 Online。切换 Route 前发送 `ROUTE_SWITCH_RECEIPT`，确认当前设备操作已结束，并列明来源工作树/branch/head/dirty、上一版本事件、目标工作树/branch、下一 Revision 和 `NEXT_ACTION=RULE_REFRESH`。

## RULE REFRESH REQUIRED

每个新 Revision、resume、Route switch、Child restart、reconnect、context restore 或 long interruption 后，Candidate edit 前必须重新读取 `AGENTS.md`、本 Skill、`项目规则/W4持续探索控制契约.md`、`项目规则/实验总则.md`、`项目规则/执行约定.md`、`项目规则/服务器实验规范.md`、`项目规则/本地性能测试规范.md`，然后先发送新的 `RULE_REFRESH_RECEIPT`。同版 Compile/Correctness direct fix 前先重读 `AGENTS.md`、本 Skill、`项目规则/执行约定.md`，再发送注明读取范围的 receipt，然后只做直接修复，编辑后下一实验动作必须是 `COMPILE`；中间不插入阅读。无对应 receipt 不得 edit。

## Authoritative loop

每一轮只选择一个小技术变化，例如一个参数、一个阈值、一个分支形式、一次 buffer 使用、一个指令位置、一个算术写法或一个调度选择。一个 Revision 只能表达一个小变化；单一变化是执行约束，不是额外审核阶段。

执行顺序固定为：

```text
ONE CHANGE
→ COMPILE
→ CORRECTNESS
→ LOCAL
→ RESULT
→ COMMIT
→ VERSION_RECORD_EVENT
→ NEXT CHANGE
```

1. 编辑自己的 Candidate。
2. 编辑后的下一项实验动作必须是 server3 Compile。中间不写文档、不更新 Dashboard、不做身份计算、不做额外研究、不做测量准备、不加入第二个性能变化。
3. Compile 失败时只处理编译问题，然后再次 Compile；不得借此加入新的性能机制。
4. Compile 通过后查询目标 NPU 的 `FREE_HBM`。`FREE_HBM >= 100 MB` 时立即执行 Correctness。
5. Correctness 通过后再次确认 `FREE_HBM >= 100 MB`，满足即立即执行 Local Performance。
6. 报告 Local score、delta、samples、raw latency、jitter、`FREE_HBM`、device load、repeatability 和结果解释。
7. 提交本轮实验结果的 Git commit。失败版本、负结果和工具失败都保留。
8. 发送 `VERSION_RECORD_EVENT`，并等待该事件存在后才能开始下一普通 Revision；Record Owner 异步写入不得延误已经在途的 Compile、Correctness、Local 或 commit。
9. Local 改善时及时 push，并把该 Revision 标为 `CURRENT_LOCAL_BEST`；下一轮可从它继续。
10. Local 未改善时保留负结果，不提升为 Local Best；下一轮回到当前 `CURRENT_LOCAL_BEST`。

## LOCAL SCORE REQUIRED

正常完成必须报告 numeric Local score 和 numeric Local delta，以及 Parent/Candidate raw samples、Parent/Candidate medians、shape/dtype、device、free HBM、load note、current best。Compile 或 Correctness 失败时保留失败证据、Git commit/status，并发送 `VERSION_RECORD_EVENT`，其中未执行 Local 的字段使用 `NONE`，不得补造数值。`VERSION_RECORD_EVENT REQUIRED`；`NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS`。

Local accumulation 只能由一连串完整的小变化循环组成，不能把多个独立变化折叠进一个 Revision。

## 资源准入

server3 的 Compile、Correctness、Local、Profile 共用同一个准入条件：

```text
目标 NPU FREE_HBM >= 100 MB → 该阶段允许立即执行
```

以下事实都不是执行 Gate，只记录不阻塞：AICore utilization 非零、Vector/Core busy、VLLM 驻留、其他用户进程、device 非 idle、系统 load 高、没有 exclusive lease、没有 exclusive authorization、旧 lease、未知 lease owner、不存在 clean window。

禁止因为 other process、load、lease 或缺少 exclusive permission 停止。lease 只是协调元数据，不是执行权限；忽略它的准入语义，也不得删除、覆盖、伪造或重写他人的 lease。

Profile 与 Local 同样适用该准入条件。

Local 必须在负载较高时照常执行并记录负载上下文，不允许 `LOAD_HIGH → SKIP_LOCAL`。`FREE_HBM`、`DEVICE_LOAD`、`AICORE_LOAD`、`OTHER_PROCESS_PRESENT`、`LOAD_NOTE` 是测量上下文，不是测量 Gate。

只有以下情况可以因设备资源停止：所有可用 NPU `FREE_HBM < 100 MB`；真实执行出现 OOM / allocation failure / runtime resource failure；启动自己的任务会实际破坏其他用户任务；server3 不可连接。设备选择可用 `工具/server3-resource-policy.sh` 的 `choose_eligible_device`。

## C2C receipt

每个阶段结束都向 Main 发送：

```text
ROUTE_EVENT
ROUTE = <route>
REVISION = <revision>
LAST_ACTION = <action>
NEXT_ACTION = <action>
CHANGE = <one small change>
COMPILE = <PASS|FAIL|NOT_RUN>
CORRECTNESS = <PASS|FAIL|NOT_RUN>
FREE_HBM_MB = <value or UNKNOWN>
DEVICE_ID = <id or NONE>
LOCAL_SCORE = <value|NONE>
LOCAL_DELTA = <value|NONE>
CURRENT_LOCAL_BEST = <revision|NONE>
GIT_COMMIT = <commit or NONE>
PUSH = <YES|NO|PENDING>
BLOCKER = <text or NONE>
```

`BLOCKER` 只允许取真实设备资源阻塞：所有可用 NPU FREE_HBM < 100 MB、OOM / allocation failure、任务会破坏其他用户任务、server3 不可连接。不得填写 `NO_EXCLUSIVE_DEVICE`、`ACTIVE_LEASE` 或 `HIGH_LOAD_ONLY`。

如果 Main 返回 `PROCESS_DEVIATION`，停止无关动作，立刻把下一动作设为 Compile，并恢复到：

```text
EDIT → COMPILE → CORRECTNESS → LOCAL
```

## VERSION_RECORD_EVENT

每次 `RESULT → COMMIT` 完成后，Route Agent 必须同时向 Main 发送 `ROUTE_EVENT + VERSION_RECORD_EVENT`。记录动作只能发生在 RESULT 之后，不能插入 `EDIT → COMPILE` 之间。

```text
VERSION_RECORD_EVENT
ROUTE = <route>
REVISION = <revision>
DIRECT_PARENT = <recorded parent or UNKNOWN>
SINGLE_CHANGE = <one small change>
FOCUS_AXIS = <recorded axis or UNKNOWN>
FOCUS_VALUE = <recorded value or UNKNOWN>
COMPILE = <PASS|FAIL|NOT_RUN>
CORRECTNESS = <PASS|FAIL|NOT_RUN>
LOCAL_SCORE = <value|NONE>
LOCAL_DELTA = <value|NONE>
LOCAL_BEST = <revision|NONE>
OFFICIAL_SCORE = <value|NONE>
ONLINE_STATE = <state|NONE>
GIT_COMMIT = <commit or NONE>
PUSH = <YES|NO|PENDING>
BRANCH = <branch or UNKNOWN>
STATUS = <status>
EVIDENCE_NOTE = <path and short note>
```

改进结果的顺序：

```text
RESULT → COMMIT → ROUTE_EVENT + VERSION_RECORD_EVENT
→ PUSH → follow-up ROUTE_EVENT + VERSION_RECORD_EVENT with PUSH=YES and LOCAL_BEST
```

负结果的顺序：

```text
RESULT → COMMIT → ROUTE_EVENT + VERSION_RECORD_EVENT with LOCAL_BEST unchanged
→ NEXT ONE CHANGE from CURRENT_LOCAL_BEST
```

`VERSION_RECORD_EVENT` 不包含来源或对象身份字段。Record Owner 负责异步落盘，Route Agent 不直接写共享账本、路线图或 Dashboard。

恢复本身不产生 `VERSION_RECORD_EVENT`，也不虚构 Revision。server3 不可达时保留最后完成动作、精确 `NEXT_ACTION` 和 `BLOCKER=SERVER3_UNAVAILABLE`；不得编辑 Candidate 或启动下一 Revision。连接恢复必须由新的正式记录确认，不能把用户转交的旧超时回执当作本轮新连通测试。

已批准 Route 内的普通下一 Revision 为 `NO MAIN APPROVAL REQUIRED`。Route Agent 长期复用同一 owner、context、branch 和 worktree；`Main MUST NOT MANUALLY MICRO-MANAGE EVERY REVISION`。

## 结果边界

Local 是本地测量结果，不代表 Official Score。Online timing 由 Main 协调，Online decision 由 Planning / Review Layer 作出，正式提交由 Online Owner 完成。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
