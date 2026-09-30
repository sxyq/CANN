# MAIN-2 Wave-2 Campaign Status

- 更新时间：2026-09-30
- 控制分支：`main2/orthogonal-explore`
- 控制工作树：`/Users/sunyiyang/Desktop/Project/cann-main2`
- 范围：仅五条已批准路线的 Track-B handoff；本轮不选路线假设。

## 当前状态

| 路线 | 分支 / 工作树 | 当前 Agent | 状态 |
|---|---|---|---|
| M2-1 INTERPASS-PIPELINE-CHAMPION-X | `w2/m2/interpass` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-interpass` | 首次 ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` | `LOST/UNRECOVERABLE`；replacement 未派发 |
| M2-2 CROSSROW-PIPELINE-CHAMPION-X | `w2/m2/crossrow` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-crossrow` | 无 | worktree 已有；尚未派发 |
| M2-3 UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` | 无 | 尚未建立；未派发 |
| M2-4 PARAM-RESIDENCY-CHAMPION-X | `w2/m2/param-residency` | 无 | 尚未建立；未派发 |
| M2-5 ROW-OCCUPANCY-CHAMPION-X | `w2/m2/row-occupancy` | 无 | 尚未建立；未派发 |

- 当前确认运行的 Route Agent：0。
- M2-1 旧 ID 的本轮 Agent Center 状态：`Working 0`，按完整 ID 搜索结果为 `No matching tasks`；先前状态读取也返回 `not_found`。
- 本轮未收到 429。
- `MAIN_SELECTED=NONE`
- `SERVER_BUILD=0`，`CORRECTNESS_RUN=0`，`PERFORMANCE_RUN=0`，`ONLINE_SUBMISSION=0`
- 未改动 Candidate、kernel、build、runtime 或共享路线记录。

## 停止位置

当前工具清单未提供 `multi_agent_v1`；本机 `codex agents` 可浏览 Agent Center，但没有 v1 Route Agent 派发命令。为避免用普通任务或线程代替 Route Agent，本轮未创建 replacement，也未启动 M2-2。两个现有 route worktree 和 branch 均保留且状态干净。

恢复派发能力后，先在 M2-1 原 worktree 与 branch 启动且仅启动一个 replacement；确认其状态为 running 并有 Track-B bootstrap/research 活动后，再启动 M2-2。其余路线继续依原定 2+2+1 顺序。遇到 429 即停止后续派发。
