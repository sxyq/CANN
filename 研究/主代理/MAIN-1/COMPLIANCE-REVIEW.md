# MAIN-1 COMPLIANCE-REVIEW

- Date: 2026-09-29
- Scope: DUAL MAIN COMPLIANCE + ONLINE CLOSURE
- Branch: `main1/champion-exploit`
- HEAD: `b6d787d4`

## PROCESS_DEVIATION = YES

本轮不伪造历史 revert。从当前时刻起修正。

---

## 1. LOCAL_REJECTED 后没有 explicit revert commit

| REVISION | EXPECTED_ROLLBACK | ACTUAL_BEHAVIOR | WHY_HISTORICAL_REVERT_CANNOT_BE_RETROACTIVELY_CREATED | CURRENT_BRANCH_STATE |
|---|---|---|---|---|
| R31A V022 (LOCAL_REJECTED) | `git revert` or restore commit to V016 | 下一 Revision V023 直接以 V016 为 DIRECT_PARENT，无显式 revert commit | V023-V028 已在 V016/V024/V025/V026 链上继续；回滚会破坏后续正确代码 | LOCAL_BEST=V028，链完好 |
| R31B V018 (NEEDS_ONE_MORE_LOCAL) | N/A（非 REJECTED） | 保持 Candidate | — | LOCAL_BEST=V017 |
| R31B V019 (NEEDS_ONE_MORE_LOCAL) | N/A | 保持 Candidate | — | LOCAL_BEST=V017 |

**结论**：R31A V022 存在缺失 revert commit。从下一次 LOCAL_REJECTED 开始严格执行 Step1/2/3（保存源码 → 保存证据 → 显式 restore）。

---

## 2. Child 在 Main approval 前创建性能 Revision

| ROUTE | REVISION | 偏差 | 处置 |
|---|---|---|---|
| EPILOGUE-ARITH-CHAMPION-X | V001 | Agent 自行选定 NORM-HOIST 并建源码，Main 事后追认 | PROCESS_VIOLATION 记录；追认有效（假设合格）；后续已改为 Main 先选定 |

**结论**：1 次 preapproval violation。已纠正（后续 H2/H5/H9 等均为 Main 先选定）。

---

## 3. LOCAL_ACCEPTED / WORTHY 但没有 Online result

| ROUTE | REVISION | LOCAL | ONLINE_RECOMMENDATION | Judge result |
|---|---|---|---|---|
| R31B | V017 | LOCAL_ACCEPTED | WORTHY | **缺失** |
| R31A | V028 | LOCAL_ACCEPTED | WORTHY | **缺失** |
| STORE-EPILOGUE-X | V003 | LOCAL_ACCEPTED | WORTHY | **缺失** |
| EPILOGUE-ARITH-CHAMPION-X | V002 | LOCAL_ACCEPTED | WORTHY | **缺失** |

**结论**：4 个候选均待 Online closure。本轮第一优先级。

---

## 4. Online package 已准备但 Judge Owner 未执行

| ROUTE | REVISION | package 路径 | submission.asc SHA |
|---|---|---|---|
| R31B | V017 | `线上结果/R31B/V017/` | `7c168eaf…` |
| R31A | V028 | `线上结果/R31A/V028/` | `ee52831c…` |
| STORE-EPILOGUE-X | V003 | `线上结果/STORE-EPILOGUE-X/V003/` | `0cdef265…` |
| EPILOGUE-ARITH | V002 | `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/` | `00a5c818…` |

**结论**：4 个包就绪，均未提交。本轮串行提交。

---

## 5. 结果只在 Main branch，没有进入 canonical main

**检查**：Main-1 的 LOCAL_ACCEPTED / Online package / ledger 更新均在 `main1/champion-exploit`，canonical `origin/main` 未收到。

**结论**：需要 DUAL-MAIN CONSOLIDATION。按指令由明确的 CONSOLIDATION OWNER 执行。

---

## 6. 共享总账在 Main-1 / Main-2 两条 branch 各自分叉

**检查**：Main-1 修改了 `技术路线/路线成绩表.tsv`、`技术路线/全版本记录.tsv`、`调度/当前任务.tsv`。Main-2 在 `main2/orthogonal-explore` 上独立修改。

**结论**：共享总账已分叉。需要在 canonical consolidation 时合并（只合并事实行，不合并 Kernel）。

---

## 7. LOCAL_BEST / SERVER_BEST / OFFICIAL_BEST 字段分离

**检查**：已按要求分离。所有本地收益（-14%、-8%、-6%）均记为 LOCAL_DELTA，不冒充 Official Score。

**OFFICIAL_BEST 当前值**（不变）：

| ROUTE | OFFICIAL_BEST | LOCAL_BEST |
|---|---|---|
| R31B | V011 / 45.16 | V017 |
| R31A | V016 / 45.00 | V028 |
| STORE-EPILOGUE-X | V002 / 45.07 | V003 |
| EPILOGUE-ARITH | 无 | V002 |

---

## 修正承诺

1. 从下一次 LOCAL_REJECTED 开始：保存源码 → commit+push → 保存证据 → commit+push → 显式 restore → commit+push。
2. 后续性能 Revision：Track-B → Main review → MAIN_SELECTED=YES → 声明 → 实现。
3. Online 串行提交，每次完整落盘。
4. 不伪造历史。
