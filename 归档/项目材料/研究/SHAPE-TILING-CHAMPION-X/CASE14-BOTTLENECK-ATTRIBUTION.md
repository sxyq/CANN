# Case 14 瓶颈归属 — 只读研究

日期：2026-09-29
性质：Track-B 只读研究（MAIN-1 指令 B），不产生性能 Revision，不占预算。
用途：供 STORE-EPILOGUE-X / R31B / EPILOGUE-ARITH lanes 与 Planning 参考。
数据来源：V002 多行补测的 same-binary 父版 medians、`线上结果/R31B/V011/submission.asc` 源码结构、
`研究/OFFICIAL-CASE-ANALYSIS.md` 的 case 表。

## 1. 问题

Official case 14：16486.82 µs vs best 3750.12 µs，缺口 4.40x，是全部 15 个 case 里最大的绝对缺口。
它落在「Very wide/very large」段。要回答：这 4.4x 差在哪一段——tile 往返、store、调度、还是归约？

## 2. 已排除：tile 往返不是主导

### 2.1 机制幅度核对（V002 多行补测）

同一 V002 binary，FP32 D=32768，device events，same-binary 父版 medians：

| 形状 | 每核行数（8 核） | 父版 median |
|---|---|---|
| 2×32768 | 1（2 核活跃） | 11.43 µs |
| 8×32768 | 1（8 核活跃） | 12.18 µs |
| 16×32768 | 2（8 核活跃） | 20.46 µs |

每核 1 行 → 2 行的增量 = 20.46 − 12.18 = **8.28 µs/行**（D=32768 的单行成本）。

V002 把 tile 往返从 8 降到 6（-25%）。若每趟往返有 1 µs 级固定开销，应省约 2 µs/行；
即使每趟只有 0.1 µs，也应省约 0.2 µs/行。
实测 paired delta 在 r2/r8 是 −0.08/−0.06 µs（在空白噪声带内），r16 是 −0.16 µs（方向还不稳）。
**实测比机制预测小一个数量级以上** → tile 往返的固定开销 ≤ 单行成本的 5%，不是主导项。

### 2.2 跨 D 的单价

| 形状 | 父版 median | 增量 |
|---|---|---|
| 2×16384 | 7.23 µs | — |
| 2×32768 | 11.43 µs | +4.20 µs |
| 8×16384 | 8.28 µs | — |
| 8×32768 | 12.18 µs | +3.90 µs |

D 翻倍带来约 4 µs/行增量，两组独立测点一致。说明成本随元素数近似线性，
主导项是 **O(D) 的向量工作与每 tile 的同步**，不是「每行固定开销」也不是「每趟往返固定开销」。

## 3. 单行 8.28 µs 花在哪（源码结构枚举）

`ProcessWideFp32FullCacheRows`（`线上结果/R31B/V011/submission.asc:2082-2246`）对 D=32768、tile=4096：

**Pass 1（读 x/res + 算 y + 平方和）**，每 tile：
- 2× `Load`（x、residual）
- `SyncMTE2ToV`
- `Add`（y = x+res）+ `PipeBarrier`
- `Mul`（y²）+ `PipeBarrier`
- `ReduceSum`（tile partial）
- `SyncVToMTE2`

**Pass 2（读 gamma/bias + 归一化输出）**，每 tile：
- 等上一 tile 的 MTE3 排空（2-deep store queue）
- 2× `Load`（gamma、bias）
- `SyncMTE2ToV`
- `Muls`（×invRms）+ `PipeBarrier`
- `Mul`（×gamma）+ `PipeBarrier`
- `Add`（+bias）+ `PipeBarrier`
- `SetFlag/WaitFlag` + `Store` + `SetFlag`（MTE3 流水）

8 个 tile ⇒ 每行约 8×(2 次 Load×2 遍 + 5 个向量算子 + 3 个 PipeBarrier×2 遍 + 2 次完整 MTE 往返 + store 流水)。
按 8.28 µs/行摊：每 tile 约 1.0 µs，其中向量段 + 同步段是主菜，往返固定开销只占很小一部分。

## 4. 归约段可以排除为「主缺口」

- 归约本身是 8 次 `ReduceSum` partial + 1 次收尾 `ReduceSum` + 2 次 `GetValue` + 1 次 `Sqrt`，
  相对每 tile 的 5 个向量算子和 3 个 PipeBarrier，占比不大。
- 但有一个**独立问题**：宽 FP32 归约的 invRms 跨 run 非确定约 0.8%
  （`研究/SHAPE-TILING-CHAMPION-X/WIDE-FP32-NONDETERMINISM.md`）。这影响正确性判定，
  不太可能是 4.4x 性能缺口的来源（BF16 走同一套 partial 索引却逐位可复现，且 BF16 只慢在输出量化）。

## 5. store 段：已知有空间，但不是 4.4x

- STORE-EPILOGUE-X V002 的输出回写合并在 Champion 上是 `LOCAL_ACCEPTED`，Official 45.07
  （相对 45.16 只差 0.09 分）。也就是说 store 侧的最大已知单项收益在这个量级。
- case 14 的 store 流量确实大（宽 D + 多行），但 4.4x 缺口不可能只靠回写合并填平。
- 结论：store 是「可再收的次要项」，不是主缺口。

## 6. 调度 / 核占用：最可疑的主缺口

把 8.28 µs/行的形状成本与 Official case 14 的 16.5 ms 对齐：
16.5 ms ÷ 8.3 µs ≈ **2000 行** 的 D=32768 规模（或等效元素量）。
best 实现 3.75 ms 意味着它的单行成本只有约 1.9 µs——比我们低 4.4x。

我们这一侧的结构性开销候选：

| 候选 | 说明 | 为什么能解释 4.4x |
|---|---|---|
| **每 tile 3 个 `PipeBarrier<PIPE_V>`** | Pass1/Pass2 各 3 个，8 tile ⇒ 每行约 48 次 V 流水停顿 | 若 barrier 一次 50–100 ns，48 次就是 2.4–4.8 µs/行，与 8.28 µs 同量级 |
| **MTE2/V/MTE3 顺序发射** | Pass1 是 Load→等→算→等；Pass2 是算→等→store | 双缓冲/事件流水可以把 Load 与算重叠；ASYNC-TRIPLE-X 在此方向做过但 507035 |
| **逐 tile 的 2 次 `SyncMTE2ToV` / `SyncVToMTE2`** | 每 tile 一对，8 tile ⇒ 16 次硬同步 | 每次若 100 ns 级，就是 1.6 µs/行 |
| **Pass2 的 2-deep store queue 仍要等 MTE3 排空** | 换 tile 的 gamma/bias staging 前必须 drain | 尾部串行 |

这三个都在「调度 / 流水」轴上，而不是 tile 大小、store 合并、归约数学。

## 7. 给各 lane 的结论

| lane | 结论 |
|---|---|
| **SHAPE-TILING-CHAMPION-X** | tile 往返轴已两档无信号 + 多行补测幅度核对失败。**tile 常量类改动应停**。 |
| **R31B / EPILOGUE-ARITH** | 主缺口更可能在每 tile 的 `PipeBarrier` 数量与 MTE 流水深度，而不是 tile 粒度。V011 的 2-deep MTE2/MTE3 事件只覆盖了窄切片。 |
| **STORE-EPILOGUE-X** | store 合并仍是可收项，但已知单项收益是 0.09 分量级，填不满 4.4x。 |
| **Planning** | case 14 需要的是「每 tile 同步次数 / 流水深度」这一类机制，历史上 ASYNC-TRIPLE-X 在此方向失败（507035），需要更强的正确性护栏再试。 |

## 8. 建议的下一步（供 Main/Planning 选，不自行开 Revision）

1. **分段计时**：在同一 paired_runner 里给 Pass1 / Pass2 分别打 event，直接量出两段占比。
   这是只读的测量补全，不改 kernel 数值路径。
2. **barrier 计数枚举**：把每行的 `PipeBarrier` / `Sync*` 次数列成表，与 8.28 µs/行对齐，
   估出每次同步的单价。
3. 若 1、2 证实同步主导 → 下一个性能假设应是「减少每 tile 同步次数 / 加深流水」，
   归 R31B 或 EPILOGUE-ARITH lane，不归本 lane（本 lane 只做 tiling）。

## 证据

- 多行 same-binary medians：`本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/multiscale-same-binary.log`
- 多行 P/C：`本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/multiscale-paired.log`
- 机制幅度核对：`本地实验/SHAPE-TILING-CHAMPION-X/V002/local-result.json` → `MULTISCALE_TIMING.mechanism_magnitude_check`
- 源码：`线上结果/R31B/V011/submission.asc:2082-2246`（`ProcessWideFp32FullCacheRows`）
- case 表：`研究/OFFICIAL-CASE-ANALYSIS.md`
