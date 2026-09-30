# MAIN-2 Wave-2 Campaign Status

- 更新时间：2026-09-30
- 控制分支：`main2/orthogonal-explore`
- 控制工作树：`/Users/sunyiyang/Desktop/Project/cann-main2`
- 范围：仅五条已批准路线的 Track-B handoff；本轮不选路线假设。

## 当前状态

| 路线 | 分支 / 工作树 | 当前 Agent | 状态 |
|---|---|---|---|
| M2-1 INTERPASS-PIPELINE-CHAMPION-X | `w2/m2/interpass` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-interpass` | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15`; prior ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` retained as LOST/UNRECOVERABLE | `RUNTIME=RUNNING`; agent reported Track-B research entered |
| M2-2 CROSSROW-PIPELINE-CHAMPION-X | `w2/m2/crossrow` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-crossrow` | `01a0f129-4b39-73c0-bc7f-7d4e2fafb611` | handoff complete; commit `963507930c4752573a7211ca2e54429d6cba49a3` pushed clean; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`; `MAIN_SELECTED=NONE` |
| M2-3 UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-ub` | 未启动 | worktree 已建立且干净，HEAD=`ed860e392d7604694ac6664da60aff1fc1f4c04f`；Agent 未启动，当前接口未提供 `multi_agent_v1__spawn_agent` |
| M2-4 PARAM-RESIDENCY-CHAMPION-X | `w2/m2/param-residency` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-param` | 未启动 | worktree 已建立且干净，HEAD=`ed860e392d7604694ac6664da60aff1fc1f4c04f`；Agent 未启动，当前接口未提供 `multi_agent_v1__spawn_agent` |
| M2-5 ROW-OCCUPANCY-CHAMPION-X | `w2/m2/row-occupancy` | 无 | worktree 尚未建立；未派发 |

- Batch 1 已达成：M2-1、M2-2 均有活跃 Agent，且均已进入 Track-B；本批次 `429=0`。
- M2-2 已交付 Track-B handoff：`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`、`MAIN_SELECTED=NONE`；commit `963507930c4752573a7211ca2e54429d6cba49a3` 已推送。
- M2-1 首次 Agent ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 `LOST/UNRECOVERABLE`；replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15` 当前 runtime running，并报告 Track-B research entered。
- Batch 2（M2-3 / M2-4）worktree 均从 `origin/main` HEAD `ed860e392d7604694ac6664da60aff1fc1f4c04f` 建立并保持干净；Agent 未启动。当前工具接口没有 `multi_agent_v1__spawn_agent`，未尝试 CLI 或线程替代派发。
- `429=0`；M2-5 worktree 与 Agent 均未建立。
- `MAIN_SELECTED=NONE`
- `SERVER_BUILD=0`，`CORRECTNESS_RUN=0`，`PERFORMANCE_RUN=0`，`ONLINE_SUBMISSION=0`
- 未改动 Candidate、kernel、build、runtime 或共享路线记录。

## 停止位置

M2-3 / M2-4 worktree 已准备好，Route Agent 尚未启动；当前等待运行时提供 `multi_agent_v1__spawn_agent`，不使用 CLI 或线程替代。Batch 3（M2-5）未开始。`MAIN_SELECTED=NONE`；在 v1 派发能力可用并确认 Batch 2 两条 Route Agent 的 runtime、Track-B 进入状态及 429 情况前，不启动 Batch 3。
