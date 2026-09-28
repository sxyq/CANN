# REVISION DECLARATION — REDUCE-HIER-X V004

Date: 2026-09-29
Approver: MAIN-2（MAIN-APPROVAL 2026-09-29，路径 A：直接实现，不做父版 profiling）
Route Agent: LANE M2-1 REDUCE-HIER-X（worktree /Users/sunyiyang/Desktop/Project/cann-m2-reduce, branch m2/reduce-hier）

## Declaration block

```text
ROUTE            = REDUCE-HIER-X
REVISION         = V004
REVISION_KIND    = PERFORMANCE_SINGLE_HYPOTHESIS
DIRECT_PARENT    = FROZEN_R31B_V011
PARENT_SOURCE_SHA= a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE     = 45.16 (OFFICIAL_ANCHOR)
CONTEXT_CLASS    = FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
SINGLE_HYPOTHESIS= N1 Binned streaming single-stage square-sum
WHY_NOT_DUPLICATE= see below
```

## SINGLE_HYPOTHESIS（唯一概念性性能变化）

multi-tile 归约从「per-tile ReduceSum → partial bank → collapse ReduceSum」两级结构，
改为单级 binned streaming 累加：

- 每个 tile 的平方向量 `Mul` 结果按固定宽度 `kBinWidth=128`（32B 对齐）分块 `Add`
  进 128 元 FP32 累加器（bin j 收所有 i≡j (mod 128) 的平方）；Add 链零中间
  PipeBarrier（V003 已证安全）。
- 行尾一次 `ReduceSum(acc, work, kBinWidth)` 得到行 squareSum，原有 GetValue 链
  （squareSum + invRms 共 2 次/行）不变。
- 每行归约侧 ReduceSum 调用从 tileCount+1 次（per-tile + collapse）降到 1 次。

只落三个 multi-tile 站点：

| 站点 | 函数 | 现状 |
|---|---|---|
| S1 | `Process()` 通用 first-pass 循环（tileCount≥2 时） | per-tile `ReduceSum(reduceFp32[tileIndex],…,valid)` + 行尾 `ReduceSum(…,tileCount)` |
| S2 | `ProcessWideFp32FullCacheRows` | per-tile `ReduceSum(reduceLocal[batchRow*stride+tile],…)` + per-row collapse |
| S3 | `ProcessFp32FullRowOutputPipelined`（tileCount=2） | per-tile `ReduceSum(reduceLocal[col/kTileElems],…)` + collapse |

**不变**：square Mul、mean/epsilon 尾部、invRms 序列、GetValue 次数、输出遍、
行调度、dtype dispatch、DMA（Load/Store 布局与次数）、tile 宽度、tileElems、
既有 PipeBarrier / SyncVToS / SyncSToV 位置、tileCount==1 的单 tile 路径（负对照，
保持父版结构）。

## OFAT 说明

一个概念变化：「平方如何变成行 squareSum」。附带的 workspace 改动只有一处——
wide FP32 路径的 `reduceFp32Buf_` 尺寸从 `wideFullYRows_*kWideFullYReduceStride`
扩到 `kWideFullYMaxRows*kWideFullYReduceStride + kBinWidth`（+≤512 B），用于承载
跨 tile 存活的累加器。`ChooseWideFullYRows` 的输入与决策（wideFullYRows_、
tileElems）不动，不挤 tile 预算。

## WHY_NOT_DUPLICATE

- ≠ V001 fold：V001 保留 per-tile ReduceSum、用 1 元 Adds 折叠 partial；V004 取消
  per-tile ReduceSum、把原始平方向量分箱累加，结构两级→单级。
- ≠ V002 tree：V002 是 per-tile 手写 Add 树 + per-level PipeBarrier（+6.6% 证伪）；
  V004 无树、零 barrier、跨 tile 分箱。V002 失败根因不适用。
- ≠ V003 short-span：V003 保留 per-tile ReduceSum 并折短 span；V004 不折 span，
  取消 per-tile 调用。
- ≠ REDUCE-INVSCALE-X OPT-4 whole-row（FP32 需 D×4 B 判死）：V004 用 W=128 元
  累加器代替整行缓冲，UB 代价 512 B。
- ≠ UB-LIVENESS-X 向量归约想法（另一 kernel 的死字节处置）。

## EXPECTED_SHAPES / PROBES

主探针：1x32768 FP32（tileCount=8）、1x16384 FP32（4）、8x8192 FP32（2）；
负对照：1x4096 FP32（tileCount=1，不触发，预期 Δ≈0）。
预注册否证：主探针配对中位数未稳定超过噪声带 → 归约份额确认过小。

## UB / DMA / SYNC / PRECISION IMPACT

- UB：S1/S3 用既有 reduceFp32Buf_（4096 float）的前 128 槽作累加器；S2 扩
  reduceFp32Buf_ ≤512 B（见上）。不新建大 buffer，不动 tile 预算。
- DMA：无变化（Load/Store 次数、宽度、时序不动）。
- SYNC：每行 SyncVToS/GetValue 次数不变；ReduceSum 调用次数下降（若其内部含 V/S
  交接则随调用消失）；既有 PipeBarrier 位置全部保留，不新增、不删除。
- PRECISION：低。平方非负，分箱只改求和关联（ULP 级）；须过完整正确性矩阵
  （FP32/FP16/BF16 × multi-tile + single-tile + 非对齐 D）。

## 证据目录

本地实验/REDUCE-HIER-X/V004/（submission.asc、submission.sha256、source-meta.json、
diff.patch、local-result.json、support/）。

## 状态

声明完成 → 源码实现 → server3 compile/link → correctness → same-binary → 交错 P/C
→ 本地结论（规范枚举）→ handoff 回 Main-2。不自行 Online。
