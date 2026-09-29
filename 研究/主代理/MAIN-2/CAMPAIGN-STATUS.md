# MAIN-2 Long-Run Orthogonal Explore Campaign

- 授权来源：C2C CONTROL / Planning Review Layer（本轮消息）
- 阶段：LONG-RUN / MULTI-AGENT / ORTHOGONAL-EXPLORE
- 记录日期：2026-09-29
- 记录者：MAIN-2

## 0. Canonical 同步

| 项 | 值 |
|---|---|
| 动作 | `git fetch origin` + `git merge origin/main` + `push` |
| MERGE_BASE | `141a6549` |
| MAIN2_HEAD | `7fd752f75fb38c3f95dd29374ea79cb8375e03c3` |
| REMOTE_HEAD | 同上（local == remote） |
| 冲突 | 无 Candidate/evidence 冲突；origin/main 仅改 `调度/主代理分工.md` |
| 禁止项遵守 | 未 rebase / 未 reset / 未 force push |

## 1. Active Lane 与 worktree

| Lane | Route | Branch | Worktree | Child Agent | Official Best | 状态 |
|---|---|---|---|---|---|---|
| M2-1 | REDUCE-HIER-X | `m2/reduce-hier` | `../cann-m2-reduce` | general-1 | V003=44.24 | research phase |
| M2-2 | VECTOR-MATH-X | `m2/vector-math` | `../cann-m2-vector` | general-2 | V001=44.22 | research phase |
| M2-3 | COEFF-LOCALITY-X | `m2/coeff-locality` | `../cann-m2-coeff` | general-3 | V001=44.16 | research phase |
| M2-4 | ASYNC-OVERLAP-CHAMPION-X | `m2/async-overlap-champion` | `../cann-m2-async` | general-4 | 无 | new route research |
| M2-5 | MULTIROW-DMA-CHAMPION-X | `m2/multirow-dma-champion` | `../cann-m2-multirow` | general-5 | 无 | new route research |

五条 branch 均已 `push -u origin`。每个 child 独立 worktree / branch / context，不共享可写 Candidate tree。

## 2. Evidence-only（非 Active）

| Route | 本轮用途 |
|---|---|
| DTYPE-SPECIAL-X | 只读证据 / reference |
| SCHED-CHAMPION-X | 调度证据 / donor reference |
| UB-LIVENESS-X | correctness-gap 证据；禁止 MAIN-2 建 V004 |

## 3. 预算与闭环规则

- 每 Lane soft = 6 Revision，有新信息/新收益可到 10。
- 连续 3 个独立 Revision 无收益且无新信息 → `LANE_NEEDS_PLANNING_REVIEW`（不是 PARK）。
- 当前 Revision 未闭环，不得创建下一 Revision。
- MAIN-2 可在 5 条 lane 内批准下一 OFAT hypothesis，但不写 Kernel。
- 不开第 6 条路线。
- PLANNING_DECISIONS_CHANGED = 0（本轮不改 Planning 生命周期决定）。
- NEW_ROUTES_OUTSIDE_APPROVED_5 = 0。

## 4. Online 条件式批准

- CONDITIONAL ONLINE APPROVAL：单 Revision 显著稳定超噪，或同一 Local Best chain 连续 2–3 次 LOCAL_ACCEPTED。
- 触发后 MAIN-2 做 Main Review → 统一 Judge Owner 提交。
- Route child 不提交 Online。

## 5. Git 纪律

- 一个技术改动 / server build / correctness / performance / Online / rollback 各自独立 commit+push。
- 禁止把代码修改、server result、rollback、下一技术假设塞进同一巨大 commit。
- LOCAL_REJECTED：保留失败源码与 evidence → commit/push → 显式 revert 到 LOCAL_BEST → push → 再开下一 Revision。
- 禁止 reset / 删 commit / 覆盖历史。
- Main-2 branch 只存控制、全局记录、evidence reference、Online package；child Kernel 留在各自 branch。

## 6. 共享总账更新权

仅 MAIN-2 单线程更新（且只改自己 Route 行）：

- `技术路线/全版本记录.tsv`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `调度/本地线上校准.tsv`
- `研究/主代理/MAIN-2/本地评分器校准.md`

## 7. 进度日志

### 2026-09-29 · campaign start

- canonical merge 完成，HEAD `7fd752f7`。
- 5 worktree 创建并 push。
- 5 child 已 spawn（general-1..5），第一阶段任务均为只读研究 + hypothesis package handoff，等待 MAIN-2 批准后才写 Kernel。

### 2026-09-28/29 · first implementation wave

| Lane | Revs closed | 最佳 | 状态 |
|---|---|---|---|
| M2-1 REDUCE-HIER-X | V004 LOCAL_REJECTED, V005 LOCAL_REJECTED | Official 44.24 (V003) | **LANE_NEEDS_PLANNING_REVIEW**（五变体全伪） |
| M2-2 VECTOR-MATH-X | V003 实现中（SEQ-FUSE-2） | Official 44.22 (V001) | active |
| M2-3 COEFF-LOCALITY-X | V003 LOCAL_REJECTED, V004 NEEDS_ONE_MORE_LOCAL | Official 44.16 (V001) | **LANE_NEEDS_PLANNING_REVIEW**（四维探尽） |
| M2-4 ASYNC-OVERLAP-CHAMPION-X | V001 LOCAL_ACCEPTED→Official 44.17 REJECT; V002–V004 mixed | LOCAL_BEST=V001 | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-5 MULTIROW-DMA-CHAMPION-X | V001 LOCAL_REJECTED, V002 NEEDS_ONE_MORE_LOCAL 终局 | none | **LANE_NEEDS_PLANNING_REVIEW**（等 C2 裁定） |

Online：ASYNC V001 一次正式提交（6abae4a0，Pass 15/15，44.17，REJECT vs 45.16）。

关键发现：
- 归约轴五变体全伪；父版 ReduceSum 接近最优。
- param MTE2 时序在常量 tileElems 下证伪。
- 命令计数与指令形态均非 DMA 主导成本。
- NarrowMid 调度类改动天花板（每行 squareSum 读取硬下限）。
- **测量装置 candidate 侧偏置 ~2.8%**；p10 在带载窗口对小信号乐观偏置。
- vcadd mode=0 输出粒度为每 64 FP32 一个和。
- DataCopyParams.blockLen 为 32B 单位。

Planning 待裁：C2 三选项、UB-GAP-CLUE、四条 lane 生命周期、W3 vs STORE-EPILOGUE 划界、null-binary 对照授权、Official case shape map。
