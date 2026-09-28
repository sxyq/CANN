# HANDOFF-TO-MAIN — REDUCE-HIER-X Track-B（本轮假设包）

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
FROM=Route Agent（LANE M2-1 REDUCE-HIER-X）
TO=Main-2
PACKAGE=研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES-NEXT.md
STATUS=Track-B 完成，等待 Main-2 批准；批准前不写任何 Kernel

---

## 1. 建议批准哪一条

**建议批准 N1 — Binned streaming single-stage square-sum（W 元向量累加器 + 单次收尾
ReduceSum），DIRECT_PARENT = FROZEN_R31B_V011。**

理由（按权重）：

1. **新结构信息成立**：V003 已实测「零中间 barrier 的 V-pipe Add 链」正确性安全，
   V002 的失败根因（per-level PipeBarrier）可以被设计排除；平方项非负使结合顺序
   风险低。这两点是 V001–V003 立项时没有的。
2. **打的是没被打过的杠杆**：V001 只去掉 collapse（弱信号已测）；N1 去掉的是随
   tileCount 增长的 per-tile ReduceSum 调用项，把每行归约侧 V/S 从 tileCount+2
   压到 3。这与 fold / tree / short-span 三个已证伪形态都不重合（对照见假设包
   第 3 节 DUPLICATE_CHECK）。
3. **UB 代价最小**：W=128 只需 512 B，可从 reduceFp32Buf_（16 KB，活跃 partial 槽
   仅 ≤8 个）划出，不挤 tile 预算——COEFF-LOCALITY-X V001 的失败模式不会重演。
4. **OFAT 干净**：一个概念变化（平方如何变成行 squareSum），先只落 S1/S2/S3。

先验要放诚实：large-D 归约份额已三次证伪（估计 ~20%），N1 的预期收益是「中性到
小胜」，不是大胜。它的价值在于用最小代价回答「per-tile 调用项是不是最后一点剩余
空间」；若仍中性，归约拓扑轴在本谱系可视为走完。

## 2. 立项前置（请 Main-2 二选一）

- **路径 A（接受新结构信息）**：直接批准 N1，按标准流程 Revision 声明 → 编译 →
  正确性 → same-binary → 配对测量。
- **路径 B（坚持新瓶颈证据）**：先做假设包第 4 节 E5——一次「只跑父版、不改 Kernel」
  的 profiling 会话（msprof 分解 V/S / MTE / pipe 占用），拿到分解数据再批 N1。
  E2/E3（源码 op-count 表、ReduceSum/BlockReduceSum 契约核对）两条纯读工作可并行
  由本路线完成，不需要设备窗口。

我个人倾向路径 A：N1 的 UB 风险与正确性风险都低，测量成本一次配对即可否证；E5 的
信息增量更值得留给「N1 也中性之后」的去向判断（PARK 还是换轴）。

## 3. 次优先

N3（内建向量输出归约 primitive）排第二：它是唯一对准短行多行带（case 7 类，
归约尾部 + handoff 估计占非 DMA 时间 35–45%）的变量，而 V001–V003 主探针都在大 D。
前置是 E3 的 API 契约只读核对——若 `ReduceSum` 内部交接为假或内建变体无优势，
N3 直接判 INFEASIBLE，不占用实验预算。

N2（group-span）/ N4（多行同时归约）保持 NEEDS_MORE_EVIDENCE，不建议本轮立项。

## 4. V003 状态（供记录核对）

V003 **已闭环**：本地 `NEEDS_ONE_MORE_LOCAL`（合法结论）、线上 44.24（6abab2bb…，
15/15 Pass，三方 SHA 一致，formalResultEligible=true）、registry 已记处置与 pivot。
无待收口工作。V003 不是 LOCAL_ACCEPTED，Official 44.24 低于 anchor 45.16，因此下一
Revision 的合格起点是 FROZEN_R31B_V011，不是 V003。

## 5. 请 Main-2 决定的事项

1. N1 是否批准（路径 A / 路径 B）。
2. DIRECT_PARENT 是否同意 = FROZEN_R31B_V011。
3. E2/E3 是否授权由本路线继续（纯读，无设备）。
4. 若 N1 之后仍无信号，是否向 Planning 提报「归约拓扑轴剩余空间耗尽」的事实
   （PARK 与否由 Planning 决定）。

## 6. 本轮边界声明

未改 Kernel、未建 V004、未跑正式 performance、未提交 Online、未动共享调度与他路线
实现、未改技术路线成绩与版本记录。等待 Main-2 批准后再进入实现。
