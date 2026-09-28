# HANDOFF-TO-MAIN — REDUCE-HIER-X（V005 完成，LOCAL_REJECTED → LANE_NEEDS_PLANNING_REVIEW）

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
FROM=Route Agent（LANE M2-1 REDUCE-HIER-X）
TO=Main-2
STATUS=V005 LOCAL_REJECTED；已显式回退 FROZEN_R31B_V011；本 Lane = LANE_NEEDS_PLANNING_REVIEW；STOP

---

## 1. V005 结论

**LOCAL_REJECTED。** N3（ReduceSum→WholeReduceSum）也回归，幅度小于 V004 但方向稳定。

| 项 | 结果 |
|---|---|
| SINGLE_HYPOTHESIS | 同站点同 span 同两级拓扑，归约 primitive：`ReduceSum`（vcadd mode=1 + V/S + get_acc_val + S/V + S_MTE3）→ 向量输出 `WholeReduceSum`（vcadd mode=0） |
| DIRECT_PARENT | FROZEN_R31B_V011（a8c19a19…） |
| SOURCE_SHA | 5fa32526…（含一次 in-revision API-usage correctness fix） |
| BUILD / LINK | PASS / PASS |
| CORRECTNESS | PASS（16/18 bad=0；宽 FP32 两条父版预存，candidate max_abs 与父版一致） |
| same-binary | 1x32768 双侧 PASS；1x16384 双侧 PASS；8x8192 候选侧 BLOCKED；1x4096 对照双侧 BLOCKED |
| **P/C 主探针** | **1x32768 +7.50%（5/6 favP，干净 pair 全 favP）；1x16384 +8.11%（6/6 favP）** |
| 8x8192 | -1.26% 近噪声（干净 pair -3.1..+3.7%），不足以扭转 |

判定：两个主探针双侧 same-binary 合格、11/12 干净 pair favP、退化 4–12% 超出噪声带
（P MAD/med ≤ 0.05）。稳定回归。

## 2. 根因与新硬件事实

实现过程中发现（并已记录进事实包）：

1. **`vcadd` mode=0 的输出粒度是每 natural repeat（64 FP32）一个和**，不是任意 count
   一个总和。mode=1（进硬件 acc）才能跨 repeat 累加。因此「一次调用换一次调用」的
   纯 primitive 交换在 span>64 时**不可实现**；必须两段式。
2. 首版实现（count-mode mask + 单次 WRS）正确性大败（max_abs ~20），同 revision 改为
   生产 rms_norm 的两段式（先 64 元粒度 partial，再一次收尾）后正确性 PASS。
3. **两段式净收益为负**：每 tile 2 条 vcadd（零同步） vs 父版 1 条 vcadd + 3 硬同步 +
   标量回路，主探针稳定慢 7–8%。省下的同步链代价 < 额外 vcadd 代价。
4. `ReduceSum` 的同步链（V/S + get_acc_val + S/V + S_MTE3）已由工具链源码实锤，但它
   不是可回收的主成本——这否定了 E3 的收益预估。

## 3. 五个 Revision 全伪对照（供转 Planning）

| Rev | 轴 | 主探针 | 结论 |
|---|---|---|---|
| V001 | fold 时机 | 8x8192 -3.8% | NEEDS_ONE_MORE_LOCAL |
| V002 | per-tile primitive（手写树） | 1x32768 +6.6%（5/0） | LOCAL_REJECTED |
| V003 | span（折短） | 1x32768 -0.5% 中性 | NEEDS_ONE_MORE_LOCAL（Official 44.24） |
| V004 | 结构（两级→单级分箱） | 1x32768 +23.9% / 1x16384 +21.2%（各 6/0） | LOCAL_REJECTED |
| V005 | primitive（两段式 WRS） | 1x32768 +7.5% / 1x16384 +8.1% | LOCAL_REJECTED |

**reduction structure / span / primitive 轴在 FROZEN_R31B_V011 已用尽；父版 `ReduceSum`
接近最优。** 详见 `本地实验/REDUCE-HIER-X/V005/REVERT-AND-LANE-NEEDS-PLANNING-REVIEW.md`。

## 4. 本 Lane 状态

按你上一条指示（N3 也证伪则非 PARK）：**LANE_NEEDS_PLANNING_REVIEW**。
不自行 PARK、不建 V006、不改共享成绩记录。事实包已写好，请转 Planning 决定
KEEP/PARK/CLOSE。残余假设 N2/N4 先验极低（理由见事实包 2.3）。

## 5. 请 Main-2 处理

1. 登记 V005 到 `技术路线/全版本记录.tsv` / `路线成绩表.tsv`（LOCAL_REJECTED）。
2. 将事实包转 Planning：五次证伪 + 硬件事实 + Lane 状态。
3. 五套证据（V001–V005）与 E3 契约记录保留为 donor，尤其 V005 的 vcadd mode 语义
   与 `ReduceSumImpl` 同步链事实（对其他路线的归约/向量侧改动有参考价值）。

## 6. 本轮边界

未提交 Online、未建 V006、未自行 PARK、未改共享成绩记录、未碰他路线。
