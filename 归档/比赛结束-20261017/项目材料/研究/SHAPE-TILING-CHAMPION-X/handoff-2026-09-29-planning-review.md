# SHAPE-TILING-CHAMPION-X Handoff — 2026-09-29（分段计时 + LANE_NEEDS_PLANNING_REVIEW）

来源：MAIN-1 DIRECTION（指令 C 分段计时 → 指令 D 分向）。
本文件是闭环 handoff，同时是 `LANE_NEEDS_PLANNING_REVIEW` 上报（本路线不自行 PARK）。

## 指令 C：分段计时结论

完整数据见 `研究/SHAPE-TILING-CHAMPION-X/CASE14-SEGMENTED-TIMING.md`。

**Pass1（平方和归约遍）≈ 63–73%，Pass2（归一化+affine+store）≈ 26–38%。**

| 形状 | full | pass1only | pass2 | Pass1 | Pass2 |
|---|---|---|---|---|---|
| 2×32768 | 12.32 µs | 7.78 | 4.70 | 63.2% | 38.2% |
| 8×32768 | 12.34 µs | 8.44 | 3.94 | 68.4% | 31.9% |
| 16×32768 | 20.54 µs | 13.06 | 7.50 | 63.6% | 36.5% |
| 8×16384 | 8.04 µs | 5.90 | 2.12 | 73.4% | 26.4% |
| 2×16384 | 7.66 µs | 5.36 | 2.26 | 70.0% | 29.5% |

**同步/流水已确认主导**，三条证据合看：

1. tile 往返 8→6 只省 0.16 µs → 同步的自身延迟不是大头（每 tile 固定开销 ≈ 0.08 µs）。
2. msprof 管线合计 **1.23 / 4**（vec 0.463、mte2 0.443、scalar 0.157、mte3 0.170）→ 平均只 1.23 条管线在忙，
   各自 16–46%。同步把 MTE2 / V / MTE3 排成了串行。完全重叠的上限约 **2.16×**。
3. 每行约 **110 次**同步/barrier 原语（41 `PipeBarrier<PIPE_V>` + 16 `SyncMTE2ToV` + 9 `SyncVToMTE2`
   + 4 `SyncVToS/SyncSToV` + 16 `SetFlag` + ~24 `WaitFlag`），Pass1/Pass2 两遍都是
   Load→等→算→等→Store 的顺序结构。

case 14 的 4.4× ≈ 2.16×（重叠空间）× 约 2×（标量尾 15.7% + 每 tile 3 个 `PipeBarrier` 融合
+ 向量算子序列去 barrier）。只做一半拿不到 4.4×。

## 指令 D：LANE_NEEDS_PLANNING_REVIEW

**本 lane 的 tiling 轴已证伪，建议 Planning 审视路线去留。** 理由：

| 项 | 事实 |
|---|---|
| 已用预算 | 闭环性能 Revision 2/6（V001、V002） |
| 连续无收益 | 2，但第 3 轮（多行补测 + 分段）产出了机制证伪，不是空转 |
| tile 轴证伪 | H1（D≤16384 档）与 H2（D=32768 档）均无信号；多行补测 r2/r8/r16 均在噪声带内或方向不稳；机制幅度核对显示实测比预测小一个数量级 |
| 剩余假设 | H3 SPLIT_D / H4 MERGE_N 正确性风险高（历史五模式全败 + 宽 FP32 归约非确定），且属 reduction/归约边界，按分工不归本 lane |
| H5 预算矩阵 | 单点档位均无信号，扫档性价比低，建议不做 |
| 真实主缺口 | case 14 的 4.4× 在**管线重叠**（约 2.16×）+ **标量尾/barrier 融合**（约 2×），属 R31B / EPILOGUE-ARITH 边界 |

**本路线不自行 PARK / CLOSE / MERGE。** 请 Planning 裁定：

1. 关闭本 lane 并把 tile-UB 结论并入技术知识库；或
2. 保留 slot 但改派新主题（需 Planning 重新指定，本 lane 只做 tiling/tile-policy）；或
3. 与 R31B / EPILOGUE-ARITH 合并（tile 轴的证伪结论作为其输入）。

## 交给其他 lane 的输入

| 对象 | 可直接用的东西 |
|---|---|
| R31B / EPILOGUE-ARITH | Pass1/Pass2 占比表；管线合计 1.23 的 msprof 分解；每行 110 次同步的分类计数；2.16× 重叠上限 |
| STORE-EPILOGUE-X | Pass2 占 26–38%，store 流水是 Pass2 的一部分；合并回写仍可收但主战场在重叠 |
| Planning | 单行 D=32768 成本 8.28 µs、D 翻倍 +4 µs 的定价；机制幅度核对方法论（已采纳为 lane 标准） |

## 本轮新增证据

- `研究/SHAPE-TILING-CHAMPION-X/CASE14-SEGMENTED-TIMING.md`（分段计时 + 管线分解 + 同步计数）
- `本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/segmented.log`（31 交错 pair × 5 形状原始样本）
- `本地实验/SHAPE-TILING-CHAMPION-X/V002/support/segmented/`（仪表件 `parent_pass1only.asc` SHA `180cc47c…` + seg_runner）
- 前序：`CASE14-BOTTLENECK-ATTRIBUTION.md`、`handoff-2026-09-29-multiscale.md`、`handoff-2026-09-28.md`

## 预算与状态

- 闭环性能 Revision：**2/6**（V001、V002）。本轮分段计时是只读测量补全，不占预算。
- 连续无收益：2。第三轮为机制证伪 + 分段实测，产出信息增量。
- 状态：**`LANE_NEEDS_PLANNING_REVIEW`**（等 Planning 裁定，本路线不自行 PARK）。
