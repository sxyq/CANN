# MAIN-2 Wave-2 Launch Log

## 2026-09-30

| 次序 | 路线 | Agent ID / 结果 | 记录 |
|---|---|---|---|
| 历史派发 | M2-1 | `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` | 曾见 `pending_init`，后见 `running`，但未见 bootstrap/research 活动；另一状态入口返回 `not_found`。保留该 ID 于 lineage。 |
| 当前状态读取 | M2-1 | 同上 | Agent Center 显示 `Working 0`；按完整 ID 搜索为 `No matching tasks`。结合此前 `not_found`，记为 `LOST/UNRECOVERABLE`。 |
| Replacement | M2-1 | 未创建 | 当前工具清单没有 `multi_agent_v1`；CLI 没有该接口的派发命令。无 429。 |
| Batch 1 第二路线 | M2-2 | 未创建 | 等待 M2-1 replacement 确认 running 且进入 Track-B；本轮未派发。 |
| Batch 2 / Batch 3 | M2-3 / M2-4 / M2-5 | 未创建 | 前序批次未确认，未派发。 |

### Worktree 留存状态

| 路线 | 分支 | HEAD | 状态 |
|---|---|---|---|
| M2-1 | `w2/m2/interpass` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | 存在；干净；未改动 |
| M2-2 | `w2/m2/crossrow` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | 存在；干净；未改动 |

无 Agent 被报告为 active。未收到 429。当前停止点为 v1 派发能力不可用，等待 Main 提供可用的同一派发接口后继续。
