---
name: cann-online-owner
description: CANN AddRmsNormBias Online Owner Skill。唯一 Judge submitter 按 W4 多版本授权提交真实 Candidate 并保留真实结果。
---

# Online Owner

## W4 Official Scoring Override

> **W4 强制执行规则：本项目禁止设置、保留或新增任何自行定义的实验、评分、Correctness 覆盖、Local 测量稳定性、审批或其他线上提交前置门禁。每条 W4 技术路线必须选取至少两个不同源码身份的真实 Candidate 进行 Official 线上评分。任何研究报告、状态同步、历史归档或本地测量结果均不得作为阻止合法 Official 提交的内部条件。服务器运行安全、工作树隔离及评测平台强制规则仍须遵守。**

授权为 `USER_AUTHORIZED_W4_MULTI_SUBMISSION`：覆盖 W4-R01 至 W4-R15，每条路线至少两个源码身份不同的真实 Candidate，长期目标不少于 30 次 Official Judge 提交。无需逐 Candidate Planning 或 Main 批准，也不要求项目自设 Compile、Correctness、Local、Local Best、Local 稳定性或研究完成条件。只按平台强制规则、账户权限、Judge 实际配额及服务器安全处理真实限制。

## 角色链

```text
Route Agent source identity → Main dispatch → Online Owner official submit → Judge receipt → Record Owner
```

- Route Agent 不正式提交 Online；
- Main 协调路线、候选和提交顺序，不提交、不追加逐次批准；
- Planning / Review Layer 负责最终路线组合与冠军版本决定，不逐次批准本授权内的提交；
- Online Owner 是唯一正式 Judge submitter，只使用唯一现有提交脚本 `工具/cannjudge-submit.mjs`；
- Record Owner 根据真实 receipt 异步更新共享成绩，不控制 Online 流程。

## 候选安排

每条 Route 准备至少两个源码 SHA256 不同的真实性能 Candidate。若该 Route 只有研究记录或一个 Candidate，则由 Route Agent 在本 Route 自己的 worktree 中继续产生真实版本，不得把研究报告、Parent、测试代理或重复源码算作 Candidate。相同源码 SHA256 不构成第二种源码身份。

Local 数据可用于同 Route 最佳/次佳排序；如只有不稳定观察，允许标记 `PROVISIONAL_LOCAL_RANK` 并记录不确定性；若 Local 缺失则标记 `LOCAL_RANK=UNKNOWN`。这些标签只表达选择依据，不控制提交权限。Compile、Correctness、Local 和 Full Correctness Coverage 都是开发及问题定位工具，不是项目自设 Official 前置。

## 提交流程

1. 从该 Route 提供的 Candidate 文件和来源信息选择一个尚未评测的源码身份。记录 Route、Revision、Source SHA256、来源 Git commit 及所用文件；该信息用于来源追溯，不扩展成额外审核。
2. 仅通过 `工具/cannjudge-submit.mjs` 发起 Judge 提交。不得创建第二套提交脚本或绕开平台。
3. 实际确认账户权限、Judge 当前是否接受提交、真实剩余额度和平台强制字段。平台拒绝、账号无权或真实配额用尽时，保存实际响应并转向其他可执行提交；不创建项目自设审批、等待或 Correctness/Local 流程。
4. 同一路线提交第二个源码身份不同的 Candidate。不得对仍处于 `PENDING` 的同一 Candidate 重复提交；平台确认未受理首次请求后，按真实状态处理重试。
5. Official 失败也作为真实事件保存，不删失败记录，不只汇报成功分数。至少 30 次目标以 15 条 Route 每条至少两个不同来源源码身份为计划依据；Judge 实际接收数量和状态以真实响应为准。
6. 每次提交完成或返回状态后，立即将事实 receipt 发给 Main 与 Record Owner，不必等待其他 Route 或 Local 工作。

Official Score 只取自 Judge 响应，禁止用 Local 数值代替。用户已授权本轮多版本范围，无需逐候选 Planning 批准。Candidate 仅由所属 Route Agent 在其自己的 worktree 修改；Online Owner 不改 Candidate、不进入其他 Route worktree、不决定 Route 生命周期、不写共享成绩表。

## 结果保存

每个 Judge 响应保存到既有 `线上结果/<ROUTE>/<REVISION>/` 位置，至少包括：

- Route、Revision、Source SHA256、Git commit 与提交源码路径；
- Submission ID、提交时间、官方状态、Correctness 和 Official Score；
- 15 个 Case 的 `timeUs`、`bestTimeUs`，以及平台返回的分项得分（若有）；
- 官方原始结果 JSON、平台失败原因及工具日志。

不猜测缺失 case 数据、得分或官方状态。每项提交都发送：

```text
ONLINE_EVENT
ROUTE = <W4-R01..W4-R15>
REVISION = <revision>
SOURCE_SHA256 = <actual source identity>
GIT_COMMIT = <source commit>
SOURCE_PATH = <candidate file>
SUBMISSION_ID = <id or NONE>
SUBMISSION_TIME = <timestamp or UNKNOWN>
STATUS = <actual Judge status>
OFFICIAL_CORRECTNESS = <actual result or UNKNOWN>
OFFICIAL_SCORE = <value or NONE>
CASE_TIME_US = <15 values or UNKNOWN>
CASE_BEST_TIME_US = <15 values or UNKNOWN>
RESULT_JSON = <path or NONE>
LOCAL_RANK = <rank|PROVISIONAL_LOCAL_RANK|UNKNOWN>
LOCAL_OBSERVATION = <actual facts or NONE>
PLATFORM_RESPONSE = <actual reason or NONE>
```

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
