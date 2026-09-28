# REVISION-DECLARATION — ASYNC-OVERLAP-CHAMPION-X V003

```text
ROUTE                ASYNC-OVERLAP-CHAMPION-X
REVISION             V003
DIRECT_PARENT        R31B-V011
PARENT_SHA           a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH   线上结果/R31B/V011/submission.asc
PARENT_SCORE         45.16
OFFICIAL_ANCHOR      45.16
CONTEXT_CLASS        CHAMPION_PIPELINE_SCHEDULE
SINGLE_HYPOTHESIS    W4 GetValue handoff 收敛：
                     在 ProcessNarrowMidOverlap 的 invRms 尾，把两轮
                     SyncVToS→GetValue→SyncSToV 往返合并为一轮：
                     一次 SyncVToS 读出 squareSum，在标量侧算出
                     meanSquare 与 invRms（含 sqrt），一次 SyncSToV 后直接
                     用于 Muls。算术顺序与运算对象保持不变（同一平方和、
                     同一 invRowWidth+epsilon、同一 1/sqrt、同一 Muls）。
APPROVAL             MAIN-2 2026-09-28: W4 APPROVED as V003; OFAT 只改标量
                     handoff 次数/合并方式；精度风险高，必须 OUTHASH 逐位对照，
                     任何数值差异先停报。
PARENT_CHOICE        从 Champion 种子干净起（不继承 V001/V002）：本机制是
                     per-row 标量往返轴，与 H1/W2 正交。
```

## EXPECTED_SHAPES

- 主测：NarrowMid 带 `rowWidth ∈ (128, 4096]`，localRows≥2/核。
  按 Main 指定：512×2048 fp32、512×1024 fp32（SB 稳定）+ 128×2048。
- 对照：不进入 NarrowMid 的形状（wide / generic）应无差异。
- dtype：FP32/FP16（BF16 的 store xBuf_ 别名不属本假设，不动）。

## OFAT_DIFF（唯一概念性变化）

落点：`ProcessNarrowMidOverlap` 的 invRms 尾（parent 约 L564–574）。

```text
现状（每行两轮往返）：
  SyncVToS()
  squareSum = reduceLocal.GetValue(0)
  meanSquare = squareSum * invRowWidth + epsilon
  SyncSToV()
  Duplicate(xFp32, meanSquare, 1)
  Sqrt(xFp32, xFp32, 1)
  PipeBarrier
  SyncVToS()
  invRms = 1.0f / xFp32.GetValue(0)
  SyncSToV()

改为（每行一轮往返）：
  SyncVToS()
  squareSum = reduceLocal.GetValue(0)
  meanSquare = squareSum * invRowWidth + epsilon
  invRms = 1.0f / std::sqrt(meanSquare)      // 同一公式，标量侧完成
  SyncSToV()
  （删除 Duplicate / Sqrt / 中间的 SyncVToS / 第二次 GetValue / SyncSToV）
```

只改标量 handoff 次数与 Duplicate/Sqrt 的落侧；`Muls(valueLocal, valueLocal, invRms, valid)`
调用不变。不改 DMA、不改 store 暂存、不改 BF16 别名。

## 精度风险（高，按 Main 要求）

- 现状：`std::sqrt` 由向量 `Sqrt` 完成，`1.0f /` 由标量除法完成。
- 改后：`std::sqrt` 由标量完成，`1.0f /` 同一标量除法。
- **sqrt 的执行侧从 Vector 变为 Scalar**，是本假设唯一可能引入数值差异的点。
- 实现后必须 OUTHASH 逐位对照 parent；**任何 bit 差异立即停下报 Main-2**，
  不继续测时、不自行换方案。

## WHY_NOT_DUPLICATE

- vs V001（H1 inter-pass prologue）：不同轴（标量往返 vs DMA 发射时机）。
- vs V002（W2 行间发射重排）：W2 已终局混杂；本假设砍的是 GetValue 往返本身。
- vs VECTOR-MATH-X（invRms 数学序列 / vector denominator）：那是把 meanSquare
  移到向量侧（改公式路径）；本假设是减少标量往返次数，公式与运算对象不变。
- vs REDUCE-HIER：不动 ReduceSum 拓扑。
- 不加 buffer/queue/MTE3 stage。

## UB / DMA / SYNC IMPACT

| 项 | 影响 |
|---|---|
| UB | 不变（xFp32 缓冲仍在，只是本段不再用它做 Duplicate/Sqrt） |
| DMA | 不变 |
| CORE | 不变 |
| SYNC | 每行 invRms 尾的 V↔S 往返从 2 轮减到 1 轮（少一对 SyncVToS/GetValue/SyncSToV） |
