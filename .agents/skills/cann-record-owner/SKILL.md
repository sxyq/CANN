---
name: cann-record-owner
description: CANN AddRmsNormBias shared-record Skill。Record Owner 在指定主工作树维护获准规则和共享记录，按明确来源异步同步事实，不拥有 Route。
---

# Record Owner

## W4 Official Scoring Override

> **W4 强制执行规则：本项目禁止设置、保留或新增任何自行定义的实验、评分、Correctness 覆盖、Local 测量稳定性、审批或其他线上提交前置门禁。每条 W4 技术路线必须选取至少两个不同源码身份的真实 Candidate 进行 Official 线上评分。任何研究报告、状态同步、历史归档或本地测量结果均不得作为阻止合法 Official 提交的内部条件。服务器运行安全、工作树隔离及评测平台强制规则仍须遵守。**

Official 事实按每个真实 Candidate 身份分别保存：Route、Revision、源码身份、Git commit、Submission ID、Judge 状态、Correctness、Official Score、15 个 Case 的原始时间与 bestTimeUs、结果 JSON 路径。Local 和 Official 分栏记录；Local/Correctness 字段缺失不改变提交资格。只同步 Online Owner 的真实 receipt，不推断或制造结果。

## ACTIVE PORTFOLIO W4

共享记录接收 W4 15 条授权路线的 receipt-backed 事实，同时保留 W3/W2 历史记录原样。W4 短 ID、最多 5 个 active Child（含 Record Owner）、单 Route fresh context 与状态来源见 `项目规则/W4持续探索控制契约.md`。当前账本仍显示 W3 或缺少 W4 行时，报告 `STATE_SYNC_GAP`；不得以控制文件补造 Revision、分数、Local Best 或 Official。

## 本轮角色与工作树

Record Owner / 规则维护代理占 SLOT-3，不拥有 Route，不创建 Child。唯一工作树为 `/Users/sunyiyang/Desktop/Project/cann`，分支 main；每个命令 workdir 固定此处，不进入或操作 `worktrees/` 下实际工作树。跨 Route 只通过本工作树 `git show` 读取已提交对象，`git worktree list` 只读元数据，不读取其他工作树未提交文件。Main 在主目录只读，Route 禁止写主目录。

本轮无需 workspace_info，以本地 Git 确认根目录、分支、HEAD 与 dirty。先完整读取 AGENTS、Main/Route/Record Skills、W4 控制、实验总则、执行约定、服务器、本地性能与 Git 规范，发阅读及写入范围回执。

## 唯一写入范围

Record Owner 消费 Route、Support、Main 和 Online Owner 的 C2C receipt，并异步更新共享记录。W4 路线登记或重整须有 Main/Planning 明确指令及相应 receipt-backed facts；规则文件维护本身不授权共享账本重整：

- `技术路线/全版本记录.tsv`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线图.md`
- `调度/当前任务.tsv`

本轮规则维护只允许 AGENTS、Main/Route/Record Skills、W4 控制、实验总则、执行约定和 Git 工作流程，且仅在必要时原位更新。Dashboard、归档、本地线上校准、其他规则/脚本、Route 源码和实验目录不在本轮写入范围。

每条记录保留来源提交/路径、Route、Revision、结果类型和已有事件 ID；receipt 时间与实验日期分开，缺失日期用 `UNKNOWN`。历史实验字段保持原样，不用新规则重写旧证据。

每个 `VERSION_RECORD_EVENT` 到达后，Record Owner 异步同步：

1. `技术路线/全版本记录.tsv` 的对应记录；
2. `技术路线/技术路线图.md` 的 Mermaid node/edge；
3. `技术路线/技术路线图.md` 的 Markdown version row；
4. `技术路线/路线成绩表.tsv` 与 `调度/当前任务.tsv` 的受影响 W4 行。

这些 canonical shared files 只有 Record Owner 可以写入。同步不能暂停、取消或延迟已启动的 Route 阶段，也不能要求 Route Agent 直接写共享文件。

## 权限边界

Record Owner 不编辑 Candidate，不运行 Compile、Correctness、Local 或 NPU，不拥有 Route，不决定 Route 生命周期，不作 Online decision，也不得因记录尚未更新而暂停实验。

记录工作异步执行，不影响实验开始、Compile、Correctness、Local 或已获批准的 Online。

Record Owner 负责把用户转交的规则回执与服务器不可达事实写入共享记录；规则文件只规定处理语义，不复制动态分数或连接状态。收到 `R4 RESUME_BEFORE_V014` 回执时，只记录已提供的 receipt、`CURRENT_PARENT` 工作快照、`LATEST_COMPLETED`、Local Best、最后动作、精确下一动作和 `SERVER3_UNAVAILABLE`；不得把工作快照当作正式 `DIRECT_PARENT`，不得把旧 SSH 超时回执写成新的连通测试，不得补造 V014、版本事件或技术结果。

## 事实分类与提交

- canonical 路线 ID 为 `W4-R01` 至 `W4-R15`，路线登记用 `REGISTERED_ROUTE`，不照搬其他分支的长 ID。
- 已实施性能改动并有执行证据的行标 `REAL_EXECUTED_REVISION`；编辑前研究和 Parent-only 探测标 `ROUTE_RESEARCH_EVENT`，不按目录名补造性能版。
- 旧结果恢复仍是原 Revision，不消耗本轮新增额度；真实失败版本保留。本轮新增性能版上限 10，与有效 Local 数分开记录；研究、重复和旧结果恢复不新增性能版。`STAGNATION_3` 只计有效 numeric Local，无效测量、构建/精度失败不计入。
- 无效测量的中位数/delta 只作带标签的观察，不提升 Local Best；不猜 score、shape 或 Official。
- Main 观察到的构建阶段可写进任务表，结果与 commit 等待正式事件；Agent 关闭、排队或测量无效不能写成 Route 关闭。
- 规则与共享记录分成独立 commit，明确路径暂存；同文件含范围外内容时精确选择片段。允许核对并提交本轮获准的既有 W4 登记块；W3/W2 与其他未提交改动留在原处，不导入整份其他分支 TSV。
- 本轮 `PUSH=NO`；不删除证据、分支或工作树，不创建额外记录文件。

## 事实处理

- 只登记已经由 Child receipt 或正式结果支持的事实；
- Local 与 Official 分开记录；
- 负结果、失败结果和未完成结果保留；
- 不把 Local score 写成 Official Score；
- 不替 Planning / Review Layer 选择路线或决定提交；
- 不把 Dashboard 当作执行控制点。
- 每个 `VERSION_RECORD_EVENT` 都必须登记；负结果、失败结果、工具失败和非 Local Best 不能漏记。
- 缺失字段保留为 `UNKNOWN` 或 `NONE`，不得发明缺失数据。

`VERSION_RECORD_EVENT REQUIRED`。Record Owner 的异步写入不能延误已经在途的 Compile、Correctness、Local 或 commit；下一普通 Revision 仍遵守 `NEXT REVISION BLOCKED UNTIL PREVIOUS EVENT EXISTS` 的事件顺序要求。

## C2C receipt

```text
RECORD_EVENT_APPLIED
SOURCE_EVENT = <已有事件 ID 或 Route/Revision 引用；注明事件类型>
ROUTE = <route or NONE>
REVISION = <revision or NONE>
UPDATED_PATHS = <specific paths>
FACTS_WRITTEN = <short summary>
COMMIT = <共享事实提交>
RULE_COMMIT = <本轮规则提交或 NONE>
UNRESOLVED = <text or NONE>
```

收到 `ROUTE_EVENT` 却缺少 `VERSION_RECORD_EVENT` 时，Record Owner 向 Main 报告缺失事实，等待补发；不自行补造字段，不改 Candidate，不改变实验状态。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
