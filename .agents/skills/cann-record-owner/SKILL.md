---
name: cann-record-owner
description: CANN AddRmsNormBias shared-record Skill。Record Owner 是共享账本和 Dashboard 的唯一写入者，只把已产生的事实异步写入正式记录。
---

# Record Owner

## 唯一写入范围

Record Owner 消费 Route、Support、Main 和 Online Owner 的 C2C receipt，并异步更新共享记录：

- `技术路线/全版本记录.tsv`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线图.md`
- `调度/当前任务.tsv`
- `调度/本地线上校准.tsv`
- 当前 Dashboard 的数据或说明

每条记录保留来源路径、Route、Revision、结果类型和 receipt 时间。历史实验字段保持原样，不用新规则重写旧证据。

每个 `VERSION_RECORD_EVENT` 到达后，Record Owner 异步同步：

1. `技术路线/全版本记录.tsv` 的对应记录；
2. `技术路线/技术路线图.md` 的 Mermaid node/edge；
3. `技术路线/技术路线图.md` 的 Markdown version row；
4. 必要时的 `调度/当前任务.tsv`、`调度/本地线上校准.tsv` 和当前 Dashboard。

这些 canonical shared files 只有 Record Owner 可以写入。同步不能暂停、取消或延迟已启动的 Route 阶段，也不能要求 Route Agent 直接写共享文件。

## 权限边界

Record Owner 不编辑 Candidate，不运行 Compile、Correctness、Local 或 NPU，不拥有 Route，不决定 Route 生命周期，不作 Online decision，也不得因记录尚未更新而暂停实验。

记录工作是异步 bookkeeping，永远不是实验开始、Compile、Correctness、Local 或 Online 的前置条件。

## 事实处理

- 只登记已经由 Child receipt 或正式结果支持的事实；
- Local 与 Official 分开记录；
- 负结果、失败结果和未完成结果保留；
- 不把 Local score 写成 Official Score；
- 不替 Planning / Review Layer 选择路线或决定提交；
- 不把 Dashboard 当作执行控制点。

## C2C receipt

```text
RECORD_EVENT
SOURCE_EVENT = <ROUTE_EVENT|SUPPORT_FINDING|ONLINE_EVENT>
ROUTE = <route or NONE>
REVISION = <revision or NONE>
UPDATED_PATHS = <specific paths>
FACTS_WRITTEN = <short summary>
UNRESOLVED = <text or NONE>
```

收到 `ROUTE_EVENT` 却缺少 `VERSION_RECORD_EVENT` 时，Record Owner 向 Main 报告缺失事实，等待补发；不自行补造字段，不改 Candidate，不改变实验状态。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
