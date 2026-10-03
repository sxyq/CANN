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

## 项目状态看板

- 看板入口：`归档/任务看板/index.html`；正式数据仍以本文件列出的项目记录为准。
- 单项任务超过 5 个步骤或预计超过 30 分钟时，开始实现前先在看板写入目标、阶段、卡点、待答问题和未获答复时的默认动作。
- 每完成一个独立步骤，重建看板快照：`node 归档/任务看板/refresh.mjs`。
- 新的线上脚本调用需在 `归档/任务看板/submit-events.tsv` 记录开始与结果；历史调用总数没有统一来源时保持未知。
- 看板只放状态摘要和证据路径，不复制 Candidate 源码、凭据或整段对话，也不代替 Planning 改路线决定。

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

---

## 群聊情报提取（工具/提取/）

从手机 QQ 数据库增量提取比赛群聊天记录，筛选对解题有价值的技术讨论。
**只提取指定群；数据库用完即删，不留副本。**

```bash
cd 工具/提取
./run.sh            # 增量提取 + 提分筛选（日常用这条）
./run.sh --status   # 查看增量状态
./run.sh --full     # 全量重抓
```

前置条件：手机连 USB + USB 调试 + root（KernelSU/Magisk）。
首次需 `./run.sh --setup` 装 `sqlcipher3`。

关键前提（实测，勿改）：
- 题目是 `AddRmsNormBias` 单一算子，case1~case15 是同一算子的 15 个测试形状
- 群号在 `group_msg_table."40027"`，**不是**官方文档写的 `40030`（后者是空列）
- 密钥 = `md5(md5(nt_uid) + rand)`，rand 在文件头 offset **0x2E**
- SQLCipher：`page_size=4096, kdf_iter=4000, HMAC_SHA1, PBKDF2_HMAC_SHA512`
- 必须剥 1024 字节头；库有损坏页，需 rowid 游标分页

只读操作，不attach/hook QQ 进程，无封号风险。
细节见 `工具/提取/README.md`。
