# MAIN-2 COMPLIANCE-REVIEW

- 记录日期：2026-09-29
- 阶段：DUAL MAIN COMPLIANCE + ONLINE CLOSURE
- 范围：MAIN-2 名下 lane（REDUCE-HIER-X / VECTOR-MATH-X / COEFF-LOCALITY-X / ASYNC-OVERLAP-CHAMPION-X / MULTIROW-DMA-CHAMPION-X）及 evidence-only 路线
- 原则：不篡改历史；未做过的 revert 不伪造；从当前时刻起按新纪律执行

---

## 1. 六条偏差逐条检查

### 1.1 LOCAL_REJECTED 后是否缺少 explicit revert commit

| Route | Rev | verdict | 期望回退 | 实际行为 | 判定 |
|---|---|---|---|---|---|
| REDUCE-HIER-X | V004 | LOCAL_REJECTED | restore FROZEN_R31B_V011 | commit `79f04197` 显式回退 | 合规 |
| REDUCE-HIER-X | V005 | LOCAL_REJECTED | restore FROZEN_R31B_V011 | commit `ae187c0d` 显式回退 | 合规 |
| MULTIROW-DMA | V001 | LOCAL_REJECTED | restore FROZEN_R31B_V011 | V002 声明父版=FROZEN，workspace 现为 V002 SHA；**无独立命名 revert commit** | **PROCESS_DEVIATION = YES（历史）** |
| COEFF-LOCALITY-X | V001 | LOCAL_REJECTED | LOCAL_BEST=NONE，父版本就是 FROZEN | V002/V003 父版=FROZEN | 合规（无 LOCAL_BEST 可恢复） |
| COEFF-LOCALITY-X | V003 | LOCAL_REJECTED | 同上 | V004 父版=FROZEN | 合规 |
| VECTOR-MATH-X | V003 | NEEDS_ONE_MORE_LOCAL | 不适用（非 REJECTED） | Candidate 保留 | 合规 |
| ASYNC V002–V004 | — | NEEDS_ONE_MORE_LOCAL | 不适用 | Candidate 保留 | 合规 |

**REVISION / EXPECTED_ROLLBACK / ACTUAL_BEHAVIOR / WHY_HISTORICAL_REVERT_CANNOT_BE_RETROACTIVELY_CREATED / CURRENT_BRANCH_STATE**

```text
REVISION: MULTIROW-DMA-CHAMPION-X V001
EXPECTED_ROLLBACK: explicit revert/restore commit after LOCAL_REJECTED
ACTUAL_BEHAVIOR: 失败源码与 evidence 已保留（47e37ab7 / b5a04e99 / 23bb728c / bc4eb36c）；
                 下一 Revision V002 声明直接以 FROZEN_R31B_V011 为 DIRECT_PARENT，
                 workspace 现为 V002 SHA 2c23ce32…，未生成独立 revert commit
WHY_HISTORICAL_REVERT_CANNOT_BE_RETROACTIVELY_CREATED:
  当时未按现行三步纪律留下 revert 提交；现在补一个“当时的 revert”会伪造历史轨迹
CURRENT_BRANCH_STATE: m2/multirow-dma-champion，V002 Candidate=NEEDS_ONE_MORE_LOCAL 终局，
                      源码钉在 V002；V001 证据永久保留
PROCESS_DEVIATION = YES
```

### 1.2 child 在 Main approval 前创建性能 Revision

| 检查 | 结果 |
|---|---|
| 研究阶段 | 5 个 child 均先只读研究 + hypothesis package，STOP 等批准 |
| 实现阶段 | 批准后才写 Kernel / 创建 Rev 目录 |
| 越权 | **未发现** child 先写性能 Kernel 再追认 |

判定：**合规**（MAIN-2 campaign 流程满足 Track-B → Main review → MAIN_SELECTED → declaration → implementation）。

### 1.3 LOCAL_ACCEPTED / WORTHY 但没有 Online result

| Route | Rev | LOCAL | ONLINE_WORTHY | Online result | 判定 |
|---|---|---|---|---|---|
| ASYNC-OVERLAP-CHAMPION-X | V001 | LOCAL_ACCEPTED（仅一个合格计时形状） | YES（辅助形状方向证据） | **44.17 Pass 15/15 REJECT** | ONLINE_CLOSED |
| SCHED-CHAMPION-X | V002 | LOCAL_ACCEPTED | NO（历史 KEEP_ACCUMULATING） | 41.94 已有 | 历史已闭环 |
| STORE-EPILOGUE-X | V002 | LOCAL_ACCEPTED | FUTURE_CANDIDATE | 45.07 已有（MAIN-1 轴） | 非 MAIN-2 本轮 |

判定：**无缺口**。本轮唯一新 LOCAL_ACCEPTED（ASYNC V001）已完成 Online。

### 1.4 Online package 已准备但 Judge Owner 未执行

| Package | 状态 |
|---|---|
| ASYNC-OVERLAP-CHAMPION-X V001 | Judge 已执行（submission `6abae4a0694b590c3c51d6a3`） |
| 其他 MAIN-2 lane | 无待提交 package |

判定：**无缺口**。

### 1.5 Main-2 事实进入 canonical 的状态

**PROCESS_DEVIATION = YES（历史结构）**

- 截至本文件记录时，Main-2 wave-1 事实（含 ASYNC V001 result / ledger / calibration）已进入 `origin/main`。
- MAIN-1 EPI V002 待 Consolidation Owner 收入 canonical。
- 历史分支分叉与收口过程保留在 §7，不改变当前路线生命周期。

### 1.6 共享总账在 Main-1 / Main-2 两条 branch 分叉

**PROCESS_DEVIATION = YES（结构）**

- `技术路线/全版本记录.tsv`、`路线成绩表.tsv`、`调度/本地线上校准.tsv` 在 main1 / main2 各自有增量。
- 本轮 Canonical consolidation 只合并 **事实行**，不 merge child kernel。
- 不重排全表；只做最小追加/行级更新。

---

## 2. Git rollback 纪律（从现在强制）

### 2.1 新 Revision 三步（LOCAL_REJECTED 时）

```text
Step 1  commit: route(X): V00N hypothesis implementation   → push
Step 2  commit: evidence(X): V00N LOCAL_REJECTED           → push
Step 3  commit: revert(X): restore LOCAL_BEST after V00N rejection
        （git revert <candidate-code-commit> 或显式 restore）→ push
```

禁止：reset / checkout 覆盖不提交 / 直接复制旧源码无轨迹 / 删除失败 commit。

### 2.2 历史缺口

- MULTIROW V001：见 §1.1，不追溯伪造。
- 其余 MAIN-2 REJECTED 版本父版本就是 FROZEN 或已有显式回退。

### 2.3 NEW_REVERTS（本整改轮）

本整改轮不新建性能 Revision，故 **NEW_REVERTS = 0**。

---

## 3. 字段分离（LOCAL_BEST / SERVER_BEST / OFFICIAL_BEST）

检查原则：`-14% / -8% / -6%` 一类只能出现在 `LOCAL_DELTA` / `最佳Local表现`，不得写入 `OFFICIAL_BEST`。

| Route | LOCAL_BEST | SERVER_BEST | OFFICIAL_BEST | 字段分离 |
|---|---|---|---|---|
| REDUCE-HIER-X | V003 近中性（未推进为胜） | V003 | V003 = 44.24 | OK |
| VECTOR-MATH-X | V001 batched（registry） | V002 fail / V001 | V001 = 44.22 | OK |
| COEFF-LOCALITY-X | NONE | V001 | V001 = 44.16 | OK |
| ASYNC-OVERLAP-CHAMPION-X | V001 = 128×16384 FP16 −1.3..−2.2%（唯一合格计时形状） | V001 | V001 = 44.17 | OK（Local% 仅在 Local 字段） |
| MULTIROW-DMA-CHAMPION-X | NONE | NONE | NONE | OK |

判定：**当前记录字段分离合规**。后续写入继续强制。

---

### 3.1 ASYNC V001 Local 口径

- `LOCAL_VERDICT=LOCAL_ACCEPTED` 只依据 `128×16384 FP16` 这一合格计时形状。
- BF16、`96×16384 FP16` 等其他形状只作为辅助方向证据，不合并为额外的合格 Local verdict。
- `ONLINE_WORTHY=YES` 使用上述辅助方向证据作为送 Online 的理由；Online 返回 `44.17`、`15/15 PASS`、相对 `45.16` 为 `REJECT` 后，状态为 `ONLINE_CLOSED`。

---

## 4. Online gap 分类（MAIN-2 五条 lane）

```text
AS_OF=2026-09-29
SCOPE=MAIN-2 R2 五条 lane：REDUCE-HIER-X / VECTOR-MATH-X / COEFF-LOCALITY-X / ASYNC-OVERLAP-CHAMPION-X / MULTIROW-DMA-CHAMPION-X
```

| Route | Evidence Revision / probe | 分类 | 理由 | 动作 |
|---|---|---|---|---|
| REDUCE-HIER-X | V001–V005 | **D 机制未形成可用收益** | V002/V004/V005 为 `LOCAL_REJECTED`；V001/V003 未形成 `LOCAL_ACCEPTED`，没有可送 Online 的当前 Candidate | ONLINE_NOT_REQUIRED |
| VECTOR-MATH-X | V001–V003；S3-TAIL-PROBE | **D 机制未形成可用收益** | V002 的中等形状信号因 same-binary 不合格而不可用；V003 方向混合；S3 probe 显示每行节省为负（约 −0.012µs） | ONLINE_NOT_REQUIRED |
| COEFF-LOCALITY-X | V001–V004 | **D 机制未形成可用收益** | V001 回退；V002 为 `NO_UB_BUDGET`；V003 时序方向被否；V004 残差不可从测量偏差中分离，没有 `LOCAL_ACCEPTED` | ONLINE_NOT_REQUIRED |
| MULTIROW-DMA-CHAMPION-X | V001–V002 | **D + C** | V001 `LOCAL_REJECTED`；V002 terminal `NEEDS_ONE_MORE_LOCAL`，组合差异处于噪声内，没有 `LOCAL_ACCEPTED` | ONLINE_NOT_REQUIRED |
| ASYNC-OVERLAP-CHAMPION-X | V001 | **已闭环** | 一个合格计时形状加辅助方向证据触发 Online；Official `44.17`、`15/15 PASS`，低于 `45.16`，结果为 `REJECT` | ONLINE_CLOSED |

**CALIBRATION_SUBMISSION_RECOMMENDATION = NONE**

理由：本 wave 无「本地信号弱但校准价值极高且尚未有 Official」的候选。ASYNC 已提供 FP 样本；COEFF/REDUCE/VECTOR/MULTIROW 均为证伪或噪声，强行提交只消耗 Online 名额、不增加 evaluator 可分性。历史 FN 样本（COEFF V001）已在校准表。

```text
UNKNOWN_SCOPE=MAIN-2 R2 四条未闭环 lane
UNKNOWN_COUNT=0
```
历史总账中的其他 `UNKNOWN` 文本不纳入本表统计。

---

## 5. 本整改轮 MAIN-2 动作清单

1. 本文件（COMPLIANCE-REVIEW.md）
2. ASYNC-OVERLAP-CHAMPION-X V001 Online 闭环一致性核验
3. Online gap 分类（上表）
4. Canonical consolidation（Main-2 事实并入 `main`）
5. 不开 wave-2、不建新 Revision、不改 Planning 生命周期

---

## 6. PROCESS_DEVIATIONS 汇总

| 类型 | 数量 | 说明 |
|---|---|---|
| historical_missing_reverts | 1 | MULTIROW V001 |
| child_preapproval_violations | 0 | — |
| results_not_in_canonical | 0 | Main-2 wave-1 与 Main-1 四条 Online closure 事实均已进入 `origin/main` |
| shared_ledger_divergence | 1（结构） | main1/main2 各自增量，本轮收口 |

## 7. Canonical consolidation 状态（AS_OF=2026-09-29）

截至本文件记录时，Main-2 事实与 Main-1 四条 Online closure 事实均已进入 `origin/main`。该状态不改变路线生命周期。

| 事项 | 当前状态 |
|---|---|
| Main-2 wave-1 事实（含 ASYNC V001 Online package / result / ledger / calibration） | **IN_CANONICAL=YES** |
| MAIN-1 EPI V002 | **IN_CANONICAL=YES**；Official 44.96；identity PASS |
| 含 MAIN-1 EPI V002 的完整 canonical 集合 | **CANONICAL_CONSOLIDATED=YES** |

### 7.1 历史整合记录

- `247f268a`：Main-2 wave-1 初次收口阶段记录。
- `8162dfaa`：第二轮冲突处置阶段记录。
- `244a0bb3`：Main-1 TSV 行级语义映射阶段记录。

以上提交号只作历史记录说明，不作为当前 canonical 状态指针；当前状态以本节 `AS_OF` 及 `origin/main` 为准。
