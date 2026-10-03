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
| M2-2 VECTOR-MATH-X | V001–V003 + S3 probe 已收口 | Official 44.22 (V001) | **LANE_NEEDS_PLANNING_REVIEW**（SEQ-FUSE-2 轴关闭） |
| M2-3 COEFF-LOCALITY-X | V003 LOCAL_REJECTED, V004 NEEDS_ONE_MORE_LOCAL | Official 44.16 (V001) | **LANE_NEEDS_PLANNING_REVIEW**（四维探尽） |
| M2-4 ASYNC-OVERLAP-CHAMPION-X | V001 LOCAL_ACCEPTED→Official 44.17 REJECT; V002–V004 mixed | LOCAL_BEST=V001 | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-5 MULTIROW-DMA-CHAMPION-X | V001 LOCAL_REJECTED, V002 NEEDS_ONE_MORE_LOCAL 终局 | none | **LANE_NEEDS_PLANNING_REVIEW**（等 C2 裁定） |

Online：ASYNC V001 以及 Main-1 的 R31B V017、R31A V028、STORE V003、EPI V002 均已完成正式提交；五条均 15/15 PASS，但均低于各自提交参照。

关键发现：
- 归约轴五变体全伪；父版 ReduceSum 接近最优。
- param MTE2 时序在常量 tileElems 下证伪。
- 命令计数与指令形态均非 DMA 主导成本。
- NarrowMid 调度类改动天花板（每行 squareSum 读取硬下限）。
- **测量装置 candidate 侧偏置 ~2.8%**；p10 在带载窗口对小信号乐观偏置。
- vcadd mode=0 输出粒度为每 64 FP32 一个和。
- DataCopyParams.blockLen 为 32B 单位。

Planning 待裁：C2 三选项、UB-GAP-CLUE、四条 lane 生命周期、W3 vs STORE-EPILOGUE 划界、null-binary 对照授权、Official case shape map。

---

## 8. 最终回执（2026-09-28/29 campaign wave-1）

### HEAD

| 项 | 值 |
|---|---|
| MAIN2_HEAD | `2da5620a88ab0827b8d2be839f0c60264a1d92bf` |
| REMOTE_HEAD | 同上（local == remote） |

### 五条 Lane

| Lane | closed | accepted | rejected | corr-fail | server best | online | official best | next | lane state |
|---|---|---|---|---|---|---|---|---|---|
| M2-1 REDUCE-HIER-X | 5 (V001–V005) | 0 | 2 (V002,V004,V005) | 0 | Official 44.24 V003 | 0 new | 44.24 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-2 VECTOR-MATH-X | 3 (V001–V003)+S3 | 0 (V001 batched hist.) | 0 | 0 | Official 44.22 V001 | 0 new | 44.22 | Planning | **LANE_NEEDS_PLANNING_REVIEW**（SEQ-FUSE-2 轴关闭） |
| M2-3 COEFF-LOCALITY-X | 4 (V001–V004) | 0 | 2 (V001,V003) | 0 | Official 44.16 V001 | 0 new | 44.16 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-4 ASYNC-OVERLAP-CHAMPION-X | 4 (V001–V004) | 1 (V001) | 0 | 0 | LOCAL_BEST V001 | 1 (44.17 REJECT) | 44.17 | Planning | **LANE_NEEDS_PLANNING_REVIEW** |
| M2-5 MULTIROW-DMA-CHAMPION-X | 2 (V001–V002) | 0 | 1 (V001) | 0 | none | 0 | none | Planning（C2） | **LANE_NEEDS_PLANNING_REVIEW** |

### SERVER

| 项 | 值 |
|---|---|
| max compile concurrency | 5 lanes 并行（每 lane 串行） |
| max jobs/card | ≤1 性能任务/卡（正式测时） |
| HBM block | 编译按 FREE_HBM≥100MB；未因 AICore/VLLM/d7 停编译 |
| performance runs | REDUCE V004/V005、VECTOR V003+S3、COEFF V003/V004、ASYNC V001–V004、MULTIROW V001/V002 均完成配对或记 BLOCKED |

### GIT

| 项 | 值 |
|---|---|
| main2 commit count | 22（自 BASE 141a6549） |
| lane push count | reduce 19 / vector 10 / coeff 11 / async 27 / multirow 21 |
| revert count | 2 显式回退（REDUCE V004、V005 → FROZEN）；MULTIROW V001 回退 |

### CALIBRATION

| 项 | 值 |
|---|---|
| new online | 5（ASYNC V001 + Main-1 四条；均 15/15 PASS） |
| false positive | 5（五个重点样本均为 Local 正向、Official 下降） |
| false negative | 0 新增（COEFF V001 历史 FN 保留） |
| evaluator proposal | SERVER_EVALUATOR_CANDIDATE_V1 草案（SHADOW_ONLY，未启用） |

### 约束遵守

```text
PLANNING_DECISIONS_CHANGED = 0
NEW_ROUTES_OUTSIDE_APPROVED_5 = 0
```

### 关键可复用事实

1. 归约轴五变体全伪；父版 `ReduceSum` 接近最优。
2. V/S handoff 成本 0.049 µs/row；纯 V 链更贵 +0.012。
3. param MTE2 时序非 large-D 瓶颈（常量 tileElems 证伪）。
4. DMA 命令计数与指令形态均非主导；流水重叠才是。
5. NarrowMid 每行 squareSum 读取是硬下限。
6. **测量装置 candidate 侧偏置 ~2.8%**；p10 带载乐观偏置。
7. S1 形状加长（9–15µs 带）可绕开短 kernel same-binary 门槛。
8. vcadd mode=0 每 64 FP32 一和；DataCopyParams.blockLen=32B。
9. FullCache invRms 占用 xBuf_ 限制 prologue 窗口。
10. UB-GAP-CLUE：FROZEN parent FP32 wide 非确定（bad 随 run 变）。

### STOP 条件

五条 lane 全部 `等待 Planning`，wave-1 收口。等待 Planning 裁定生命周期、C2 取舍、UB-GAP、W3 划界、null-binary 授权与 case map。

---

## 9. Wave-2 Next Score Wave — 2026-10-03

```text
OFFICIAL_CHAMPION=R31B V011
OFFICIAL_SCORE=45.16
DIRECT_ONLINE_SUBMISSION=0
```

### Current local engineering scores

| Route | Revision | Local score | Quality / state |
|---|---|---:|---|
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V001 / H3 | -24.193320% | POOR |
| HOTLOOP-ADDR-HOIST-CHAMPION-X | V002 / H1 | +0.954352% | POOR / LOCAL_NEUTRAL; no V003 |
| UB-LIVENESS-X | V001 | -20.878573% | POOR / engineering-reference only; no V002 |
| HOTLOOP-BRANCH-HOIST-CHAMPION-X | V002 / HBH-09 | -0.048802% | POOR / Planning review; no V003 |
| REDUCE-FINALIZE-HANDOFF-CHAMPION-X | V001 / RFH-1 | +0.553205% | POOR / LOCAL_NEUTRAL; no V002 |
| TILECOUNT-STATIC-UNROLL-CHAMPION-X | none | NO SCORE | H1/H2/H3 final gate FAIL; MAIN_SELECTED=NONE |

### New events

- REDUCE V001 exact R31B V011 sibling baseline and RFH-1 declaration:
  `w2/m2/reduce-finalize` commit `c7bad949`; parent/submission SHA is
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
  Candidate source `83d569d2…f78338df3` is commit `15803855`. Build PASS is
  committed at `bc558fa3`; correctness PASS at `3052b573` (BF16 [1,32768],
  FP16/BF16 [1,16384], BF16 [1,4096] control; all bad=0). Device-event runner
  probes are built (`a875d789`). Three interleaved BF16 [1,32768] device-event
  pairs produced Parent average 20.486667 us and Candidate average 20.600000
  us (+0.553205%, Candidate slower); pair directions were mixed. Parent
  same-binary MAD/median=18.56%, so quality is POOR and verdict is
  LOCAL_NEUTRAL, engineering-score only; no Local Best advance. Raw/performance
  evidence commit `e9cc2dfb`, score commit `0c07dcd7`. d5 lease
  `M2-REDUCE-V001-D5-20261003T182337Z` was released (control commit
  `e81b82b1`). No Online and no V002.
- TILECOUNT final gate is committed in `w2/m2/tilecount-unroll` at
  `e36220ed`. Exact Parent source and compile artifact identity were found,
  but the available HiIPU device binary could not be disassembled to verify
  loop backedges/index work. All three hypotheses fail the required codegen
  gate; this is `UNVERIFIED`, not evidence that the compiler already unrolled
  them. No V001 was created.

```text
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```
