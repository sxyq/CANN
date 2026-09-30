# MAIN-2 Wave-2 Launch Log

## 2026-09-30

### 首次派发状态快照

| 次序 | 路线 | Agent ID / 结果 | 记录 |
|---|---|---|---|
| 历史派发 | M2-1 | `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` | 曾见 `pending_init`，后见 `running`，但未见 bootstrap/research 活动；另一状态入口返回 `not_found`。保留该 ID 于 lineage。 |
| 当前状态读取 | M2-1 | 同上 | Agent Center 显示 `Working 0`；按完整 ID 搜索为 `No matching tasks`。结合此前 `not_found`，记为 `LOST/UNRECOVERABLE`。 |
| Replacement | M2-1 | 未创建 | 当前工具清单没有 `multi_agent_v1`；CLI 没有该接口的派发命令。无 429。 |
| Batch 1 第二路线 | M2-2 | 未创建 | 等待 M2-1 replacement 确认 running 且进入 Track-B；本轮未派发。 |
| Batch 2 / Batch 3 | M2-3 / M2-4 / M2-5 | 未创建 | 前序批次未确认，未派发。 |

以上记录是首次派发状态快照，后续事实见下节。

### Main-issued 状态快照（早于 Batch 1 完成事实）

| 路线 | 当前 Agent / runtime | Track-B 状态 |
|---|---|---|
| M2-1 | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15`，`RUNNING`；旧 ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 保留为 `not_found / LOST` | 已报告进入研究，但 bootstrap 未完成；Main 已续派继续，进度确认待回报 |
| M2-2 | `01a0f129-4b39-73c0-bc7f-7d4e2fafb611`，`RUNNING` | 等待 bootstrap / Track-B 进度回报 |
| M2-3 / M2-4 / M2-5 | 未创建 worktree，未派发 Agent | 未启动 |

Batch 1 双路线确认尚未完成，Batch 2 / Batch 3 未启动。未报告 429。`MAIN_SELECTED=NONE`。

### Batch 1 Main-issued 完成事实

| 路线 | 当前事实 | 记录 |
|---|---|---|
| M2-1 | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15` runtime running；旧 ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 LOST/UNRECOVERABLE | replacement 报告 Track-B research entered |
| M2-2 | Agent `01a0f129-4b39-73c0-bc7f-7d4e2fafb611` | handoff commit `963507930c4752573a7211ca2e54429d6cba49a3` pushed clean；`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`；`MAIN_SELECTED=NONE` |

Batch 1 已达成：两条路线均有活跃 Agent 且已进入 Track-B；未报告 429（`429=0`）。

### Worktree 派发起点

| 路线 | 分支 | HEAD | 状态 |
|---|---|---|---|
| M2-1 | `w2/m2/interpass` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-interpass` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | Main 报告：复用既有 worktree，派发起点干净 |
| M2-2 | `w2/m2/crossrow` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-crossrow` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | Main 报告：复用既有 worktree，派发起点干净 |

本控制 Agent 未读取 Route worktree 文件。仅在收到 Main 后续事实后再更新本日志。

### Batch 2 工作树准备快照（Route Agent spawn 前）

Canonical 根仓库先执行 `git fetch origin`。目标路径与本地/远端分支在创建前均不存在，也没有对应 worktree 登记；因此按最新 `origin/main` 建立：

| 路线 | 工作树 | 分支 | 起始 HEAD | 创建后状态 |
|---|---|---|---|---|
| M2-3 UB-LIFETIME-SAFE-CHAMPION-X | `/Users/sunyiyang/Desktop/Project/cann-w2-m2-ub` | `w2/m2/ub-lifetime-safe` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | 干净 |
| M2-4 PARAM-RESIDENCY-CHAMPION-X | `/Users/sunyiyang/Desktop/Project/cann-w2-m2-param` | `w2/m2/param-residency` | `ed860e392d7604694ac6664da60aff1fc1f4c04f` | 干净 |

该快照记录工作树刚建立时的状态：当时 M2-3 / M2-4 尚无 Agent ID，runtime 未启动，Track-B 尚未确认。后续 Main-issued 状态见下节。

### Main-issued 状态快照（先于本节更新）

| 路线 | Agent ID / runtime | Track-B 与 handoff | 其他事实 |
|---|---|---|---|
| M2-1 | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15`；旧 ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 LOST/UNRECOVERABLE | Track-B handoff complete；`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`；`MAIN_SELECTED=NONE` | commit `ca6ee4b77469de3c84c2b978a648775ee3ddf86d` pushed clean；no tests/429 |
| M2-2 | `01a0f129-4b39-73c0-bc7f-7d4e2fafb611` | Track-B handoff complete；`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`；`MAIN_SELECTED=NONE` | commit `963507930c4752573a7211ca2e54429d6cba49a3` pushed clean；no tests/429 |
| M2-3 | `01a0f174-2b77-7292-ac20-ce975a49cc04`；runtime `RUNNING` | Track-B entry not yet confirmed | Existing clean worktree / branch from `origin/main` |
| M2-4 | `01a0f174-2bea-78b0-b6b2-c4070898f5d3`；runtime `RUNNING` | Track-B entry not yet confirmed | Existing clean worktree / branch from `origin/main` |
| M2-5 | 未启动 | NOT STARTED | worktree 未建立，Agent 未派发 |

当前 running 的 Route Agent 为 2（M2-3、M2-4）；已完成 Track-B handoff 的路线为 M2-1、M2-2。M2-3 / M2-4 运行中不代表 Track-B 已确认。当前 Main-issued 批次事实 `429=0`。`MAIN_SELECTED=NONE`。

### Main-issued 最新进展

| 路线 | Agent ID / runtime | Track-B 状态 | 其他事实 |
|---|---|---|---|
| M2-1 | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15`；旧 ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 LOST/UNRECOVERABLE | handoff complete；`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`；`MAIN_SELECTED=NONE` | commit `ca6ee4b77469de3c84c2b978a648775ee3ddf86d` pushed clean；no tests/429 |
| M2-2 | `01a0f129-4b39-73c0-bc7f-7d4e2fafb611` | handoff complete；`ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`；`MAIN_SELECTED=NONE` | commit `963507930c4752573a7211ca2e54429d6cba49a3` pushed clean；no tests/429 |
| M2-3 | `01a0f174-2b77-7292-ac20-ce975a49cc04`；paused non-blockingly, then resumed on same ID; current runtime `RUNNING` | direct progress report confirms Track-B started; bootstrap incomplete | worktree remains clean from `origin/main`; no 429 |
| M2-4 | `01a0f174-2bea-78b0-b6b2-c4070898f5d3`；paused non-blockingly, then resumed on same ID; current runtime `RUNNING` | direct progress report confirms Track-B started; bootstrap incomplete | worktree remains clean from `origin/main`; no 429 |
| M2-5 | `01a0f1a0-6a4e-7e40-b0b2-966e247d59c8`；runtime `RUNNING` | Track-B entry not yet confirmed | worktree `/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy`; branch `w2/m2/row-occupancy`; created from `origin/main` at `ed860e392d7604694ac6664da60aff1fc1f4c04f`; no 429 |

Batch 2 的 M2-3/M2-4 均已确认进入 Track-B；Batch 3 的 M2-5 已派发且 runtime running，Track-B entry 待确认。Main-issued facts report no 429. `MAIN_SELECTED=NONE`。M2-1/M2-2 未运行测试；不推断 M2-3/M2-4/M2-5 测试状态。本控制 Agent 未查看任何 Route worktree。
