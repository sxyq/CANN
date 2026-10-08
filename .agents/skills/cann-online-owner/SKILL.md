---
name: cann-online-owner
description: CANN AddRmsNormBias Online Owner Skill。在当前 W4 授权范围及资格齐备时，由唯一 Judge submitter 执行正式线上提交并返回结果 receipt。
---

# Online Owner

## 当前授权

当前授权范围为 `ONLINE_AUTHORIZATION_SCOPE=W4_BEST_OF_ROUTE_VALIDATED_CANDIDATES_ONLY`。每条现有 W4 Route 最多提交一个已验证的路线内最佳 Candidate；在范围内无需另等逐 Route Planning 批准。Online Owner 在每次提交前确认该 Route 全部资格、可用提交额度和当期 Judge 规则。缺少任一条件时，该 Route 标记 `NO_ELIGIBLE_SUBMISSION`，不提交。范围外事项须另获明确授权。

## 角色链

```text
Route Agent evidence → Main summary → Online Owner qualification and formal submit
```

- Route Agent 不正式提交 Online；
- Main 只协调 timing、汇总 Child receipt 并提出 recommendation；
- Planning / Review Layer 管理授权范围之外的 Online 决策；
- Online Owner 是唯一正式 Judge submitter。

## 提交依据

Main 向 Online Owner 提供：

- starting local measurement context；
- current local best；
- 每个独立 Revision 的 Local gain；
- cumulative local gain；
- Correctness status；
- experiment count；
- 同一 Route 内候选的可比性与最佳候选依据；
- 失败和负结果的保留情况。

候选只在本 Route 内、Local 测量可比时确定最佳项；不得把不同 shape 的跨 Route Local 百分比放在一起排序。每条 Route 独立判断，W4 范围内最多提交一个候选；不得为凑数选用非最佳或不可比结果。

## 提交资格

提交前必须同时满足以下条件：

1. 对应 Candidate 的 Build（Compile）为 `PASS`；
2. 对应 Candidate 的 Correctness 为 `PASS`；
3. Local 按 `项目规则/本地性能测试规范.md` 判定有效，具备 numeric score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best；
4. 该候选由本 Route 的可比有效 Local 结果独立选为最佳；比较按相同 shape、dtype、runner、计时边界、设备条件及采样方法确认，不跨 Route 混排不同 shape 的百分比；
5. Judge 提交载荷与 Build、Correctness、Local 所验证的 Candidate 源码完全一致，并核对和记录对应 Git commit ID；
6. 确认本 Route 尚未用完每 Route 一次的授权额度、Judge 当前可用提交额度，并已确认当期 Judge 规则。

六项须全部成立。任一项缺失、失败或无法确认，该 Route 记为 `NO_ELIGIBLE_SUBMISSION`，不提交，也不改用其他 Route 候选凑数。不得在规则或本 Skill 中预先写入具体 eligible Route 名单。

符合条件后，Online Owner 使用现有正式提交工具提交一次，保留 Judge 返回的正式结果，并向 Main 和 Record Owner 发送 receipt。Online Owner 不改 Candidate，不决定 Route 生命周期，不修改共享成绩记录。

只核对实际提交源码与已验证 Candidate commit 的精确对应关系；不扩展为无关身份流程。历史证据中的旧字段保持原样。工具内部若仍产生旧摘要元数据，属于 `TOOLING_REMAINDER`；工作流不读取、不计算、不以它作为提交依据。

## C2C receipt

```text
ONLINE_EVENT
ROUTE = <route>
REVISION = <revision>
DECISION = <APPROVED|HOLD|NOT_SUBMITTED>
SUBMISSION = <id or NONE>
STATUS = <PASS|FAIL|INCONCLUSIVE|NOT_RUN>
OFFICIAL_SCORE = <value or NONE>
OFFICIAL_DELTA = <value or NONE>
LOCAL_CONTEXT = <baseline and current best>
BLOCKER = <text or NONE>
```

Online Owner 不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
