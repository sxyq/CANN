# REVISION DECLARATION — REDUCE-HIER-X V005

Date: 2026-09-29
Approver: MAIN-2（MAIN-APPROVAL 2026-09-29：E3 PASS，批准 N3 实现）
Route Agent: LANE M2-1 REDUCE-HIER-X（worktree /Users/sunyiyang/Desktop/Project/cann-m2-reduce, branch m2/reduce-hier）
E3 前置: 研究/REDUCE-HIER-X/E3-API-CONTRACT.md（PASS）

## Declaration block

```text
ROUTE            = REDUCE-HIER-X
REVISION         = V005
REVISION_KIND    = PERFORMANCE_SINGLE_HYPOTHESIS
DIRECT_PARENT    = FROZEN_R31B_V011
PARENT_SOURCE_SHA= a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE     = 45.16 (OFFICIAL_ANCHOR)
CONTEXT_CLASS    = FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
SINGLE_HYPOTHESIS= N3 ReduceSum → WholeReduceSum primitive 替换
WHY_NOT_DUPLICATE= see below
```

## SINGLE_HYPOTHESIS（唯一概念性性能变化）

**只换归约 primitive 及其后处理链。** 站点、span、两级拓扑、指令数全不动。

| | 父版（现行） | V005 |
|---|---|---|
| 调用 | `AscendC::ReduceSum(dst, src, work, count)` | `WholeReduceSum`（vcadd mode=0）经 helper |
| 硬件指令 | `vcadd(mode=1)` 结果进硬件 acc | `vcadd(mode=0)` 结果直写 dst（UB 向量侧） |
| 后处理链 | V→S 排空 + `get_acc_val()` 标量读 + 写回 + S→V + S→MTE3 | **无** |
| vcadd 条数/站点/span | — | **完全相同** |
| work buffer 参数 | 需要（sharedTmpBuffer） | 不需要 |

mask 设置与工具链 `ReduceSumImpl` 在 dav-2201 上完全同款：`set_mask_count` +
`set_vector_mask(0, count)`，调用后 `set_mask_norm` + 满 mask 恢复。因此 span 处理
（含非 2 幂尾块）与父版逐指令一致。helper `VectorWholeReduceSum(dst, src, count)` 是
该 primitive 的唯一落点。

### 落点（只这三个站点）

| 站点 | 函数 | 替换的调用 |
|---|---|---|
| S1 | `Process()` 通用 first-pass | per-tile `ReduceSum(reduceFp32[tileIndex],xFp32,residualFp32,valid)` + 行尾 `ReduceSum(scalarLocal,reduceFp32,xFp32,tileCount)` |
| S2 | `ProcessWideFp32FullCacheRows` | per-tile `ReduceSum(reduceLocal[batchRow*stride+tile],xLocal,residualLocal,valid)` + per-row collapse `ReduceSum(residualLocal,reduceLocal[batchRow*stride],xLocal,tileCount)` |
| S3 | `ProcessFp32FullRowOutputPipelined` | per-tile `ReduceSum(reduceLocal[col/kTileElems],xFp32,residualFp32,kTileElems)` + collapse `ReduceSum(residualFp32,reduceLocal,xFp32,kCacheElems/kTileElems)` |

**不动**：S4（small batched）、S5（NarrowMid）、BF16/FP16 full-row/full-tile 路径、
wide low-precision 路径——这些站点继续用 `ReduceSum`，作对照。square Mul、mean/epsilon、
invRms、GetValue 次数（2/行）、输出遍、调度、dtype、DMA、tileElems、既有
PipeBarrier/SyncVToS/SyncSToV 位置全部不动。

## 预期收益模型

multi-tile 行（tileCount=T）每行归约侧硬同步：`(T+1)×(V/S+S/V+S/MTE3)` → `1`（仅保留
既有行尾 GetValue 的一次 V/S）。T=8（1x32768）：27 次硬同步 → 1；T=2（8x8192）：9 → 1。
同时解除归约与 MTE3 store 管线的耦合。vcadd 指令数不变。

## OFAT / SINGLE_CHANGE_AUDIT 预登记

一个概念变化：归约 primitive 与它的后处理链。mask 设置保留（与 ReduceSumImpl 同款，
不是新变量）；work buffer 参数随 primitive 消失（同一变量）；helper 是唯一实现落点。

## WHY_NOT_DUPLICATE

- ≠ V001 fold（折叠时机）、≠ V003 short-span（span）：V005 不动这两者。
- ≠ V002 tree / V004 binned：那两次是手工分解归约（指令数大增，均已证伪）；V005 仍是
  每个原调用点一条 vcadd，指令数不变，只去掉 vcadd 之后的同步/标量链。
- E3 新证据：`ReduceSumImpl`（dav-2201）每次调用含 3 硬同步 + `get_acc_val` + S_MTE3
  耦合；`WholeReduceSum`→`vcadd(mode=0)` 无此链。该成本项 V001–V004 未直接攻击过。

## EXPECTED_SHAPES / PROBES

主探针沿用：1x32768 FP32（tileCount=8）、1x16384 FP32（4）、8x8192 FP32（2）；
对照：1x4096 FP32（走 S5，未动）。预注册否证：主探针配对中位数未稳定超过噪声带
→ 归约 primitive 后处理链不是可回收成本，归约轴收口。

## UB / DMA / SYNC / PRECISION IMPACT

- UB：无新分配；work buffer 参数不再传入（同名 buffer 在其他用途不变）。
- DMA：无变化。
- SYNC：每行归约侧硬同步 (T+1)×3 → 1（既有 GetValue 的一次）；不增不减既有
  PipeBarrier/SyncVToS/SyncSToV 调用点。
- PRECISION：低。同一条 vcadd 硬件求和，仅结果路由不同（acc→标量读回 vs dst 直写）。

## 证据目录

本地实验/REDUCE-HIER-X/V005/（submission.asc、submission.sha256、source-meta.json、
diff.patch、local-result.json、support/）。

## 状态

声明完成 → 源码实现 → server3 compile/link → correctness → same-binary → 交错 P/C
→ 本地结论（规范枚举）→ handoff。LOCAL_REJECTED 则证据保留 + 显式回退 +
LANE_NEEDS_PLANNING_REVIEW 事实包；LOCAL_ACCEPTED 则推进 LOCAL_BEST 并 handoff
Online recommendation 材料。不自行 Online。
