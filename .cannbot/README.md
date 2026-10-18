# CANNBot 与 Agent 资料

此处说明项目使用的 CANNBot 插件资料；Agent 定义和技能正文分别位于 `.codex/` 与 `.agents/skills/`。

## 随主线保存

- `.agents/skills/`：21 个 Ascend C 与 CANN 项目技能目录。
- `.codex/agents/`：4 个 Codex Agent 定义。
- `.codex/cannbot-plugin.json`：插件版本、当前仓库实际具备的技能入口、Agent 入口和依赖目录清单。本机安装位置不写入共享清单。
- `.cannbot/plugins/ops-direct-invoke/`：许可文本与 hook 文件。

当前保存的插件目录只含许可和 hook 资料；hook 还引用了该目录中没有的 `AGENTS.md`、`workflows/` 和插件内 `asc-devkit/`。因此这里用于保留来源，不能单独作为可运行插件安装。

## 外部依赖

依赖源码由上游仓库独立维护，父项目不复制整份第三方源码：

| 本地目录 | 上游仓库 |
|---|---|
| `.cannbot/dependencies/ops-direct-invoke/asc-devkit/` | `https://gitcode.com/cann/asc-devkit.git` |
| `.cannbot/dependencies/ops-direct-invoke/ops-tensor/` | `https://gitcode.com/cann/ops-tensor.git` |
| `ops-tensor/include/tensor_api/` | `ops-tensor` 声明的 `asc-devkit` 子模块 |

本机这两份依赖目录不是父仓库的子模块。盘点时，`asc-devkit` 有 1 个本地提交和 2,711 个工作区文件改动；`ops-tensor` 有 2 个本地提交，且其 `tensor_api` 子模块另有独立本机提交。它们的本地 Git 历史和工作区差异不随父仓库提交；需要复用这些差异时，应先单独整理并明确目标仓库，避免将第三方源码或本机改动误推入 `CANN` 主仓库。

归档中的提交队列运行时、浏览器用户数据、编译产物、测试输入输出和 profiling 原始缓存均不纳入共享资料。
