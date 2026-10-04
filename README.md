# CANN AddRmsNormBias

当前正式主线已展开在**仓库根目录**。`phase4/` 是历史阶段名称，不是当前目录入口。

新用户 / 新 Agent 先读：

1. **`AGENTS.md`** — 第一入口与职责分工
2. **`.agents/skills/cann-mainline/SKILL.md`** — 角色路由
3. **`.agents/skills/cann-*/SKILL.md`** — 当前角色工作方式
4. **`项目规则/`** — 当前主题规则
5. **`技术路线/`** — 路线总表、路线图、成绩表、全版本记录

```
cann/
├── AGENTS.md
├── README.md
├── 项目规则/          当前有效规则（中文）
├── 技术路线/          总表 / 路线图 / 成绩表 / 全版本记录 / 冠军
├── 本地实验/          <ROUTE>/<REVISION>/ 本地证据
├── 线上结果/          <ROUTE>/<REVISION>/ 正式提交证据
├── 调度/              当前任务 / 线上候选 / 校准 / 设备使用
├── 研究/              <ROUTE>/ 轨道研究
├── 工具/              CANNJudge 提交脚本
├── 归档/              重构前项目 / 历史阶段 / 历史控制 / 历史工作区
└── .agents/skills/cann-*/      角色 Skill 与兼容路由
```

比赛：2026 CANN 挑战赛·西南赛区，题目 `AddRmsNormBias`。
当前 Overall Champion 见 `技术路线/路线成绩表.tsv`。

版本事实只有两处入口：`技术路线/技术路线图.md` 是唯一人类可读版本树和 Markdown 表；`技术路线/全版本记录.tsv` 是唯一结构化账本。Route Agent 发送 `VERSION_RECORD_EVENT`，Record Owner 异步同步；Main 不读写这两份文件。Dashboard 只展示状态，不决定状态，也不控制执行。
