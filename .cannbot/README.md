# CANNBot 与 Agent 资料

此处说明项目使用的 CANNBot 插件资料；Agent 定义和技能正文分别位于 `.codex/` 与 `.agents/skills/`。

## 随主线保存

- `.agents/skills/`：21 个 Ascend C 与 CANN 项目技能目录。
- `.codex/agents/`：4 个 Codex Agent 定义。
- `.codex/cannbot-plugin.json`：插件版本、当前仓库实际具备的技能入口、Agent 入口和依赖目录清单。本机安装位置不写入共享清单。
- `.cannbot/plugins/ops-direct-invoke/`：许可文本与 hook 文件。

当前保存的插件目录只含许可和 hook 资料；hook 还引用了该目录中没有的 `AGENTS.md`、`workflows/` 和插件内 `asc-devkit/`。因此这里用于保留来源，不能单独作为可运行插件安装。

## 依赖源码快照

父仓库收录本机依赖工作区的完整源码快照，包含现存本地修改及各自许可证。上游仍由 GitCode 独立维护；父仓库保存当前文件内容，不保存这些嵌套仓库的 Git 元数据。

| 本地目录 | 上游仓库 |
|---|---|
| `.cannbot/dependencies/ops-direct-invoke/asc-devkit/` | `https://gitcode.com/cann/asc-devkit.git` |
| `.cannbot/dependencies/ops-direct-invoke/ops-tensor/` | `https://gitcode.com/cann/ops-tensor.git` |
| `.cannbot/dependencies/ops-direct-invoke/ops-tensor/include/tensor_api/` | `ops-tensor` 声明的 `asc-devkit` 子模块 |

纳入时的来源版本为：`asc-devkit` `cbe844c3f49a8f586bdf5f543b1a2cd858cc437c`（1 个本地提交，另含 2,711 个已跟踪文件的工作区改动）；`ops-tensor` `584f3f7b2870c13a298aaeb1ac903821116b59f8`（2 个本地提交）；`tensor_api` `ab8ad3aa1aacd1581ec7d6c54ceca4f1990667dd`（子模块当前提交）。这些 SHA 记录快照的起点，父仓提交中的文件树包含该时点的本机工作区内容。父仓只保存来源 SHA 和当前文件快照，不保存独立仓库的 Git 提交链；本地目录删除后，GitHub 仍可取得源码和许可文件，独立仓库的提交元数据无法从父仓恢复。

归档中的提交队列运行时、浏览器用户数据、编译产物、测试输入输出和 profiling 原始缓存均不纳入共享资料。
