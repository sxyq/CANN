# CANN AddRmsNormBias

比赛已结束，仓库当前处于只读归档状态。合并后的路线索引、全版本源码清单和可恢复原始资料见 [比赛结束归档说明](归档/比赛结束-20261017/README.md)。

基础提交源码保留在 [`源码/R31B-V011/submission.asc`](源码/R31B-V011/submission.asc)，对应正式冠军 `R31B V011`、Official `45.16`、SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。五条第二波路线的分数均未形成可靠 Local Best；N07 R005 的高分复验不稳定，不作为晋升候选。

历史实验和阶段资料仍按原证据保存在 `本地实验/`、`线上结果/`、`归档/` 与比赛结束归档中。`worktrees/`、`server_runs/`、W2/W4 临时包和本地实验缓存已迁入比赛结束归档，Git 分支与 Worktree 登记保留。

`phase4/` 是历史阶段名称，不是当前目录入口。

新用户 / 新 Agent 先读：

1. **`AGENTS.md`** — 第一入口与职责分工
2. **`归档/比赛结束-20261017/README.md`** — 全版本、源码索引与证据导航
3. **`.agents/skills/cann-mainline/SKILL.md`** — 既有工作流入口
4. **`项目规则/`** — 比赛期间规则存档；与只读归档状态冲突时，以本文件和 `AGENTS.md` 为准
5. **`技术路线/`** — 路线总表、路线图、成绩表、全版本记录

```
cann/
├── AGENTS.md
├── README.md
├── 源码/              唯一基础提交源码 R31B-V011
├── 项目规则/          比赛期间规则（历史参考）
├── 技术路线/          总表 / 路线图 / 成绩表 / 全版本记录 / 冠军
├── 本地实验/          <ROUTE>/<REVISION>/ 本地证据
├── 线上结果/          <ROUTE>/<REVISION>/ 正式提交证据
├── 调度/              比赛期间任务、线上候选、校准与设备记录
├── 研究/              历史路线研究
├── 工具/              比赛期间 CANNJudge 提交脚本
├── 归档/              比赛结束归档及更早历史材料
└── .agents/skills/    项目级 Skill
```

比赛：2026 CANN 挑战赛·西南赛区，题目 `AddRmsNormBias`。
当前 Overall Champion 见 `技术路线/路线成绩表.tsv`。
