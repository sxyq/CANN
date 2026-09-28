# TRACK-B-HYPOTHESES — ASYNC-OVERLAP-CHAMPION-X

范围：pipeline scheduling / sync placement / issue order / loop structure。
禁止项（不得作为任一假设的实质内容）：加一层 buffer、加深 queue depth、只加 MTE3 stage。
一个假设 = 一个概念性机制；推荐只实现其中一个作为 V001。

源码行号均指 seed：`线上结果/R31B/V011/submission.asc`（SHA a8c19a19…）。

---

## H1 — Inter-pass prologue param prefetch（推荐 V001）

```text
MECHANISM
  在 wide 路径 pass-1 结束、invRms 标量回路之前，就把 pass-2 tile-0 的 gamma/bias
  MTE2 发出去；invRms 回路与参数 DMA 并行，回路结束后 WaitFlag 一个已在飞的 load，
  而不是先跑完 invRms 再冷启动参数 Load。
  落点（OFAT 先只动一条路径）：ProcessWideLowPrecision
  现状顺序 L3211–3266：pass-1 尾 → L3224–3242 invRms 逐行标量往返 → L3243 SyncVToMTE2
  → L3260–3265 Load gamma/bias tile0 + SetFlag MTE2_V。
  改为：pass-1 释放 xBuf_/residualBuf_ 之后（L3212–3217 rel0/rel1 已等完）、进入 invRms
  之前，Load gamma/bias tile0 + SetFlag；invRms 回路照旧；pass-2 入口只 WaitFlag。

BOTTLENECK
  Inter-pass 边界上的标量往返 + 参数冷启动。invRms 回路含每 batchRow 两次
  SyncVToS/GetValue/SyncSToV 往返，结束后还要等一次参数 MTE2；这两段目前是串行的。

EXPECTED_SHAPES
  所有进入 wide full-y 路径的形状（rowWidth > kCacheElems）。tileCount>=1 都有 pass-2
  入口，因此 tileCount=1 也吃得到（与 ASYNC-TRIPLE-X 的 triple-stage 惰性不同）。
  batchRows 越大、invRms 回路越长，重叠窗口越大。优先 probe：large-R wide FP16/BF16
  （LP 路径本身是 T14 large-R 路径集合）。

WHY_IT_MAY_HELP
  消掉一次「标量回路完 → 再等参数 DMA」的串行段。参数不依赖 inverseRms，只有
  后续 apply 依赖；提前发 load 不改变任何数据依赖。对 batchRows>1 的 large-R 尤其
  有意义：回路本身就是多行标量往返。

WHY_IT_MAY_FAIL
  invRms 回路可能短于参数 DMA 延迟，只能盖住一部分；编译/硬件也许已把 Load 提到
  回路前（则 delta≈0）。若参数 Load 不在关键路径上，收益为零。xBuf_/residualBuf_
  在 pass-1 尾部的释放点若仍有隐含占用，提前 Load 会引入正确性风险（须先做依赖审计）。

ASCEND_FEASIBILITY
  高。纯 issue 位置移动，沿用已有 TEventID/Load/SetFlag/WaitFlag，无新 API、无 ABI
  变化。OFAT：只移动一处 Load+SetFlag 的位置，其余语句不动。

UB/CORE/DMA_IMPACT
  UB：不变（不新增、不改大小）。Core：行分配不变。DMA：字节数不变，仅参数 tile-0
  的发出时间提前。

SYNC_IMPACT
  pass-2 入口从「SyncVToMTE2 后再 Load 再进循环」改为「Load 已在飞，WaitFlag 即用」。
  须确认：pass-1 对 xBuf_/residualBuf_ 的最后一次 V 读已在 rel0/rel1 Wait 处结束，
  之后的 invRms 只碰 xFp32Buf_/residualFp32Buf_/reduceFp32Buf_，与 gBase=xBuf_、
  bBase=residualBuf_ 无重叠。原 L3243 的 SyncVToMTE2 若仅服务参数 Load，可降级为
  只针对该次 Load 的 event 顺序（仍属同一假设的 sync placement，不另计机制）。

PRECISION_RISK
  无。同一数据、同一算术顺序、同一 inverseRms；只改 DMA 发出时机。

DUPLICATE_CHECK
  vs R013：R013 是两段 double-buffer，无 inter-pass 重排。
  vs ASYNC-TRIPLE-X V001：V001 只加 MTE3 stage，其 H1 分类从未在 Champion 上落地；
    本假设是 R31B-V011 上的新父版实验，不是移植 V001。
  vs MIX-A V003/V007：那是 release 增加 / Load 前删除 sync，不在 inter-pass 边界、
    不做 prefetch 提前，目的也不同（正确性/dispatch）。
  vs R31B V006/V009：队列深度，proven-flat，本假设不改深度。
  vs R31B V011 自身：V011 只做了 unit 级 2-deep MTE2 与参数 2-deep，没有把 pass-2
    tile-0 参数 Load 提到 invRms 之前。
  不是 buffer/depth/MTE3-stage 机制。

MINIMAL_OFAT_DIFF
  把 L3260–3265 的 Load(gBase)+Load(bBase)+SetFlag 移到 L3224 之前（pass-1 释放之后），
  并让 pass-2 入口只保留 WaitFlag；若需要，把 L3243 SyncVToMTE2 收窄为该 Load 所需的
  事件顺序。除此之外零改动。

EXPECTED_LOCAL_PROBES
  同 shape 的 same-binary 先过本地测时规范；P/C 对照 FROZEN parent，优先 LP 形状
  （wide FP16/BF16 large-R，tileCount>=2 以便 pass-2 有稳定段，同时保留 tileCount=1
  形状验证「prologue 类机制不依赖 tileCount」）。device-event 优先，warmup=45，
  交错配对，保存 raw samples。
```

**Classification: READY_FOR_MAIN_REVIEW** — 单点 issue 位置移动，无精度风险，不触碰
禁止轴；建议作为 V001。

---

## H2 — LP pass-2 store 解耦（per-slot MTE3_V，store 完成不进标量 issue）

```text
MECHANISM
  ProcessWideLowPrecision pass-2 内层（L3338–3345）目前每个 batchRow 都是
  PipeBarrier → SyncVToMTE2 → SyncVToMTE3 → Store → SyncMTE3ToV，
  store 完成被立刻拉回标量 issue。改为 FullCache pass-2 已验证的形态
  （L2164–2232：SetFlag V_MTE3 / WaitFlag V_MTE3 发 store / SetFlag MTE3_V 记账，
  storeOutstanding* 延迟到槽位复用前才 WaitFlag），使 MTE3(N) 与下一个 batchRow
  的 V 工作重叠。store 源从单块 outputLocal staging 改为「本 tile 写完的 y 行区域」
  （half 路径 y 在 gammaBuf_ 的 yStore 行区域，tile 之间地址不重叠），这样不需要
  第二块 output staging，也不增加 UB。

BOTTLENECK
  LP pass-2 把 store 延迟耦合进标量路径，每个 batchRow 一次 MTE3 全等待；
  与 FullCache 的延迟等待形成对照，是 LP 与 FP32 路径在 overlap 上的真实差距点。

EXPECTED_SHAPES
  仅 wide LP 路径（FP16/BF16，rowWidth > kCacheElems）。batchRows 越大收益越大
  （更多 store 可并行挂在飞）。tileCount=1 仍有 batchRows 内层，也可受益。

WHY_IT_MAY_HELP
  把已暴露的 store 变成被 V 盖住的 store；机制分类在全项目里已有 FullCache 先例，
  说明事件方向在 910B 上可行。不加 queue 深度。

WHY_IT_MAY_FAIL
  若 y 行区域方案破坏 half 路径的 y 复用或 gammaBuf_ 生命周期，会引入正确性风险
  （须先做字节对照）。若 MTE3 本来就跟得上、等待其实空转，delta≈0。事件簿记变多
  可能在短 tile 上吃掉收益。

ASCEND_FEASIBILITY
  中高。SetFlag/WaitFlag HardEvent::V_MTE3 / MTE3_V 在本 Champion 内已大量使用；
  本假设只是把 LP 路径改成同一套延迟等待写法，不新增 API。

UB/CORE/DMA_IMPACT
  UB：总字节不变（复用已有 y 行区域 / 不新开 outputBuf_ 第二槽）。Core：不变。
  DMA：store 字节不变，仅完成可见性变化。

SYNC_IMPACT
  最高的一档。事件方向写反会读写已复用槽位，属正确性风险而非精度风险。
  实现前必须先列依赖表（谁读 yStore、谁写 yStore、store 源地址在何时安全复用）。

PRECISION_RISK
  同步正确时无精度变化（数据与算术顺序不变）。方向写错则输出错误，属 correctness。

DUPLICATE_CHECK
  vs 「加一层 buffer」：不新增槽位/不加深 queue，只改 sync 与 store 源地址。
  vs R31B V006/V009：那是深度，proven-flat；本假设是事件放置。
  vs ASYNC-TRIPLE-X H2：同一机制分类，但从未在 Champion 上实现；本假设落在
    R31B-V011 LP 路径，且 store 源重排是 Champion 特有约束下的做法。
  vs UB-LIVENESS-X：该路线是 UB 生命周期/别名全局策略；本假设只动 LP pass-2 的
    store 完成可见性，不改别名架构。实现时不得顺手改 UB 布局。
  vs MIX-A V003 V_MTE2 release：方向与对象都不同（那是 MTE2 释放，这是 MTE3 完成）。

MINIMAL_OFAT_DIFF
  只改 ProcessWideLowPrecision pass-2 内层的 store 段：替换 L3338–3345 的同步序列
  与 store 源；参数 prefetch、invRms、pass-1 一律不动。

EXPECTED_LOCAL_PROBES
  先 correctness 字节对照 vs parent，再 same-binary，再 P/C。形状：batchRows>1 的
  wide FP16/BF16（large-R）。若 batchRows==1 形状 delta≈0 属预期，不作证伪。
```

**Classification: NEEDS_MORE_EVIDENCE** — 机制在 FullCache 已有先例，但 LP 的
store 源/y 区域生命周期要做完依赖审计才可写；不宜与 H1 同时做。

---

## H3 — LP pass-2 冗余 SyncVToMTE2 下沉（issue 顺序 / sync placement）

```text
MECHANISM
  LP pass-2 内层 L3339 的 SyncVToMTE2()（Set+Wait 全管线 event 0）在每个 batchRow
  的 Store 之前执行一次。参数释放其实已由 L3346 的 SetFlag<V_MTE2>(prel) 在 tile
  末尾记账。把 L3339 这个全量 V→MTE2 栅栏从 batchRow 内层移除或下沉到 tile 边界，
  让标量 issue 不被每个 batchRow 的 MTE2 栅栏打断。

BOTTLENECK
  标量 issue 被每 batchRow 一次的全管线 V_MTE2 栅栏切开，破坏 pass-2 的连续发射。

EXPECTED_SHAPES
  wide LP，batchRows 越大越明显；batchRows==1 时该栅栏每 tile 只多一次，收益有限。

WHY_IT_MAY_HELP
  若该栅栏只为参数缓冲释放服务，则与 L3346 重复，删除/下沉是纯多余等待的消除。

WHY_IT_MAY_FAIL
  该栅栏可能在防 outputLocal / xBuf_ 别名竞争（参考 registry 里 SCHED 路径曾出现
  的 xBuf_ 别名 race）。若它是防御性的，删除会引入正确性问题。也可能栅栏开销本身
  很小，delta≈0。

ASCEND_FEASIBILITY
  高（删除或移动一行同步）。但必须先证明它没有隐藏的数据依赖。

UB/CORE/DMA_IMPACT
  UB/Core/DMA 均不变。

SYNC_IMPACT
  中：减少同步点。风险是掩盖在栅栏下的别名竞争暴露出来。

PRECISION_RISK
  无（同步正确时数据不变）；错误删除属 correctness 风险。

DUPLICATE_CHECK
  vs MIX-A V007「移除 Load 前第一个 SyncVToMTE2」：同属 sync 删除一类，但位置不同
  （本假设在 LP pass-2 Store 前的 batchRow 内层，不是 Load 前）、父版不同
  （R31B-V011 vs MIX-A V003）、目的不同（消除重复栅栏 vs 调度约束）。
  仍有同轴观感，因此建议排在 H1/H2 之后，且 V001 不要选它。
  vs R013 / ASYNC-TRIPLE-X / V006/V009：均不同轴。

MINIMAL_OFAT_DIFF
  删除或下移 L3339 一行（若下沉到 tile 边界，与 L3346 合并为同一释放点）。

EXPECTED_LOCAL_PROBES
  先跑正确性矩阵（尤其 BF16/half 多 batchRows），再 P/C。若正确性出现回归，
  立即停，改回并把该栅栏标为「必要防御」。
```

**Classification: NEEDS_MORE_EVIDENCE**（且有 MIX-A V007 同轴观感，不推荐做 V001）

---

## H4 — pass-2 参数 prefetch 与 store 的发射先后（HBM 竞争）

```text
MECHANISM
  LP pass-2 每个 tile 迭代里：L3278–3297 先发下一 tile 的 gamma/bias 2-copy MTE2，
  内层 batchRow 再发 MTE3 store。把同一迭代内「先发 store、后发参数 prefetch」对调
  （或把 store 提前到参数 prefetch 之前的空档），给 store 一个更干净的 HBM 窗口。

BOTTLENECK
  pass-2 稳态里参数 MTE2 与输出 MTE3 同迭代打 HBM；发射顺序可能影响竞争先后。

EXPECTED_SHAPES
  wide LP 多 tile 形状（tileCount>=2 的稳态）；短 tile 上效果可能被发射开销吃掉。

WHY_IT_MAY_HELP
  发射顺序若影响 HBM 竞争的先后，先发短 store 可能缩短 store 完成，改善槽位回收
  与行尾暴露段。

WHY_IT_MAY_FAIL
  MTE2/MTE3 是独立单元，HBM 竞争未必听标量发射顺序，delta≈0。延后参数 load 可能
  饿死 Vector，变成负收益。历史深度类改动在 T14 也是平的，本机制也可能平。

ASCEND_FEASIBILITY
  高：交换两段已有调用的先后，无新 API。

UB/CORE/DMA_IMPACT
  三者均不变，只改命令发射顺序。

SYNC_IMPACT
  低。store 依赖 V 完成、参数 prefetch 依赖槽位释放，两者前置条件在上一迭代已满足；
  无地址交叉。

PRECISION_RISK
  无。

DUPLICATE_CHECK
  vs H1（边界 prefetch 提前）：H1 是 pass 边界，H4 是 tile 稳态内顺序。
  vs H2（sync 原语）：H4 不改同步原语，只对调两段 issue。
  vs ASYNC-TRIPLE-X H3：同机制分类，未在 Champion 落地；本假设基于 LP 源码顺序。
  vs depth/MTE3-stage：不涉及。

MINIMAL_OFAT_DIFF
  在 LP pass-2 循环内对调「参数 N+1 prefetch 块」与「batchRow store 块」的发射
  先后（或只把 store 发射提前到 prefetch 之前）；其余不动。

EXPECTED_LOCAL_PROBES
  same-binary 先行；P/C 在 tileCount>=2 的 wide LP 形状。效应可能小或反向，
  delta 必须超过 same-binary 噪声范围才记方向。
```

**Classification: NEEDS_MORE_EVIDENCE** — 便宜但物理效应未证，建议排在 H1 之后。

---

## 假设集合汇总

| ID | 机制类别 | 落点 | 禁止轴 | 分类 | V001 建议 |
|---|---|---|---|---|---|
| H1 | issue order / inter-pass loop boundary | LP 入口参数 prefetch 提前到 invRms 前 | 无 | READY_FOR_MAIN_REVIEW | **推荐** |
| H2 | sync placement / store 完成解耦 | LP pass-2 内层 store | 不加 buffer（复用 y 行区域） | NEEDS_MORE_EVIDENCE | 备选（先做依赖审计） |
| H3 | sync placement（删除冗余栅栏） | LP pass-2 L3339 | 无 | NEEDS_MORE_EVIDENCE | 不推荐（MIX-A V007 同轴观感） |
| H4 | issue order（稳态内 MTE2/MTE3 先后） | LP pass-2 循环内 | 无 | NEEDS_MORE_EVIDENCE | 备选（H1 之后） |

明确未列入候选的机制（排除记录）：

- 再加一层 buffer / 再加深 queue depth：R31B V006/V009 在 T14 proven-flat。
- 只再加 MTE3 stage：ASYNC-TRIPLE-X V001 已做。
- 把 FullCache pass-1 改成 2-deep prefetch：需要第二块 x/res staging = buffer add，禁止。
- 归约拓扑 / invRms 数学 / dtype 分裂 / row-group 调度 / UB 别名：属 REDUCE / VECTOR-MATH /
  DTYPE / SCHED-CHAMPION / UB-LIVENESS 轴，本路线禁止触碰。

SCREENED COUNT: 4（≥3 满足）。无 INFEASIBLE。未改 Kernel、未建 V001、未跑设备。
