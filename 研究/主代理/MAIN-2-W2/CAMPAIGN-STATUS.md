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
| M2-3 UB-LIFETIME-SAFE-CHAMPION-X | `w2/m2/ub-lifetime-safe` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-ub` | `01a0f174-2b77-7292-ac20-ce975a49cc04` | `RUNTIME=RUNNING`; Track-B 已开始，尚无 handoff |
| M2-4 PARAM-RESIDENCY-CHAMPION-X | `w2/m2/param-residency` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-param` | `01a0f174-2bea-78b0-b6b2-c4070898f5d3` | Track-B handoff complete; commit `ff0e424bf1dde7c5a1fc9e4386e8178ed36e5a5d`；条件化的不同想法 1 项后假设池耗尽；`MAIN_SELECTED=NONE`；无构建、测试、server 或 Online 操作 |
| M2-5 ROW-OCCUPANCY-CHAMPION-X | `w2/m2/row-occupancy` / `/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy` | `01a0f1a0-6a4e-7e40-b0b2-966e247d59c8` | `RUNTIME=RUNNING`; 私有工作树中已有未提交 handoff（未查看）；Track-B entry 尚未确认；初始工作树由 `origin/main` 的 `ed860e392d7604694ac6664da60aff1fc1f4c04f` 建立且当时干净 |

- 当前 runtime 已确认为 running 的 Route Agent：2（M2-3、M2-5）。M2-3 已进入 Track-B，尚未 handoff；M2-4 handoff 已完成（当前 runtime 未提供）；M2-5 有未提交 handoff，Track-B entry 尚未确认。
- 已确认完成 Track-B handoff：M2-1、M2-2；两者均报告假设池耗尽、`MAIN_SELECTED=NONE`，且没有测试或 429。
- M2-1 首次 Agent ID `01a0f0ef-13f0-72e3-b66f-4c8ae589d4e4` 为 `LOST/UNRECOVERABLE`；replacement 为 `01a0f11a-bf80-7c33-8332-5a939dd96c15`，handoff commit `ca6ee4b77469de3c84c2b978a648775ee3ddf86d`。
- M2-2 handoff commit：`963507930c4752573a7211ca2e54429d6cba49a3`。
- M2-3 曾收到 Main 的非阻塞暂停请求，后以原 Agent ID 恢复；当前 runtime running，Track-B 已开始，尚无 handoff。
- M2-4 曾收到 Main 的非阻塞暂停请求，后以原 Agent ID 恢复；Track-B handoff commit 为 `ff0e424bf1dde7c5a1fc9e4386e8178ed36e5a5d`，远端分支引用已核实；假设池在提交一项条件化的不同想法后耗尽。无构建、测试、server 或 Online 操作；当前 runtime 未提供。
- M2-5 worktree `/Users/sunyiyang/Desktop/Project/cann-w2-m2-occupancy`、branch `w2/m2/row-occupancy` 创建前确认路径与分支无冲突，从 `origin/main` 的 `ed860e392d7604694ac6664da60aff1fc1f4c04f` 干净建立；Agent `01a0f1a0-6a4e-7e40-b0b2-966e247d59c8` runtime running，Track-B entry 尚未确认。Main 报告私有工作树已有未提交 handoff，本记录未查看该工作树或文件。
- M2-3 / M2-4 / M2-5 均未报告 429。
- `MAIN_SELECTED=NONE`
- 测试事实：M2-1/M2-2 未测试；M2-4 无构建、测试、server 或 Online 操作；不推断 M2-3/M2-5 的测试状态。
- 未改动 Candidate、kernel、build、runtime 或共享路线记录。

## 停止位置

M2-3 runtime running、Track-B 已开始、handoff 尚未完成；M2-4 handoff 已完成，当前 runtime 未提供；M2-5 runtime running，Track-B entry 尚待确认。三条路线均未报告 429。`MAIN_SELECTED=NONE`。
