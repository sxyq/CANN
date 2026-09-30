# MAIN-2 Wave-2 Campaign Status

- 更新时间：2026-09-30
- 控制分支：`main2/orthogonal-explore`
- 控制工作树：`/Users/sunyiyang/Desktop/Project/cann-main2`
- 范围：仅五条已批准路线的 Track-B handoff；本轮不选路线假设。

## 当前状态

| 路线 | 分支 / 工作树 | 当前 Agent | 状态 |
|---|---|---|---|
| M2-1 INTERPASS-PIPELINE-CHAMPION-X | `w2/m2/interpass` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-interpass` | replacement `01a0f11a-bf80-7c33-8332-5a939dd96c15`; prior ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` retained as LOST/UNRECOVERABLE | Track-B handoff complete; commit `ca6ee4b77469de3c84c2b978a648775ee3ddf86d` pushed clean; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`; `MAIN_SELECTED=NONE`; no tests/429 reported |
| M2-2 CROSSROW-PIPELINE-CHAMPION-X | `w2/m2/crossrow` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-crossrow` | `01a0f129-4b39-73c0-bc7f-7d4e2fafb611` | Track-B handoff complete; commit `963507930c4752573a7211ca2e54429d6cba49a3` pushed clean; `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES`; `MAIN_SELECTED=NONE`; no tests/429 reported |
| M2-3 UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-ub` | `01a0f174-2b77-7292-ac20-ce975a49cc04` | `RUNTIME=RUNNING`（同 ID 恢复）；agent 已报告进入 Track-B，bootstrap 尚未完成；工作树干净，起于 `origin/main` |
| M2-4 PARAM-RESIDENCY-CHAMPION-X | `w2/m2/param-residency` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-param` | `01a0f174-2bea-78b0-b6b2-c4070898f5d3` | `RUNTIME=RUNNING`（同 ID 恢复）；agent 已报告进入 Track-B，bootstrap 尚未完成；工作树干净，起于 `origin/main` |
| M2-5 ROW-OCCUPANCY-CHAMPION-X | `w2/m2/row-occupancy` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy` | `01a0f1a0-6a4e-7e40-b0b2-966e247d59c8` | `RUNTIME=RUNNING`；Track-B entry 尚未确认；工作树与分支从 `origin/main` 的 `ed860e392d7604694ac6664da60aff1fc1f4c04f` 建立 |

- 当前 runtime 为 running 的 Route Agent：3（M2-3、M2-4、M2-5）。M2-3/M2-4 已报告进入 Track-B，但 bootstrap 尚未完成；M2-5 的 Track-B entry 尚未确认。
- 已确认完成 Track-B handoff：M2-1、M2-2；两者均报告假设池耗尽、`MAIN_SELECTED=NONE`，且没有测试或 429。
- M2-1 首次 Agent ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 `LOST/UNRECOVERABLE`；replacement 为 `01a0f11a-bf80-7c33-8332-5a939dd96c15`，handoff commit `ca6ee4b77469de3c84c2b978a648775ee3ddf86d`。
- M2-2 handoff commit：`963507930c4752573a7211ca2e54429d6cba49a3`。
- M2-3/M2-4 曾收到 Main 的非阻塞暂停请求，之后以原 Agent ID 恢复；两者目前均为 `RUNTIME=RUNNING`。Batch 2 未报告 429。
- M2-5 worktree `/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy`、branch `w2/m2/row-occupancy` 已建立，Agent `01a0f1a0-6a4e-7e40-b0b2-966e247d59c8` runtime running；Track-B 进入情况待回报。未报告 429。
- Batch 3 已启动；无 Route Agent 报告 429。
- `MAIN_SELECTED=NONE`
- 当前测试事实仅确认 M2-1 / M2-2 未测试；不推断 M2-3 / M2-4 的测试状态。
- 未改动 Candidate、kernel、build、runtime 或共享路线记录。

## 停止位置

M2-3/M2-4 已报告进入 Track-B，bootstrap 尚未完成；两者沿用原 Agent ID 恢复后均为 running。M2-5 Agent 为 running，Track-B 进入尚待确认。`MAIN_SELECTED=NONE`。
