# Git 工作流程

## 历史：W4 Official 评分安排

> **W4 强制执行规则：本项目禁止设置、保留或新增任何自行定义的实验、评分、Correctness 覆盖、Local 测量稳定性、审批或其他线上提交前置门禁。每条 W4 技术路线必须选取至少两个不同源码身份的真实 Candidate 进行 Official 线上评分。任何研究报告、状态同步、历史归档或本地测量结果均不得作为阻止合法 Official 提交的内部条件。服务器运行安全、工作树隔离及评测平台强制规则仍须遵守。**

引用的 W4 Online 约定属于已退役阶段的历史安排，其中唯一正式提交者是 Online Owner，使用唯一现有 Judge 脚本。本轮五路线重建未指定 Online 操作。本文件保留的 Commit/Event 顺序继续约束开发期实验与 Git 溯源，不能作为新增实验环节或提交条件；当前实验沿用用户指令与现有流程。

本轮规则维护与 W4 状态更新已获用户授权精确提交并普通推送以下文件：

```text
.agents/skills/cann-online-owner/SKILL.md
.agents/skills/cann-main-orchestrator/SKILL.md
.agents/skills/cann-route-executor/SKILL.md
.agents/skills/cann-record-owner/SKILL.md
AGENTS.md
项目规则/W4持续探索控制契约.md
项目规则/实验总则.md
项目规则/执行约定.md
项目规则/Git工作流程.md
项目规则/线上提交规范.md
```

只暂存并提交上述路径中的本轮规则变更。其他已暂存、未暂存文件和未跟踪资料均不纳入。

本文件是实验 Git 顺序和安全边界的唯一说明。

## 实验版本顺序

```text
EDIT
→ COMPILE
→ CORRECTNESS
→ LOCAL
→ RESULT
→ COMMIT RESULT
→ VERSION_RECORD_EVENT
→ NEXT CHANGE
```

Compile 之前不建立提交边界。每个实验/版本至少一个独立 commit；编译、Correctness、Local 的事实可随该实验结果一起提交，失败和负结果同样提交并保留。

Local 改善时：

```text
COMMIT RESULT → VALID LOCAL RESULT → CURRENT_LOCAL_BEST
PUSH = NO
```

Local 未改善时：

```text
COMMIT NEGATIVE RESULT → KEEP HISTORY → VERSION_RECORD_EVENT
PUSH = NO
```

下一次小变化从当前 `CURRENT_LOCAL_BEST` 继续，不把多个独立变化压进一个 Revision。

实验结果的 Git 提交完成后发送 `ROUTE_EVENT + VERSION_RECORD_EVENT`；版本事件存在后才能开始下一普通 Revision，Record Owner 的异步写入不能延误已经在途阶段。版本字段和共享文件同步由 `.agents/skills/cann-record-owner/SKILL.md` 统一说明。

## 暂存范围

- 只使用 `git add <specific paths>`；
- 禁止 `git add .`、`git add -A`、`git add --all`；
- Route 分支只提交自己的 Route 文件；共享记录由 Record Owner 单独提交；
- 规则迁移、Candidate、实验证据和共享记录分开组织；
- 一个提交不混入无关路线、无关文件或未授权的用户改动；
- 同文件含范围外内容时，用精确片段暂存；W2/W3/W4 历史记录和其他既有内容继续保持原样，不纳入无关修改。

## Route 隔离

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

所有 Child 命令固定在自己的指定工作树。Route Agent 不提交其他 Route、不写主目录，也不得用 `git show` 或 `git grep` 访问其他 Route 的 Git 对象；跨 Route 信息仅使用 Main 发布的只读审计摘要或自己的 Direct Parent。Main 可在获准的本地只读审计中查看已提交证据；不读取其他工作树未提交文件。

Main 可本地只读 Git、源码和实验证据；用户明确授权时可编辑根 `AGENTS.md`，不写入其他项目文件、不运行实验或 NPU。Record Owner 占 SLOT-3，唯一工作树为 `/Users/sunyiyang/Desktop/Project/cann`、分支 main，只提交获准规则和四份共享记录，规则与共享分别 commit；不操作其他实际工作树。Online Owner 不改共享成绩记录。

Route 任务结束当前安全闭环后，保存证据与事件，关闭旧 Agent并确认，再用空槽创建 fresh context 接手下一 Route；原 branch/worktree 保留，后续同 Route 接手继续使用。全体 active Child 最多 5 个，含 Record Owner。

## 禁止操作

禁止 force push、reset、clean、历史改写、覆盖已有证据和删除失败版本。不得因整理文档删除源码、凭据、SQLite、Keychain、LaunchAgent、运行缓存或用户需要的数据。本轮仅按上列七个规则文件精确提交并普通推送；不纳入其他既有差异，不移除工作树、分支或提交。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
