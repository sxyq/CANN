# TRACK-B — MULTIROW-DMA-CHAMPION-X 候选假设

ROUTE: MULTIROW-DMA-CHAMPION-X
CONTEXT_CLASS: FROZEN_STRONG_BASELINE_TRANSPLANT
DIRECT_PARENT: R31B-V011
PARENT_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_SCORE: Official 45.16（15/15）
SEED: `线上结果/R31B/V011/submission.asc`（exact Champion source）
证据基础：`DMA-INVENTORY-R31B-V011.md`（只读盘点）、`ROUTE-DECLARATION.md`（去重对照）

范围纪律：以下全部候选只含 DMA 事务/搬运形态。不含 row scheduling、tiling、parameter locality、store merge。
排除形态重申：contiguous multi-row batch ownership（BATCH-RESIDENT）/ full-y multi-row residency（R31B V003）/ row_copy microkernel（MODE-X-R015C）均不得作为 V001。

---

## H1 — STRIDE-MULTIROW-WIDE-IN：wide full-y 输入侧 stride 多行 tile 列窗搬运（推荐 V001）

- **MECHANISM**：在 `ProcessWideLowPrecision` pass-1（V011 LP 行流水）中，把 unit 从 (row, tile) 改为 (tile, B 行)：同一 tile 列窗跨 `batchRows` 行的 x/residual 各用一条 stride 多行事务发出——`DataCopy(DataCopyExtParams(nBursts=B, blockLen=tileWidth*sizeof(T), srcStride=(rowWidth-tileWidth)*sizeof(T), dstStride=0))` 落入 B×tile 连续暂存。只处理满 tile（blockLen 与 srcStride 均 32B 对齐）；尾 tile 回退现有单 burst。数学序、逐 (行,tile) 的 Add/Mul/ReduceSum 写槽、full-y 驻留、参数加载、`Store`、所有 dispatch 门槛字节级语义不变。
- **BOTTLENECK**：wide 路径输入侧 MTE2 命令数与事件对数。现状每 batch 发 `2*batchRows*tileCount` 条单 burst + 同量级事件对；B≥2 时 stride 形态把它降到 `2*tileCount` 条。官方分差最大段 idx 14（ratio 4.40，time 16486.8µs vs best 3750.1µs）与 idx 15（9637.5µs 大时长）在 wide 大形状段；冠军 V011 的收益来自 (行,tile) 流 2-deep 流水，但发事务条数本身未减。
- **EXPECTED_SHAPES**：FP16/BF16 wide（D>8192）且 `localRows≥2`、`wideFullYRows_≥2` 的形状——FP16 y 存 half，UB 预算下 B 最容易到 2–8（如 8×16384、16×16384、8×32768、12×12288 一类 rows×D 探针）。B=1 的形状（部分 FP32/BF16 大 D）预期中性。mid/tiny/CONTIG/BATCH 路径形状预期不动。
- **WHY_IT_MAY_HELP**：(1) MTE2 发事务条数与 SetFlag/WaitFlag 对数按 B 倍下降，V011 流水的软件开销变浅；(2) 多行 burst 在同一 tile 列窗上天然 strided，正是 idea-pool R015 未做的「true multi-row stride DataCopy」；(3) 不与 V011 流水竞争——unit 变粗后 2-deep 仍可在 tile-group 之间重叠；(4) T14/T15 大时长段的每命令固定成本被摊薄。
- **WHY_IT_MAY_FAIL**：(1) B=1 时机制退化为现状（无收益、无损失）；(2) 2-deep ping-pong 与 B×tile 暂存的 UB 量相乘（2B tile），可能挤压 y 预算或逼 B 下降——必须先做 UB 数字核对，若 `ChooseWideFullYRows` 语义被牵动则要么缩 B、要么本假设不成立；(3) 若瓶颈在 V 端 ReduceSum/事件等待而不是命令条数，减发事务不动中位数；(4) 尾 tile 回退使 D 非 tile 整除的形状收益变小；(5) 历史上 nBursts>1 + pad 整行曾 RUNTIME_ERROR（R015），本假设靠 32B 对齐 + 非 Pad `DataCopy` + 尾部回退规避，但 NPU 实测前不能排除 stride 事务本身的驱动坑。
- **ASCEND_FEASIBILITY**：中高。`DataCopy` + `DataCopyExtParams` 的 nBursts/blockLen/srcStride 是标准 API；满 tile 时 blockLen（FP16 8192B / FP32 16384B 量级）与 stride 32B 对齐成立。风险在 UB 暂存布局与事件粒度，不在 API 可用性。
- **UB/CORE/DMA_IMPACT**：UB +（暂存方案）x/res 各多 (B-1) 或 (2B-2) tile（实现时给精确数字；B=2 时低精度路径可与现有 2-tile ping-pong 槽对齐，增量可控）。CORE：blockDim/所有权不动。DMA：输入事务数 `2*B*tileCount → 2*tileCount`（B 倍下降）；输出不动。
- **SYNC_IMPACT**：事件对随 unit 数下降（unit 数 B*tileCount → tileCount）；不新增 `PipeBarrier<PIPE_ALL>`；2-deep 流水语义保留（在 tile-group 粒度）。
- **PRECISION_RISK**：低。逐元素数学序不变；只是取数批大小变化。尾 tile 走原路径。仍需全 golden 矩阵（含 FP16/BF16 各 D 档）后才计时。
- **DUPLICATE_CHECK**：vs R015（整行 pad 多 burst + 8 行 ownership，RE）— 形态不同（tile 列窗 stride vs 整行 ownership），且有对齐/回退约束。vs BATCH-RESIDENT-X（contiguous batch ownership）— 不引入 ownership/batch 行选择器。vs MODE-X-R015C（row_copy 微内核）— 无独立搬运内核。vs R31B V003（full-y 驻留）— y 驻留原样保留，不改 `ChooseWideFullYRows` 语义。vs 冠军 CONTIG flat 多行 — 那是整行 flat 长 burst，这里是 pitch≠blockLen 窗口 stride。vs mode-selection — dispatch 门槛一字不动。vs SCHED-CHAMPION-X — 不改行归属。vs UB-LIVENESS-X — 暂存量只增不改生命周期别名结构；若有牵连，按证据上报 Main，不自行改 UB V004。
- **MINIMAL_OFAT_DIFF**：`ProcessWideLowPrecision` pass-1 的 unit 循环重排为 tile-outer/row-inner + 新增一个 stride 多行 Load 助手（或给 `Load` 加多行形态）+ x/res 暂存按 B 计数扩容（含 UB 数字注释）。`ProcessWideFp32*`、generic、mid、CONTIG/BATCH、host、dispatch 阈值、`Store` 全部不动。一个概念变化：**x/residual 的 tile 列窗从逐行单 burst 改为跨行 stride 多 burst**。
- **EXPECTED_LOCAL_PROBES**：(1) 先做只读 UB 数字推演（B、tile、y 预算三者关系，FP16/BF16 各 D 档）；(2) 编译 + NPU correctness（FP16/BF16 × wide D 档 + 回归 mid/small）；(3) same-binary 噪声底后，device-event 交错 P/C：FP16 8×16384、16×16384（B≥2 主场）+ FP16 4×8192 边界 + 一个 B=1 形状作阴性对照（预期 ~0）；(4) 判定用 median paired delta 与方向一致性，噪声内则 LOCAL_REJECTED（只否定该机制，不否定 stride 形态整体）。
- **CLASSIFICATION**：**READY_FOR_MAIN_REVIEW（推荐 V001）**

---

## H2 — STRIDE-MULTIROW-GENERIC-TILE-IN：generic 多 tile 路径（4096<D≤8192）同 tile 列窗 stride 多行输入

- **MECHANISM**：在 generic cacheParams 路径（L284-495）中，对 `localRows>1` 且 `tileCount≥2` 的形状，把逐 (行,tile) 的 `Load(x)/Load(res)` 改为同 tile 列窗跨 B 行的 stride 多行事务（参数同 H1：nBursts=B、blockLen=tileBytes、srcStride=(rowWidth-tile)*elem、满 tile 才合并）。y 驻留、预取逻辑、reduce 槽、输出不动。
- **BOTTLENECK**：同 H1——中宽段多 tile 输入命令条数；该段每行 2 tile × B 行 = 2B 条单 burst，可降到 2 条。
- **EXPECTED_SHAPES**：D∈(4096, 8192]、localRows≥2、tileCount=2 的形状（如 8×6144、12×5120 一类）。D≤128 tiny 与 mid 形状不动。
- **WHY_IT_MAY_HELP**：机制同 H1，作用在另一形状桶；该桶不被 wide 特化、不被 CONTIG/BATCH 门槛覆盖（CONTIG 只到 D≤2048/4096）。
- **WHY_IT_MAY_FAIL**：(1) 该段每行只有 2 个 tile，合并后事务数 2B→2，绝对条数本来就少，收益上限低；(2) cacheRow 预取（下一行首 tile）与 tile-outer 循环保真有交互，弄坏预取会退化或出错；(3) 与 H1 同型机制——若 H1 先落地，H2 是形状扩展而非新机制，必须等 H1 有结论后再立 Revision。
- **ASCEND_FEASIBILITY**：高（同 H1 API）。
- **UB/CORE/DMA_IMPACT**：暂存增量比 wide 小（tile=4096、B 可取 2–4）；CORE 不动；DMA 输入条数下降。
- **SYNC_IMPACT**：事件对减少；预取相关事件序需保持。
- **PRECISION_RISK**：低（数学序不变）。
- **DUPLICATE_CHECK**：同 H1 的排除逻辑；vs 冠军 CONTIG（D≤2048 flat）— 门槛不重叠。注意：本假设与 H1 是同一机制类型的不同形状桶，**不得与 H1 同 Revision**。
- **MINIMAL_OFAT_DIFF**：generic 路径 pass-1 的 load 发事务段 + 同一 stride 助手复用；其余不动。
- **EXPECTED_LOCAL_PROBES**：D=5120/6144/8192 这组的 P/C；H1 未收口前不启动。
- **CLASSIFICATION**：**NEEDS_MORE_EVIDENCE（排在 H1 之后，同机制第二桶）**

---

## H3 — ALIGNED-DATACOPY-FORM：32B 对齐处 DataCopyPad → DataCopy（事务指令形态）

- **MECHANISM**：在 `Load`/`Store` 助手中，当 `count*sizeof(T)` 为 32B 对齐时改发非 Pad 的 `AscendC::DataCopy`（同一 `DataCopyExtParams` 参数）；非对齐保持 `DataCopyPad`。不改 nBursts、不改循环、不改任何路径结构。
- **BOTTLENECK**：Pad 路径的每命令固定开销（对齐扩展参数、边界处理）在小 D、命令密集段可能非平凡；冠军全部用 Pad，即使 4096 元素满 tile 也如此。
- **EXPECTED_SHAPES**：命令密集的中小 D（mid/tiny/CONTIG 入口的对齐形状）；wide 满 tile 也有少量。非对齐形状（mid 的杂散宽度）预期不动。
- **WHY_IT_MAY_HELP**：单变量、diff 极小、风险低；若 Pad 开销真实存在，所有对齐路径同时受益。
- **WHY_IT_MAY_FAIL**：(1) 很可能两者微码同路，差异落在噪声内；(2) 与 multi-row 主线无关——它是「事务指令形态」而非「multi-row/stride」，信息增量独立但收益上限未知；(3) 若驱动对非 Pad 有隐藏约束（对齐边界恰在 tile 尾），可能出现边界 case 正确性问题。
- **ASCEND_FEASIBILITY**：高。
- **UB/CORE/DMA_IMPACT**：全 0。
- **SYNC_IMPACT**：无。
- **PRECISION_RISK**：无（同字节搬运）；仍要跑 golden 确认边界 tile。
- **DUPLICATE_CHECK**：历史路线未见 Pad/非-Pad 对照实验；与 R015/BATCH-RESIDENT/MODE-X/V003 四条排除形态都不同。与 DTYPE-SPECIAL-X（对齐本地拷贝助手）有轻微话题相邻——只动 GM↔UB 发事务形态，不动 dtype 专用计算路径，不越轴。
- **MINIMAL_OFAT_DIFF**：`Load`/`Store` 两个助手中各一处条件分支。
- **EXPECTED_LOCAL_PROBES**：对齐小 D（如 rows=1,D=256 FP32、33×100 对齐列宽）+ 一个 wide 满 tile 形状；先 same-binary。
- **CLASSIFICATION**：**READY_FOR_MAIN_REVIEW（备选 V001；若 Main 认为信息增量不如 H1 则降级）**

---

## H4 — STRIDE-MULTIROW-WIDE-IN-FP32：H1 机制的 FP32 wide 限定变体

- **MECHANISM**：与 H1 完全同一形态，但只改 `ProcessWideFp32FullCacheRows`（FP32 wide，官方分差最大段 idx 14 疑似所在段）。
- **BOTTLENECK**：同 H1；该路径更简单（无 V011 ping-pong，双层循环 batchRow×tile），diff 更干净。
- **EXPECTED_SHAPES**：FP32 wide 且 `ChooseWideFullYRows≥2` 的 D 档（需先推演：FP32 y=4B/行，D=12288 附近 B=2 可能成立；D=16384+ 常 B=1）。
- **WHY_IT_MAY_HELP**：(1) 路径结构简单，机制归因干净；(2) 直指 idx 14 这种 FP32-wide 抵抗型段（冠军注释：T14 resisted every FP32-wide change）。
- **WHY_IT_MAY_FAIL**：(1) FP32 wide 的 B 多半被 y 预算压到 1——机制大面积退化；(2) 若 idx 14 实际不是 FP32 wide，靶子落空（shape map 缺失）；(3) 与 H1 同机制，只能二选一先做。
- **ASCEND_FEASIBILITY**：高（同 H1）。
- **UB/CORE/DMA_IMPACT**：同 H1；FP32 暂存更贵（4B/元素），UB 余量更紧。
- **SYNC_IMPACT**：同 H1。
- **PRECISION_RISK**：低。
- **DUPLICATE_CHECK**：同 H1。
- **MINIMAL_OFAT_DIFF**：只动 `ProcessWideFp32FullCacheRows` pass-1 载入段。
- **EXPECTED_LOCAL_PROBES**：先 UB/B 推演；若 B≥2 的 FP32 wide D 档存在再谈 P/C（如 8×12288）。
- **CLASSIFICATION**：**NEEDS_MORE_EVIDENCE（先做 B≥2 存在性推演；H1 优先）**

---

## 推荐 V001 SINGLE_HYPOTHESIS

| 字段 | 值 |
|---|---|
| ROUTE | MULTIROW-DMA-CHAMPION-X |
| REVISION | V001（待 Main-2 批准后创建） |
| DIRECT_PARENT | R31B-V011 |
| PARENT_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_SCORE | 45.16（Official 15/15） |
| CONTEXT_CLASS | FROZEN_STRONG_BASELINE_TRANSPLANT |
| SINGLE_HYPOTHESIS | **H1 STRIDE-MULTIROW-WIDE-IN**：wide 低精度 full-y 路径输入侧，同 tile 列窗跨 B 行用一条 stride 多行 `DataCopy`（nBursts=B, blockLen=tileBytes, srcStride=(rowWidth-tile)*elem）替代逐行单 burst；满 tile 才合并，尾 tile 回退；其余全部不动 |
| WHY_NOT_DUPLICATE | 不等同 contiguous multi-row batch ownership（无 ownership/batch 行选择器，不搬连续整行）；不等同 full-y multi-row residency（y 驻留与 `ChooseWideFullYRows` 语义原样）；不等同 row_copy microkernel（无独立搬运内核，完整 AddRmsNormBias 数学保留）；冠军 CONTIG flat 多行与 V011 2-deep 流水都不含 nBursts>1/stride 事务；dispatch 门槛不动，不与 mode-selection 重叠 |
| EXPECTED_SHAPES | FP16/BF16 wide D>8192 且 localRows≥2、wideFullYRows_≥2（主靶 8×16384、16×16384、8×32768、12×12288 这组探针）；B=1 形状为阴性对照；mid/small/CONTIG/BATCH 回归必须不动 |
| OFAT | 一个概念变化：输入侧 tile 列窗发事务形态。不碰 Store、gamma/bias、dispatch、所有权、tile 宽度 |
| 风险前置 | 实现前先做 UB 数字推演（B×tile 暂存 vs y 预算）；若推演显示必须改 `ChooseWideFullYRows` 语义 → 停下报 Main，不得自行扩成 full-y 改动 |

选择 H1 的理由：它是去重结论点名的空白方向（true multi-row stride DataCopy）在冠军 hot path 上的最小落点；FP16 wide 是 `wideFullYRows_≥2` 最成立的形状组（y=half）；V011 刚在此形状组验证过收益方向；H3 作备选（更小但信息增量弱）；H2/H4 是同机制的第二桶/变体，须等 H1 结论。

## 停止条件

本文件已达 Track-B 返回条件（≥3 个已筛选假设 + 推荐 V001）。等 Main-2 批准后才创建 V001 与改 Kernel；未批准前 STOP。
