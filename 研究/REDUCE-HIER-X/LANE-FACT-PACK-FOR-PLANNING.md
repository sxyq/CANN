# LANE FACT PACK FOR PLANNING — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
LANE=M2-1（Main-2 体系）
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-reduce
BRANCH=m2/reduce-hier
DATE=2026-09-29
LANE_STATUS=LANE_NEEDS_PLANNING_REVIEW（不是 PARK）
AUTHOR=Route Agent（LANE M2-1 REDUCE-HIER-X）
PURPOSE=收口交付：给 Planning 的完整事实包。只陈述事实与证据位置，不作路线生命周期决定。

---

## 1. 一句话结论

归约结构轴（reduction structure / span / primitive）在 FROZEN_R31B_V011 谱系上
经五个 Revision 独立验证后**已用尽**：2 弱信号 3 失败，无一 LOCAL_ACCEPTED。
父版的 `AscendC::ReduceSum`（一条 `vcadd` 进硬件 acc + 标量回路）在 dav-2201 上
接近最优形态；任何形式的重组都在指令数或同步链上净亏。

## 2. 五变体对照表（完整）

基线：FROZEN_R31B_V011（SOURCE_SHA `a8c19a19…`，Official 45.16）。
主探针：1x32768 FP32（tileCount=8）、1x16384 FP32（4）、8x8192 FP32（2）。
测量：runner_ref.inc device-event，warmup=45，samples=41，same-binary blocks=2
+ 6 组交错 P/C。全部 P/C 为「(C−P)/P」百分比，正 = 候选更慢。

| Rev | 假设 | 单变量（只改一项） | 主探针配对中位数 | 方向一致性 | 本地结论 | 根因 |
|---|---|---|---|---|---|---|
| V001 | H1 eager fold | partial 折叠时机：per-tile partial 立即折入累加器，去 collapse | 8x8192 -3.8%（n=2）；其余形状 same-binary FAIL | 弱 | NEEDS_ONE_MORE_LOCAL | collapse 不是主要成本 |
| V002 | H2 手写 Add 树 | per-tile 归约 primitive：pairwise Add 树替代 ReduceSum | 1x32768 **+6.6%**；8x8192 +0.9% | 5/0 favP | **LOCAL_REJECTED** | per-level PipeBarrier 链成本 > 省下的 ReduceSum V/S |
| V003 | H3 短 span | 归约 span：barrier-free 折到 kReduceSpan=128 再 ReduceSum | 1x32768 -0.5%；8x8192 -3.3%；1x16384 -1.5% | 混合/中性 | NEEDS_ONE_MORE_LOCAL（Official 44.24） | 短 span 不带来预期大胜 |
| V004 | N1 分箱单级 | 归约结构：两级（per-tile + collapse）→ 单级 128 元分箱累加器 + 一次 ReduceSum(128) | 1x32768 **+23.9%**；1x16384 **+21.2%** | 6/0、6/0 favP | **LOCAL_REJECTED** | 手工分解的 Add 指令调度开销 > 省下的 ReduceSum 调用 |
| V005 | N3 primitive 替换 | 归约 primitive + 后处理链：ReduceSum（vcadd mode=1 + 3 硬同步 + get_acc_val + S_MTE3）→ WholeReduceSum（vcadd mode=0 向量直写） | 1x32768 **+7.5%**；1x16384 **+8.1%** | 5/6、6/6 favP | **LOCAL_REJECTED** | 两段式 WRS 的额外 vcadd 代价 > 省下的同步链 |

V003 线上：Official 44.24（submission `6abab2bb…`，15/15 Pass，三方 SHA 一致），
低于 anchor 45.16。其余四个 Revision 未送线上。

### 2.1 三个轴的端点都已测过

| 轴 | 已测端点 | 结果 |
|---|---|---|
| fold 时机（partial 何时合并） | eager fold（V001） vs deferred bank（父版） | 中性 |
| span（每次归约的宽度） | 折短到 128（V003） vs 整 tile 4096（父版） | 中性 |
| structure（两级 vs 单级） | 单级分箱（V004） vs 两级（父版） | 大败 |
| primitive（谁来做归约） | 手写树（V002）／两段式 WRS（V005） vs ReduceSum（父版） | 大败／小败 |

## 3. 根因（证据支撑）

1. **手工分解归约的指令调度数本身就是成本。** V002（树 + barrier 链）与 V004
   （分箱 32 条 Add/tile）都试图减少或消除 `ReduceSum` 调用，都因指令数大增而
   大败（+6.6% / +21–24%）。V004 零 barrier 仍败 → 问题不在 barrier，在指令数。
2. **省下的同步链代价低于额外指令代价。** V005 保持每 tile 归约的向量算术总量不变
   但换用两段式向量输出（0 同步 vs 父版 3 同步/tile），仍稳定慢 7–8%。
3. **缩短 span 无收益**（V003 中性）：`ReduceSum` 内部对短 span 并不更贵。
4. 合并结论：**父版 `ReduceSum` = 一条 `vcadd` 进硬件 acc + 一次标量读回**，
   在 dav-2201 上就是「一条向量指令 + 最小后处理」的形态；任何替代形态的指令数
   都不少于它，而同步链又不够贵到值得消除。

## 4. vcadd mode 语义（工具链源码 + 实测，含反例）

来源：CANN 8.5.0.alpha002，`tikcfw/impl/dav_c220/kernel_operator_vec_reduce_impl.h`、
`impl/utils/kernel_utils_struct_param.h`、生产算子 `opp/.../rms_norm_grad/reduce_common.h`。

### 4.1 指令事实

| 项 | 事实 |
|---|---|
| `vcadd` 末参数 | mode=1 → 结果进硬件 acc 寄存器（须 `get_acc_val` 标量读回）；mode=0 → 每 repeat 一个和直写 dst |
| 输出粒度 | **mode=0 每 natural repeat（FP32 = 64 元）一个和**；跨 repeat 累总和只有 mode=1 + acc 能做 |
| `ReduceSumImpl`（dav-2201，现用） | `set_mask_count(0,count)` + `vcadd(mode=1)` + **V→S 排空** + **`get_acc_val`** + 写回 UB + **S→V** + **S→MTE3**。每次调用 3 硬同步 + 1 标量寄存器读 + 与 store 管线耦合 |
| `WholeReduceSum` | `vcadd(mode=0)`，纯 V 指令，无 work buffer，无同步 |
| `BlockReduceSum` / `PairReduceSum` | `vcgadd` / `vcpadd`，同样纯 V（本次未用） |
| mask | normal bit-mask 最多 128 元（FP32 一个 repeat 用 64 位）；count-mode（`set_mask_count`）可表达大 count，但只影响 mode=1 的 acc 累加范围，**不改变 mode=0 的输出粒度** |

### 4.2 反例：V005 首版错误实现（保留为 API-usage 教训）

首版（SOURCE_SHA `fafd1a66253eb74051b12f285992fe157d79000d32d78a411623f986f103e58f`）：

```cpp
// 错误假设：mode=0 + count-mode mask + repeat=1 即「整段一个和写 dst[0]」
AscendCUtils::SetMaskCount<float>();
AscendCUtils::SetMask<float>(0, static_cast<uint64_t>(count));
AscendC::WholeReduceSum<float, false>(dst, src, count, 1, 1, 1, 8);
```

实测结果：所有走 S1/S2/S3 的形状正确性全错（max_abs ~20）；未动路径全对。
原因：`vcadd` mode=0 只写每 64 元一个 partial（count=4096 → 64 个 partial 落在
dst[0..63]，dst[0] 只含前 64 个平方和），下游当成总和使用 → 平方和严重偏小。

正确形态（与生产 rms_norm `ReduceSumHalfInterval` 一致，SOURCE_SHA
`5fa325263d18fc1aa49a7ac0c1130e7cceb5948e4423bba2ee10e6e2f6bfdd0d`）：

```cpp
// 两段式：每 64 元一个 partial → 再收尾成一个和；normal bit-mask ≤64
SetMask<float>(64);
WholeReduceSum<float, false>(work, src, MASK_PLACEHOLDER, full, 1, 1, 8); // full = count/64
// 尾块、收尾同理，最后一次 WholeReduceSum(repeat=1) 得 dst[0]
```

两段式正确性 PASS，但每 tile 2 条 vcadd（vs 父版 1 条 + 同步链）→ 主探针慢 7–8%。
**「一次调用换一次调用」的纯 primitive 交换在 span>64 时硬件上不可实现。**

## 5. 已测 / 未测 / 为何不再测

### 5.1 已测（5 个 Revision + 1 个契约核对）

| 项 | 状态 | 证据 |
|---|---|---|
| V001 eager fold | 已测，NEEDS_ONE_MORE_LOCAL | `本地实验/REDUCE-HIER-X/V001/` |
| V002 手写 Add 树 | 已测，LOCAL_REJECTED | `本地实验/REDUCE-HIER-X/V002/` |
| V003 短 span + 单 barrier | 已测，NEEDS_ONE_MORE_LOCAL + Official 44.24 | `本地实验/REDUCE-HIER-X/V003/`、`线上结果/REDUCE-HIER-X/V003/` |
| V004 分箱单级 | 已测，LOCAL_REJECTED | `本地实验/REDUCE-HIER-X/V004/` |
| V005 primitive 两段式 WRS | 已测，LOCAL_REJECTED | `本地实验/REDUCE-HIER-X/V005/` |
| E3 工具链契约核对 | 已完成（只读） | `研究/REDUCE-HIER-X/E3-API-CONTRACT.md` |

覆盖的站点：S1 `Process()` 通用 first-pass、S2 `ProcessWideFp32FullCacheRows`、
S3 `ProcessFp32FullRowOutputPipelined`（含 FP32/FP16/BF16 经 S1 的路径）。
对照站点（未动、负对照行为正常）：S4 small batched、S5 NarrowMid、BF16/FP16
full-row/full-tile、wide low-precision。

### 5.2 未测

| 项 | 类别 | 未测原因 |
|---|---|---|
| N2 group-span（G 个 tile 平方拼接后一次 ReduceSum） | span 加宽 | 先验极低：与 V003 同轴（span）；V005 已证指令数是主导项，拼接需额外 UB 区且不减指令数。V004 的 UB 教训（不挤 tile 预算）使可行实现更窄 |
| N4 多行同时归约（batched 短行带一次归约出多行 partial） | 结构（行间归约组织） | 形状带（短行多行）与主探针不同，但其结构（多行一次归约）无任何正面证据；且短行带主要成本按 weak-case 分析偏 launch/Init 与 handoff（后者半属 invRms 轴） |
| S4/S5/BF16/FP16/wide-lowprecision 站点上的归约改造 | 站点扩展 | 这些是负对照；S1/S2/S3 的五次证伪已覆盖同类机制，扩站点只是重复 |
| 短行多行带（case 7 类）为**主探针**的归约实验 | 测量带 | 五次实验主探针在 large-D；短行带归约份额小（单 tile 无 collapse），且其可动部分（GetValue/invRms）属 VECTOR-MATH 轴 |
| 父版 msprof 级时间分解（E5） | 瓶颈证据 | 未获授权执行；其信息增量对「已五次证伪」的收口判断不再关键 |

### 5.3 为何不再测（停因）

1. **三个轴的端点全部实测过**（§2.1），没有第四个独立的归约轴剩余。
2. **根因已定位**（§3）：父版 `ReduceSum` 是一条 vcadd + 最小后处理；替代形态指令数
   必不少于它，同步链又不够贵。继续在归约拓扑上找赢面与已有证据矛盾。
3. **Main-2 已定调**：「N2/N4 先验极低，不再自行批准新 Revision」，本 Lane 转
   LANE_NEEDS_PLANNING_REVIEW。
4. 归约侧对 large-D 主瓶颈的贡献已被五次实验证明很小（V003 弱信号 ~0.5–3.3% 即上限）；
   与 registry 既有结论「reduction V/S is NOT the primary bottleneck」一致并加强。

## 6. 若 Planning 要重启本 Lane，需要什么新证据

| 重启条件 | 具体所需 | 取得方式 |
|---|---|---|
| A. 新瓶颈证据 | 某形状上归约侧（ReduceSum 调用/V/S）占 kernel 时间 ≥30% 的测量证据（当前书面估计 20–45% 是推断值，非实测） | 父版 msprof 一次会话（需 Planning/MAIN 授权设备窗口；只跑父版不改码） |
| B. 新结构信息 | 硬件上存在**一条向量指令、输出一个总和、零标量回路**的归约原语（本次 E3 已穷举 Block/Pair/Repeat/WholeReduceSum 与 ReduceSum，结论是不存在 span>64 的单指令向量直写总和） | 若未来工具链新增此类 API 或新指令，需重新核对契约 |
| C. 形状带变更 | 明确指定短行多行（case 7 类）为主探针并给出该带的瓶颈归属证据（归约 vs launch/Init vs invRms handoff） | 书面 case 形状映射实证 + 只读 op-count |
| D. 组合授权 | 明确批准 N4（多行同时归约）单独验证，并接受其形状带与五次实验不同 | Planning 明示 |
| E. 跨轴组合 | 归约改动与另一条 LOCAL_ACCEPTED 机制组合的组合版本授权（OFAT 要求 WHY_THIS_COMBINATION_IS_NEW） | Planning 明示 |

没有 A–E 中任一新证据时，重启归约拓扑实验与五次既有结果矛盾。

## 7. 证据索引

| 内容 | 路径 |
|---|---|
| 五套 Revision 证据包（声明/源码/diff/build/correctness/timing raw） | `本地实验/REDUCE-HIER-X/V001/` … `V005/` |
| V003 线上结果 | `线上结果/REDUCE-HIER-X/V003/` |
| Track-B 假设池（历史 N1–N4） | `研究/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md`、`TRACK-B-HYPOTHESES-V002.md`、`TRACK-B-HYPOTHESES-NEXT.md` |
| E3 工具链契约核对 | `研究/REDUCE-HIER-X/E3-API-CONTRACT.md` |
| V005 回退 + LANE 事实包（前一版） | `本地实验/REDUCE-HIER-X/V005/REVERT-AND-LANE-NEEDS-PLANNING-REVIEW.md` |
| Main-2 批准记录 | `研究/REDUCE-HIER-X/MAIN-APPROVAL-V001/002/003.md` |
| 五次实验的 local-result.json（verdict 根因） | 各 `本地实验/REDUCE-HIER-X/V00N/local-result.json` |

## 8. 本文件边界

只写文档，不改 Kernel、不建新 Revision、不改共享成绩记录、不作 PARK/CLOSE 决定。
路线生命周期决定权在 Planning。当前源码身份 = FROZEN_R31B_V011（`a8c19a19…`）。
