# REVISION-DECLARATION — ASYNC-OVERLAP-CHAMPION-X V001

```text
ROUTE                ASYNC-OVERLAP-CHAMPION-X
REVISION             V001
DIRECT_PARENT        R31B-V011
PARENT_SHA           a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH   线上结果/R31B/V011/submission.asc
PARENT_SCORE         45.16 (Official, 15/15)
OFFICIAL_ANCHOR      45.16
CONTEXT_CLASS        CHAMPION_PIPELINE_SCHEDULE
SINGLE_HYPOTHESIS    H1 Inter-pass prologue param prefetch:
                     在 ProcessWideLowPrecision 中，把 pass-2 tile-0 的 gamma/bias
                     MTE2 从 invRms 标量回路之后提前到回路之前发出，使参数 DMA 与
                     invRms 回路重叠。pass-2 入口只 WaitFlag 已在飞的 load。
APPROVAL             MAIN-2 2026-09-28: H1 APPROVED as V001; LP path only;
                     FullCache 同类改动留给后续 Revision。
```

## EXPECTED_SHAPES

- 主测：large-R wide FP16/BF16（进入 `ProcessWideLowPrecision` 的唯一门是
  `rowWidth > kCacheElems(8192)` 且 T∈{half,bf16}）。
- tileCount = ceil(rowWidth / kWideFullYTileElems(4096))；LP 路径最小 tileCount=3
  （rowWidth>8192）。主推 rowWidth=16384/32768，rows 取较大值以获得 batchRows>1。
- 对照：不进入 LP 的形状（如 FP16 width≤8192 或 FP32 wide）应无任何指令流变化，
  用作 code-path 对照；不作主收益主张。

## OFAT_DIFF（唯一概念性变化）

落点：`ProcessWideLowPrecision`（submission.asc 约 L3076–3365）。

```text
现状顺序：
  pass-1 块结束（rel0/rel1 已 Wait，xBuf_/residualBuf_ 空闲）
  → invRms 标量回路（L3224–3242）
  → SyncVToMTE2()（L3243）
  → 分配 prd0/prd1/prel0/prel1 + gBase/bBase
  → Load gamma/bias tile0 + SetFlag MTE2_V(prd0)（L3260–3265）
  → pass-2 tile 循环

改为（H1）：
  pass-1 块结束（rel0/rel1 已 Wait）
  → 分配 prd0/prd1/prel0/prel1 + gBase/bBase
  → Load gamma/bias tile0 + SetFlag MTE2_V(prd0)     【发射点提前】
  → invRms 标量回路（与参数 DMA 重叠）
  → （删除原 SyncVToMTE2：其唯一作用是把参数 Load 排到 invRms V 之后；
     现 Load 已提前，顺序改由 pass-1 的 rel0/rel1 释放保证）
  → pass-2 tile 循环（首 tile 只 WaitFlag 已在飞 load）
```

多处编辑共同实现同一假设（发射点移动 + 收窄对应 sync），`SINGLE_CHANGE_AUDIT` 以
本文件为准判定为同一概念性变化。

## WHY_NOT_DUPLICATE

- vs R013：R013 是两段 double-buffer，无 inter-pass 发射重排。
- vs ASYNC-TRIPLE-X V001：V001 是「只加 MTE3 stage」；H1 分类从未在 R31B-V011 落地。
- vs MIX-A V003/V007：不同路径、不同目的（release 增加 / Load 前删 sync ≠ 参数 prefetch 提前）。
- vs R31B V006/V009：不改 queue depth（T14 proven-flat）。
- 不加 buffer、不加 queue、不加 MTE3 stage。

## UB / DMA / SYNC IMPACT

| 项 | 影响 |
|---|---|
| UB | 不变（不新增、不改大小；仍复用 xBuf_/residualBuf_ 作 gBase/bBase） |
| DMA | 字节不变；仅 gamma/bias tile-0 的发出时间提前到 invRms 之前 |
| CORE | 行分配不变 |
| SYNC | 删除 L3243 SyncVToMTE2；参数 Load 改由 pass-1 rel0/rel1 释放排序。事件：prd0 MTE2_V 在 Load 后 Set；pass-2 首 tile WaitFlag 同一事件 |

## PRECISION_RISK

无。同一数据、同一算术顺序、同一 inverseRms；只改 DMA 发出时机。

## 数据依赖审计（实现前）

- pass-1 对 xBuf_/residualBuf_ 的 V 读在 rel0/rel1 WaitFlag（L3212–3217）后结束。
- invRms 回路只碰 xFp32Buf_ / residualFp32Buf_ / reduceFp32Buf_，与 gBase=xBuf_、
  bBase=residualBuf_ 无重叠。
- gamma/bias GM 读不依赖 inverseRms。
- 因此提前 Load 无 RAW/WAW 风险；原 SyncVToMTE2 对该 Load 而言是多余的。
