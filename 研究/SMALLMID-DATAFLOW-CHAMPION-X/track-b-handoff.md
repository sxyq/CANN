# SMALLMID-DATAFLOW-CHAMPION-X Track-B 交接

状态：`MAIN_SELECTED=YES`，V001 Candidate 已实现并待 Main 安排独立 server3 设备/job；尚未构建、跑正确性或测时。

## 路线范围

- `ROUTE_BOUNDARY`：仅限 `D<=4096` 的 V011 小/中 D 路径，研究核内既有行段的固定开销、UB 生命周期、参数准备与已有路径选择。研究函数限 `ProcessNarrowMidOverlap`、`ProcessSmallFp32ContiguousBatched` 及其既有 dispatch。
- `ALLOWED_CHANGES`：获 Main 选择并创建 Revision 后，一次只改一个候选机制；可在既有 `beginRow/localRows` 范围内调整数据暂存、系数转换复用、batch 内等待位置或小/中 D 函数分派条件。不得扩大每核行段。
- `FORBIDDEN_CHANGES`：不得改 `blockCount`、`blockIdx`、`baseRows`、`extraRows`、`beginRow`、`localRows` 的计算或核间行归属；不得新建 rowGroup、调整每核行数、跨核搬行、改 tile 几何或处理 `D>4096`。不得与 SYNC、STORE、INTERPASS/CROSSROW 或算术候选叠加；未收到 `MAIN_SELECTED=YES` 前不动 Kernel/Candidate、不建 Revision、不构建、不测正确性或时延、不访问 server3、不做 Online、不改共享台账。
- row-to-core 的硬限制来自 V011 `Process()`：`baseRows=rowCount/blockCount`、`extraRows=rowCount%blockCount`、`beginRow=blockIdx*baseRows+min(blockIdx,extraRows)`、`localRows=baseRows+(blockIdx<extraRows)`。本路线只能读这些值决定是否命中既有核内路径；其值及每行所属 core 必须与 V011 完全相同。
- `PARENT_SOURCE_SHA`：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。R31B V011，Official anchor `45.16`。已核对 `source-meta.json` 三个 source 字段、`submission.sha256` 与提交的 `submission.asc` 实际 SHA，完全一致；V011 的父版记录为 V010。
- `MAIN_SELECTION_CONFIRMATION`：Main-1 在 `研究/主代理/MAIN-1-W2/campaign-status.md` 记录本路线选择 `SMD-H6`；依据已推送 receipt commit `2a27be0b`。该记录确认 Main-2 `PARAM-RESIDENCY` 是 `D>8192` 的 GM cache-policy 路径，本提案是 `D<=4096` mid path 的 BF16 参数转换复用，两者机制和范围不同。
- `V001_REVISION_DECLARATION`：`本地实验/SMALLMID-DATAFLOW-CHAMPION-X/V001/revision-declaration.md`；必须先于 Candidate 源码提交。

## Wave-2 边界

| 相邻方向 | 本路线关系与限制 |
|---|---|
| ROW-OCCUPANCY / `SCHED-CHAMPION-X` | 对方只改 row ownership、core assignment、row-group scheduling。本路线固定上述 V011 行分配公式；`localRows` 只作既有路径条件，不据此改变分配。Wave-2 selective-fastpath 的整段 donor 分派也不移植。 |
| SYNC / `SYNC-TOPOLOGY-CHAMPION-X` | 对方已提交假设落在 wide FP32/low-precision 路径。本路线不改那些函数或事件；SMD-H5 只重排 small FP32 contiguous batch 的 MTE3 等待，机制相邻，需 Main 确认跨路线归属后才可能选择。 |
| STORE / `STORE-EPILOGUE-W2-X` | 对方改 wide path 的 Store 发出、chunk 或启用 predicate。本路线保留每个既有 batch 的 Store 地址、长度、次数與调用形态，不合并写回。 |
| INTERPASS/CROSSROW | 不改两遍处理边界，不把一行交给另一 core。SMD-H5 可能让同一 core 既有行段中 batch N 的 Store 与 batch N+1 的计算重叠；这是跨 batch 的时间重叠，不改 ownership。是否归 SYNC 或 INTERPASS/CROSSROW lane 由 Main 确认，未确认前不选它。 |
| EPI arithmetic | 不改变 `invRms -> norm -> gamma -> bias` 的逐元素表达式、舍入或顺序。SMD-H6 只提前并复用 BF16 gamma/bias 到 FP32 的精确转换；属于参数准备，和算术 lane 相邻但不是其表达式重排。 |

## 已提交证据

- 父版与路径：`线上结果/R31B/V011/source-meta.json`、`submission.sha256`、`submission.asc`、`diff.patch`；`submission.asc:171-245` 是固定行归属与 dispatch，`:499-619` 是 mid overlap，`:1241-1255` 是小/中宽度常量，`:1576-1662` 是 FP32 small batch，`:1670-1774` 是 low-precision small batch。
- H001 重复证据：`归档/历史阶段/retired-routes/H001/HANDOFF.md`；`线上结果/H001/V008/submission.asc:130-177,268-269` 已有多行 `CopyRows`、`DataCopyPad` 与 padding 后 batch 搬入；V008 15/15、29.04，路线主变化在 wide reduction。
- SMD-H3 被拒前的候选细节保留供重复审计：目标为 `R={64,256,1024}`、`D={65,73,127}`、FP32/FP16/BF16；原提案是在既有 `localRows` 内用 32-byte padded row stride 与逐行 `DataCopyPad`，试图共用 MTE2-to-Vector 等待。该候选无独立收益数据；padding 可能抵消固定成本节省，且输出仍逐行搬运。因 H001/MID-X 已有相同 padded multi-row staging 机制，现标记 `DUPLICATE_REJECTED`，不计入候选。
- MID-X 重复证据：`归档/历史工作区/MID-X/architecture-metadata.md`、`V001/kernel.asc:40-47,110-151`、`V002/kernel.asc:9,45-60,177-188`、`V003/kernel.asc:4,40-47,141-145`。V001/V002 已有 padded row staging 与 `batchRows`；V003 的 D<=128 tiny 分支仍复用相同搬入机制。V001 有部分 mid/wide case 耗时记录、线上 2/15；V002/V003 为 Runtime Error。这些整版结果不代表该搬运子机制单独失败。
- 路线索引：`技术路线/技术路线总表.md`、`技术路线/技术路线图.md`、`技术路线/全项目成绩与技术路线盘点.md`、`技术路线/全版本记录.tsv`、`技术路线/路线成绩表.tsv`、`调度/当前任务.tsv`、`调度/主代理分工.md`、`调度/本地线上校准.tsv`。
- Wave-2 已提交 handoff：SYNC `9071292b:研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md`；STORE `415a2429:研究/STORE-EPILOGUE-W2-X/TRACK-B-HYPOTHESES.md`；SELECTIVE-FASTPATH `9e5a5731:研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md`；EPI `0ce5441e:研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md`；row ownership 规则 `研究/SCHED-CHAMPION-X/TRACK-B-BRIEF.md`。
- R31A/R31B、MIX 与 Wave-1 证据：`本地实验/R31A/V021/`、`线上结果/R31A/V024/`、`线上结果/R31A/V028/`、`线上结果/R31B/V017/`、V011 行源码与 `技术路线/全版本记录.tsv`；`线上结果/MIX-A/V003/`、`V004/`、`V005/`、`本地实验/MIX-A/V007/`；`线上结果/STORE-EPILOGUE-X/V003/`、`线上结果/EPILOGUE-ARITH-CHAMPION-X/V001/`、`V002/`。Official case 到 shape/dtype 对照缺失，见 `研究/OFFICIAL-CASE-ANALYSIS.md`；下述局部 shape 不能映射成特定 Official case。

历史边界摘要：R015/FULL-R015 已覆盖多行 DMA（FULL-R015 为 1/15 Runtime Error）；R013/FULL-R013 覆盖通用双缓冲；R014 覆盖参数驻留；R028/FULL-R028 覆盖标量同步削减。H001/MID-X 已覆盖 padded multi-row 输入搬运。R31B V010 有 mid overlap 记录，但没有有效的 parent/candidate 配对本地数据；V014 将 aligned 小 D batch cap 从 8 提至 32，仅有未提交构建记录、无正确性或性能结果；V015 的 FP32 single-row mid dispatch 本地配置失败，记录为 `MEASUREMENT_BLOCKED`。纯静态参数预留因缺少运行时开销证据未列为候选；tiny dispatch 因与 MID-X V003、R31B V015 相邻且缺少本地数据未列为候选。MIX-A V003 改 `localRows==1` 分派并加释放同步，属于多项变化；V004/V005 的 tiny FastKernel 分派 Official 低于 V003。Wave-1 R31B V017 Official 44.68、STORE V003 44.38、EPI V002 44.96，均低于 45.16；这些邻近路线只作重复与风险证据，不外推为本假设结果。

## 假设池

### SMD-H2：FP16 mid 输出复用已消费的 x UB

- `HYPOTHESIS_ID`: `SMD-H2-FP16-MID-OUTPUT-ALIAS`
- `MECHANISM`: 对 `D={2049,3072,3073,4095}` 中实际命中 FP16 `ProcessNarrowMidOverlap` 的 shape，归约完成后用 `xBuf_` 承接转换后的输出，省去该路径的独立 `outputBuf_` 预留；其余 shape 保持 V011。
- `BOTTLENECK`: mid FP16 每核为单行输出再保留一份 tile-sized UB。
- `DIRECT_PARENT`: R31B V011；`PARENT_SOURCE_SHA` 如上；Official `45.16`。
- `TARGET_SHAPES`: FP16 `R={1,8,32,128}`、`D={2049,3072,3073,4095}`；必须记录实际分支和 `localRows`。
- `TARGET_DTYPES`: FP16。
- `WHY_IT_MAY_HELP`: 该路径可少占最多 `4096*sizeof(half)` UB；同函数 BF16 分支已在输出阶段使用 `xBuf_`。
- `WHY_IT_MAY_FAIL`: 不减 GM 字节或向量算术；释放 UB 未必改变时延。提前覆盖 x 输入或过早复用其地址会造成数据竞争。
- `UB_IMPACT`: 目标 FP16 路径少一份最多 4096-element 输出缓冲；不增加 UB。
- `DMA_IMPACT`: GM 读写字节、方向、次数与 Store 长度不变。
- `SYNC_IMPACT`: 不改事件数量与位置；保留下一行输入复用前的 `V_MTE2`/`MTE3_V` 等待及最终 drain。
- `PRECISION_RISK`: 低；转换与 affine 次序不变，主要风险为 UB 生命周期。
- `DUPLICATE_CHECK`: `线上结果/R31B/V011/submission.asc:599-610` 已给出 BF16 同函数的 x-buffer 写回先例；未见 FP16 该分支采用此 buffer。`UB-LIVENESS-X V003` 是两遍间 phase-role alias，机制相邻；本项仅在单行 reduce 已消费输入后复用 x-buffer，不改变跨 pass 生命周期。保留为窄 dtype/path 候选，重复风险中。
- `RELATED_OLD_ROUTES`: R31B V011；UB-LIVENESS-X V001-V003；H001 V008；MID-X V001-V003。
- `PROPOSED_ONE_FACTOR_DIFF`: 僅改 FP16 目標 shape 的 `outputBuf_` 初始化條件與 `ProcessNarrowMidOverlap` 輸出 tensor 來源；不移動任何等待、DMA 或算術。
- `MINIMAL_EXPERIMENT`: Main 选择后，对 R={1,8,32,128}、D={2049,3072,3073,4095} 先核对实际 dispatch、父子正确性及 Store 完成前 x-buffer 不被覆盖；通过后同设备交错测时，并含 D=2048/4096 邻近控制。不得改变 blockCount 或行分配。
- `UNCERTAINTY`: 中；有 BF16 同函数先例，FP16 生命周期与时延收益未验证。

### SMD-H5：FP32 small batch 输出双槽延后复用等待

- `HYPOTHESIS_ID`: `SMD-H5-DEFER-BATCH-STORE-WAIT`
- `MECHANISM`: 仅对 `ProcessSmallFp32ContiguousBatched`，把现有 `valueFp32Buf_` 分成两个最多 4096-element 槽交替写入；MTE3 完成等待移至对应槽下次复用前，保留最终 drain。先确认现有缓冲容量至少容纳两槽。
- `BOTTLENECK`: 当前每个 local batch Store 后立即 `SyncMTE3ToV`，挡住后续 batch 的 value 缓冲计算。
- `DIRECT_PARENT`: R31B V011；`PARENT_SOURCE_SHA` 如上；Official `45.16`。
- `TARGET_SHAPES`: FP32 `D={64,256,512,1024,2048}`；选用现有 launch 下可产生 1、2、3+ local batches 的 R，并记录 `blockCount/localRows/batchRows`。
- `TARGET_DTYPES`: FP32。
- `WHY_IT_MAY_HELP`: 每批最多 4096 elements；现有 value UB 若满足双槽容量前置条件，可让前批 Store 与后批计算重叠。
- `WHY_IT_MAY_FAIL`: 单 batch 没有重叠；Store 可能短于下一批准备；等待或槽复用顺序错误会覆盖未完成输出。
- `UB_IMPACT`: 总 buffer 不扩大，仅分成两个最多 4096-element 槽；容量不足则此项不可行，不得缩 tile 或改分配。
- `DMA_IMPACT`: GM 字节、Store 次数、每笔地址及长度不变。
- `SYNC_IMPACT`: 把 batch 尾部全局等待移到同槽复用点；事件数量不增加，最后等待全部未完成 Store。
- `PRECISION_RISK`: 无算术改动；风险集中在 MTE3 完成与 UB 槽生命周期。
- `DUPLICATE_CHECK`: 与 Wave-2 SYNC H1 的 wide FP32 full-cache 入口等待、R31B V017 的 wide low-precision drain 相邻，但函数、D 区间和槽位不同；与 STORE 路线不同，保留每批 Store 形态。可能涉及 same-core 跨 batch Store/compute overlap，须由 Main 确认是否归 INTERPASS/CROSSROW lane。
- `RELATED_OLD_ROUTES`: R013/FULL-R013；R31B V011/V017；ASYNC-TRIPLE-X；Wave-2 SYNC、STORE。
- `PROPOSED_ONE_FACTOR_DIFF`: 只改 small FP32 contiguous batch 的 value slot offset 与 MTE3 wait 时点；不改 batchLimit、batch 内容、Store 形态、算术或 row ownership。
- `MINIMAL_EXPERIMENT`: Main 选择后先静态确认 value buffer 容量与槽复用顺序；correctness 覆盖 1/2/3+ batches 和尾批，确认最终 drain。通过后测 D={64,256,512,1024,2048}，带单 batch 与不命中 dispatch 的控制，按同设备交错 P/C。
- `UNCERTAINTY`: 中高；buffer 容量与实际 Store 可隐藏时间未测；跨路线归属待 Main 确认。

### SMD-H6：BF16 mid 参数转换每核一次

- `HYPOTHESIS_ID`: `SMD-H6-BF16-MID-PARAM-CAST-ONCE`
- `MECHANISM`: 当 `ProcessNarrowMidOverlap` 中 `localRows>1` 时，加载 gamma/bias 后一次性转入已有 `gammaFp32Buf_`/`biasFp32Buf_`；各行复用转换结果，移除循环内重复的两次 `ToFloat`。不命中该条件时保持原代码。
- `BOTTLENECK`: BF16 mid 路径已驻留 gamma/bias，却在每个 local row 上重复转换相同参数。
- `DIRECT_PARENT`: R31B V011；`PARENT_SOURCE_SHA` 如上；Official `45.16`。
- `TARGET_SHAPES`: BF16 `D={2049,3073,4095}`。V011 的 `run_kernel` 按 `availableCoreNum` 与 `rowCount` 计算 `blockCount`，并将其截到 `UINT32_MAX`；本地可达性探针取正的设备 `availableCoreNum=A`、leading-dimension 乘积 `rowCount=2*A`，则每个 core 的既有公式得 `localRows=2`。不改 `blockCount` 或行归属。
- `TARGET_DTYPES`: BF16。
- `WHY_IT_MAY_HELP`: 复用每核常量参数，把两条 D 长度 Cast 从每行各做一次降为每核各做一次；V011 已分配这两块 FP32 缓冲，并在 generic cache、small low-precision batch 与 BF16 full-tile 函数中采用过一次转换后复用。
- `WHY_IT_MAY_FAIL`: 目标 D/R 可能不命中 `ProcessNarrowMidOverlap` 或 `localRows<=1`；首行输入前执行的转换及每行条件分支开销可能抵消省下的转换；已有缓冲可能存在未识别的该函数内生命周期约束。
- `UB_IMPACT`: 不新增或扩大 UB；复用已分配的 `gammaFp32Buf_`、`biasFp32Buf_`，目标宽度不超过 4096。
- `DMA_IMPACT`: 参数 GM Load 次数、字节数及输入/输出 DMA 不变。
- `SYNC_IMPACT`: 保留现有参数 `MTE2_V` 等待及释放次序；只把转换放在参数就绪后、行循环前。
- `PRECISION_RISK`: 低；BF16 到 FP32 是精确扩展，逐元素 `Mul/Add` 顺序不变；仍需验证输出一致。
- `DUPLICATE_CHECK`: `Process()` 在 mid dispatch 后提前 return，绕过下方 `cacheParams && cacheParamFp32` 的每核参数转换；V011 small-low-precision batch 与 BF16 full-tile 路径已展示相同复用模式。R014/FULL-R014 和 COEFF-LOCALITY-X 研究参数驻留/搬运，不等同于减少此处逐行转换；EPI-W2 仅相邻，因本项不改 post-invRms 算术表达式或次序。
- `RELATED_OLD_ROUTES`: R014/FULL-R014；R31B V011；COEFF-LOCALITY-X；DTYPE-SPECIAL-X；Wave-2 EPI。
- `PROPOSED_ONE_FACTOR_DIFF`: 仅在 BF16 `ProcessNarrowMidOverlap` 的 resident-parameter 分支增加两次一次性转换，并让该分支逐行读取 FP32 参数缓冲；不改输入转换、ReduceSum、invRms、输出转换、等待或 Store。
- `MINIMAL_EXPERIMENT`: 声明提交后仅实现预转换复用。目标 D 均满足 `128<D<=4096`、不命中 `D<=2048` 的 low-precision contiguous 分支，并进入 `ProcessNarrowMidOverlap`。BF16 narrow `Init` 为两个参数各分配 8192 个 FP32 元素；mid 分支提前返回，所用缓冲在本函数内无并行消费者。Main 分配独立设备/job 后，对三个 D 各用 `rowCount=2*availableCoreNum` 跑正确性并核对实际 dispatch、localRows 与输出；通过后按统一协议测试。不得改行分配或其他机制。
- `UNCERTAINTY`: 中；V011 控制流和缓冲生命周期已静态确认，官方 testcase 的 shape 映射仍缺失；收益尚未构建、验证或测量。

### SMD-H3：重复项，拒绝保留

- `HYPOTHESIS_ID`: `SMD-H3-UNALIGNED-LOCAL-BATCH`
- `STATUS`: `DUPLICATE_REJECTED`；不计入有效候选数。
- `MECHANISM`: 在既有每核行段内对非对齐 tiny-D 使用 32-byte padded row stride 与逐行 `DataCopyPad` 暂存，再逐行归约。
- `DUPLICATE_CHECK`: H001 V008 `CopyRows` 已对多行做 padded-stride `DataCopyPad`；MID-X V001/V002/V003 均已有 padded row staging、`batchRows` 及 D<=128 路径。改成只针对 V011 tiny D 不改变主要机制，故拒绝；新增 SMD-H6 为参数 Cast 复用，机制独立。
- `EVIDENCE_PATHS`: `线上结果/H001/V008/submission.asc:130-177,268-269`；`归档/历史工作区/MID-X/V001/kernel.asc:40-47,110-151`、`V002/kernel.asc:9,45-60,177-188`、`V003/kernel.asc:4,40-47,141-145`。

## 交接给 Main

- `VALID_NON_DUPLICATE_CANDIDATES`: 3（SMD-H2、SMD-H5、SMD-H6）。未写 `ROUTE_HYPOTHESIS_POOL_EXHAUSTED`。
- `CHILD_RECOMMENDED_HYPOTHESIS`: `SMD-H6-BF16-MID-PARAM-CAST-ONCE`；Main-1 已记录选择，见 receipt `2a27be0b`。
- `PROPOSED_ONE_FACTOR_DIFF`: 每个有效假设各有独立差异说明；不得组合。
- `EXPECTED_LOCAL_PROBES`: Main 已选择 SMD-H6。源码身份与 diff 静态核对完成；compile、correctness 和 timing 等 Main 分配独立设备/job 后进行。核对实际分支、`blockCount/localRows` 与逐元素输出；correctness PASS 后才按规范交错测时。官方 shape 映射仍未知。
- `OPEN_QUESTIONS`：
  1. H001/MID-X 的多行 padding 先例覆盖面明确；SMD-H3 已拒绝。Main 是否认可 SMD-H2 的单行输入消费后复用与 UB-LIVENESS-X 的跨 pass alias 为不同局部生命周期，需由 Main 判断。
  2. H5 的 `valueFp32Buf_` 实际可用容量是否至少 8192 elements，以及跨 batch Store/compute 是否属于当前 INTERPASS/CROSSROW lane，均未从本路线证据确定。
  3. 本地参数化探针可确定触发 H6；实际 Official shape 的 `rowCount/availableCoreNum` 分布未知，不能外推覆盖率。
  4. V011 的 Official testcase 缺少 shape/dtype 映射；局部探针无法单独证明总分收益。
  5. Wave-2 peer handoff 均以已提交版本为依据；未读取其他 Agent 的未提交材料、私有上下文或工作树。

Main 已选定 SMD-H6。V001 Candidate 已按声明实现；构建、正确性与测时等 Main 分配独立设备/job 后再做。本路线不改共享记录，不实施 Online。
