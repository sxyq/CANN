---
name: cann-route-executor
description: CANN AddRmsNormBias Route Agent 执行 Skill。负责单 Route 的一修改循环、server3 Compile、Correctness、Local measurement、结果提交和版本 Git。
---

# Route Executor

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

只读写自己的 Route worktree，不读取其他 Route worktree，不改共享调度、共享成绩或 Dashboard，不决定 Route 生命周期，不正式提交 Online。

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
→ NEXT CHANGE
```

1. 编辑自己的 Candidate。
2. 编辑后的下一项实验动作必须是 server3 Compile。中间不写文档、不更新 Dashboard、不做身份计算、不做额外研究、不做测量准备、不加入第二个性能变化。
3. Compile 失败时只处理编译问题，然后再次 Compile；不得借此加入新的性能机制。
4. Compile 通过后运行 Correctness。
5. Correctness 通过后运行 Local Performance。
6. 报告 Local score、delta、samples、raw latency、jitter、device load、repeatability 和结果解释。
7. 提交本轮实验结果的 Git commit。失败版本、负结果和工具失败都保留。
8. Local 改善时及时 push，并把该 Revision 标为 `CURRENT_LOCAL_BEST`；下一轮可从它继续。
9. Local 未改善时保留负结果，不提升为 Local Best；下一轮回到当前 `CURRENT_LOCAL_BEST`。

Local accumulation 只能由一连串完整的小变化循环组成，不能把多个独立变化折叠进一个 Revision。

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
LOCAL_SCORE = <value|NONE>
LOCAL_DELTA = <value|NONE>
CURRENT_LOCAL_BEST = <revision|NONE>
GIT_COMMIT = <commit or NONE>
PUSH = <YES|NO|PENDING>
BLOCKER = <text or NONE>
```

如果 Main 返回 `PROCESS_DEVIATION`，停止无关动作，立刻把下一动作设为 Compile，并恢复到：

```text
EDIT → COMPILE → CORRECTNESS → LOCAL
```

## 结果边界

Local 是本地测量结果，不代表 Official Score。Online timing 由 Main 协调，Online decision 由 Planning / Review Layer 作出，正式提交由 Online Owner 完成。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
