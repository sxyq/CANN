# CASE47-SMALL-CLUSTER-CHAMPION-X / Active Core 与 Ownership 复核

DATE: 2026-10-03
TRACK: B
WORKTREE: `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/case47-small-cluster`
BRANCH: `w2/m1/case47-small-cluster`
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
MAIN_SELECTED: NONE

## 范围与已知输入

只复核 Active Core Count、block 到 row 的 ownership 和相关历史方向。没有创建 Revision、编辑 Candidate、运行设备、编译、正确性或计时工作，也不提出路线去留决定。Official 耗时不用于推测 testcase 类型。

| case | testcase ID | shape | dtype | M | D | runtime `availableCoreNum` | 实际 dispatch / ownership |
|---|---|---|---|---|---|---|---|
| 4 | `6a9a9a99bf41025d6013eb96` | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |
| 7 | `6a9a9a99bf41025d6013eba2` | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |

Support-A 正在查询 case map；本路线沿用其结果，不重复发起询问。只收到 family 标签时，shape 与 dtype 仍为 `UNKNOWN`。

Direct Parent 源码给出的通用规则是 `blockCount=min(availableCoreNum,M)`；普通行路径把连续行分给各 block，`baseRows=M/blockCount`，余数分给前面的 block。它不能单独说明 case4/7 的运行时值。精确 ownership 还需输入元数据和 runtime `availableCoreNum`。来源：handoff `track-b-case4-case7.md` §Direct Parent 源码事实，所引 `submission.asc:159-176,3493-3551`。

## 重复性复核

| 方向 | 仓内记录 | 与本路线的关系 |
|---|---|---|
| R016 / FULL-R016 | D 档 rows-per-task、`taskCount=ceil(M/rowsPerTask)` 与 `min(availableCoreNum,taskCount)`；`技术路线/技术路线总表.md` §一、§九之二；`研究/SCHED-ROWGROUP-X/next-hypotheses.md:11-23` | 泛化的按 M/D/core 数设行粒度或 block 数已覆盖。 |
| SCHED-ROWGROUP-X | 基于 R016 加入 `rowGroup=32/gcd(rowBytes,32)`；其后续记录含 core-fill、even-split、cyclic/contiguous、去除 rowGroup 下限及 core-scaling sweep。见 `研究/SCHED-ROWGROUP-X/next-hypotheses.md:14-39,173-196,202-233,237-263`。 | Active Core 与 row ownership 的主轴已有多项具体假设和辨别方案。 |
| SCHED-CHAMPION-X | V001/V002 对 FROZEN R31B-V011 改用行组 ownership；V002 条件为 `totalGroups*2 >= min(blockCount,rowCount)`；V003 另试尾组折叠。见 `技术路线/全版本记录.tsv:65-67`。 | 行组 ownership、保留活动核与尾组分配均已在强父版上有记录，不作为新机制重提。 |
| ROW-OCCUPANCY | 当前 worktree 的正式研究、路线、调度与版本记录中未找到独立的精确同名条目。 | 这个名称本身没有可核的独立机制；按已有 R016/SCHED 公式分类，不能据名称另立方向。 |
| TINY | `MIX-A V004/V005` 记录 tiny `D<=128` 的 FastKernel 分派；`R31A V009` 记录 FP32 tiny R1-only setup；`R31B V014` 仅 build pass，correctness 与 Local timing 未做。见 `技术路线/全版本记录.tsv:15-17,33,44`。 | Tiny dispatch、batch cap 与 setup 方向已有历史记录；无输入映射时不能归到 case4/7，也不能从总分归因单 case。 |
| FASTPATH | `调度/当前任务.tsv:47` 当前记录为 `SELECTIVE-FASTPATH-CHAMPION-X` H1 BF16 D32768 full V017 donor qualification，状态 `QUALIFICATION_READ_ONLY / NOT_STARTED / NOT_ELIGIBLE`，并要求确认精确 M 与 dispatch 可达性后才考虑 Revision。 | 该 lane 的当前记录不提供 case4/7 映射或实际 ownership；是否同域未知。仅凭 D32768 任务标签不能认定 case4/7 属于该路。 |

## 候选假设

下列条目用于界定可证伪条件，不代表新路线获选。每项性能假设均为 `MAIN_SELECTED=NONE`。case4 和 case7 的 shape/dtype 均为 `UNKNOWN`。

### H1：行数不足可用核数，限制 row-parallel 上限

- `MECHANISM`：若 `M < availableCoreNum`，Parent 的 block 数为 M，每个 block 至多对应一段 row；仅靠 row-parallel ownership 不会启动更多 block。
- `BOTTLENECK`：潜在的 row-parallelism 上限。是否适用于 case4/7 未知；D 是否足以摊销额外阶段也未知。
- `EXPECTED_SHAPES / DTYPES`：case4 shape=`UNKNOWN`, dtype=`UNKNOWN`; case7 shape=`UNKNOWN`, dtype=`UNKNOWN`。必要条件仅为实际 `M < availableCoreNum`，不对 D 档作推断。
- `WHY_IT_MAY_HELP`：若必要条件成立，增加同一行内的并行工作可能提高活动核数。
- `WHY_IT_MAY_FAIL`：D-split 需要 partial 合并及跨 block 阶段；已有 R008、C001、R31B V008 相关失败记录。
- `ASCEND_FEASIBILITY`：handoff 记录当前 `run_kernel` 没有 workspace 参数；按现有调用面，需 workspace 或额外 launch 的实现不可直接落地。
- `UB/CORE/DMA_IMPACT`：增加活动核；增加 partial 写读；UB 与 GM 影响取决于拆分实现，当前无法量化。
- `SYNC_IMPACT`：需要跨 block 汇总或第二阶段。
- `PRECISION_RISK`：partial 合并会改变加法次序。
- `DUPLICATE_CHECK`：与 R008 / C001 / R31B V008 的跨核 D-split 重合；当前调用面另有可行性限制。
- `MINIMAL_OFAT_DIFF`：先由 Support-A 映射得出 shape/dtype，再读取同一运行时的 `availableCoreNum` 并计算 M。若 `M>=availableCoreNum`，此假设的前提即不成立；若 `M<availableCoreNum`，也只确认 row-parallel 上限，不证明性能收益。任何性能验证需 Main 另行选择与批准。
- `CLASSIFICATION`：`INFEASIBLE`（以当前调用接口实现 D-split）；现象是否存在仍 `NEEDS_MORE_EVIDENCE`。
- `MAIN_SELECTED`：`NONE`。

### H2：任务粒度或 32B 行组下限改变活动 block 数

- `MECHANISM`：rows-per-task、rowGroup 或两者共同约束 taskCount，进而改变可启动 block 数和每 block 行数。
- `BOTTLENECK`：行组较大时任务数可能少于可用核数；对 case4/7 是否发生未知。
- `EXPECTED_SHAPES / DTYPES`：case4 shape=`UNKNOWN`, dtype=`UNKNOWN`; case7 shape=`UNKNOWN`, dtype=`UNKNOWN`。需实际 D、元素字节数、M 与 runtime 核数才能求 `rowGroup` 和 taskCount。
- `WHY_IT_MAY_HELP`：较细的安全任务粒度可能减少空闲核，前提是 rowGroup 下限确实约束了目标输入。
- `WHY_IT_MAY_FAIL`：放宽行组可能增加未对齐行访问成本或正确性风险；保留行组又可能限制并行度。
- `ASCEND_FEASIBILITY`：host 侧整数分配可表达；安全边界依赖每行拷贝与写入规则。
- `UB/CORE/DMA_IMPACT`：UB 算术可保持不变；活动 block 与 per-core 行段变化；DMA 字节可能不变，访问连续性可能改变。
- `SYNC_IMPACT`：按行独立时无新增跨 block 同步。
- `PRECISION_RISK`：保持每行运算次序时低；ownership 本身不应改算术。
- `DUPLICATE_CHECK`：R016/SCHED H1 覆盖 core-fill 与 rows-per-task；R012/SCHED-ROWGROUP、SCHED-CHAMPION V001/V002 覆盖 rowGroup；SCHED H7 已提出去除 rowGroup 下限的独立变体。
- `MINIMAL_OFAT_DIFF`：取得 case map 后，只静态代入 Direct Parent 与 SCHED 公式，比较 taskCount、blockCount、每 block 行数。若结果落在既有公式内，不新增假设；若超出，先由 Main 审阅差异。
- `CLASSIFICATION`：`DUPLICATE`。
- `MAIN_SELECTED`：`NONE`。

### H3：降低活动核数可抵消每 block 的固定开销

- `MECHANISM`：在 Parent 给出的 blockCount 上限内，使用更小的 block 数，减少每 block 初始化或调度成本。
- `BOTTLENECK`：单 block 工作量很小时的 per-block 固定开销；现有记录把它列作 core-fill 的失败风险，未证明减少 block 能获益。
- `EXPECTED_SHAPES / DTYPES`：case4 shape=`UNKNOWN`, dtype=`UNKNOWN`; case7 shape=`UNKNOWN`, dtype=`UNKNOWN`。需先有精确输入及 dispatch。
- `WHY_IT_MAY_HELP`：若固定成本占主要部分，较少 block 可能缩短总执行路径。
- `WHY_IT_MAY_FAIL`：每 block 行数增加会加长串行工作；可能抵消节省的初始化成本。
- `ASCEND_FEASIBILITY`：host 改 launch width 可表达，无需新 Ascend C API；本轮不改源码。
- `UB/CORE/DMA_IMPACT`：UB 与总 DMA 字节可保持；活动核减少、每核工作增加。
- `SYNC_IMPACT`：无新增跨 block 同步。
- `PRECISION_RISK`：每行算术次序不变时低。
- `DUPLICATE_CHECK`：SCHED H9 已给出 requestedBlocks core-scaling sweep，用于观察每 block 成本与收益拐点；SCHED-CHAMPION V002 也已有 active-core 保留条件。没有理由再提出同类 sweep。
- `MINIMAL_OFAT_DIFF`：SCHED H9 已记载最小辨别法：同一已资格化形状下改变 requestedBlocks，交错比较各核数。对 case4/7 的适用性需先由输入映射确认；执行还需 Main 批准。
- `CLASSIFICATION`：`DUPLICATE`。
- `MAIN_SELECTED`：`NONE`。

### H4：尾行或映射顺序造成每核工作差异

- `MECHANISM`：改变 task extent 均分，或在 cyclic 与 contiguous ownership 间切换；活动 block 数保持不变。
- `BOTTLENECK`：尾部 row 数不均或连续内存访问局部性；仅当实际 M 与 block 数满足相应条件时才可见。
- `EXPECTED_SHAPES / DTYPES`：case4 shape=`UNKNOWN`, dtype=`UNKNOWN`; case7 shape=`UNKNOWN`, dtype=`UNKNOWN`。
- `WHY_IT_MAY_HELP`：均分可能缩短最长 per-core row loop；连续 ownership 可能改善连续访问。
- `WHY_IT_MAY_FAIL`：Parent 已将 M 均分为 `baseRows` 与最多相差一行的余数；若每行工作相同，均分收益有限。`M<=blockCount` 时 cyclic/contiguous 映射等价。
- `ASCEND_FEASIBILITY`：host 侧 extent 或映射变化可表达；本轮不改源码。
- `UB/CORE/DMA_IMPACT`：UB 与活动核数不变；DMA 总量不变，地址连续性可能不同。
- `SYNC_IMPACT`：无新增跨 block 同步。
- `PRECISION_RISK`：按行独立且算术不变时低。
- `DUPLICATE_CHECK`：SCHED H2 覆盖 even-split task extents；SCHED H6 覆盖 cyclic/contiguous；SCHED-CHAMPION V003 覆盖尾组折叠。
- `MINIMAL_OFAT_DIFF`：先取得 M 与 blockCount，静态展开 Parent 的每 block row 区间。若 M 不大于 blockCount，cyclic/contiguous 无法区分；其他差异也已落入 SCHED H2/H6 的既有方法。
- `CLASSIFICATION`：`DUPLICATE`；case 条件是否满足仍未知。
- `MAIN_SELECTED`：`NONE`。

### H5：Tiny / FastPath dispatch 改变每核批量或 ownership

- `MECHANISM`：case 命中特定 tiny 或 fastpath 分支时，其 localRows、tile 或每 block 工作量可能不同于通用路径。
- `BOTTLENECK`：分支选择与每核工作量的匹配程度；case4/7 是否命中相关条件未知。
- `EXPECTED_SHAPES / DTYPES`：case4 shape=`UNKNOWN`, dtype=`UNKNOWN`; case7 shape=`UNKNOWN`, dtype=`UNKNOWN`。历史 tiny 记录含 `D<=128`；当前 FASTPATH 资格任务指向 BF16 D32768，但都不能代替目标 case 输入。
- `WHY_IT_MAY_HELP`：只有目标输入满足既有分支条件且 ownership 有差异时，专用路径才可能改变每核工作量。
- `WHY_IT_MAY_FAIL`：case 可能根本不命中该分支；历史 Official 记录缺少 case 输入映射，不能把整体验证结果归因到 case4/7。R31B V014 没有 correctness 或 Local timing 结果。
- `ASCEND_FEASIBILITY`：静态读取 Direct Parent 的分支条件即可判定可达性；本轮不改代码。
- `UB/CORE/DMA_IMPACT`：依具体分支而定，目标 case 实际影响 UNKNOWN。
- `SYNC_IMPACT`：目标 dispatch 未知，影响 UNKNOWN。
- `PRECISION_RISK`：目标 dtype 与分支未知，风险 UNKNOWN。
- `DUPLICATE_CHECK`：与 MIX-A tiny dispatch、R31A tiny setup、R31B V014 small-D batch cap 相邻；与当前 SELECTIVE-FASTPATH 任务是否同域 UNKNOWN。
- `MINIMAL_OFAT_DIFF`：复用 Support-A 给出的 `(index,testcaseId,shape,dtype)`，静态代入 Direct Parent 的 guard；再与 FASTPATH 现有资格记录核对 M 与 dispatch 可达性。若仅有 family 标签，停止并保留 UNKNOWN。
- `CLASSIFICATION`：`NEEDS_MORE_EVIDENCE`。
- `MAIN_SELECTED`：`NONE`。

## 当前结论与下一输入

现有记录足以判定：泛化的 Active Core 数、rows-per-task、32B rowGroup、尾部均分、cyclic/contiguous 和 core 数扫描均已有对应路线或研究假设；`ROW-OCCUPANCY` 目前没有独立正式记录。Tiny 与 FastPath 的重叠取决于 case4/7 的真实分支可达性，尚不能确认。

最小下一输入是 Support-A 输出的 testcase4 与 testcase7 输入映射，须包含可追溯的 shape、dtype；展开实际 ownership 还需 runtime `availableCoreNum` 来源。拿到输入映射只用于静态复核，不代表获准创建 Revision 或运行实验。所有性能假设继续保持 `MAIN_SELECTED=NONE`。
