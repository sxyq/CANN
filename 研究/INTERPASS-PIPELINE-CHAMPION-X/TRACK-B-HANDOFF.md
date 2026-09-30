# INTERPASS-PIPELINE-CHAMPION-X — Wave-2 Track-B Handoff

- DATE: `2026-09-30`
- ROUTE: `INTERPASS-PIPELINE-CHAMPION-X`
- BRANCH: `w2/m2/interpass-pipeline`
- TRACK: `WAVE-2 TRACK-B ONLY`
- STATUS: `RESEARCH_COMPLETE; WAITING_FOR_MAIN_REVIEW`
- MAIN_SELECTED: `NONE`
- DIRECT_PARENT: `R31B V011`
- PARENT_OFFICIAL_SCORE: `45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_IDENTITY_CHECK: `sha256sum 线上结果/R31B/V011/submission.asc` 与 `source-meta.json`、`submission.sha256` 一致。
- REVISION_CREATED: `NO`
- KERNEL_CHANGED: `NO`
- SERVER3_USED: `NO`

本文件只交接研究假设，不是 Main selection，不批准实现，也不改变路线生命周期。

## ROUTE_BOUNDARY

只研究同一 row 内 `pass-1 -> reduction finalization -> pass-2 affine` 三段之间的 stage overlap：

- pass-1 最后一个 tile 的 V 尾段与 pass-2 setup；
- reduction finalization / scalar RMS 尾段与 pass-2 gamma/bias prepare；
- pass-2 first-tile parameter wait 与 parameter-independent normalization prelude；
- 使用父版已经存在的两个 parameter staging slot 做 inter-pass warm start。

所有候选都保持父版 row ownership、tileWidth、tileCount、Store 形态、算术次序和 retained-y 语义。主探针固定为单 row/core，避免把收益归因到跨 row pipeline。

## ALLOWED_CHANGES

- 未来只有在 Main 明确选择后，才可移动同一 row 内既有 gamma/bias `Load`、对应 `SetFlag/WaitFlag` 和 pass-2 first-tile wait 的相对位置。
- 可复用 V011 已有 `xBuf_/residualBuf_` 两个 parameter staging slot 和四个 parameter event ID；不得增加深度。
- 可为跨 pass handoff 增加最小状态位，记录 tile-0/tile-1 已在哪个现有 slot 预取；不得改变 buffer 大小或数据布局。
- 每个未来 Revision 只能实现本文一个 hypothesis，且直接父版仍须单独由 Main 确认。

## FORBIDDEN_CHANGES

- 本轮禁止任何 `.asc/.cpp/.h/.hpp/CMake/runner/host wrapper` 修改；禁止 Revision、build/link、correctness、timing、profiling、server3、Online/Judge。
- 禁止跨 row pipeline、next-row prologue、row scheduling、blockCount/ownership 修改。
- 禁止 tile search、tileWidth/tileCount 策略修改、dtype dispatch 修改。
- 禁止 Store primitive、Store 次数、Store 粒度、drain policy、MTE3 queue 修改。
- 禁止 reduction 算法、invRms 数学、affine 数学顺序或精度路径修改。
- 禁止新 UB buffer、queue depth 增长、unsafe UB alias、扩大 parameter residency 生命周期。
- 禁止更新 `技术路线/`、`调度/`、`本地实验/`、`线上结果/` 等 canonical shared ledger/evidence。

## PARENT BOTTLENECK OBSERVATION

V011 `ProcessWideLowPrecision` 的相关顺序为：

1. pass-1 使用两槽 MTE2 staging，结束后 drain `rel0/rel1` 并释放 pass-1 event；
2. 对 retained row 的 tile partial 做 reduction finalization，经过 V/S 往返得到 `invRmsValues`；
3. 执行一次 `SyncVToMTE2()`；
4. 重新分配 pass-2 parameter events，才发出 gamma/bias tile-0 DMA；
5. pass-2 循环入口立即等待 tile-0 ready，然后执行 retained-y normalization 和 affine。

因此 tile-0 parameter DMA 没有利用 pass-1 最后一个 tile 的 V 尾段、reduction finalization 或 pass-2 first-tile parameter-independent normalization 窗口。父版证据位置为 `线上结果/R31B/V011/submission.asc:3110-3344`。

## HYPOTHESES

### H1 — Final pass-1 unit uses the idle slot to prefetch pass-2 tile 0

- HYPOTHESIS_ID: `IPP-H1-TAIL-SLOT-HANDOFF`
- STATUS: `NEEDS_MORE_EVIDENCE`
- MECHANISM: 在同一 row 的最后一个 pass-1 unit 开始 V 计算后，确认相反的 input staging slot 已完成 `V_MTE2` release；立即把 pass-2 gamma/bias tile 0 搬入该空闲 slot。pass-1 当前 unit 继续只使用自己的 slot，reduction finalization 后 pass-2 从已标记的预取 slot 开始。不得提前覆盖当前 unit 或仍 outstanding 的 input slot。
- BOTTLENECK: pass-1 最后一个 tile 的 Add/Cast/Mul/ReduceSum V 尾段期间，MTE2 无下一份 x/residual 工作；父版直到 reduction 完成后才开始 parameter DMA。
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: 主探针 `1x12288`、`1x16384`、`1x32768`；复制控制可用 `2x16384` 且 `blockCount=2`，保证每 core 仍只有一行。
- TARGET_DTYPES: `FP16`, `BF16`
- WHY_IT_MAY_HELP: 比 reduction-window prefetch 更早一个 V tail 窗口发出 tile-0 parameter DMA；若末 tile V 链足够长，DMA 可在进入 reduction finalization 前已经部分或全部完成。
- WHY_IT_MAY_FAIL: 最后 unit 的相反 slot 可能仍有尚未消费的 release event；增加 slot-handoff 分支和 event 状态可能抵消收益。若 MTE2 命令发出受 V/S 或 pipe ordering 限制，提早发出不代表实际传输重叠。
- UB_IMPACT: `0`；只使用现有两个 `xBuf_/residualBuf_` slot，不改大小、不别名 retained-y/reduction buffer。
- DMA_IMPACT: DMA 次数、字节数、地址和 tile 大小不变；仅 tile-0 gamma/bias issue 时间前移。
- SYNC_IMPACT: 不删除任何同步；需要消费空闲 slot 的旧 `V_MTE2` release，再沿用 pass-2 tile-0 `MTE2_V` ready event。pass-1 当前 slot 的 release/drain 保留。
- PRECISION_RISK: 数学无变化；正确性风险为错认空闲 slot、event 二次消费或在 pass-1 仍读 slot 时覆盖。
- DUPLICATE_CHECK: 最接近 R013/FULL-R013 与 V011 pass-1 next-unit prefetch，但它们预取的是同一 pass 的下一个输入；本项把父版现有 slot 在最后 unit 时跨 pass 交给 parameter DMA。Wave-1 `SYNC-TOPOLOGY H2` 调整的是 pass-2 内当前 wait 与下一 tile Load 的顺序，没有在 pass-1 尾段发 tile-0。不是 queue depth 增长。
- RELATED_OLD_ROUTES: `R013/FULL-R013`, `R31B V011`, `ASYNC-TRIPLE-X`, Wave-1 `SYNC-TOPOLOGY-CHAMPION-X H2`。
- MINIMAL_EXPERIMENT: Main 选择后，仅对最后 pass-1 unit 增加空闲 slot 的 tile-0 gamma/bias Load、ready event 和 pass-2 起始 slot 标记；后续 tile prefetch、reduction、Store 全部保持 V011。先做上述 FP16/BF16 正确性，再按当前协议做 same-binary 与至少 4 组交错 P/C。
- INDEPENDENT_EXPERIMENT_VALUE: 与 H2 的差别是额外覆盖最后 pass-1 V tail；若 H1 优于 H2 才能证明该 tail 窗口有独立价值，否则选择更简单的 H2 即可。

### H2 — Issue pass-2 tile 0 before reduction finalization

- HYPOTHESIS_ID: `IPP-H2-REDUCE-FINAL-PARAM-PREFETCH`
- STATUS: `READY_FOR_MAIN_REVIEW`
- MECHANISM: pass-1 两个 input slot 已完成 release/drain 后，先分配父版已有 pass-2 parameter events，并把 gamma/bias tile 0 发到 slot 0；随后原样执行该 row 的 reduction finalization、V/S 往返和 invRms 计算。pass-2 开始时才等待 tile-0 ready。
- BOTTLENECK: 父版 reduction finalization 和多次 V/S scalar handoff 与 tile-0 parameter DMA 完全串行；gamma/bias 地址和长度不依赖 invRms。
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: `1x12288`, `1x16384`, `1x32768`，以及 `2x16384 blockCount=2` 单行/core 控制；D 越大，tile-0 DMA 越长但只覆盖一次 inter-pass gap。
- TARGET_DTYPES: `FP16`, `BF16`
- WHY_IT_MAY_HELP: reduction finalization 包含 ReduceSum、V/S、Duplicate、Sqrt 和 scalar reciprocal，给 tile-0 的两笔 parameter DMA 提供自然隐藏窗口；不依赖跨 row 工作。
- WHY_IT_MAY_FAIL: MAIN-2 Wave-1 已发现 large-D 的 parameter MTE2 通常不是主瓶颈；tile-0 DMA 可能早已很短，收益只剩 event/branch 成本。MTE2 与 V/S 是否能按预期并行仍需硬件验证。
- UB_IMPACT: `0`；pass-1 input buffer 生命周期结束后才复用父版 slot 0。
- DMA_IMPACT: 次数、字节数和顺序不变；只有 tile-0 issue 点从 reduction 之后移动到 reduction 之前。
- SYNC_IMPACT: pass-1 release waits 全保留；父版 `SyncVToMTE2()` 不作为独立删除项。tile-0 `WaitFlag<MTE2_V>` 延迟到 pass-2 first use。
- PRECISION_RISK: 低；数学和 dataflow 不变。风险仅为 event 生命周期跨过 reduction scalar section 后是否仍合法。
- DUPLICATE_CHECK: MAIN-2 初始化报告把 `pass-2 tile-0 MTE2 提前到 FinishRms 之前` 明确列为 Champion 上尚可能独立的 inter-pass 空间。本项正好限定该差异。不同于 R028/FULL-R028 的 scalar sync reduction（改变标量汇总策略），也不同于 Wave-1 `SYNC H2/H3` 的 pass-2 内 wait/work 重排。
- RELATED_OLD_ROUTES: `R028/FULL-R028`, `R31B V011`, `COEFF-LOCALITY-X`, Wave-1 `SYNC-TOPOLOGY-CHAMPION-X`。
- MINIMAL_EXPERIMENT: 只把现有 tile-0 gamma/bias Load、`SetFlag<MTE2_V>(prd0)` 和 parameter event allocation 移到 pass-1 release drain 后、reduction-final loop 前；pass-2 首次消费前保留 wait。未来先测 `1x16384` 与 `1x32768` FP16/BF16，再以 D=8192 非 wide 路径作空白控制。
- INDEPENDENT_EXPERIMENT_VALUE: 它单独回答 reduction-final scalar window 能否隐藏首个 parameter DMA，是四项中最干净的 inter-pass 因果探针。

### H3 — Delay first-tile parameter wait until the first parameter-consuming op

- HYPOTHESIS_ID: `IPP-H3-FIRST-TILE-NORM-BEFORE-WAIT`
- STATUS: `READY_FOR_MAIN_REVIEW`
- MECHANISM: tile-0 DMA 的 issue 点保持在父版 reduction 之后；仅在 pass-2 tile 0，把入口 `WaitFlag<MTE2_V>(prd0)` 移到第一次读取 gamma/bias 之前。等待期间先执行该 row/tile 已可做的 retained-y prepare：FP16 的 `ToFloat(yTile)` 与 `Muls(invRms)`，BF16 的 `Muls(valueTile, invRms)`。tile 1+ 完全保持父版顺序。
- BOTTLENECK: 父版 tile-0 在 parameter ready wait 期间没有发出任何 V 工作，但 normalization prelude 只依赖 retained y 和已完成的 invRms。
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: `1x12288`, `1x16384`, `1x32768`; `D=8192` 为未命中 wide path 控制。
- TARGET_DTYPES: `FP16`, `BF16`
- WHY_IT_MAY_HELP: 即使 parameter DMA 不能跨 reduction 发出，也可用 first-tile normalization V 链覆盖部分 ready latency；只影响一次 inter-pass warm-up，不改变稳态 tile pipeline。
- WHY_IT_MAY_FAIL: MTE2 和 V 可能争用 issue/pipe 资源；normalization prelude 太短时收益落在噪声内。FP16 的 `FromFloat(outputLocal)` 必须仍在 gamma/bias ready 后，避免 output/parameter pipeline 交叉扩大。
- UB_IMPACT: `0`；不新增 tensor，不改变任何 alias。
- DMA_IMPACT: `0`；issue 点、次数、字节和 slot 全不变。
- SYNC_IMPACT: 同一个 tile-0 ready wait 只向后移动，不删除；tile 1+ waits 和 release events 不变。
- PRECISION_RISK: 低；每元素 `normalize -> gamma -> bias` 顺序保持。风险是把 wait 越过了第一条参数读取，因此 diff 必须以首次 gamma/bias consumer 为硬边界。
- DUPLICATE_CHECK: Wave-1 `SYNC-TOPOLOGY H3` 在 FP32 full-cache path 把既有 `Muls` 放到 parameter wait 前，机制家族相似；本项只针对 V011 的 LP wide inter-pass first tile，且 FP16 还包含 retained half 到 FP32 的 prepare，后续 tile 不改。应标为 `PARTIAL_ADJACENCY`，不能声称全项目未试。独立价值在 dtype/path/仅首 tile边界。
- RELATED_OLD_ROUTES: Wave-1 `SYNC-TOPOLOGY-CHAMPION-X H3`, `R31B V011`, `EPILOGUE-ARITH-CHAMPION-X`, `EPILOGUE-FUSE-X`。
- MINIMAL_EXPERIMENT: 只对 `tile==0` 拆分 normalization prelude 和 parameter-consuming affine；把 `prd0` wait 放在第一条参数 consumer 前。先验证 FP16/BF16 全目标 shape 与父版结果，再配对测量；若 tile 1+ diff 非空则 OFAT 失败。
- INDEPENDENT_EXPERIMENT_VALUE: 它测试的是 wait placement，而 H1/H2/H4 测试 DMA issue placement；即使跨 reduction prefetch 无效，本项仍可能隐藏 first-tile wait。

### H4 — Prime both existing parameter slots during reduction finalization

- HYPOTHESIS_ID: `IPP-H4-TWO-SLOT-INTERPASS-PRIME`
- STATUS: `NEEDS_MORE_EVIDENCE`
- MECHANISM: pass-1 两槽完全释放后，在 reduction finalization 前使用父版已有 slot 0/1 和 `prd0/prd1`，依次发出 gamma/bias tile 0 与 tile 1；pass-2 从已 ready 的两槽开始，tile 2 起恢复 V011 的稳态 ping-pong。不得增加第三槽、第三 event 或 buffer 字节。
- BOTTLENECK: 父版 pass-2 warm-up 只有 tile 0 被预取；tile 1 要等 tile 0 ready 后才 issue，导致初始两 tile 无法利用 reduction-final 窗口充分预热。
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- TARGET_SHAPES: 只适用于 `tileCount>=2`：`1x12288`, `1x16384`, `1x32768`; `D=12288/16384` 可检验两槽预热对短 tileCount 的影响，`D=32768` 检验进入稳态后的 warm-start 占比。
- TARGET_DTYPES: `FP16`, `BF16`
- WHY_IT_MAY_HELP: reduction-final window 较长时，两组 parameter DMA 都可能在 pass-2 前完成，消除 tile0→tile1 的 cold-start bubble；完全复用 V011 两槽稳态结构。
- WHY_IT_MAY_FAIL: 两笔 MTE2 DMA 可能只在同一队列串行，第二笔反而延后第一笔 ready；更多跨边界 outstanding events 增加控制成本。若 tile 0 计算足以覆盖 tile 1 DMA，父版稳态已无 bubble。
- UB_IMPACT: `0`；使用现有两槽，明确禁止 2→3 depth 或额外 buffer。
- DMA_IMPACT: 次数、总字节、tile 大小不变；只把 tile-1 issue 从 tile-0 wait 后移动到 reduction 前。
- SYNC_IMPACT: 两个既有 ready events 跨 reduction 存活；pass-2 对 tile0/tile1 分别等待原 ready event，tile2 起恢复原 release-before-reuse 规则。
- PRECISION_RISK: 数学无变化；风险为 event outstanding 顺序、slot parity 和 tileCount==1 边界。
- DUPLICATE_CHECK: 与 R31B Track-B H5 的 `2-deep -> 3-deep` 不同，本项绝不增加 queue depth；与 V011 稳态 2-deep 不同之处只有两槽都跨 inter-pass 提前 prime。与 H2 相比，新增的单因素是 tile-1 也跨边界 issue；若 Main 要严格测第二槽增量，可先有 H2 结果后再由 Planning 决定父版，但本研究不创建链。
- RELATED_OLD_ROUTES: `R013/FULL-R013`, `R31B V009/V011`, R31B Track-B `H5`, `ASYNC-TRIPLE-X`。
- MINIMAL_EXPERIMENT: 仅在 `tileCount>=2` 时于 reduction 前发 tile0/tile1；pass-2 前两轮不再重复 issue，tile2 起原逻辑逐字保持。未来先静态审计 event 数量和 slot parity，再做 correctness 与 P/C。
- INDEPENDENT_EXPERIMENT_VALUE: 它回答“父版已有两槽能否通过跨 pass warm start 消除第二个 cold bubble”，不是更深 queue，也不是单 tile prefetch。

## DUPLICATE AUDIT

### R001–R029 and FULL series

The table names the corresponding `FULL-Rxxx` implementation for every historical
route. `ADJACENT` means the mechanism is similar enough to require a separate
control; it is not claimed to be globally novel. A proposal that collapses the
adjacent mechanism into this route is explicitly `DUPLICATE_REJECTED` below.

| Route / FULL implementation | Similar historical mechanism | This route's real difference | Independent experiment value / disposition |
|---|---|---|---|
| R001 / FULL-R001-TWO-PASS-SCAN-V001 | Two-pass scan establishes the same pass-1/pass-2 algorithm boundary. | No algorithm or pass count changes; only same-row issue/wait placement changes. | Baseline prerequisite; inter-pass overlap remains independently testable. |
| R002 / FULL-R002-RETAINED-Y-V001 | Retained-y avoids rereading the second pass. | Retained-y contents and capacity remain unchanged; no new retention. | Dataflow prerequisite only; no duplicate performance proposal. |
| R003 / FULL-R003-PURE-ASCENDC-DIRECT-V001 | Direct AscendC execution and its launch structure. | No launch, host, or API change; only a future in-kernel stage order would differ. | Engineering baseline; no overlap mechanism to reuse. |
| R004 / FULL-R004-LOW-PRECISION-MIDDLE-V001 | Low-precision middle representation. | Dtype/cast mode and arithmetic precision remain V011. | Precision control; no inter-pass stage hypothesis. |
| R005 / FULL-R005-LARGE-TILE-V001 | Tile-size change. | `tileWidth` and `tileCount` are frozen. | Tile search is forbidden and is not a candidate here. |
| R006 / FULL-R006-REDUCTION-ARCH-V001 | Reduction architecture/layout. | Reduction implementation and ownership remain byte-for-byte conceptual parent behavior. | Reduction architecture is out of scope; only its existing finalization time is an overlap window. |
| R007 / FULL-R007-REDUCE-SUM-V001 | Reduce-sum implementation. | No reduction primitive, grouping, or order changes. | Reduction arithmetic is a control, not a proposed mechanism. |
| R008 / FULL-R008-TILE-CROSS-CORE-V001 | Cross-core tile distribution. | One row remains owned by one core; no cross-core or cross-row work. | Explicit boundary control; cross-core pipeline is not retained. |
| R009 / FULL-R009-COPY-CENTRIC-V001 | Copy primitive and copy-centric scheduling. | DataCopy/DataCopyPad form, byte counts, and addresses remain unchanged. | Copy policy is frozen; only an existing parameter issue point may move. |
| R010 / FULL-R010-TAIL-CENTRIC-V001 | Tail/alignment handling for the last tile. | H1 uses the last tile only as an overlap window; tail length and handling do not change. | `ADJACENT`; H1 has value only if its new cross-pass issue is isolated from tail logic. |
| R011 / FULL-R011-MANUAL-VECTOR-REDUCTION-V001 | Manual vector reduction. | No vector reduction instruction sequence or scalar result changes. | Reduction instruction tuning is out of scope. |
| R012 / FULL-R012-ALIGNMENT-ROWGROUP-V001 | Alignment and row-group layout. | Alignment, row grouping, and row ownership remain fixed. | Layout is a control; no duplicate stage-overlap mechanism. |
| R013 / FULL-R013-DOUBLE-BUFFER-PIPELINE-V001 | Two-buffer MTE2/V pipeline. | H1/H2/H4 cross the pass boundary using the already existing slots; H4 does not add queue depth. | `ADJACENT`; the independent factor is inter-pass warm start, so same-pass prefetch alone is not reused. |
| R014 / FULL-R014-PARAMETER-RESIDENCY-ARCH-V001 | Gamma/bias residency. | Parameters remain one tile at a time and do not become row/batch resident. | Full residency is `DUPLICATE_REJECTED`; first-tile issue timing remains a narrower probe. |
| R015 / FULL-R015-MULTI-ROW-DMA-V001 | Multi-row DMA. | Every parameter DMA remains one tile for one row. | Cross-row batching is `DUPLICATE_REJECTED`; single-row issue timing is still isolated. |
| R016 / FULL-R016-SCHEDULING-ARCH-V001 | Row/core scheduling and ownership. | `blockCount`, ownership, and batch-row scheduling remain unchanged. | Scheduling changes are outside the route boundary. |
| R017 / FULL-R017-FP32-ALL-MIDDLE-V001 | FP32 intermediate/whole-middle precision path. | Target is V011 FP16/BF16 LP wide path; FP32 path is a blank control. | Precision-path change is out of scope; H3's LP wait placement can be compared against it. |
| R018 / FULL-R018-CAST-RINT-OUTPUT-V001 | Output cast/rounding behavior. | Output cast and rounding remain unchanged and occur at the parent boundary. | Arithmetic precision is frozen; no duplicate overlap mechanism. |
| R019 / FULL-R019-NORMALIZATION-V001 | Normalization operation placement. | H3 moves no math across the affine dependency; it only delays the parameter wait until the first parameter read. | `ADJACENT`; isolated wait placement has independent value from normalization redesign. |
| R020 / FULL-R020-SQRT-NORMALIZATION-V001 | Sqrt/normalization arithmetic. | Sqrt, reciprocal, and affine arithmetic remain unchanged. | Arithmetic redesign is out of scope; finalization duration is only a scheduling window. |
| R021 / FULL-R021-CANN9-BUILD-CHAIN-V001 | CANN9 build/toolchain setup. | No build or toolchain file is touched in Track-B. | Infrastructure only; no performance hypothesis. |
| R022 / FULL-R022-AUTO-CODEGEN-V001 | Generated-code path. | No code generation or host wrapper change. | Infrastructure only; no performance hypothesis. |
| R023 / FULL-R023-ENGINEERING-CONVENTION-V001 | Engineering conventions and project integration. | No convention, wrapper, or integration change. | Process baseline; no overlap mechanism. |
| R024 / FULL-R024-CPU-VALIDATION-MATRIX-V001 | CPU validation matrix. | No validation harness or correctness method is changed in this round. | Validation evidence is deferred until Main selects one hypothesis; no duplicate mechanism. |
| R025 / FULL-R025-SUBMISSION-INTEGRITY-V001 | Submission/source integrity. | No source package or submission behavior is changed. | Submission-only route; no performance mechanism. |
| R026 / FULL-R026-PERFORMANCE-MEASUREMENT-V001 | Performance measurement procedure. | No measurement procedure is changed; future probes follow the existing protocol. | Methodology control; not a candidate optimization. |
| R027 / FULL-R027-GPU-MIGRATION-REFERENCE-V001 | GPU reference/migration comparison. | No GPU implementation or reference path is imported. | Reference-only route; no NPU inter-pass mechanism. |
| R028 / FULL-R028-SCALAR-SYNC-REDUCTION-V001 | Scalar synchronization/reduction shortening. | H2/H4 keep all V/S and scalar math; they only issue parameters during that existing work. | Removing/shortening the scalar sync is `DUPLICATE_REJECTED`; preserving it as the overlap window is independently testable. |
| R029 / FULL-R029-WIDE-CACHED-ROW-V001 | Wide cached-row/shape dispatch. | No mode predicate, cache residency, or shape dispatch changes. | Full cached-row or residency variants are `DUPLICATE_REJECTED`; this route tests same-row stage overlap only. |

### R31A / R31B / MIX / derived routes

| 对照 | 相似机制与证据 | 真正不同点 | 处置 |
|---|---|---|---|
| R31B V006 | 2-deep MTE3 store queue | 本路线不碰 Store/MTE3；只做 MTE2 parameter inter-pass handoff | `NOT_DUPLICATE_BY_PIPE` |
| R31B V009 | FP32 full-y MTE2 queue depth 2，Official 未改善 | 不加深 queue；目标是 LP path 的跨 pass cold-start | H4 需以其 flat 结果作为失败先验 |
| R31B V011 | LP pass-1 与 pass-2 各自 2-deep MTE2 | 直接父版；缺口是两个 steady-state rings 之间仍串行 warm-up | 四项均以此缺口为 OFAT |
| R31B V016 | FP16/BF16 tile 4096→8192 | tile 不动 | `NOT_DUPLICATE_BY_SCOPE` |
| R31B V017 | pass-2 MTE3 completion wait 延后 | 本路线不改 store wait/drain | Store 方向 `DUPLICATE_REJECTED` |
| R31B Track-B H3 | 删除 pass-2/store 前与 inter-pass `SyncVToMTE2` | 单独“缩短/删除 inter-pass 全同步”与其相同 | `DUPLICATE_REJECTED`，不列为第五个 hypothesis |
| R31B Track-B H5 | pass-1 staging 2→3 | 新 buffer/queue depth 超边界，且历史 flat 风险高 | `DUPLICATE_REJECTED` |
| R31A V021 | store wait 推迟 | MTE3/store，不是 MTE2 inter-pass parameter warm-up | `ADJACENT_NOT_SAME` |
| R31A V024/V028 | norm-hoist；batch affine 尾 barrier 删除 | 算术位置/barrier，不是跨 pass parameter issue | 不吸收；单纯 barrier 删除不作为本路线项 |
| MIX-A V003 | 增加 V_MTE2 release sync + dispatch 限制 | 本路线不增/删 ownership gate；event 必须保持正确释放 | 作为 correctness 风险对照 |
| MIX-A V007 | 删除 Load 前第一个 `SyncVToMTE2` | 纯同步删除与本路线被拒项相邻，且未建立稳定收益 | `DUPLICATE_REJECTED` for pure sync deletion |
| ASYNC-TRIPLE-X | 只加 MTE3 stage/triple overlap | 本路线无 MTE3、无新 buffer；MAIN-2 已禁止重复纯 MTE3 | `NOT_DUPLICATE_BY_PIPE`; 纯 MTE3 提案拒绝 |
| COEFF-LOCALITY-X | parameter MTE2 时序/局部性，V001 +19.2% 退化，V002 无 UB budget | 不扩大 residency/UB；只移动已有首 tile issue | 负面先验：收益预期应保守 |
| EPILOGUE-FUSE / EPILOGUE-ARITH | affine 算术重排、融合、barrier grouping | H3 只把既有 normalization prelude 放到 wait 前，算术顺序不变 | `PARTIAL_ADJACENCY`, not arithmetic fusion |

### Wave-1 committed handoffs

| Wave-1 路线 | 相似项 | 本路线边界差异 | 处置 |
|---|---|---|---|
| `SYNC-TOPOLOGY-CHAMPION-X` commit `6a19f78c` | H2: pass-2 内先发下一 tile DMA；H3: FP32 Muls 覆盖 parameter wait | H1/H2/H4 处理 LP inter-pass first issue；H3 仅 LP first tile normalization。必须记录 partial adjacency | H3 可独立评审；不得声称机制家族全新 |
| `STORE-EPILOGUE-W2-X` commit `80676bc7` | Store issue/chunk/eligibility | Store policy 完全不动 | `NOT_DUPLICATE_BY_SCOPE` |
| `SELECTIVE-FASTPATH-CHAMPION-X` commit `9e5a5731` | 用完整 donor 做 shape dispatch | 本路线不导入 donor、不新增 dispatch | `NOT_DUPLICATE_BY_SCOPE` |
| `EPI-ARITH-CHAMPION-W2-X` commit `5031a5fa` | affine barrier grouping / VMLA / scale fold | 不改 affine op 或 batch grouping | `NOT_DUPLICATE_BY_SCOPE` |
| `SMALLMID-DATAFLOW-CHAMPION-X` commit `47dacfa` | small/mid output alias、tiny batch、store wait | 本路线只作用 LP wide D>8192、无 alias/store | `NOT_DUPLICATE_BY_SCOPE` |

### Wave-2 sibling boundary audit

审计时 `w2/m2/crossrow-pipeline`、`w2/m2/param-residency`、`w2/m2/row-occupancy`、`w2/m2/ub-lifetime-safe` 相对 `origin/main` 尚无 committed research delta，因此不读取任何 sibling 私有上下文，也不伪造其具体 hypothesis。仅按本任务硬边界避让：

- cross-row pipeline：本路线所有主探针单 row/core，禁止 next-row overlap；
- parameter residency：parameter 仍按 tile 搬运，不扩大生命周期；
- row occupancy：不改 row/core 数、blockCount 或 batchRows 决策；
- UB lifetime safe：不新增 alias，所有复用都必须发生在父版 release event 已确认之后。

## DUPLICATE_REJECTED ITEMS

1. `IPP-D1-DELETE-INTERPASS-SYNC`: 直接删除/缩短 `SyncVToMTE2()`；与 R31B Track-B H3、MIX-A V007 同机制，`DUPLICATE_REJECTED`。
2. `IPP-D2-QUEUE-DEPTH-3`: pass-1/pass-2 2→3 或新增第三 buffer；与 R31B Track-B H5、R013/R31B queue-depth 历史重复且越界，`DUPLICATE_REJECTED`。
3. `IPP-D3-PASS2-STORE-DRAIN`: 延后 pass-2 MTE3 wait；R31B V017、STORE/ASYNC 历史已覆盖，且属于 Store policy，`DUPLICATE_REJECTED`。
4. `IPP-D4-CROSSROW-EPI-PROLOGUE`: row N epilogue 与 row N+1 prologue overlap；与 `EPI-PIPE-3` 相同且任务明禁跨 row，`DUPLICATE_REJECTED`。
5. `IPP-D5-FULL-PARAM-RESIDENCY`: 跨 tile/batch 常驻 gamma/bias；R014/FULL-R014、R030、COEFF/BATCH 已覆盖，且属于 sibling parameter-residency 边界，`DUPLICATE_REJECTED`。

## EVIDENCE_PATHS

- `AGENTS.md`
- `.agents/skills/cann-mainline/SKILL.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/本地性能测试规范.md`
- `项目规则/服务器实验规范.md`
- `项目规则/Git工作流程.md`
- `项目规则/线上提交规范.md`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `技术路线/路线成绩表.tsv`
- `技术路线/全版本记录.tsv`
- `调度/当前任务.tsv`
- `线上结果/R31B/V011/{submission.asc,submission.sha256,source-meta.json,result.json,diff.patch}`
- `线上结果/R31B/V017/{diff.patch,source-meta.json,result.json}`
- `本地实验/R31B/V016/{source-meta.json,local-result.json,route-handoff.md}`
- `研究/主代理/MAIN-1/初始化报告.md`
- `研究/主代理/MAIN-2/初始化报告.md`
- `研究/NEXT-TRACK-B-OVERNIGHT.md`
- commit `b02e240c:研究/R31B/next-hypotheses.md`
- commit `cb304fba:研究/R31B/handoff-20260929.md`
- commit `6a19f78c:研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md`
- commit `80676bc7:研究/STORE-EPILOGUE-W2-X/TRACK-B-HYPOTHESES.md`
- commit `9e5a5731:研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md`
- commit `5031a5fa:研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md`
- commit `47dacfa:研究/SMALLMID-DATAFLOW-CHAMPION-X/track-b-handoff.md`

## PROPOSED_ONE_FACTOR_DIFF

Main 若后续选择，只能四选一：

- H1：仅把 tile-0 parameter DMA 发到最后 pass-1 unit 的空闲 slot；
- H2：仅把 tile-0 parameter DMA 从 reduction 后移到 reduction 前；
- H3：仅把 pass-2 first-tile ready wait 后移到首次 parameter consumer；
- H4：仅用父版现有两槽在 reduction 前 prime tile0+tile1。

不得在同一 Revision 合并 H1/H2/H3/H4，也不得顺手删除同步、加深 queue、改 Store 或改 tile。

## EXPECTED_LOCAL_PROBES

本轮不运行。若未来获得 Main selection，建议按现行唯一测时规范：

1. Identity/correctness matrix：FP16/BF16 `1x12288`, `1x16384`, `1x32768`；另用 `2x16384, blockCount=2` 验证每 core 单行复制行为。
2. 空白控制：FP16/BF16 `1x8192`（不走 LP wide path）；FP32 `1x16384`（本路线不改 FP32 path）。
3. 每个 active shape 先独立 same-binary PASS，再做至少 4 组相邻交错 P/C；保留 device-event raw、MAD/median、block drift、paired delta 和 load evidence。
4. H1/H2/H4 重点记录 tile-0/tile-1 event slot、tileCount、实际 tileWidth；H3 记录 first parameter consumer 前 wait 的准确位置。
5. 判定必须同时超过该 shape 当轮 noise floor，并与“只节省一次 inter-pass cold-start”的机制幅度相符；小局部收益不得外推 Official。

## CHILD_RECOMMENDED_HYPOTHESIS

`IPP-H2-REDUCE-FINAL-PARAM-PREFETCH`

理由：它最直接命中 MAIN-2 已确认仍可能独立的 inter-pass prologue prefetch 空间；不需要在仍活跃的 pass-1 slot 上做复杂 handoff，不改变 wait 数量、queue depth、UB、Store 或数学，OFAT 和正确性边界最清楚。推荐只表示 child research 排序，`MAIN_SELECTED` 仍为 `NONE`。

## OPEN_QUESTIONS

1. DAV-2201 上 tile-0 parameter MTE2 是否能与 V/S scalar finalization 真正并行，还是只提前进入同一串行命令队列？
2. V011 的 event ID 是否允许 `MTE2_V` ready event 跨 V/S、S/V 往返长期 outstanding；若不允许，H2/H4 需判 `INFEASIBLE`，不能用全同步删除绕过。
3. H1 最后 pass-1 unit 的相反 slot 在所有 tileCount 奇偶下是否都有可证明的 release-consumed 状态？需要逐 parity 静态审计。
4. H3 中 FP16 `ToFloat + Muls` 和 BF16 `Muls` 能覆盖多少 tile-0 DMA，是否因 MTE2/V issue 竞争反而延长首 tile？
5. H4 连续发两组 parameter DMA 时，硬件 FIFO 是否导致 tile-1 命令阻塞 tile-0 completion 可见性？
6. Official case shape map 仍缺失；本地单 row/core probe 只能验证机制，不能声称对应某个 Official testcase。
7. Wave-2 sibling handoff 尚未 committed；Main review 前应再次核对是否出现 parameter-residency、UB-lifetime 或 cross-row 机制碰撞。

## HANDOFF ASSERTIONS

```text
TRACK_B_RESEARCH_STARTED=YES
MAIN_SELECTED=NONE
REVISION_CREATED=NO
KERNEL_CHANGED=NO
BUILD_LINK_RUN=NO
CORRECTNESS_TIMING_PROFILING=NO
SERVER3_USED=NO
ONLINE_JUDGE_USED=NO
CANONICAL_LEDGER_CHANGED=NO
ROUTE_LIFECYCLE_CHANGED=NO
```
