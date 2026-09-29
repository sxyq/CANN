# MAIN-1 COMPLIANCE-REVIEW

- Date: 2026-09-29
- Scope: DUAL MAIN COMPLIANCE + ONLINE CLOSURE
- Branch: `main1/champion-exploit`
- Main-1 source branch HEAD at result closure: `2e70116283527354e555530ab647c95f7733ec34`
- Canonical consolidation target: `main` / `origin/main`

## PROCESS_DEVIATION = YES

本轮不伪造历史 revert；从当前时刻开始按三步流程执行。

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
| R31B | V017 | LOCAL_ACCEPTED | WORTHY | `6abb80ba694b590c3c970983`, Pass 15/15, Official 44.68, identity PASS; LOCAL_ONLINE_FALSE_POSITIVE |
| R31A | V028 | LOCAL_ACCEPTED | WORTHY | `6abb7eb5694b590c3c95c879`, Pass 15/15, Official 44.07, identity PASS; LOCAL_ONLINE_FALSE_POSITIVE |
| STORE-EPILOGUE-X | V003 | LOCAL_ACCEPTED | WORTHY | `6abb8182694b590c3c978d30`, Pass 15/15, Official 44.38, identity PASS; LOCAL_ONLINE_FALSE_POSITIVE |
| EPILOGUE-ARITH-CHAMPION-X | V002 | LOCAL_ACCEPTED | WORTHY | `6abb8840694b590c3c9b6db3`, Pass 15/15, Official 44.96, identity PASS; LOCAL_ONLINE_FALSE_POSITIVE |

**结论**：4 个候选均已完成 Online closure；四条结果均通过 15/15，均未超过各自线上参照，Local Best 证据保留。

---

## 4. Online package 与 Judge Owner 执行结果

| ROUTE | REVISION | package 路径 | submission.asc SHA | Judge result / identity |
|---|---|---|---|---|
| R31B | V017 | `线上结果/R31B/V017/` | `7c168eaf…` | `6abb80ba694b590c3c970983` / 44.68 / identity PASS |
| R31A | V028 | `线上结果/R31A/V028/` | `ee52831c…` | `6abb7eb5694b590c3c95c879` / 44.07 / identity PASS |
| STORE-EPILOGUE-X | V003 | `线上结果/STORE-EPILOGUE-X/V003/` | `0cdef265…` | `6abb8182694b590c3c978d30` / 44.38 / identity PASS |
| EPILOGUE-ARITH-CHAMPION-X | V002 | `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/` | `00a5c818…` | `6abb8840694b590c3c9b6db3` / 44.96 / identity PASS |

**结论**：4 个包均由 Judge Owner 串行提交；每条均已保存 `result.json`，并按独立事实推送。

---

## 5. Main-1 结果进入 canonical main 的收口

**最终事实**：Main-1 的 Local、Online package、结果和账本更新均已在 `main1/champion-exploit` 推送；四条 Online 结果（含 EPI V002）均已由 Consolidation Owner 收入 `origin/main`。

**当前状态**：`RESULTS_IN_CANONICAL=YES`；不再存在 Main-1 结果只停留在分支的缺口。

---

## 6. 共享总账分支分叉与收口

**历史事实**：Main-1 修改了 `技术路线/路线成绩表.tsv`、`技术路线/全版本记录.tsv`、`调度/当前任务.tsv`；Main-2 在 `main2/orthogonal-explore` 上独立修改。

**当前状态**：共享总账的分支差异已由单一 Consolidation Owner 收口到 canonical；只纳入事实、证据引用、Online package/result、scoreboard、ledger、calibration 和调度状态，未纳入 child Kernel。

---

## 7. LOCAL_BEST / SERVER_BEST / OFFICIAL_BEST 字段分离

**字段分离结果**：所有本地收益（-14%、-8%、-6%）均记为 LOCAL_DELTA，没有写成 Official Score。

**OFFICIAL_BEST 当前值**（不变）：

| ROUTE | OFFICIAL_BEST | LOCAL_BEST |
|---|---|---|
| R31B | V011 / 45.16 | V017 |
| R31A | V016 / 45.00 | V028 |
| STORE-EPILOGUE-X | V002 / 45.07 | V003 |
| EPILOGUE-ARITH-CHAMPION-X | V002 / 44.96 | V002 |

---

## 纪律承诺

1. 从下一次 LOCAL_REJECTED 开始：保存源码 → commit+push → 保存证据 → commit+push → 显式 restore → commit+push。
2. 后续性能 Revision：Track-B → Main review → MAIN_SELECTED=YES → 声明 → 实现。
3. Online 串行提交，每次完整落盘。
4. 不伪造历史。

---

## 8. Main-1 Online closure 结果

| ROUTE | REVISION | LOCAL_DELTA | OFFICIAL_SCORE | OFFICIAL_DELTA | DIRECTION | NEXT ACTION |
|---|---|---|---:|---:|---|---|
| R31B | V017 | bf16-wide -14.3%; fp16-wide -6.7~-8.5% | 44.68 | -0.48 vs 45.16 | LOCAL_ONLINE_FALSE_POSITIVE | 保留 V017 Local Best；等 Planning |
| R31A | V028 | chain -8.3%; V028 -1.09% | 44.07 | -0.93 vs 45.00 | LOCAL_ONLINE_FALSE_POSITIVE | 保留 V028 Local Best；等 Planning |
| STORE-EPILOGUE-X | V003 | -5.64% primary; -3.15% wide | 44.38 | -0.69 vs V002 45.07 | LOCAL_ONLINE_FALSE_POSITIVE | 保留 V003 Local Best；等 Planning |
| EPILOGUE-ARITH-CHAMPION-X | V002 | -3.13~-6.64% cross-window; -3.62% | 44.96 | -0.20 vs 45.16 anchor | LOCAL_ONLINE_FALSE_POSITIVE | 保留 V002 Local/Official Best；不新建 Revision |

`LOCAL_SHA = SIDECAR_SHA = REMOTE_SHA` 已在四条结果中确认。Official 结果均未超过 Overall Champion 45.16，因此没有 NEW_OFFICIAL_CHAMPION_CANDIDATE。

---

## 9. Local→Official calibration（与 Main-2 合并口径）

| ROUTE | REVISION | LOCAL_PARENT | LOCAL_METRIC | LOCAL_DELTA | SERVER_METRIC | OFFICIAL_PARENT | OFFICIAL_SCORE | OFFICIAL_DELTA | DIRECTION_MATCH | CLASSIFICATION |
|---|---|---|---|---|---|---|---:|---:|---|---|
| R31B | V017 | V016 | bf16-wide D32768；fp16-wide 多组配对 | -14.3%；-6.7~-8.5% | 15/15 PASS | V011 / 45.16 | 44.68 | -0.48 | CONTRADICT | FALSE_POSITIVE |
| R31A | V028 | V026 | D24576 双设备配对；V024–V028 链 | -1.09%；链累计 -8.3% | 15/15 PASS | V016 / 45.00 | 44.07 | -0.93 | CONTRADICT | FALSE_POSITIVE |
| STORE-EPILOGUE-X | V003 | V002 | 8x16384 primary；1x32768 / 1x16384 辅助 | -5.64%；-3.15%；-2.90% | 15/15 PASS | V002 / 45.07 | 44.38 | -0.69 | CONTRADICT | FALSE_POSITIVE |
| EPILOGUE-ARITH-CHAMPION-X | V002 | V001 | 1x32768、2x16384 cross-window | -3.13~-6.64%；-3.62% | 15/15 PASS | R31B V011 / 45.16 | 44.96 | -0.20 | CONTRADICT | FALSE_POSITIVE |
| ASYNC-OVERLAP-CHAMPION-X | V001 | R31B V011 | 128x16384 FP16；BF16/96x16384 为辅助方向 | -1.3~-2.2% primary | 15/15 PASS | R31B V011 / 45.16 | 44.17 | -0.99 | CONTRADICT | FALSE_POSITIVE |

本轮五个重点样本的分类计数：`TP=0, FP=5, TN=0, FN=0`。历史补充样本 `COEFF-LOCALITY-X V001` 继续保留既有 `FALSE_NEGATIVE` 标签：本地单形状退化，但 Official 44.16 曾作为该路线的有效结果；它不计入上面五个重点样本。
