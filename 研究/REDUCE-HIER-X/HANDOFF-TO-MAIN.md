# HANDOFF-TO-MAIN — REDUCE-HIER-X（V004 完成）

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
FROM=Route Agent（LANE M2-1 REDUCE-HIER-X）
TO=Main-2
STATUS=V004 本地结论已出 = LOCAL_REJECTED；已按指示显式回退到 FROZEN_R31B_V011；STOP

---

## 1. V004 结论

**LOCAL_REJECTED。** N1（binned streaming single-stage square-sum）被证伪。

| 项 | 结果 |
|---|---|
| SINGLE_HYPOTHESIS | N1：per-tile ReduceSum + collapse → 128 元分箱累加器（零 barrier Add 链）+ 行尾一次 ReduceSum(128) |
| DIRECT_PARENT | FROZEN_R31B_V011（a8c19a19…） |
| V004 SOURCE_SHA | 1926a2f91491981f06424502dde96356661b5d5c5dc30ae190a0e5935265218d |
| SINGLE_CHANGE_AUDIT | PASS（一个概念变化；只落 S1/S2/S3；tileCount==1 与 S4/S5 未动） |
| BUILD / LINK | PASS / PASS（server3 dav-2201，BUILD_RC=0） |
| CORRECTNESS | PASS（16/18 bad=0；1x16384/1x32768 FP32 父版预存 golden 差异，候选 max_abs 持平或更好） |
| same-binary | 1x32768 双侧 PASS；1x16384 双侧 PASS；8x8192 候选侧 BLOCKED（单块污染）；1x4096 双侧不合格 |
| **P/C 主探针** | **1x32768 +23.89%（6/0 favP）；1x16384 +21.17%（6/0 favP）** |
| 8x8192 干净 pair | +10..18% favP（pair 2/4/5/6） |
| 1x4096 对照 | 无信号（S5 未动，符合预期） |

判定依据：两个主探针形状双侧 same-binary 合格、6 组配对全部 favP、退化幅度远超噪声带
（P MAD/med ≤ 0.05）。这是稳定回归，不是噪声。

## 2. 根因（事实推断）

零 barrier 的分箱 Add 链正确性安全（V003 的结构信息成立），但不更快：把每个 4096 元
`ReduceSum` 拆成 32 次小 `Add` 调度（D=32768 一行 256 次 Add + 1 次 ReduceSum，对比父版
9 次 ReduceSum），指令调度开销超过了省下的 ReduceSum 调用成本。与 V002 的教训同源不同
形：V002 输在 barrier 链，V004 barrier 为零仍输——**手工分解归约的指令数本身就是成本**。
`ReduceSum` 作为 API 调用在本工具链已接近该形状带的最优形态。

## 3. 归约拓扑轴剩余空间（事实陈述，供转 Planning）

四个 large-D 归约拓扑变体全部证伪：

| Revision | 机制 | 结果 |
|---|---|---|
| V001 | eager fold（partial 折叠时机） | 8x8192 -3.8% 近中性 |
| V002 | 手写 Add 树替代 ReduceSum | 1x32768 +6.6% LOCAL_REJECTED |
| V003 | 短 span ReduceSum + 单 barrier | 1x32768 -0.5% 中性，Official 44.24 |
| V004 | 分箱单级（去 per-tile 调用） | 1x32768 +23.9% LOCAL_REJECTED |

两种结构端点都输或持平：减少调用数（手工 Add）明显更慢；缩短 span（预折）中性。
**reduction topology 在 FROZEN_R31B_V011 谱系已无剩余赢面；reduction V/S 不是 large-D
主瓶颈** 这一结论由四次独立实验支持。按 Main-2 指示：不自行 PARK，该事实由 Main-2 转
Planning 决定路线生命周期。

## 4. 回退状态

CURRENT_SOURCE = FROZEN_R31B_V011（a8c19a19…）。server3 工作区已覆盖回 parent 并核对
SHA。V004 失败证据全部保留于 `本地实验/REDUCE-HIER-X/V004/`（含 171 个 raw timing 文件）。
共享成绩记录未动（由 Main-2 登记）。

## 5. 请 Main-2 处理

1. 登记 V004 到 `技术路线/全版本记录.tsv` 与 `路线成绩表.tsv`（LOCAL_REJECTED，
   evidence: 本地实验/REDUCE-HIER-X/V004/）。
2. 将「归约拓扑轴剩余空间耗尽（四次证伪）」事实转 Planning，由 Planning 决定
   KEEP/PARK/关闭。
3. Track-B 残余假设（N2 group-span / N3 内建 primitive / N4 多行归约）建议一并转呈：
   N3 是唯一尚未在本谱系测过的 primitive 级变量（若 E3 契约核对确认 ReduceSum 真有
   内部交接），但先验已大幅下调——手工/替代归约路径两次大败。
4. E2/E3 纯读工作未在本轮展开（V004 流程优先）；如需要可下轮补。

## 6. 本轮边界

未提交 Online、未建 V005、未改共享成绩记录、未碰 DTYPE-SPECIAL-X / SCHED-CHAMPION-X /
UB-LIVENESS-X 实现。等待 Main-2 指示后 STOP。
