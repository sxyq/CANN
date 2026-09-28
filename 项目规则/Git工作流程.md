# Git 工作流程

本文件是当前有效规则；与归档旧规则冲突时以本文件为准。

本文件规定本仓库的提交粒度、暂存范围与推送纪律。它细化 `实验总则.md` 第 G/H 节与 `执行约定.md` 第 A/J 节，不改变路线治理与证据留存要求。

## 迁移后目标路径

```text
项目规则/       规则文档（本目录）
技术路线/       路线总表、成绩表、全版本记录
调度/           共享状态：当前任务.tsv、线上候选.tsv、本地线上校准.tsv、服务器设备使用.tsv
本地实验/       逐 Revision 本地证据
线上结果/       逐 Revision 正式提交证据
研究/           路线内假设研究
归档/           历史材料，只读，不为美化改写
```

Canonical 集成仓库：`/Users/sunyiyang/Desktop/Project/cann`。实验分支以实际 Git 状态为准。

## 1. 提交粒度：一个独立事实 = 一次提交

- 一个独立事实（一次源码修改、一份测量证据、一条调度更新、一次规则修订）→ 一次 `git add <明确路径>` → `commit` → `push`。
- 禁止大包提交：一次提交里混入多条路线、多个无关事实或“顺手”的其他改动。
- Candidate 源码、测量证据与共享控制文件**分开**提交。
- Route 分支推送与共享控制（`调度/`）提交分开进行。

## 2. 暂存范围：显式路径

- **禁止 `git add .`**、`git add -A`、`git add --all` 等全量暂存。
- 暂存前先运行 `git status` 与该路线的 `git diff`，核对单变量范围与实际改动文件。
- 只暂存该 Route 的源码或证据文件；共享状态只暂存被改动的具体 `.tsv` 文件。
- Route 分支只提交该 Route 的文件；共享控制只做最小行级改动，且只能更新本 Main 所有路线的行。
- 修改共享状态前重新读取最新 HEAD 与文件内容，不整体重新生成共享文件。

## 3. 每次代码修改后的 checkpoint

```text
git status + git diff（该 Route）
→ 核对单变量范围（SINGLE_CHANGE_AUDIT）
→ 计算源码 SHA
→ git add <明确路径>
→ commit
→ push
→ fetch 后确认本地 HEAD 与该路线远端分支 HEAD 一致
```

提交后必须核对身份：本地 SHA = server3 收到的 SHA；线上还要求 `LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA`（见 `执行约定.md` 第 J 节）。

## 4. 禁止操作

- 禁止 force push。
- 禁止盲目 `git reset`、`git checkout` 覆盖、`git clean` 清理，尤其不得用它们删除或覆盖脏的 Candidate、已有证据或 Git 历史。
- 禁止覆盖已有证据目录：每个 Revision 目录只写一次。
- 禁止凭记忆改写分数、Parent、Official Score：此类改动必须带对应证据文件。
- 未获授权不删除源码、凭据、SQLite、Keychain、LaunchAgent、运行缓存与用户数据。
- 清理必须保留 Git 历史与用户数据。

## 5. 分支与 worktree

- 每条 Route 一个 branch、一个可写 worktree、一个 context（见 `执行约定.md` 第 A 节）。
- 不与其他 Agent 共用可写 Candidate 工作树；不修改其他 Route 的源码。
- Route 内连续 Revision 沿用原分支与原 worktree；路线生命周期决定权在规划层，不通过删分支、改分支名自行处置。

## 6. 提交信息

- 中文或项目既有风格均可，写清“改了哪个事实”。
- 不在提交信息里写凭据、Token、Cookie、授权头。
- 规则文档改动用 `docs:` 前缀，证据记录用对应路线的既有前缀，与仓库历史保持一致。

## 7. 外部推送边界

- GitHub push 属于外部操作：准备与本地核对完成后先汇报，未获明确确认不执行非本任务要求的推送。
- 本任务的推送目标分支以任务指令为准（当前为 `exp/independent-breadth`）。
- CANNJudge 提交不是 Git 操作，见 `线上提交规范.md`。
