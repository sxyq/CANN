# TINY-FIXED-OVERHEAD-CHAMPION-X — ACTIVE_CORE_COUNT 与固定成本补充

DATE: 2026-10-03
TRACK: `TRACK-B`（只读）
REVISION: `NONE`
MAIN_SELECTED: `NONE`
OFFICIAL_CHAMPION: `R31B-V011`, score `45.16`, 15/15
DIRECT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

本文件补充既有五假设，重点复核 `ACTIVE_CORE_COUNT`、case 1/3/5 的共享路径证据、与 `SELECTIVE-FASTPATH-CHAMPION-X` 的重复边界和最小验证条件。不改 Candidate、不创建 Revision、不运行设备或构建；不作路线生命周期决定。

## 结论摘要

- V011 与 case 1、3、5 的公开结果可证实共享同一份提交源码、`run_kernel` Host wrapper、dtype 分派语句和 Kernel 模板入口。它们是否使用相同 dtype specialization、`Process` 分支、wide 子路径、实际 block 数和每 block 行数，现有证据不能确认。
- 三个 testcase ID 在 V011 与 SCHED-ROWGROUP-X V001 结果中一致，但两份 Official JSON 均不带 workload shape、dtype、launch metadata 或 dispatch trace。case 1/3/5 的耗时不能用来推断这些字段。
- `ACTIVE_CORE_COUNT` 改变 Host launch block 数及 device ownership，是 R016/FULL-R016、SCHED-ROWGROUP 与 CASE47-H2 已覆盖的调度轴。它保留为 `DUPLICATE`；当前不提出把该方向恢复成实现项。
- `SELECTIVE-FASTPATH` 当前公开调度项针对 BF16、D=32768 的完整 V017 donor。这个组合在拿到 Official workload manifest 后必须从 TINY 的 V011 内部探针集合排除。除此之外，case 与 donor 是否重合仍未知，不因工作项标题直接断言已覆盖或不重叠。
- 下文五个候选机制彼此独立，但它们都只能在 Official path map 证明至少一个目标 case 命中所需条件后进入 Main 审阅；本文件不授权 Revision 或运行。

## Official 路径映射审计

目标 testcase：

| Official case | Testcase ID | V011 timeUs | 结果中 shape/dtype/path |
|---|---|---:|---|
| 1 | `6a9a9a99bf41025d6013eb8a` | 5.44 | 未提供 |
| 3 | `6a9a9a99bf41025d6013eb92` | 5.16 | 未提供 |
| 5 | `6a9a9a99bf41025d6013eb9a` | 9.93 | 未提供 |

可确认的共同层次：

1. `run_kernel` 对三个 case 使用同一份 V011 Host 入口实现。Host 按输入 dtype 选择 `float`、`half` 或 `bfloat16_t` Kernel 实例，再以计算所得 `blockCount` 发射 grid。
2. 每个 Kernel block 都进入同一模板入口 `add_rms_norm_bias_custom<T>`，创建 `AddRmsNormBiasKernel<T>`，调用 `Init` 后调用 `Process`。
3. `Process` 首先读取 `widePath_`；非 wide 分支再按模板 dtype、rowWidth、对齐条件和 `localRows` 选择多条专用函数或通用路径。
4. 因而“共用入口”成立；“共用 specialization/dispatch/ownership”没有成立证据。即使模板类型相同，`Process` 仍可能因 width、alignment、wide 与每 block ownership 条件走不同路径。

需要取得的官方逐例 manifest 至少应给出 testcase ID、输入完整 shape、dtype、epsilon、布局及设备可用核数。要确认 dispatch，还需以这些输入重放 Host 计算并记录 `rowCount`、`rowWidth`、`availableCoreNum`、`blockCount`、每个代表性 `blockIdx` 的 `localRows`，以及实际命中的函数名。Official 计时起止点也须单独确认；没有该定义，不把 Host wrapper 的前置验证、Kernel entry 或 launch 开销说成 `timeUs` 的组成部分。

## 固定成本分类

下表区分源码中可见的工作与需要目标编译产物/计时边界才能确认的成本。源码出现某个 API 不等同于该 API 在 Official 计时内产生同等成本。

| 成本 | V011 可见行为 | 证据边界 |
|---|---|---|
| Host wrapper | 验证指针、rank、dtype/shape 一致性和乘法溢出；展平 leading dimensions；计算 `blockCount`；选择 dtype Kernel 并发射。 | 是否纳入 Official `timeUs` 未知。case 输入未知，不能估计各 Host 分支成本。 |
| Kernel entry / template dispatch | 每个 block 建立 op，调用 `Init`、`Process`；`Process` 先 wide/non-wide，再按 dtype、宽度、对齐及 ownership 派发。 | 源码控制流可读；目标 specialization 及指令成本未从 Official metadata 映射。 |
| `InitBuffer` / Pipe 初始化 | `Init` 设置 GM tensor、wide 标志；依据 `rowWidth`、dtype 调用多个 `pipe_.InitBuffer`，wide 时选择 full-y rows/tile。 | 需要确定工具链把这些操作落实为哪些设备指令/资源描述；不得假设所有调用按源码顺序形成 kernel-body 固定时长。 |
| Event ID 管理 | 窄中路径申请/释放 3 个 event ID；其他流水路径还有各自 event ring。 | 条件分支、ID allocator 的设备端实现和目标路径命中未知；静态 API 调用数不直接等于 event 硬件事件数。 |
| `SetFlag` / `WaitFlag` 与 PipeBarrier | V011 在 DMA 与向量消费处同步；通用路径有 `SyncMTE2ToV`、`SyncVToMTE2`、`SyncVToS`、`SyncSToV` 及 `PipeBarrier<PIPE_V>`。 | 依赖边有数据正确性职责；仅在确定无消费者/无依赖且生成代码确实保留时，才可讨论单项成本。不能按源码行数删除。 |
| 每 block ownership | non-wide `Process` 计算商、余数、`beginRow`、`localRows`；wide helper 中也可见类似公式。 | 公式是否被目标编译器简化、各 Official case 是否走 non-wide/同一 helper 均未知。 |
| single-tile loop | 通用 pass-1/pass-2 均以 `col += kTileElems` 遍历，且计算 `tileCount`；窄中专用函数则是一行一组固定宽度操作。 | 只有命中通用分支且 D 符合 tile 条件，循环才可视为单次；编译器是否展开需看目标产物。 |
| DMA / 数值工作 | 各路径执行其对应输入、参数与输出搬运、归约及 affine 运算。 | 这是固定控制成本研究的对照项；各 case 的真实元素量与数据类型仍未知，不能跨 case 估算。 |

这些项目可能处在不同计时层级：Host 前置验证在 launch 前；Kernel entry、设备 dispatch、初始化和设备同步若被计时，需由 Official 计时定义及目标工具链证据确认。以下假设均只试图改变一个设备侧机制，不声称能节省 Host 时间或 kernel launch latency。

## ACTIVE_CORE_COUNT 与去重

V011 Host 取正的 `availableCoreNum`（否则用 1），将其限制到 `rowCount` 和 `UINT32_MAX`，把结果同时用于 launch `blockCount` 与 Kernel ownership 参数。降低 active count 会改变 grid、每个 block 的行范围及 `localRows`，继而可能改变参数驻留和被选专用路径条件；它不是仅改一个无关 dispatch 标签。

已有材料表明：

- R016/FULL-R016 已将按 D 档的 rows-per-task 和 block 数作为调度机制。
- SCHED-ROWGROUP-X 明确研究可用 core、task 粒度、活跃 block 与 ownership；其公开 V001 结果在目标 testcase ID 上使用另一份源码，但该结果不提供 shape/dtype，也不能用来断言 V011 路径。
- SCHED-CHAMPION-X V002 加入 active-core 保留条件，进一步说明同一调度轴已有直接探索。
- CASE47 H2 记录将 active-core / rows-per-task 列为该路线调度假设。
- SELECTIVE-FASTPATH 当前任务目标为完整 V017 donor 条件选择；该机制改变 donor 函数体与分派选择，不以减少 `blockCount` 为目标。它与 ACTIVE_CORE_COUNT 机制不同，但 TINY 输入命中关系仍需要 manifest。

判定：`TINY-H1-ACTIVE-CORE-COUNT = DUPLICATE`。保留本节作去重依据，不产生新的 core-count 变体，也不改变 Main 或 Planning 对路线的决定权。

## 五个独立、可证伪的假设

共同资格前提：Official manifest 和 V011 Host/Kernel replay 必须证明至少一个 case 1/3/5 命中每条假设所需路径；case 不能按耗时分桶挑选。满足路径后，先做静态源码/编译产物探针；只有目标机制仍存在、且构建与 correctness 获得授权并通过，才进入项目既有 same-binary 及交错 Parent/Candidate 流程。时序验证须有对应路径、dtype、精确 shape 的 Parent 基线资格。所有本地样本只作本地证据，不改写 Official 结果。

### H1 — 等行数 ownership 算术直达

- **机制及与其余假设的边界：** 保持 block 数、函数 dispatch、缓冲、DMA、同步不变；仅在 `rowCount == blockCount` 时，把每 block 的商余数公式改为 `beginRow=blockIdx`、`localRows=1`。不调整 core 数。
- **可证伪预期：** 目标编译产物若已经把商/余数化成等价直接映射，或替代式没有减少设备端整数指令，则“ownership 标量算术可省”不成立。
- **最小本地验证（仅 Official path map 满足后）：** 先用相同工具链生成 Parent 的该 dtype Kernel 目标产物，确认除法/取余/边界选择指令；以静态映射覆盖全部 blockIdx 并比较 Parent 与快速式行归属。授权后仅构建此单项变更，做目标 shape correctness；加一个 `rowCount != blockCount` 控制验证 fallback。若产物指令未减少，停止，不做计时。
- **重复边界：** 不改变 `blockCount` 或 rows-per-task，机制不同于 R016/SCHED；但只要 Full-path 或专用 helper 另有 ownership 公式，需先证明该 case 命中此处 `Process` 算式。与 SELECTIVE donor 逻辑不同；若 manifest 显示目标使用 BF16、D=32768 完整 V017 donor，则该样本排除。
- **状态：** `NEEDS_MORE_EVIDENCE`；未获选择。

### H2 — 非 wide entry 的 dtype dispatch 收敛

- **机制及与其余假设的边界：** 对已确认的一个 dtype 与非 wide 路径，仅评估模板/分支编译后是否留下可省的重复设备侧选择；Host dtype dispatch、宽度专用函数、ownership、InitBuffer 和数据计算均不改。若源码模板实例化已消除这些条件，则假设立即被否证，不发明新分派方案。
- **可证伪预期：** 目标 specialization 的汇编若不含动态 dtype 分支，或路径选择在 Kernel 内由编译期常量折叠，则没有可测的 device-dispatch 节省。
- **最小本地验证（仅 Official path map 满足后）：** 对唯一命中的精确 dtype/shape 编译 Parent，审阅对应 specialization 的控制流与目标分支；把 Host wrapper 的 launch 之前工作和 kernel-body 指令分别标记。只有确认 kernel-body 动态选择仍存在，且 Main 明确选择一个最小源代码变更后，才做 correctness 与对应 shape 的本地配对测量；不得借机为三个 case 增加完整 donor 分派。
- **重复边界：** SELECTIVE-FASTPATH 选择完整 V017 donor 并增加输入条件，作用范围包括 donor 函数体；H2 只询问 V011 已选 dtype specialization 内部是否还保留无效设备分支。路径/shape 不清楚时不能声称两者正交；BF16 D=32768 donor 样本排除。
- **状态：** `NEEDS_MORE_EVIDENCE`；首先依赖目标 specialization 产物。

### H3 — 已命中函数内移除一段冗余 PipeBarrier

- **机制及与其余假设的边界：** 只针对 Official path map 中命中的一个函数，逐条分析相邻 vector 写后、读前的依赖；若其中恰有一个 barrier 不承载跨指令读写依赖，才提出删去这一处。不得删 event、改变 SetFlag/WaitFlag、改变 issue 次序或合并其他 barrier。
- **可证伪预期：** 若每处候选 barrier 都是生产者到消费者的数据可见性/执行依赖，假设即不成立；若编译器已经消除候选 barrier，亦不成立。
- **最小本地验证（仅 Official path map 满足后）：** 将命中函数展开成向量生产者/消费者清单，逐项核对 buffer alias、PIPE_V 操作顺序和 DMA 同步；对目标产物核实 barrier 指令是否存在。只有找到有证据支持的单个冗余点并经 API/正确性审阅后，才允许单点候选；先做目标 correctness，之后才可做该形状 same-binary 与交错测量。无法证明冗余即停止。
- **重复边界：** 不移动 SELECTIVE-FASTPATH 的 wide MTE3_V 等待点，也不改 donor；但任何落入其 BF16 D=32768 path 的探针排除。与 CASE47 issue 次序假设按具体函数和同步边逐一对照，不能仅凭都含同步操作判为不同。
- **状态：** `NEEDS_MORE_EVIDENCE`；风险高于纯标量控制项。

### H4 — 已证明 `tileCount == 1` 的通用路径移除循环控制

- **机制及与其余假设的边界：** 仅当精确 Official 输入命中通用两遍路径，且数学条件保证 tileCount 为 1 时，把 pass-1 与 pass-2 各自的 loop control 改成单次 body；保留其中所有 Load、向量操作、ReduceSum、barrier、V/S handoff、Store 及计算顺序。不改变 InitBuffer、ownership 或 dispatch。
- **可证伪预期：** 若路径不是 generic、tileCount 不为 1，或目标编译器已完全展开两段循环，假设不成立。若静态指令差异仅转成等量分支/代码开销，也不继续。
- **最小本地验证（仅 Official path map 满足后）：** 用 manifest 重放 `Process` 前序分支，确认精确函数及 tileCount；比对 Parent 目标产物的 loop back-edge/计数指令。仅在仍有循环控制时做单函数体变更与 correctness；再按该 shape/dtype 建 Parent same-binary 基线并进行交错配对。保存一条相邻 tileCount 大于 1 的非目标 control，仅核实其 fallback 未变。
- **重复边界：** SELECTIVE 的 V017 wide donor 不属于本假设目标；若 target manifest 指向 donor，则排除。不得将 donor 输出流水/等待移动与本项组合。
- **状态：** `NEEDS_MORE_EVIDENCE`。

### H5 — 窄中单行 event ID 管理

- **机制及与其余假设的边界：** 只在已确认命中 `ProcessNarrowMidOverlap` 且 `localRows==1` 时，判断三个已分配 event ID 中是否存在生命周期完整、真实且可证明无用的单一 ID。每次最多研究一个 ID；不改变 Load、SetFlag/WaitFlag、scalar tail、PipeBarrier、输出或其他 ID 的分配释放。现有 H5 草案聚焦多行未使用的 `paramReady`，本补充选用单行这一不同条件，避免把两个条件混作一项。
- **可证伪预期：** 若 API 要求固定配对生命周期、allocator 编译为无可省设备工作、目标编译产物无对应成本，或该 ID 仍有生产者/消费者，则假设不成立。
- **最小本地验证（仅 Official path map 满足后）：** 先查本地 Ascend C API 约束及 allocator 编译实现；以目标编译产物对应 ID 的管理指令为证据。只有 API 允许条件管理、且能指出一个实际可省操作时，才在 Main 选定后做最小变更与 correctness。再对精确 shape 做 same-binary 与交错测量；条件不合法或产物无差异时停止，不在设备上试错 event 生命周期。
- **重复边界：** SELECTIVE 移动 wide donor 的 MTE3_V 等待位置；本项关注窄中 V011 的 event ID 配置，目标条件和 event 方向不同，但 case 输入不明时不称为已证实不重叠。完整 V017 donor 输入排除。
- **状态：** `NEEDS_MORE_EVIDENCE`；须先解决 API 生命周期问题。

五项的状态只反映证据成熟度；未选任何一项。`ACTIVE_CORE_COUNT` 原 H1 维持 `DUPLICATE`，不计入上述五项。

## SELECTIVE-FASTPATH 去重规则

当前共享调度记录给 SELECTIVE-FASTPATH 的任务是“BF16 D32768 full V017 donor qualification”，要求先核实 M 和 dispatch reachability，当前尚未开始 qualification。它不是 V011 内部单条屏障或标量算术的同义说法。去重按机制和输入分别处理：

1. 对外部探针：若 Official manifest 的 dtype 为 BF16 且 D=32768，并且执行完整 V017 donor，则从 TINY V011 内部假设的本地目标样本中排除，避免将替换 donor 的结果记到 V011 小开销轴。
2. 对剩余 case：仍需从 testcase ID 的 manifest 确认 shape/dtype，再重放 V011 分派；SELECTIVE 任务只验证其指定完整 donor 条件，不能由此推断 case 1/3/5 命中或不命中该条件。
3. 机制审阅时，按改动对象分开：donor 选择/函数体、Host active block 数、Kernel ownership 公式、已选 specialization 的设备分支、PipeBarrier、loop control、event ID 生命周期。不得把“都有条件语句”作为重复或不重复的证据。

## 允许继续的最小证据顺序

1. 只读取得 Official workload manifest 和 Official 计时边界说明；以 testcase ID 关联，不从延迟推测 shape/dtype。
2. 用 manifest 输入重放未改的 V011 Host/Kernel 路径，产出 case 1/3/5 的 specialization、dispatch、blockCount/localRows 与函数名映射；同表列出是否满足 SELECTIVE donor 条件。
3. 每个目标机制单独核对原始 V011 目标编译产物。若指令/资源层没有该开销，否证对应假设并停止该项。
4. Main / Planning 选择前，不创建 Revision、不改 Candidate、不构建、不跑 correctness 或设备性能。
5. 只有 Official path map 满足条件且后续权限明确时，才对单一假设做项目规定的 exact-source build、correctness、本地 same-binary、Parent/Candidate 交错测量；保留原始结果。不能用未命中 case 或不同 dtype 的测量代替。

## 本轮未确认事项

- 三个 testcase 的完整 shape、dtype、epsilon、布局与 `availableCoreNum`。
- 每个 case 的 dtype specialization、实际 dispatch、`blockCount`、`localRows` 及路径函数。
- Official `timeUs` 是否包括 Host wrapper、launch、Kernel entry，或只包括设备执行。
- V011 对应目标二进制/汇编中 ownership 除法、分支、循环、barrier 和 event 管理的保留情况。
- SELECTIVE donor 是否会覆盖 case 1/3/5 中任一 testcase。

MAIN_SELECTED 保持 `NONE`；路线生命周期不作判断。
