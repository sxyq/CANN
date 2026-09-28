# HANDOFF-TO-MAIN — REDUCE-HIER-X（E3 完成，申请放行 V005）

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
FROM=Route Agent（LANE M2-1 REDUCE-HIER-X）
TO=Main-2
STATUS=E3 只读核对完成 = **PASS**；申请放行 V005 实现（N3）；等确认前不动 Kernel

---

## 1. E3 结论（摘要，全文见 `研究/REDUCE-HIER-X/E3-API-CONTRACT.md`）

**PASS — N3 可行，且优势有实现层证据。**

在 CANN 8.5.0.alpha002 / dav-2201 工具链头文件里核对到：

| 契约项 | 结论 |
|---|---|
| 向量输出内建归约 | 存在：`WholeReduceSum`（推荐）/ `BlockReduceSum` / `RepeatReduceSum` / `PairReduceSum` |
| 输出形态 | 纯 UB 向量侧（`vcadd(..., mode=0)` 直接写 dst），无标量回路 |
| 当前 `ReduceSum` 是否有标量 handoff | **有，已实锤**（见下） |
| 4096 元 tile 适用性 | 适用：mask-count 模式 count 上限约 49152 FP32，4096 与任意尾块 count 均在范围内 |
| dtype | half / float，FP32 可用 |
| 与 V004（N1）的区别 | N1 手工拆出数百条 Add（指令数成本，已证伪）；N3 仍是**一条 vcadd**，只去掉 vcadd 之后的同步链 |

**实锤证据**（`impl/dav_c220/kernel_operator_vec_reduce_impl.h` L146–168）：
`ReduceSumImpl` 每次调用 = 1 条 `vcadd(mode=1)` + **V→S 排空** + **`get_acc_val()` 标量读**
+ **S→V** + **S→MTE3**。即每 tile 一次归约付 3 次硬同步 + 1 次标量寄存器读，还把归约与
store 管线耦合。Level-0 `WholeReduceSum` = 同一条 `vcadd` 但 mode=0，结果直接落 dst，
零同步、零标量。

**N3 预期**：multi-tile 行（tileCount=T）每行归约侧硬同步 `(T+1)×3` → `1`（仅保留既有
行尾 GetValue 的一次 V/S）。T=8 时 27 次 → 1 次；同时解除与 MTE3 的耦合。指令数不变。

## 2. 四个 large-D 归约变体全伪对照表（供转 Planning）

| Rev | 假设 | 单变量 | 结果（主探针） | 本地结论 | 根因 |
|---|---|---|---|---|---|
| V001 | H1 eager fold | partial 折叠时机（去 collapse，保留 per-tile ReduceSum） | 8x8192 -3.8%（n=2）；其余 same-binary FAIL | NEEDS_ONE_MORE_LOCAL | collapse 不是主要成本 |
| V002 | H2 手写 Add 树 | per-tile 归约 primitive（树替代 ReduceSum） | 1x32768 **+6.6%** favP 5/0；8x8192 +0.9% | LOCAL_REJECTED | per-level PipeBarrier 链成本 > 省下的 V/S |
| V003 | H3 短 span | 归约 span（折到 128 再 ReduceSum，零中间 barrier） | 1x32768 -0.5% 中性；8x8192 -3.3% 近噪声；1x16384 -1.5% | NEEDS_ONE_MORE_LOCAL（Official 44.24） | 短 span 不带来预期大胜 |
| V004 | N1 分箱单级 | 归约结构（两级→单级，128 元累加器 + 一次 ReduceSum） | 1x32768 **+23.89%** favP 6/0；1x16384 **+21.17%** favP 6/0 | LOCAL_REJECTED | 手工分解的 Add 指令调度开销 > 省下的 ReduceSum 调用 |

四者共同结论：**reduction topology 在 FROZEN_R31B_V011 谱系已无赢面；reduction V/S
不是 large-D 主瓶颈。** 两种结构端点（减调用数、缩 span）均输或持平。

## 3. N3 为何不重复上述四条

| 对照 | 区别 |
|---|---|
| V002 tree | V002 手写 Add 树（数百条指令 + barrier 链）；N3 仍是一条 `vcadd`，只去掉其后的同步/标量链 |
| V004 binned | V004 把 4096 元拆成 32 条 Add；N3 指令数与父版相同（T+1 条），变量只有"谁来收 vcadd 的结果" |
| V003 short-span | 不动 span |
| V001 fold | 不动折叠时机 |

N3 打的是 E3 实锤的新成本项（每次 `ReduceSum` 的 3 硬同步 + 标量 acc 读 + MTE3 耦合），
这个成本项 V001–V004 都没被直接攻击过（V002/V004 试图消灭调用但输在指令数，V003 只改 span）。

## 4. 申请与待确认

1. **申请放行 V005 实现**：N3 = 归约 primitive 替换（`ReduceSum` → `WholeReduceSum`
   mask-count 重载，同站点同 span 同两级拓扑，先落 S1/S2/S3），DIRECT_PARENT =
   FROZEN_R31B_V011，SINGLE_HYPOTHESIS 声明将写在实现前。
2. 若放行：流程按标准（declaration → 源码 → compile → correctness → same-binary →
   交错 P/C → 本地结论 → handoff）。
3. 若不放行或 N3 也被证伪：本 Lane 转 LANE_NEEDS_PLANNING_REVIEW（按你上一条指示，
   不是 PARK），归约轴事实由你转 Planning。
4. E2（源码 op-count 表）可与 V005 并行补做，或按你需要再安排。

## 5. 本轮边界

E3 纯读，无编译、无设备、无改码、无新 Revision。未建 V005、未提交 Online、未动共享
成绩记录与他路线。等你确认后才写 V005。
