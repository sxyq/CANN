---
name: cann-online-owner
description: CANN AddRmsNormBias Online Owner Skill。Planning 批准后由唯一 Judge submitter 执行正式线上提交并返回结果 receipt。
---

# Online Owner

## 角色链

```text
Route Agent → Main recommendation → Planning / Review decision → Online Owner formal submit
```

- Route Agent 不正式提交 Online；
- Main 只协调 timing、汇总 Child receipt 并提出 recommendation；
- Planning / Review Layer 决定是否 Online；
- Online Owner 是唯一正式 Judge submitter。

## 提交依据

Main 向 Planning / Review Layer 报告：

- starting local baseline；
- current local best；
- 每个独立 Revision 的 Local gain；
- cumulative local gain；
- Correctness status；
- experiment count；
- 失败和负结果的保留情况。

没有固定百分比、固定 Revision 数、每次正向结果必提或固定时间间隔的提交要求。积累的每个版本仍只改变一件事。

Planning 批准后，Online Owner 使用现有正式提交工具完成一次提交，保留 Judge 返回的正式结果，并向 Main 和 Record Owner 发送 receipt。Online Owner 不改 Candidate，不决定 Route 生命周期，不修改共享成绩记录。

正式提交不依赖旧版来源字段、对象字段或额外资格流程。工具内部若仍产生旧摘要元数据，属于 `TOOLING_REMAINDER`；工作流不读取、不计算、不以它作为提交依据。

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
