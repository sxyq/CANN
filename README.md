# 2026 CANN 挑战赛 · 西南赛区

独立 CANN 比赛仓库，题目为 `AddRmsNormBias`。

| 内容 | 位置 |
| --- | --- |
| Agent 工作约定 | [AGENTS.md](AGENTS.md) |
| 比赛文档 | [文档/](文档/) |
| Ascend C 源码 | [源码/](源码/) |
| 调研、来源和验证 | [调研/](调研/) |
| 路线最高成绩 | [调研/路线最高分.md](调研/路线最高分.md) |
| 提交包目录 | [提交/](提交/) |
| 临时结果 | [临时/](临时/) |

完整入口文档见 [文档/README.md](文档/README.md)。

目标远端仓库：`https://github.com/sxyq/CANN.git`。当前分支为 `main`，本地只有这一套工作树；路线最高成绩见 [调研/路线最高分.md](调研/路线最高分.md)。当前候选文件为 `提交/混合方案/H001-正确性优先/V003/kernel.asc`；V002 已完成服务器 CANN 编译和模板 case0 运行，完整精度与性能仍待验证。

OpenAI 关于 Navier-Stokes 与 Erdős unit distance problem 的公开资料，以及可迁移到本题的 Agent 研究组织方式，见 [调研/归档/OpenAI高难题Agent方法与社区讨论.md](调研/归档/OpenAI高难题Agent方法与社区讨论.md)。

目录命名约定：仓库级目录优先使用中文；CANN 工具要求的 `op_host`、`op_kernel` 以及标准构建文件名保持原名。
