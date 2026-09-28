---
name: cann-mainline
description: CANN AddRmsNormBias 主线工作方式技能。项目启动、路线研究、Revision 生命周期、Track-A/B、server3 验证、本地性能判定、Online 提交、记录同步、Git 提交。当需要在本仓库开展任何实验、路线、Revision、编译、正确性、测时或线上提交工作时使用。
---

# CANN AddRmsNormBias 主线工作方式

本技能描述**怎么工作**，不复制会变的成绩。成绩、路线状态、Parent 关系一律读正式记录。

## A. 项目启动流程

进入项目后按序读取，禁止只依赖聊天记忆：

1. `AGENTS.md`
2. `项目规则/实验总则.md` → `执行约定.md` → `本地性能测试规范.md`
3. `技术路线/技术路线总表.md`
4. `技术路线/技术路线图.md`
5. `技术路线/路线成绩表.tsv`
6. `技术路线/全版本记录.tsv`
7. 若是具体 Route：`研究/<ROUTE>/` 与当前 `本地实验/<ROUTE>/<REV>/`

启动时确认：Overall Champion、该 Route 的 LOCAL_BEST / OFFICIAL_BEST / CURRENT_CANDIDATE、以及 Planning 是否有未执行决定。

## B. 路线探索流程（Track-B）

技术路线先进入路线池。来源包括：

R001–R029、FULL-R、R030/R031、H00N、MIX、Integration、MAIN-1、MAIN-2、后续新增。

每个方向先写清楚：

- 解决什么问题
- 修改哪里（文件/函数级）
- 适合哪些 shape / dtype
- 为什么可能有效、为什么可能失败
- 是否与已有路线重复
- 最小验证方法

**每轮研究 3–5 个不同方向 → Planning / Review 选择 1 个 → 才创建 Revision。**

Route Agent **不能**自己从研究列表里选一个直接开写。

Track-B 只研究：不改 Kernel、不创建 Revision、不擅自选方向。

## C. 一个 Revision 一个核心变化

```
ONE REVISION = ONE CONCEPTUAL PERFORMANCE CHANGE
```

只能测一个核心技术变化。Build Fix / Correctness Fix 可修当前版本，但不得顺手加第二个性能优化。`SINGLE_CHANGE_AUDIT` 必须为 PASS 才可进 Online 资格评估。

## D. Track-A / Track-B

**Track-A**：完成当前 Candidate —— 编译 → 正确性 → 本地性能 → Review → 批准后再 Online。

**Track-B**：等机器/等结果时，继续研究下一轮 3–5 个不同方向（只研究，不动手）。

## E. Route Ownership

```
1 Route = 1 Agent = 1 Worktree = 1 Branch = 1 Context
```

同一路线由同一负责人继续。重新创建干净 worktree 不改变路线历史。

从 canonical 创建 worktree：

```bash
git worktree add <path> -b <route-branch> exp/independent-breadth
```

## F. 分工

| 角色 | 做 | 不做 |
|---|---|---|
| Planning / Review Layer | 路线池、选路、继续/暂停/合并/替换/关闭、新路线、Online 决定 | — |
| Codex Main | 调度、审阅、Git、记录、整合、server3 协调、Online 准备 | 不写 Candidate Kernel |
| Route Agent | 自己的 Candidate、编译、Correctness、Local、Fix、下一轮研究 | 不改共享调度、不提交 Online、不改他路线 |

**路线生命周期决定权（PARK/CLOSE/ABANDON/MERGE/REPLACE/新增或释放 slot）只属于 Planning / Review Layer。** Main 与 Route Agent 只报告事实、提出建议。

## G. server3 流程

```
确定一个修改 → 创建 Revision → 同步源码 → server3 编译 → 正确性 → 本地性能 → Review → Online
```

编译类任务：`FREE_HBM >= 100 MB` 即可继续；最多 8 张卡并行 8 个不同 Revision。

**不得**因 AICore 占用、VLLM 存在、其他进程、d7 而停止编译。不得杀他人任务。

Correctness 能安全运行就执行。Performance 才需要更严格的单卡安排（lease + same-binary + noise floor + 交错 P/C）。

连接：`ssh cann-server3`（别名已配置）。用户 `lelinfeng`，实验目录在 `/home/data4t2/lelinfeng/`。

## H. 本地性能流程

不能只跑一次。必须：

1. Parent 自己是否稳定（same-binary）
2. 判断该 shape 正常波动（noise floor）
3. Parent/Candidate 同条件、交替（interleaved）运行
4. 多组样本，保存原始数据
5. 比较收益与正常波动
6. 判断多轮方向是否一致

记录：Parent latency、Candidate latency、Local delta、波动、方向一致性、LOAD_QUALITY。

**不要伪造 Local Official Score。** 只用 LOCAL_ACCEPTED / LOCAL_REJECTED / NEEDS_ONE_MORE_LOCAL / MEASUREMENT_BLOCKED 等本地 verdict。

仅 LOCAL_ACCEPTED 才推进 LOCAL_BEST。

## I. Local / Official 分离

每条 Route 同时维护三者，互相独立：

- `OFFICIAL_BEST`
- `LOCAL_BEST`
- `CURRENT_CANDIDATE`

本地快 ≠ 线上一定高。线上分高不能倒推历史本地表现。已有 Official Score 的 Revision 不必为了“本地分”再强制提交。

## J. Online 流程

只有经过：真实编译 → 正确性 → 可信本地结果 → Review 的 Candidate，才可能进 Online。

两级：Codex Main 给 ONLINE_RECOMMENDATION（WORTHY/NOT_WORTHY）；Planning / Review Layer 给 ONLINE_DECISION（APPROVED/HOLD）。只有 APPROVED 才由 Judge Owner 提交。

正式提交只能由统一 Judge Owner 执行：

```bash
npm run cannjudge:submit -- --yes --source <exact-file>
```

提交前记录 SOURCE_PATH / 行数 / 字节数 / LOCAL_SHA / SIDECAR_SHA；提交后核对 LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA。不一致 → `INPUT_IDENTITY_MISMATCH`，`formalResultEligible=false`。

结果记录：Submission ID、PASS/FAIL、Official Score、相对 OFFICIAL_ANCHOR 的变化、PROMOTE/REJECT/INCONCLUSIVE。

## K. Local / Online 校准

每次 Online 结果回看本地当时怎么看、线上实际怎样，写入 `调度/本地线上校准.tsv`：

方向一致 / 方向相反 / 本地数据不足 / 哪些 shape 易误导。

长期维护本地测试可信度。历史已知：小局部百分比可能 FALSE_POSITIVE。

## L. Git 流程

完成一个独立事实就 commit + push：

- 建立一个版本 → commit/push
- 编译结果 → commit/push
- 正确性结果 → commit/push
- 本地性能结果 → commit/push
- 线上结果 → commit/push
- 路线记录同步 → commit/push

禁止：`git add .`、大包提交、force push、盲目 reset、`git clean` 删证据。

每次提交前：`git status` → `git diff` → `git add <明确路径>` → `git commit` → `git push` → 确认远端同步。

## M. 记录同步（三处同步）

每个 Revision 新事实完成后至少更新：

1. Revision 自己的记录（`本地实验/<ROUTE>/<REV>/` 或 `线上结果/<ROUTE>/<REV>/`）
2. `技术路线/全版本记录.tsv`
3. `技术路线/路线成绩表.tsv` / `技术路线/技术路线图.md`

有 Online 时再更新：4. `调度/本地线上校准.tsv`

不能只改一个 JSON 就算完成。

## N. 历史路线

R001–R029 不准丢，作为技术知识库长期保留（见 `技术路线/技术路线总表.md` 与 `归档/`）。

同时维护 FULL-R、R030/R031、H00N、MIX、Integration、MAIN-1、MAIN-2 的关系。

## O. Mixed Route

混合路线必须写清：

- 由哪些机制组成
- 各组成是否单独验证
- 组合版本结果、Local、Official
- 失败发生在哪里

**组合失败不等于所有组成技术都失败。**

## P. 证据与失败保留

失败版本必须保留：BUILD_FAILED、CORRECTNESS_FAILED、LOCAL_REJECTED、Official REJECT、WA、Runtime Error、Compile Error 全部是实验资产。禁止只留 Best。

证据包最低五件套：`submission.asc`、`submission.sha256`、`source-meta.json`、`diff.patch`、`local-result.json`（Online 另加 `result.json`）。

## Q. 结束条件

当前无未解决 Candidate 且无明确分配时，向 Planning / Review Layer 报告并停止；不要自行开新路线或新 Revision。
