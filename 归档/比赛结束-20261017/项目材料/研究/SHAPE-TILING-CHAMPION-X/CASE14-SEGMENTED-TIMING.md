# Case 14 分段计时 — 同步/流水主导确认

日期：2026-09-29
性质：Track-B 只读测量补全（MAIN-1 指令 C），不改 kernel，不占性能 Revision 预算。
对象：R31B-V011 parent（`a8c19a19…`），原 `submission.asc` 逐字节未改。
仪表件：`本地实验/SHAPE-TILING-CHAMPION-X/V002/support/segmented/parent_pass1only.asc`
（在 `ProcessWideFp32FullCacheRows` 的 batch 循环内、Pass2 之前插一条 `continue`，跳过输出遍。SHA `180cc47c…`，仅作测量，不是 Candidate）。

## 1. 分段定义

`ProcessWideFp32FullCacheRows`（`线上结果/R31B/V011/submission.asc:2082-2246`）的两遍：

- **Pass1（平方和归约遍）**：读 x/residual → y = x+res → y² → per-tile `ReduceSum` partial → 收尾 `ReduceSum` → invRms。
- **Pass2（归一化 + affine + store 遍）**：读 gamma/bias → y×invRms → ×gamma → +bias → 2-deep MTE3 流水写 output。

分段方法：同一 runner 内交错测量 `full` 与 `pass1only`（device events，warmup=45，31 交错 pair），
`pass2 = full − pass1only`。full 的 median 与此前 same-binary 的 median 对齐（如 16×32768：20.54 vs 20.46），
说明仪表件没有引入额外扰动。

## 2. 分段占比（device events，device 4）

| 形状 | full | pass1only | pass2 | **Pass1 占比** | **Pass2 占比** |
|---|---|---|---|---|---|
| 2×32768 | 12.32 µs | 7.78 µs | 4.70 µs | **63.2%** | 38.2% |
| 8×32768 | 12.34 µs | 8.44 µs | 3.94 µs | **68.4%** | 31.9% |
| 16×32768 | 20.54 µs | 13.06 µs | 7.50 µs | **63.6%** | 36.5% |
| 8×16384 | 8.04 µs | 5.90 µs | 2.12 µs | **73.4%** | 26.4% |
| 2×16384 | 7.66 µs | 5.36 µs | 2.26 µs | **70.0%** | 29.5% |

结论：**Pass1 ≈ 63–73%，Pass2 ≈ 26–38%**。两遍都不是可以忽略的尾巴；Pass1 更重但 Pass2 超过三成。

pass1only 随行数正确缩放（8× → 16× 的 pass1only 从 8.44 到 13.06），确认 `continue` 插在了行循环内，
多行全部计入。

## 3. 管线占比（msprof ai-core，同一 binary，未改动）

对 parent kernel 的 535 个 task 样本按耗时分桶后取中位数：

| 桶 | aiv_vec | aiv_scalar | aiv_mte2 | aiv_mte3 | 管线合计 |
|---|---|---|---|---|---|
| <8 µs | 0.401 | 0.237 | 0.368 | 0.149 | 1.155 |
| 8–10.5 µs | 0.444 | 0.189 | 0.410 | 0.163 | 1.206 |
| ≥13 µs（case-14 同量级） | **0.463** | **0.157** | **0.443** | **0.170** | **1.233** |

字段：`aiv_vec_ratio`（向量计算）、`aiv_scalar_ratio`（标量）、`aiv_mte2_ratio`（DMA 读）、`aiv_mte3_ratio`（DMA 写）。
原始数据：`msprof --ai-core=on --task-time=l1`，见 `本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/` 下的导出。

**读法**：每条管线的 busy 时间占墙钟的比例。四条相加 = 1.23，意味着任一时刻平均只有 **1.23 条管线在忙**，
而硬件可同时跑 V / MTE2 / MTE3 / S。四条各自只有 16–46% busy，说明**大量时间是空转等待同步**，
不是某条管线打满。

若四条完全重叠，墙钟 ≈ max(busy) = 0.463 × 当前 → **约 2.16× 的空间**，且这是不改任何算法、
只把现有同步排布拉开重叠就能拿到的上限。

## 4. 每行同步/barrier 计数（源码枚举，D=32768，tile=4096，N=1）

`ProcessWideFp32FullCacheRows` 的循环体展开后：

**Pass1 每 tile**：2 Load、1 `SyncMTE2ToV`、2 `PipeBarrier<PIPE_V>`、1 `ReduceSum`、1 `SyncVToMTE2`
**Pass1 每行尾**：1 `ReduceSum`、2 `SyncVToS`、2 `SyncSToV`、1 `PipeBarrier`、1 `SyncVToMTE2`
**Pass2 每 tile**：2 Load、1 `SyncMTE2ToV`、3 `PipeBarrier<PIPE_V>`、1 Store、2 `SetFlag`、约 3 `WaitFlag`

8 tile 合计每行：

| 原语 | 次数/行 |
|---|---|
| `PipeBarrier<PIPE_V>` | 41 |
| `SyncMTE2ToV` | 16 |
| `SyncVToMTE2` | 9 |
| `SyncVToS` + `SyncSToV` | 4 |
| `SetFlag` | 16 |
| `WaitFlag` | ~24 |
| **合计同步/barrier 类** | **约 110** |
| Load / Store | 40 |
| `ReduceSum` | 9 |

按单行 8.28 µs 摊，每次同步类原语约 75 ns。数量级合理，但**关键不在单次延迟，在串行化**。

## 5. 三个证据合看：为什么判「同步/流水主导」

| 证据 | 说明 | 指向 |
|---|---|---|
| tile 往返 8→6 只省 0.16 µs | 每 tile 固定开销（含其同步）≈ 0.08 µs，占单行 8.28 µs 的 1% | 同步的**自身延迟**不是大头 |
| 管线合计 1.23 / 4 | 平均只有 1.23 条管线在忙，各自 16–46% | 同步在**串行化管线**，损失 2.16× |
| Pass1 63–73% / Pass2 26–38% | 两遍都重，且两遍内部各自都是 Load→等→算→等→Store 的顺序结构 | 两遍都没吃到跨管线重叠 |

**结论：同步/流水是主缺口。** 但要说清楚是哪种「同步问题」：

- 不是「同步指令太多、单次太慢」——那样减 tile 往返会有效，实测无效。
- 是「同步把 MTE2 / V / MTE3 排成了串行」——管线合计 1.23 是直接证据。
  只要把 Pass1 的 Load 与算重叠、Pass2 的算与 Store 重叠（深度 ≥2 的双缓冲 / 事件流水），
  不改任何数学就有约 2.16× 空间。

**还差的另一半**：2.16× 距 case 14 的 4.4× 还有约 2×。剩余来源候选：
- 标量段 15.7%（`GetValue` / `Duplicate` / `Sqrt` / invRms 尾部）在行数少时摊不薄；
- 每 tile 的 3 个 `PipeBarrier<PIPE_V>` 可以用事件替代，让 V 段内部也流水；
- 向量算子序列（Add→Mul→ReduceSum / Muls→Mul→Add）每步一个 barrier，可融合或去 barrier。

## 6. 给 Main / 各 lane 的话

| 对象 | 结论 |
|---|---|
| **SHAPE-TILING-CHAMPION-X** | tiling 轴已证伪（两档 tile + 多行补测 + 机制幅度核对）。本 lane 不再有可开的 OFAT 假设（H3/H4 正确性风险高且属 reduction/归约边界）。**建议 `LANE_NEEDS_PLANNING_REVIEW`。** |
| **R31B / EPILOGUE-ARITH** | 主缺口确认为**管线重叠不足**。现有 2-deep MTE2/MTE3 事件只覆盖窄切片，Pass1/Pass2 的主体仍是串行。目标：管线合计从 1.23 拉向 3–4。上限约 2.16×。 |
| **STORE-EPILOGUE-X** | Pass2 占 26–38%，其中 store 流水是 Pass2 的一部分；合并回写仍可收，但主战场在重叠。 |
| **Planning** | case 14 的 4.4x = 约 2.16×（重叠）× 约 2×（标量尾 + barrier 融合 + 算子序列）。两半都需要，只做一半拿不到 4.4x。 |

## 7. 方法说明与限制

- 分段用的是**仪表副本**（插一条 `continue`），不是改动 parent 源码。原 `submission.asc` SHA 仍是 `a8c19a19…`。
- 减法得到的 Pass2 份额包含 Pass1/Pass2 之间的重叠损失，因此是「可归因到 Pass2 的增量」，不是 Pass2 的纯执行时间。
- msprof 的管线占比是**全 kernel** 的，没有按 Pass1/Pass2 拆分；与减法结果合看互补。
- 管线合计 1.23 的「2.16× 空间」是理想完全重叠的上限，实际受数据依赖与 UB 容量约束会打折扣。

## 证据

- 分段原始样本：`本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/segmented.log`
- 仪表件：`本地实验/SHAPE-TILING-CHAMPION-X/V002/support/segmented/parent_pass1only.asc`（SHA `180cc47c…`）
- 仪表 runner：`本地实验/SHAPE-TILING-CHAMPION-X/V002/support/segmented/{seg_runner.cpp,seg_pair.asc,CMakeLists.txt}`
- msprof：`msprof --application=run_one.sh --ai-core=on --task-time=l1`，输出 `op_summary_*.csv`
- 源码：`线上结果/R31B/V011/submission.asc:2082-2246`
- 前序：`研究/SHAPE-TILING-CHAMPION-X/CASE14-BOTTLENECK-ATTRIBUTION.md`
