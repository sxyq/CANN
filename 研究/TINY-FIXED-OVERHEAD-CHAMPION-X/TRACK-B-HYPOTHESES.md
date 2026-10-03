# TINY-FIXED-OVERHEAD-CHAMPION-X — Track-B 假设研究

STATUS: `TRACK_B_RESEARCH`; `REVISION=NONE`
WORKTREE: `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/tiny-fixed-overhead`
BRANCH: `w2/m1/tiny-fixed-overhead`
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

本记录只提出待审阅的 Track-B 假设，不选实现项。复审后，H1 `ACTIVE_CORE_COUNT` 与 R016/SCHED 的调度轴重复，保留为重复性记录，不再作为首选；H2 的机制边界清楚，可供 Main 审阅，但输入适用条件仍待证实。当前没有 `MAIN_SELECTED=YES`，不创建 Revision。

## 范围与证据边界

- 初始目标为 Official case 1、3、5；只有可靠 Official workload metadata 证明其他 case 命中同一条 Tiny 源码路径时，才加入范围。
- `线上结果/R31B/V011/result.json` 中 case 1、3、5 的 `timeUs` 分别为 5.44、5.16、9.93 µs，均低于 10 µs；对应 testcase ID 为 `6a9a9a99bf41025d6013eb8a`、`6a9a9a99bf41025d6013eb92`、`6a9a9a99bf41025d6013eb9a`。该文件的 case 记录包含 ID、时间、状态、分数，没有输入 shape、dtype 或 Kernel dispatch 字段。
- `线上结果/R31B/V011/source-meta.json` 确认 Official source SHA256；它没有 case 到 shape/dtype 的对应关系。父源码 sidecar 与 `submission.asc` 的 SHA256 一致。
- `研究/OFFICIAL-CASE-ANALYSIS.md` 按耗时量级推断 case 类别，并明确把部分形状描述写成推测。这里仅引用其中的耗时数据，不将时间档当作 shape、dtype 或 dispatch 证据。
- R31B-V011 的 `Tiny D` 注释只描述一个宽度条件；`Process` 内还有 dtype、`localRows`、对齐和宽度特化。当前没有可靠资料证明 case 1、3、5 共用同一个函数路径。
- 现有共享记录尚无 TINY 路线行。本文件不创建或改动共享记录。

在取得以 testcase ID 为键的 Official workload manifest 前，所有假设均把 shape / dtype 写作未知；不根据耗时猜测 M、D 或 dtype。

## 本轮成熟度

| 假设 | 成熟度 | 关键缺证据 / 约束 |
|---|---|---|
| H1 ACTIVE_CORE_COUNT | `DUPLICATE` | 与 R016/SCHED 的 rows-per-task、active-core 调度同轴；case 输入映射仍未知。保留记录，不列入待选实现。 |
| H2 ROW-OWNERSHIP-FASTFORM | `READY_FOR_MAIN_REVIEW`（机制审阅） | 仅在 `rowCount == blockCount` 生效；需确认目标输入条件及编译后除法/取余是否仍存在。未满足输入清单前不得据此选 Revision。 |
| H3 GENERIC-SINGLE-TILE | `NEEDS_MORE_EVIDENCE` | 需确认目标输入命中通用路径、`tileCount=1`，并证明生成代码仍有循环控制。 |
| H4 VALUE-BUFFER-FOOTPRINT | `NEEDS_MORE_EVIDENCE` | 需核实 UB 对齐/索引上界，并证明缩小配置会改变生成布局或资源占用。 |
| H5 CONDITIONAL-PARAM-EVENT | `NEEDS_MORE_EVIDENCE` | 需确认目标输入命中窄中函数且 `localRows>1`，补齐 event API 生命周期约束和生成代码证据。 |

`READY_FOR_MAIN_REVIEW` 只表示 H2 的机制定义适合审阅，不代表输入命中已知、方向已选择或获准实现。所有假设共同受 case 1/3/5 输入清单缺失这一限制。

## 源码路径与开销项

来源均为固定 Direct Parent：`线上结果/R31B/V011/submission.asc`。

| 开销项 | 当前可见事实 | 证据位置 | 尚缺证据 |
|---|---|---|---|
| Host entry | `run_kernel` 读取 host metadata，执行指针、rank、dtype、维度和乘法溢出验证，再发起 Kernel。 | `run_kernel`，L3479 起 | Official `timeUs` 的计时边界是否包括 wrapper 与 launch 尚未知；暂不把 Host 分支计入收益估算。 |
| Device entry / dispatch | `add_rms_norm_bias_custom` 对每个 block 建立 `op`，依次调用 `Init`、`Process`。`Process` 先分 wide path，再根据 dtype、宽度、对齐和 `localRows` 选择多个专用函数。 | L159、L3469 | 三个目标 case 各自命中的函数未知。 |
| Block/core ownership | Host 取 `availableCoreNum`，限制到 `rowCount` 与 `UINT32_MAX` 后作为 `blockCount` 和 launch 数。非 wide path 以商、余数、`blockIdx` 算出 `beginRow/localRows`。 | L171-L176、L3534-L3548 | 目标 case 的 M、可用核数和实际 `blockCount` 未知。 |
| InitBuffer | `D > kCacheElems(8192)` 走 wide 配置并返回；其余路径至少配置 `kTileElems(4096)` 的输入/工作 buffer，若干参数或 value buffer 配到 `kCacheElems`。BF16 另有 FP32 参数 buffer。 | L60-L155、L1241-L1288 | `InitBuffer` 对 Official kernel 时间、UB 占用和可驻留 block 数的贡献没有当前逐例资料或 ISA 证据。 |
| Event | `ProcessNarrowMidOverlap` 分配 `inputReady`、`paramReady`、`inputRelease` 三个 event ID。`paramReady` 只在 `localRows==1` 的分支读写；多行时仍执行分配与释放调用。 | L499-L509、L513-L518、L537-L543、L576-L578、L617-L619 | 该路径是否覆盖目标 case未知；`AllocEventID` 的可见调用是否产生可观设备指令也未知。 |
| Barrier / V-S handoff | 通用两遍路径和窄中路径存在多个 `PipeBarrier<PIPE_V>`；通用路径还通过 `SyncVToS` / `SyncSToV` 包围 scalar `GetValue`。这些边各自承载数据依赖。 | L299-L366、L407-L499、L3407-L3444 | 未取得目标 case 的实际路径及编译后指令；不先验删除同步。 |
| Single-tile loop | 通用路径的 pass-1 与 pass-2 都按 `col += kTileElems` 循环；`tileCount` 由运行时 `rowWidth` 计算。只有通用路径且 `D <= 4096` 时，两段循环才必定各执行一次。 | L289-L300、L366-L373、L1242 | 目标 case 是否到达通用路径、是否满足 D 条件均未知；编译器是否已展开也未知。 |

V011 中可见的主要非 wide 路径包括：FP32 小行批处理；FP16/BF16 对齐小行批处理；D=4096/8192 的多行专用处理；`128 < D <= 4096` 的窄中 overlap；以及回落到通用两遍 tile 路径。这里的 `Tiny` 是耗时分组，不是源码中唯一的函数名。

单独按输入条件切换 Kernel 路径暂不列为假设：case 到 dispatch 的对应关系缺失，且 SELECTIVE-FASTPATH 正在研究按输入条件切换完整 V017 donor。待 shape/path 对照资料齐全后，再判断 V011 内部 dispatch 分支是否存在独立可测空间。

## 假设

### H1 — ACTIVE_CORE_COUNT 参数轴

HYPOTHESIS_ID: `TINY-H1-ACTIVE-CORE-COUNT`
STATUS: `DUPLICATE`
TARGET_CASES: case 1、3、5；加例规则见范围段。
TARGET_SHAPES_DTYPES: M、D、dtype 未知。参数有意义的前提是 `rowCount >= 2` 且当前实际 `blockCount > 1`；M=1 时现有 wrapper 已将 blockCount 限为 1。
MECHANISM: 只改变 `ACTIVE_CORE_COUNT` 上限，保持 Direct Parent 的算术、buffer、路径条件与 DMA 不变。每个候选 Revision 只固定一个 count；单独观察 block 数减少后，单核行数、启动分布与每核固定准备成本如何变化。
BOTTLENECK: 短 Kernel 中多 block 的启动/每核准备成本可能占比偏高。
WHY_IT_MAY_HELP: 小 M 情况下，降低 block 数可能减少参与工作的 core 数；`localRows` 增大时也可能复用每核参数读取。
WHY_IT_MAY_FAIL: 每核串行行数增加会压低并行度；`localRows` 改变还会触发已有参数驻留/专用分支，收益来源可能随之变化。M=1 时该参数轴没有空间。
ASCEND_FEASIBILITY: `blockCount` 已同时控制 launch 尺寸和每核 ownership 公式，可在 host 侧加单一 count 上限；每次仅测一个 count。
UB_CORE_DMA_IMPACT: UB 配置不变；active block 数变化；逻辑 DMA 总量不变，每核 DMA 与参数复用会随 `localRows` 改变。
SYNC_IMPACT: 同步序列保持原样；各 core 的行数可能变化。
PRECISION_RISK: 算术次序按既有函数路径保持；仍需逐目标形状做 Correctness。
SOURCE: `run_kernel` L3534-L3548；`Process` L171-L176、L242-L249。
DUPLICATE_CHECK: 与 R016/FULL-R016 的 D 档 rows-per-task 和 block 数选择、SCHED-ROWGROUP-X 的任务/行组所有权，以及 SCHED-CHAMPION-X V002 的 active-core 保留条件属于同一调度轴。TINY H1 将 `blockCount` 再加固定上限，虽把探索方向放在减少并行 block 的一侧，仍未形成与已有调度机制独立的新轴；CASE47 的 H2 已将“按有效任务数调 active core / rows-per-task”判为 `DUPLICATE`。不作为首选，不新开该方向。SELECTIVE-FASTPATH 的 V017 等待点与 STORE V003 分块写回不是 core-count 改动；shape/path 映射仍未知，不据此推断 case 覆盖。
MINIMAL_OFAT_DIFF: 仅作历史草案：若 Main 明确提出与 R016/SCHED 不同的研究问题，才考虑在 V011 wrapper 增加一个固定 `ACTIVE_CORE_COUNT` 上限；当前重复项状态下不实现。
EXPECTED_LOCAL_PROBES: 当前不安排探针。只有 Main 先确认存在独立于 R016/SCHED 的机制问题并选定研究后，才记录映射输入的 Parent `blockCount/localRows`，再决定是否需要构建与统一 Parent/Candidate 测量。

### H2 — 等行数分配的 ownership 快速式

HYPOTHESIS_ID: `TINY-H2-ROW-OWNERSHIP-FASTFORM`
STATUS: `READY_FOR_MAIN_REVIEW`
TARGET_CASES: case 1、3、5；加例规则见范围段。
TARGET_SHAPES_DTYPES: 要求 `rowCount == blockCount`；M、D、dtype 未知。
MECHANISM: 保持 blockCount 不变，仅在 `rowCount == blockCount` 时将商余数 ownership 公式替换为 `beginRow=blockIdx`、`localRows=1`。其他输入仍走 V011 公式。
BOTTLENECK: 每个非 wide block 都执行运行时 64 位除法、取余和边界选择。
WHY_IT_MAY_HELP: 等行数分配时结果固定，直接映射可省去每 block 的 ownership 算术与两次边界选择。
WHY_IT_MAY_FAIL: 编译器可能已简化相关指令；标量算术时间可能远低于 DMA、归约、同步和 buffer 准备成本；新增分支也有成本。
ASCEND_FEASIBILITY: 仅新增等式条件与 ownership 赋值，不动 row 的计算或输出地址公式。
UB_CORE_DMA_IMPACT: 不变。
SYNC_IMPACT: 不变。
PRECISION_RISK: 低；行号必须与 Parent 完全一致，越界和逐行输出仍需 Correctness 覆盖。
SOURCE: `Process` L171-L176；Host blockCount 规则 L3534-L3542。
DUPLICATE_CHECK: SELECTIVE-FASTPATH 的公开 handoff 描述的是按 BF16 D=32768 选择完整 V017 donor；本项保留 V011 的 dtype/width dispatch，仅简化等行数 ownership 计算。机制不同，Official case 是否命中条件仍未知。
MINIMAL_OFAT_DIFF: 只增加 `rowCount == blockCount` ownership 快速式；不改 ACTIVE_CORE_COUNT、dispatch 条件或函数主体。
EXPECTED_LOCAL_PROBES: 至少需要一个经 workload metadata 确认 `rowCount == blockCount` 的目标输入。先比较 blockIdx 到 beginRow 的静态映射，再做 Correctness 和统一 Parent/Candidate 测量；加入一个 `rowCount != blockCount` 控制确认 fallback。
REVIEW_SCOPE: 仅审阅“设备侧等行数时简化 ownership 算术”是否值得保留；不得把此状态理解为 case 命中已确认或实现获批。若编译器已消除除法/取余，假设应降为 `DUPLICATE` 或停止。

### H3 — 通用单 tile 路径移除循环控制

HYPOTHESIS_ID: `TINY-H3-GENERIC-SINGLE-TILE`
STATUS: `NEEDS_MORE_EVIDENCE`
TARGET_CASES: case 1、3、5；仅纳入实际到达 V011 通用两遍路径的 case。
TARGET_SHAPES_DTYPES: 要求 `D <= 4096` 且没有提前命中 dtype/alignment 专用函数；具体 M、D、dtype 未知。
MECHANISM: 保留既有运算、加载、store、同步与 buffer，仅将通用 pass-1/pass-2 的运行时列循环改为一个 tile body。所有其他输入保持原路径。
BOTTLENECK: 单 tile 行仍计算 `tileCount` 并执行两段 loop test/increment。
WHY_IT_MAY_HELP: 精确单 tile 分支可减少列循环的标量控制指令，避免动态 tileCount 计算。
WHY_IT_MAY_FAIL: 编译器可能已展开循环；分支和重复代码可能抵消节省；小 case 的主成本也可能在 entry、MTE2、V/S handoff 或 store 同步。
ASCEND_FEASIBILITY: 只有路径与 D 确认后才可增加单 tile body；不改变 Vector API 调用与元素次序。
UB_CORE_DMA_IMPACT: UB、core 数与 DMA 量不变。
SYNC_IMPACT: 同步调用原样保留。
PRECISION_RISK: 低；逐元素指令、计数与顺序须和通用父路径一致。
SOURCE: 通用 pass-1 L289-L342、pass-2 L366-L499；`kTileElems=4096` 位于 L1242。
DUPLICATE_CHECK: SELECTIVE-FASTPATH 的 V017 donor 条件为 BF16 D=32768，属于 wide path，不属于本项要求的通用单 tile 条件。case 映射缺失时仍不宣称 workload 不交叠。
MINIMAL_OFAT_DIFF: 仅对一个已确认命中通用且单 tile 的 target 增加直接 body；不改 branch selection、ACTIVE_CORE_COUNT、buffer 大小或同步。
EXPECTED_LOCAL_PROBES: 先用精确 testcase 输入确认函数路径和 `tileCount=1`；以编译后指令确认 loop 控制仍存在。随后做目标 Correctness、same-binary 和 Parent/Candidate 交错测量，并保留其他 dispatch 路径作为控制。

### H4 — 缩小通用路径的单个 value buffer

HYPOTHESIS_ID: `TINY-H4-VALUE-BUFFER-FOOTPRINT`
STATUS: `NEEDS_MORE_EVIDENCE`
TARGET_CASES: case 1、3、5；仅纳入通用路径且使用 `valueFp32Buf_` 行缓存的 case。
TARGET_SHAPES_DTYPES: 要求 `rowWidth <= 8192`、`cacheRow=true`；精确 D/dtype 未知。实际分派还受 dtype、M 和对齐影响。
MECHANISM: 只把 `valueFp32Buf_` 容量从固定 8192 个 float 改成精确 D 所需且符合 UB 对齐的容量；其余 TBuf、tile 大小、路径与运算保持 V011 原样。
BOTTLENECK: 小 D 行仍为 `valueFp32Buf_` 配置完整 cache 容量。
WHY_IT_MAY_HELP: 若 buffer 配置或较大 UB 占用影响每核准备成本/资源驻留，缩小这一处可减轻该成本。
WHY_IT_MAY_FAIL: `InitBuffer` 可能只形成静态 UB 布局，大小变化不一定生成可测设备指令；其他固定 buffer 仍在；微小容量也受最小对齐约束。
ASCEND_FEASIBILITY: 先确认 TBuf 对齐要求与所有访问边界；每个 Revision 只变这一处容量公式。
UB_CORE_DMA_IMPACT: 仅 `valueFp32Buf_` 的 UB 容量随 D 缩小；core 数和 GM DMA 不变。
SYNC_IMPACT: 不变。
PRECISION_RISK: 无算术变化；需验证各目标路径所有索引仍在容量内。
SOURCE: 非 wide `InitBuffer` L116-L149；`valueTile` 取值与写入 L289-L342、L366-L410；`kCacheElems=8192` 位于 L1288。
DUPLICATE_CHECK: SELECTIVE-FASTPATH 选择完整 V017 wide donor；本项限于 V011 非 wide 通用行缓存的一个 UB buffer，不改 V017 的 tile、store wait 或 donor 分派。Official case 到路径的映射仍待取得。
MINIMAL_OFAT_DIFF: 仅改变 `valueFp32Buf_` 的容量公式；不同时缩小 gamma/bias、x/residual、reduce buffer，也不改 tile loop。
EXPECTED_LOCAL_PROBES: 先核实目标路径对 buffer 最大索引、对齐和生成 UB 布局的要求；编译并做目标 Correctness。若生成代码/资源报告没有任何变化，停止此假设；有可见变化后再做 same-binary 与交错 P/C。

### H5 — 多行窄中路径省去未使用的 paramReady event

HYPOTHESIS_ID: `TINY-H5-CONDITIONAL-PARAM-EVENT`
STATUS: `NEEDS_MORE_EVIDENCE`
TARGET_CASES: case 1、3、5；仅纳入 `ProcessNarrowMidOverlap` 且 `localRows > 1` 的 case。
TARGET_SHAPES_DTYPES: 要求 `128 < D <= 4096`、没有被前置特化分支提前返回、并且每 block 至少两行；M、D、dtype 未知。
MECHANISM: 当 `residentParams=true` 时，`paramReady` 当前不执行 SetFlag/WaitFlag，但仍有 AllocEventID/ReleaseEventID 调用。只研究让该 event 的分配/释放也受 `!residentParams` 控制；`inputReady` 和 `inputRelease` 生命周期保持原样。
BOTTLENECK: 多行路径可能为没有消费者的 paramReady event 支付每 block event 管理成本。
WHY_IT_MAY_HELP: 条件路径可少一次 event ID 申请与归还。
WHY_IT_MAY_FAIL: AllocEventID/ReleaseEventID 可能被工具链静态化；条件申请可能不符合 TPipe event 管理限制，或影响后续 event ID 分配。必须先查明 API 约束，不能只凭源码未使用就改动。
ASCEND_FEASIBILITY: 需要确认 event allocator 是否允许分支内申请/释放、ID 配对规则和编译器生成物；当前 API Skill 未提供该细节，假设暂不具备实现条件。
UB_CORE_DMA_IMPACT: UB、core 数、DMA 量不变。
SYNC_IMPACT: 目标路径的 MTE2_V event 操作序列不变；只尝试移除未用 ID 的申请/释放。
PRECISION_RISK: 算术不变；event 生命周期错误会造成运行异常或数据竞争，风险集中在同步正确性。
SOURCE: `ProcessNarrowMidOverlap` L499-L509、L513-L518、L537-L543、L576-L578、L617-L619；命中条件位于 `Process` L234-L240。
DUPLICATE_CHECK: SELECTIVE-FASTPATH 公布的 V017 机制移动 wide BF16 pass-2 的 MTE3_V 等待点；本项涉及非 wide 窄中路径里未使用的 MTE2_V `paramReady` ID，位置、方向和前置条件不同。case 到路径的对应关系还未证明。
MINIMAL_OFAT_DIFF: 先从 Ascend C API 资料确认条件式 event 生命周期合法；获准实现后仅条件化 paramReady 的申请/释放，不改任何 SetFlag/WaitFlag 或其他 event。
EXPECTED_LOCAL_PROBES: 先取得命中该函数且 `localRows>1` 的精确输入；核对编译器产物中的 Alloc/Release 指令是否存在。只有确认存在且 API 生命周期合法，才编译、跑目标 Correctness、做 same-binary 和 Parent/Candidate 交错测量。加入 `localRows==1` 控制确认原 event 路径不变。

## 跨路线机制与重复性核对

依据限于公开研究记录、正式差异和实验摘要；未读取 SELECTIVE-FASTPATH 或 CASE47 的私有 Candidate 源码。

| 对照路线 / 机制 | 对应 TINY 假设 | 当前判断 |
|---|---|---|
| R016/FULL-R016：按 D 段设置 rows-per-task，block 数取可用 core 与 task 数的较小者。 | H1 active-core 上限；H2 device ownership fastform | H1 与 host 调度的核数/每核行数轴重复。H2 不改 block 数或 task 粒度，只在 `rowCount == blockCount` 时简化设备侧行号算术，机制不同；仍需确认编译器是否已做同样化简。 |
| SCHED-ROWGROUP-X：继承 R016 调度段并加入 32B row-group ownership；SCHED-CHAMPION-X V002 对组切分增加 `totalGroups * 2 >= min(blockCount,rowCount)` 条件。 | H1；H2；H4 | H1 与 R016/SCHED 的调度和 active-core 轴重复。H2 只在等行数时简化公式，不改组边界。H4 是缩小单个 UB buffer，与 row-group ownership 不同；UB 布局收益尚无证据。CASE47 报告的 INTEGRATION-X 退化不能用来推断 case 输入或 H4 表现。 |
| CASE47 H1：未对齐短行的 padded-UB 多行 epilogue；H2：active-core / rows-per-task；H3：D 维拆分归约；H4：wide tile 档；H5：MTE2 与标量尾部 issue 次序。 | H1-H5 | H1 与 CASE47 H2 同为重复调度轴。H4 与 CASE47 H1 同属 UB 资源设计但改动不同；H3 与 CASE47 H4 都涉及 tile 场景但一个删单 tile 循环控制、一个调 wide tile 宽度。H5 与 CASE47 H5 都在事件/issue 邻域，TINY 只拟条件化未被消费的 `paramReady` ID 申请/释放，不移动等待顺序。H2/H3 与 CASE47 其余机制无相同改动。均不能由耗时标签推定 case 是否命中。 |
| SELECTIVE-FASTPATH H1：BF16 wide pass-2 的 MTE3_V 等待位置；当前 cycle H3：FP32 wide output 分成两个连续 chunk 写回，并保留 event ring。选择完整 donor，未命中时回退 V011。 | H1-H5 | 这些 donor 改写 wide store/event 路径，不调 block 数。TINY H5 涉及窄中路径 `paramReady` ID 的申请/释放，位置与动作不同；属于同步邻近项，需用函数命中和编译产物确认边界。H1-H4 不改 donor store 机制。SELECTIVE 的 H1 qualification 未完成；local 摘要不证明 Official case 覆盖。 |

本节证据路径：`研究/SCHED-ROWGROUP-X/next-hypotheses.md`、`本地实验/SCHED-ROWGROUP-X/V001/diff.patch`、`本地实验/SCHED-CHAMPION-X/V002/diff.patch`、`本地实验/SCHED-CHAMPION-X/V002/source-meta.json`、`worktrees/w2/m1/case47-small-cluster/研究/CASE47-SMALL-CLUSTER-CHAMPION-X/track-b-case4-case7.md`、`worktrees/w2/m1/selective-fastpath/研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md`。这些资料用于机制对照；路线各自的 Local 摘要不能替代 TINY 对 V011 的直接验证。

H1 的 `DUPLICATE` 仅裁定本条假设的机制重复，不决定 TINY 路线生命周期。H2-H5 的 shape / dtype 仍未知；本表不据 Official 耗时标签推测它们与 SELECTIVE 或 CASE47 的探针重叠。

## 尚需取得的资料与停止点

1. 以 Official testcase ID 为键的 workload manifest：输入全 shape、dtype、epsilon 及数据布局。
2. 能将 testcase 输入映射到 `run_kernel` 的 dispatch 结果：`rowCount`、`rowWidth`、dtype、`availableCoreNum`、`blockCount`、实际 `localRows` 与命中的函数。
3. Official `timeUs` 的计时起止定义，以判断 Host wrapper、Kernel entry 与设备执行各自是否计入。
4. H2 对应的目标编译产物，确认等行数 ownership 中除法/取余是否仍存在。
5. H3 的通用单 tile 路径命中与循环控制生成物；H4 的 buffer 对齐、索引上界及 UB 布局/资源变化。
6. H5 的 `AllocEventID/ReleaseEventID` Ascend C API 生命周期约束，以及目标路径编译产物中相关调用是否保留。

缺少第 1、2 项时，不扩展目标 case、不填写形状、不提交假设供 Main 做实现选择。H2 可审阅机制定义，但不可据此建立 Revision。收到资料后先更新本目录的 Track-B 事实，再等待 Main / Planning 决定；本轮不做 Candidate 修改、Revision、设备操作、构建、正确性运行、测量、Online 或 push。
