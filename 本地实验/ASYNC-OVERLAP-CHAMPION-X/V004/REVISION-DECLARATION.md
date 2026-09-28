# REVISION-DECLARATION — ASYNC-OVERLAP-CHAMPION-X V004

```text
ROUTE                ASYNC-OVERLAP-CHAMPION-X
REVISION             V004
DIRECT_PARENT        R31B-V011
PARENT_SHA           a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH   线上结果/R31B/V011/submission.asc
PARENT_SCORE         45.16
OFFICIAL_ANCHOR      45.16
CONTEXT_CLASS        CHAMPION_PIPELINE_SCHEDULE
SINGLE_HYPOTHESIS    W1 FullCache inter-pass prologue：
                     在 ProcessWideFp32FullCacheRows 中，把 pass-2 tile-0 的
                     gamma/bias MTE2 提前到 invRms 回路最后一次 Sqrt 之后发出
                     （此时 xBuf_/residualBuf_ 已释放），与 invRms 尾部的标量
                     往返重叠；pass-2 tile-0 不再重复 Load。
APPROVAL             MAIN-2 2026-09-28: W1 APPROVED as V004;
                     不加 buffer/queue；不改 LP（V001 已做）。
PARENT_CHOICE        从 Champion 种子干净起（不继承 V001–V003）。
```

## EXPECTED_SHAPES

- 主测：FP32 wide `rowWidth > kCacheElems(8192)`（FullCache 路径），kernel >20 µs。
  例：128×16384 fp32、256×16384 fp32、128×32768 fp32。
- 对照：不进入 FullCache 的形状（NarrowMid / generic / LP）应无差异。
- **注意**：parent 在 FP32 wide 上有已知非确定路径（MULTIROW UB-GAP-CLUE）；
  正确性对照须避开或标注该路径。

## OFAT_DIFF（唯一概念性变化）

落点：`ProcessWideFp32FullCacheRows`（parent 约 L2139–2193）。

```text
现状：
  invRms 回路（L2139–2156，xBuf_/residualBuf_ 被 V 用作 scratch）
  → SyncVToMTE2()（L2157）
  → pass-2 tile 循环：每 tile Load gamma/bias（L2191–2192）→ SyncMTE2ToV

改为：
  invRms 回路；最后一次 Sqrt+PipeBarrier 之后：
    SyncVToMTE2()            （V 已放 xBuf_/residualBuf_）
    Load gamma/bias tile0    【提前发出，与随后的标量尾重叠】
  → invRms 最后一轮标量尾（SyncVToS/GetValue/SyncSToV）照旧
  → SyncVToMTE2()（L2157 保留，顺序化后续 MTE2）
  → pass-2 tile 循环：tile0 跳过 Load（已在飞），保留 SyncMTE2ToV 等待；
    tile≥1 Load 照旧
```

只改 pass-2 tile-0 参数 Load 的发出时机；不改算术、不改 DMA 字节、
不改 store 暂存、不改 LP 路径。

## 精度

- 算术完全不变（同一 Load、同一 buffer、同一 invRms、同一 pass-2 计算）。
- 应 **bit-identical**；若 OUTHASH 出现差异按 V003 裁定处理（容差内可接受，
  但须先报）。

## WHY_NOT_DUPLICATE

- vs V001（H1 on LP）：同机制不同路径（FullCache vs LP）；Main-2 明示
  「不改 LP（V001 已做）」，本假设是 H1 在 FP32 wide 的延伸。
- vs V002（W2 NarrowMid 行间发射）：不同函数不同边界。
- vs V003（W4 GetValue handoff）：不动标量往返次数。
- 不加 buffer/queue/MTE3 stage。

## UB / DMA / SYNC IMPACT

| 项 | 影响 |
|---|---|
| UB | 不变 |
| DMA | 字节不变；tile-0 参数 Load 提前到 invRms 尾 |
| CORE | 不变 |
| SYNC | 多一次 SyncVToMTE2（提前 Load 前）；pass-2 tile0 的 Load 删除，SyncMTE2ToV 保留 |
