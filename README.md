# 2026 CANN 挑战赛 · 西南赛区

独立 CANN 比赛仓库，题目为 `AddRmsNormBias`。

| 内容 | 位置 |
| --- | --- |
| Agent 工作约定 | [AGENTS.md](AGENTS.md) |
| 比赛文档 | [文档/](文档/) |
| Ascend C 源码 | [源码/](源码/) |
| 调研、来源和验证 | [调研/](调研/) |
| 提交包目录 | [提交/](提交/) |
| 临时结果 | [临时/](临时/) |

完整入口文档见 [文档/README.md](文档/README.md)。

目标远端仓库：`https://github.com/sxyq/CANN.git`。远端 `origin/main` 最近确认的提交为 `ff7ba7987fe544efcaf82f5f95ad80abfef5bbce`；本地跟踪状态显示 `main` 领先 3 个提交，包含首版候选 `6309117`、推送状态记录和直调入口调整 `7f6277d`。最近一次 HTTPS 推送未返回确认。首版候选文件为 `提交/首版/kernel.asc`；源码和文档已完成本机静态审阅，真机编译、精度与性能仍待 CANN/NPU 环境确认。

OpenAI 关于 Navier-Stokes 与 Erdős unit distance problem 的公开资料，以及可迁移到本题的 Agent 研究组织方式，见 [调研/openai-agent-research.md](调研/openai-agent-research.md)；多平台源码和社区证据见 [调研/多Agent研究/研究简报.md](调研/多Agent研究/研究简报.md)。

目录命名约定：仓库级目录优先使用中文；CANN 工具要求的 `op_host`、`op_kernel` 以及标准构建文件名保持原名。
