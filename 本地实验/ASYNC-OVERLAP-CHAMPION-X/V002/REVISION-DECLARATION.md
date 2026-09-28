# REVISION-DECLARATION — ASYNC-OVERLAP-CHAMPION-X V002

```text
ROUTE                ASYNC-OVERLAP-CHAMPION-X
REVISION             V002
DIRECT_PARENT        R31B-V011
PARENT_SHA           a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH   线上结果/R31B/V011/submission.asc
PARENT_SCORE         45.16 (Official, 15/15)
OFFICIAL_ANCHOR      45.16
CONTEXT_CLASS        CHAMPION_PIPELINE_SCHEDULE
SINGLE_HYPOTHESIS    W2 NarrowMid 行间发射重排：
                     在 ProcessNarrowMidOverlap 中，把下一行的 x/res MTE2 提前到
                     当前行 invRms 标量回路之前发出（inputRelease 之后），使输入 DMA
                     与 invRms 标量尾重叠；同时把 SyncMTE3ToV 从循环头下沉到真正
                     覆写 store 源缓冲之前。BF16 因 store 读 xBuf_ 保持原序。
APPROVAL             MAIN-2 2026-09-28: W2 APPROVED as V002; OFAT 只改
                     ProcessNarrowMidOverlap 行间发射顺序/同步位置。
PARENT_CHOICE        从 Champion 种子干净起（不继承 V001）：V001 是新机制落点
                     （NarrowMid，V001 未触碰），且 V001 的 H1 在 Official 已 REJECT，
                     不把 H1 假阳性面带进 V002。Main-2 默认此选择。
```

## EXPECTED_SHAPES

- 主测：NarrowMid 带 `rowWidth ∈ (128, 4096]`，rows 取到 kernel >20 µs。
  对准 Official case 6/7/4/8（16–70 µs 档）。
- 控制组：机制不触发的形状（如 width≤128 走 generic，或 width>8192 走 wide），
  应与 parent 无差异。
- dtype 覆盖 FP32 / FP16 为主（BF16 路径本次不改）。

## OFAT_DIFF（唯一概念性变化）

落点：`ProcessNarrowMidOverlap`（parent 约 L499–620）。

```text
现状（每行）：
  [loop 头] 若上一行在飞：WaitFlag V_MTE2(inputRelease) → SyncMTE3ToV()
  Load x/res → SetFlag MTE2_V(inputReady)
  （非 resident）Load 参数 → SetFlag MTE2_V(paramReady)
  WaitFlag inputReady → V 计算 → SetFlag V_MTE2(inputRelease)
  invRms 标量尾（SyncVToS/GetValue ×2 往返）
  （非 resident）WaitFlag paramReady
  输出计算 → SyncVToMTE3() → Store

改为（FP32 / FP16）：
  [prologue] 先发第 0 行 Load + SetFlag inputReady
  每行：
    （非 resident）Load 参数 + SetFlag paramReady
    WaitFlag inputReady → V 计算 → SetFlag V_MTE2(inputRelease)
    **若有下一行：WaitFlag inputRelease → Load 下一行 x/res → SetFlag inputReady**
      （此刻发出，与随后的 invRms 标量尾重叠）
    invRms 标量尾
    （非 resident）WaitFlag paramReady
    SyncMTE3ToV()        （下沉：只在覆写 store 源 valueLocal/outputBuf_ 之前）
    输出计算 → SyncVToMTE3() → Store

BF16（store 读 xBuf_）：保持原序不变，不做提前 Load。
```

多处编辑共同实现同一假设（提前发下一行 Load + 下沉 store 等待），属同一概念性变化。

## WHY_NOT_DUPLICATE

- vs V001（H1 inter-pass prologue on LP）：不同函数（NarrowMid vs LP）、不同边界
  （行间 vs pass 间）、不同机制段。V001 未触碰 NarrowMid。
- vs MIX-A V003/V007：那是 release 增加 / Load 前删 sync，目的为正确性/dispatch；
  本假设是为 overlap 的发射重排。
- vs R31B V006/V009：不改 queue depth（T14 proven-flat）。
- vs ASYNC-TRIPLE-X：不加 MTE3 stage。
- 不加 buffer、不加 queue、不加 MTE3 stage、不动 reduction/math/store merge。

## UB / DMA / SYNC IMPACT

| 项 | 影响 |
|---|---|
| UB | 不变 |
| DMA | 字节不变；下一行 x/res Load 的发出时间提前到当前行 invRms 之前 |
| CORE | 行分配不变 |
| SYNC | SyncMTE3ToV 从循环头移到输出计算前；新增一次 inputRelease Wait 以便提前 Load；事件总量不增 |

## PRECISION_RISK

无。同一数据、同一算术顺序；只改 DMA 发出时机与 store 等待位置。

## 数据依赖审计

- xBuf_/residualBuf_ 的 V 读在 `SetFlag V_MTE2(inputRelease)` 后结束；
  提前 Load 只需 WaitFlag 该 release。
- invRms 标量尾只碰 xFp32Buf_/residualFp32Buf_/reduceLocal，与 xBuf_/residualBuf_ 无重叠。
- FP32 store 源是 valueLocal；FP16 store 源是 outputBuf_；两者与下一行 Load 的
  目标 xBuf_/residualBuf_ 不重叠 → 下沉 SyncMTE3ToV 后下一行 Load 可先行。
- BF16 store 源是 xBuf_（与 Load 目标冲突）→ 该 dtype 不改，避免引入别名竞争。
