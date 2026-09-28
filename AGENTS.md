# CANN AddRmsNormBias — Agent 入口

本文件是 `/Users/sunyiyang/Desktop/Project/cann` 内所有 Agent 的第一入口。
新 Agent 不需要知道 Phase3 / Phase4 的历史演变，直接按本文件与项目级 Skill 工作。

## 项目是什么

- 比赛：2026 CANN 挑战赛·西南赛区，题目 `AddRmsNormBias`
- 目标：Official Score > 50（当前 Overall Champion 见下）
- 当前正式主线已展开在**仓库根目录**；`phase4/` 是历史阶段名称，不是当前目录入口

## 当前事实（只从正式记录读取，不靠聊天记忆）

| 事实 | 记录位置 |
|---|---|
| Overall Champion / Official 最佳 | `技术路线/路线成绩表.tsv`、`线上结果/` |
| 各 Route 当前 Candidate / LOCAL_BEST | `技术路线/路线成绩表.tsv` |
| 全部 Revision 与 Parent 关系 | `技术路线/全版本记录.tsv` |
| 路线状态与调度 | `调度/当前任务.tsv` |
| 本地↔线上校准 | `调度/本地线上校准.tsv` |

当前 Overall Champion 以 `技术路线/路线成绩表.tsv` 中 `OVERALL` 行为准。
修改任何分数、Parent、Official Score 必须有对应证据文件，不得凭记忆改写。

## 谁负责什么

| 角色 | 职责 | 不得做 |
|---|---|---|
| **Planning / Review Layer**（外部） | 路线池、选路、继续/暂停/合并/替换/关闭、新路线、Explore/Exploit 布局、是否 Online | — |
| **Codex Main** | 调度、审阅证据、Git、记录、整合、server3 协调、Online 准备 | 不写 Candidate Kernel |
| **Route Agent** | 写自己的 Candidate、编译、Correctness、Local performance、Build/Correctness Fix、下一轮研究 | 不改共享调度、不改他路线源码、不自行提交 Online |

### 【路线生命周期决定权】

Route Agent 和 Codex Main **都不得**自行决定：

`PARK` / `CLOSE` / `ABANDON` / `MERGE` / `REPLACE`
新增正式 Route、释放正式 Route slot

他们只能：**报告事实、提出建议**。
真正路线决定由外部 Planning / Review Layer 作出。Online 分两级：Main 只做 ONLINE_RECOMMENDATION，Planning 做 ONLINE_DECISION，Judge Owner 仅在 APPROVED 后提交。

## 开始任何任务必须读

1. `AGENTS.md`（本文件）
2. `项目规则/实验总则.md`
3. `项目规则/执行约定.md`
4. `技术路线/技术路线总表.md`
5. `技术路线/技术路线图.md`
6. `技术路线/路线成绩表.tsv`
7. `技术路线/全版本记录.tsv`
8. 若是具体 Route：该 Route 的研究记录与当前 Revision 目录

**禁止只依赖聊天记忆。**

## Route Agent 额外必读

- `项目规则/本地性能测试规范.md`
- `项目规则/服务器实验规范.md`
- `项目规则/线上提交规范.md`
- `项目规则/Git工作流程.md`
- 项目级 Skill：`.agents/skills/cann-mainline/SKILL.md`

## 实验完成后要更新

每个独立事实完成后，至少同步：

1. 该 Revision 自己的记录（`本地实验/<ROUTE>/<REV>/` 或 `线上结果/<ROUTE>/<REV>/`）
2. `技术路线/全版本记录.tsv`
3. `技术路线/路线成绩表.tsv` / `技术路线/技术路线图.md`
4. 有 Online 时：`调度/本地线上校准.tsv`

不能只改一个 JSON 就算完成。

## Git 基本要求

- 一个独立事实 → 一次 `git add <明确路径>` → `commit` → `push`
- 禁止：`git add .`、大包提交、force push、盲目 reset、`git clean` 删证据
- 详见 `项目规则/Git工作流程.md`

## Skill

项目级 Skill 位于：

`.agents/skills/cann-mainline/SKILL.md`

Codex/MiMoCode 通过 `.agents/skills/**/SKILL.md` 发现。
Skill 说明工作方式，**不复制会变的成绩**；成绩看正式记录文件。

## 历史资料

- 重构前项目 / 历史阶段 / 历史分支快照 / 历史控制文件 → `归档/`
- 归档内旧链接保持历史原貌，不为美化改写
- 详见 `归档/README.md`
