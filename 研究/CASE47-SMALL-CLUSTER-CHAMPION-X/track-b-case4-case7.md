# CASE47-SMALL-CLUSTER-CHAMPION-X / Track-B

DATE: 2026-10-03
STATUS: NEEDS_MORE_EVIDENCE
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

## 结论

仓内证据不能确认 Official case4 与 case7 的 shape、dtype、rows、D、实际 dispatch、core ownership 或 tile，也不能判断它们是否走同一结构路径。源码能说明 R31B-V011 对给定运行时输入的分派规则；Official 结果没有保留这两个 testcase 的运行时输入元数据。此缺口未补齐前，本研究停在假设阶段，不创建 Revision，不提出 case-specific Candidate。

本轮只以指定 Direct Parent 为代码参照；没有改用任何 Local-positive Parent。没有运行服务器、设备、编译、正确性或计时任务。

## Official 证据与缺项

| 仓内记录 | 可确认 | 不能确认 |
|---|---|---|
| `线上结果/R31B/V011/result.json` | index 4 / testcase `6a9a9a99bf41025d6013eb96` / 16.55 us；index 7 / testcase `6a9a9a99bf41025d6013eba2` / 52.34 us | 输入 rank/shape、dtype、扁平 rows、D、运行时 `availableCoreNum`、命中的源码分支及 tile |
| `线上结果/R31B/V011/source-meta.json` | 提交源码来源与 SHA256；未记录 Judge 输入元数据 | testcase ID 到输入 shape/dtype 的映射 |
| `线上结果/R31B/V011/submission.asc` | 对运行时 `TensorInfo` 的通用处理方式与分派条件 | case4/case7 对应的运行时 `TensorInfo` 值 |
| `线上结果/SCHED-ROWGROUP-X/V001/result.json` | 同一 testcase ID 在另一份 Official 结果中也出现 | 同样没有输入 shape/dtype；不能用该路线的本地探针替代 Official 映射 |
| `研究/主代理/MAIN-1/初始化报告.md` 第 454、544 行附近 | 记录指出 R31B/R31A/MIX-A 缺少 shape-to-case 映射，并将取得 Judge 映射列为未决问题 | 本报告之外的新映射来源 |
| `研究/OFFICIAL-CASE-ANALYSIS.md` | 含逐 case Official 时间 | “Case class identification”明示按时间幅度推断；本任务不采纳这类类别与 shape 判断 |

对 testcase ID 与时间的全仓搜索只命中 R31B-V011 与 SCHED-ROWGROUP-X V001 的 Official 结果文件；没有找到 testcase 输入元数据文件。`研究/CASE47-SMALL-CLUSTER-CHAMPION-X/` 在本轮前不存在，也没有既有路线记录或 Revision 目录。

### SUPPORT-A case-family 映射协同

当前分配 worktree 的研究、路线记录与 Official 结果中未找到 SUPPORT-A 的 case-family 映射。后续若收到该映射，按 `(index, testcaseId)` 与上表的 case4/case7 关联；只有映射同时给出可追溯的输入 shape、dtype 来源时，才补入逐案元数据。若映射只给 case family 标签，shape/dtype 仍记未知。此路线不等待 SUPPORT-A 的其他 lane 工作，也不把映射到手视为 Revision 批准。

缺项逐案记录：

| case | shape | dtype | rows M | D | dispatch | core ownership / active blocks | tile |
|---|---|---|---|---|---|---|---|
| 4 | 未找到 | 未找到 | 未找到 | 未找到 | 未能映射 | 规则可读，实际值未找到 | 未能映射 |
| 7 | 未找到 | 未找到 | 未找到 | 未找到 | 未能映射 | 规则可读，实际值未找到 | 未能映射 |

## Direct Parent 源码事实

以下是通用实现规则，不代表 case4 或 case7 的实际输入：

| 源码位置 | 可读事实 |
|---|---|
| `submission.asc:3493-3551` | Host 取 `D` 为 x 的最后一维，`M` 为前置维度乘积；dtype 0/1/2 分别启动 FP32/FP16/BF16 模板。launch block 数为 `min(availableCoreNum, M)`，其中 `availableCoreNum` 来自调用方。 |
| `submission.asc:159-176` | 普通行路径用 `GetBlockIdx()`；每个 block 获得连续行段，`baseRows=M/blockCount`、余数前置分配，故单次运行的精确 ownership 还需要 M、blockCount 和运行时 core 数。宽行实现也按相同的 M/blockCount 方式计算行段。 |
| `submission.asc:163-240` | `D>8192` 进入 wide 路径。非 wide 路径中，FP32 小行批处理要求本地多行且 D 满足 `%8==0`；FP16/BF16 对应的小行批处理要求 D 满足 `%16==0`。若前述专用分支未返回，`128<D<=4096` 进入 `ProcessNarrowMidOverlap`。 |
| `submission.asc:1241-1288` | 普通 tile 为 4096 elements，cache row 上限为 8192。wide full-y tile 初值为 4096、最多 8 行、UB 预算 176 KiB；`ChooseWideFullYRows` 按 dtype、D 和 buffer 预算选行数，必要时把 tile 以 512 为步长缩小到 2048。 |
| `submission.asc:202-230` | FP16/BF16 在 `D=4096` 或 `D=8192` 且本地行数大于 1 时有专用批处理/整行路径；这是源码条件，不能据此归类任何 Official case。 |

因此，case4 与 case7 是否命中同一路径，至少需要各自的 dtype、M、D；要给出实际 core ownership，还需要运行时 `availableCoreNum`。只有获得这些值后，才可把代码条件展开为准确分派与 tile 事实。

## 重复项核对

| 对照 | 已记录机制 / 结果 | 对本路线的约束 |
|---|---|---|
| R016 / FULL-R016 | 按 D 档设置 rows-per-task（16/8/4/2/1），`taskCount=ceil(M/rowsPerTask)`，block 数取 `min(availableCoreNum,taskCount)`；未对齐行会退到 1 block，`D>16384` 另有 8-block 上限。证据：`归档/历史工作区/SCHED-ROWGROUP-X/PARENT-R016-COMPILEFIX-kernel.asc:435-469`、`技术路线/技术路线总表.md:34,72`。 | R31B-V011 改用规则分行，但仍按 `min(availableCoreNum,M)` 启动 blocks。按 M/D/core 数重算一般活动核数，或复述 rows-per-task，落在已有调度轴内。 |
| SCHED-ROWGROUP-X / SCHED-CHAMPION-X | SCHED-ROWGROUP 在 R016 调度上加入 `rowGroup=32/gcd(rowBytes,32)` 并据此取整任务行数；SCHED-CHAMPION V002 记录 active-core 保留条件 `totalGroups*2 >= min(blockCount,rowCount)`。证据：`线上结果/SCHED-ROWGROUP-X/V001/submission.asc:433-485`、`技术路线/全版本记录.tsv:65-67`。INTEGRATION-X 将 SCHED V002 与 VECTOR-MATH 组合后，Official case4/6/7 时间分别退至 31.32/49.04/76.45 us；这些仍不是输入 shape/dtype 证据。 | 当前泛化的 `ACTIVE_CORE_COUNT` 提议若按行数、行组数或任务数选择活动核，判为重复；V011 的 block 数已经覆盖 `min(availableCoreNum,M)`。若另指“低于该上限主动减核”，方向不同，但现有记录没有对应瓶颈证据，需先有 case 映射和可区分机制后再单列研究。 |
| ROW-OCCUPANCY | 在可检索的正式路线表、调度表、研究目录和历史工作区中，没有找到精确名为 `ROW-OCCUPANCY` 的路线或研究记录。最接近的正式机制是上列 R016/SCHED 的行任务与 active-core 设计。 | 不把未找到的标签当成路线事实；新增调度想法先与 R016/SCHED 的公式逐项比对。 |
| reduction / math | REDUCE-HIER-X V004/V005 在宽行归约变体上有稳定退化记录；VECTOR-MATH-X 的 SEQ-FUSE-2 有单独研究记录。 | 仅改 `ReduceSum`/标量 V-S 序列没有新增机制证据，先按重复方向处理。 |
| tile / UB | SHAPE-TILING-CHAMPION-X 的研究覆盖 shape-conditioned tile/UB；R31B V016 已有 FP16/BF16 wide tile 4096→8192 假设。 | 宽 tile 方向不能直接作为新路线主假设；要先证明探针 D/dtype 与现有范围不同。 |
| async / multi-row DMA | ASYNC-OVERLAP-CHAMPION-X 已研究窄中行与 wide 低精度 issue 次序；MULTIROW-DMA-CHAMPION-X 已测 stride multi-row DMA，V001 回退、V002 落在噪声内。 | 仅重排已有 MTE2/MTE3 或再次合并 multi-row DMA，按重复处理。 |

## Track-B 假设

### H1：短而未对齐行的 padded-UB 批量 epilogue

- `MECHANISM`：在保持 Host block 数与行 ownership 不变的条件下，用 `DataCopyPad` 将同一 block 的相邻短行放入每行 32B 对齐的 UB 布局；逐行归约保持不变，只研究多行 epilogue 算术能否合并调用。
- `BOTTLENECK`：短行上的逐行拷贝描述符与逐元素 epilogue 调用开销。
- `EXPECTED_SHAPES / DTYPES`：Tiny 或短 Medium；需 `M>availableCoreNum` 使某个 block 有多行；优先核实 FP32 `D%8!=0` 与 FP16/BF16 `D%16!=0` 的分支缺口。不能据此声称 case4/7 属于此类。
- `WHY_IT_MAY_HELP`：源码已有对齐宽度的小行批处理；未对齐宽度目前不能通过相同谓词，可探索把行距 padding 与多行向量 epilogue 分开处理。
- `WHY_IT_MAY_FAIL`：Level-2 `ReduceSum` 仍逐行执行，标量 V/S handoff 未减少；padding 增加 UB 与搬运字节；M 不足以形成多行时无收益。若实现退化为 multi-row burst DMA，则和既有 DMA 路线重叠。
- `ASCEND_FEASIBILITY`：`DataCopyPad` 可处理有效字节数与对齐后 UB 行距；Reduce count 必须仍使用真实 D，UB 行偏移使用 padding 后 stride。逐行归约可沿用 Level-2 接口。多行 epilogue 是否可在目标 dtype 和参数布局下合并，尚需 API 级原型论证。
- `UB/CORE/DMA_IMPACT`：UB 占用随 padded D 与批量行数增加；core 切分不变；GM 字节数可能因 padding 略增，拷贝命令数可能下降。
- `SYNC_IMPACT`：每行归约同步不变；只有 epilogue 指令/屏障数可能下降。
- `PRECISION_RISK`：保留逐行 FP32 square-sum 与原运算次序时低；FP16/BF16 epilogue 合并若改变舍入位置则需逐 dtype 验证。
- `DUPLICATE_CHECK`：与 R016/SCHED 的任务 ownership 不同；与 Parent 已有的对齐小行 batch、BATCH-RESIDENT 的多行 batch 以及 MULTIROW-DMA 有邻接。要成立，差异必须限于“未对齐 UB 行布局 + epilogue 算术”，不含新 rows-per-task、参数驻留或 multi-burst DMA。
- `MINIMAL_OFAT_DIFF`：只加一个未对齐短行分支；保持 blockCount、每行 ReduceSum、参数缓存、DMA 方案和其他 dtype 分支不变。
- `EXPECTED_LOCAL_PROBES`：先取得 case4/7 的真实输入；用 Parent 对精确 case shape 建立 same-binary，再与同 dtype 最近的对齐宽度作 guard。源码边界优先看 `D=128/129`、`D=2048/2049` 以及 `%8`/`%16` 相邻宽度；只在本路线获 Main 批准后执行。
- `CLASSIFICATION`：`NEEDS_MORE_EVIDENCE`。

### H2：按有效任务数重算 active core / rows-per-task（重复方向）

- `MECHANISM`：从 M、D 与可用核数重算 block 数或每 block 行数。
- `BOTTLENECK`：假设 block 数偏少造成的核利用不足，或过多 block 的启动成本；目前 Direct Parent 证据不支持前一种情况。
- `EXPECTED_SHAPES / DTYPES`：小 M 或小 D 的 Tiny/Medium，dtype 不构成限制；case4/7 是否满足条件未知。
- `WHY_IT_MAY_HELP`：若新证据显示某个输入未达到现有 `min(availableCoreNum,M)` 上限，才可能有未利用并行度；当前未获得 case4/7 的 M 与运行时核数，不能确认这种情况。
- `WHY_IT_MAY_FAIL`：V011 的 block 数已取 `min(availableCoreNum,M)`：当 M 小于可用核数时每行已有一个 block，当 M 不小于可用核数时可用核均已启动。改变 rows-per-task 又直接进入 R016/SCHED 的既有机制；主动减核则可能增加每核串行工作与启动摊销。
- `ASCEND_FEASIBILITY`：Host 整数公式可实现，不需新增 Ascend C API。
- `UB/CORE/DMA_IMPACT`：UB 与每行 DMA 不变；只改核数及每核行段长度。
- `SYNC_IMPACT`：没有新增跨核同步。
- `PRECISION_RISK`：行内运算顺序不变时低。
- `DUPLICATE_CHECK`：当前泛化版本与 R016 的 D 档 rows-per-task / block 数公式、SCHED-ROWGROUP 的 rowGroup 任务取整、SCHED-CHAMPION V002 的活动核保留条件重叠。若意图改为低于 V011 上限的减核策略，需另行写明启动开销或并发争用这一具体瓶颈；当前没有 case 元数据或证据支持该变体。
- `MINIMAL_OFAT_DIFF`：仅改 Host blockCount 或 task 粒度；在差异审阅前不改 Candidate。
- `EXPECTED_LOCAL_PROBES`：本轮不建议探针。若未来提出与既有调度公式不同的明确减核假设，须先取得 case4/7 精确 shape/dtype 与运行时 available core，再由 Main 审阅后确定对照；本轮不测。
- `CLASSIFICATION`：`DUPLICATE`；不得作为首选假设。

### H3：D 维跨核分段归约

- `MECHANISM`：把单行 D 拆给多个 block 计算 partial square-sum，再合并 partial 并完成归一化与输出。
- `BOTTLENECK`：Very-Large D 且 M 很小时，单行归约占用单核、总并行度受 M 限制。
- `EXPECTED_SHAPES / DTYPES`：假设 D>8192、M远小于可用核数；FP32/FP16/BF16 理论均相关；Official case7 是否符合未知。
- `WHY_IT_MAY_HELP`：增加 D 维并行度，缩短单核完整扫描一行的时间。
- `WHY_IT_MAY_FAIL`：partial 写入、跨 block 合并、额外 launch 和同步可能吞掉收益；浮点加法次序改变会影响误差。
- `ASCEND_FEASIBILITY`：Ascend C 的两阶段 Group Reduce 需要 workspace 与全局同步/第二阶段。当前 `run_kernel` 接口没有 workspace 参数，Parent 也是单次 vector kernel launch；在当前调用面下判为不可行。R31B V008 与 FULL-R029/SPLIT_D 也有历史失败记录。
- `UB/CORE/DMA_IMPACT`：单核 UB 可降，但需额外 workspace；占用更多核；输入读取可能并行，partial 还增加 GM 往返。
- `SYNC_IMPACT`：新增全局阶段同步或第二次 launch。
- `PRECISION_RISK`：partial 合并顺序改变；FP32 也需重做误差验证，低精度风险更高。
- `DUPLICATE_CHECK`：与 R008、R31B V008 的跨核 D-slice 及 R029 SPLIT_D 方向重叠。
- `MINIMAL_OFAT_DIFF`：只改变 reduction ownership 与 partial merge；其他 tile、epilogue、参数搬运不变。当前接口需先有 workspace/launch 扩展授权。
- `EXPECTED_LOCAL_PROBES`：若调用面以后支持 workspace，最小矩阵为 M=1、M=2 的 Very-Large D 加一个 M 较大的 guard；先做 correctness 与并行阶段原型，不直接进入计时。
- `CLASSIFICATION`：`INFEASIBLE`（当前调用接口）。

### H4：Very-Large D 的单一 tile 宽度档位

- `MECHANISM`：只对一个已确认的 D/dtype 区间调整 wide full-y 的 tile 宽度；保持 wide 路径、行批量和计算次序不变。
- `BOTTLENECK`：单行 tile 次数与每轮 MTE/V/S 迭代开销。
- `EXPECTED_SHAPES / DTYPES`：Very-Large `D>8192`，优先 FP16/BF16；FP32 需避开已记录的 Parent wide-FP32 数值不稳定区。case4/7 是否命中 wide 路径未知。
- `WHY_IT_MAY_HELP`：较宽 tile 有机会减少每行 tile 轮数。
- `WHY_IT_MAY_FAIL`：UB 占用会压低 `wideFullYRows_` 或迫使 tile 缩小；V011 已按 dtype、D 和 UB 预算自适应选行数；tile 次数也未必是限速项。
- `ASCEND_FEASIBILITY`：Host/Kernel 可按一个已限定条件重算 buffer 大小，但需证明所有 TBuf 在 DAV_2201 UB 预算内；不得仅改常量而漏同步改变 buffer 规划。
- `UB/CORE/DMA_IMPACT`：UB 占用上升；core 分配不变；每行 tile 数与 DMA 粒度改变，GM 总字节数基本不变。
- `SYNC_IMPACT`：每行循环/事件次数可能减少，事件依赖图不变。
- `PRECISION_RISK`：累加顺序保持时较低；tile 边界变化可能改 ReduceSum 分段与合并顺序，需测误差。
- `DUPLICATE_CHECK`：与 SHAPE-TILING-CHAMPION-X 和 R31B V016 的 tile/UB 方向邻接；R31B V016 已记录 FP16/BF16 4096→8192，不能重复声明为新机制。
- `MINIMAL_OFAT_DIFF`：仅为经 Main 认可且不在 V016 覆盖范围内的一个 D/dtype 档改 tile；不改 active core、row batch、DMA 或 epilogue。
- `EXPECTED_LOCAL_PROBES`：取得真实 case 后，Parent 精确 shape 加最近 tile 边界 guard；先核 `D=8192/8193`，若输入确属 wide 再核 `12288/16384/32768` 的同 dtype、M=1 与多行条件。此处是源码/历史探针值，不是 case 输入声明。
- `CLASSIFICATION`：`NEEDS_MORE_EVIDENCE`；需先证明与当前 SHAPE-TILING/V016 任务边界不同。

### H5：窄中行 MTE2 与标量尾部的 issue 次序

- `MECHANISM`：只重排既有输入、gamma/bias 与 invRms scalar tail 的 issue/wait 次序，不改 row ownership、数学次序或 tile 宽度。
- `BOTTLENECK`：若 MTE2 等待落在 V/S scalar tail 内，存在可重叠空档。
- `EXPECTED_SHAPES / DTYPES`：Medium `128<D<=4096` 的 generic/narrow-mid 分支，或 Very-Large FP16/BF16 pass-2；具体 dtype/path 取决于真实输入。
- `WHY_IT_MAY_HELP`：把已发出的参数或下一行输入传输藏在已有 scalar/compute 区间内。
- `WHY_IT_MAY_FAIL`：V011 的 `ProcessNarrowMidOverlap` 已并发发 input 与 param MTE2；ASYNC-OVERLAP 已测窄中行、wide 参数预取与 GetValue 合并，记录为混合或无明确改善。可用重叠窗口可能太短。
- `ASCEND_FEASIBILITY`：硬件事件可重排，但每个 buffer 必须等待上次 MTE/V/MTE3 使用完成；提前复用会产生数据竞争。
- `UB/CORE/DMA_IMPACT`：UB 可能需要额外暂存；core 不变；DMA 字节不变，目标是隐藏延迟。
- `SYNC_IMPACT`：必须维持 event 成对 Set/Wait 与 buffer 生命周期；移除等待不作为可接受实现。
- `PRECISION_RISK`：保持算术序列时低；仅重排传输事件不得改变舍入点。
- `DUPLICATE_CHECK`：与 ASYNC-OVERLAP-CHAMPION-X V001-V004 和 MULTIROW-DMA-CHAMPION-X V001/V002 已记录方向重叠。
- `MINIMAL_OFAT_DIFF`：只移动一组既有 Load/SetFlag/WaitFlag；不得增加新 buffer、DMA burst 或计算合并。
- `EXPECTED_LOCAL_PROBES`：case4/7 的精确 shape/dtype 及同一 dispatch guard；只对 `ProcessNarrowMidOverlap` 或对应 wide 函数已确认的输入提出，遵循 Parent same-binary 与交错 P/C 流程。
- `CLASSIFICATION`：`DUPLICATE`。

## 形状覆盖与停止条件

以下是从 Direct Parent 分支边界得到的未来 guard 集合，不是 case4/case7 的 shape 推断：

| 覆盖带 | 源码条件 | guard 候选 |
|---|---|---|
| Tiny | `D<=128`；小行专用 batch 还要求本地多行与 dtype 对齐条件 | `D=128/129`；FP32 相邻 `%8` 宽度；低精度相邻 `%16` 宽度 |
| Medium | `128<D<=4096`，但精确 tile 与 dtype 专用条件优先返回 | `D=2048/2049`、`D=4096/4097`；M 在可用 core 数附近 |
| Very-Large | `D>8192` wide 分支 | `D=8192/8193`；确认 wide 后再选同 dtype 的 D=12288/16384/32768 guard |

最小输入证据为 Judge 提供的 testcase4、testcase7 输入 shape 与 dtype；随后按 Direct Parent 公式计算 M/blockCount，再展开 dispatch、ownership 和 tile。若只能再获得 case index 或耗时，研究仍停在 `NEEDS_MORE_EVIDENCE`。在输入元数据到手且 Main 完成假设审阅前，不启动 Candidate、编译、设备或测量工作。
