# CASE14-INTRAROW-PARALLELISM-CHAMPION-X Track-B

状态：`NEEDS_MORE_EVIDENCE`。本记录没有选定实现方向，不创建 Revision；方向选择留给 Main。

## 证据边界

- Direct Parent 固定为 `线上结果/R31B/V011/submission.asc`，SHA256=`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`；本工作树内复算值一致。
- `线上结果/R31B/V011/result.json` 的 case14 为 `testcaseId=6a9a9a99bf41025d6013ebbe`、`timeUs=16486.82`、`bestTimeUs=3750.12`、PASS，时间比约 4.40x。结果条目没有 rows、D 或 dtype。
- Support-A 交接结论：仓内缺少 Official case 与 shape/dtype 的映射。Support-B 交接结论：`PIPELINE_ONLY_EXPLANATION=INSUFFICIENT`；历史 2.16x 上限与 msprof 数值无法从仓内原始资料复算。本记录未从时间推测 shape，也未把这些历史数值当成重新测得的结果。
- 旧文档 `研究/OFFICIAL-CASE-ANALYSIS.md` 以时间量级推测 case14 可能是宽行或多行。该推测没有输入元数据支撑，不作为本研究事实。

## 本轮证据复核（2026-10-03）

### Official case 与可执行条件

- `线上结果/R31B/V011/result.json` 将 case14 标为 `6a9a9a99bf41025d6013ebbe`：`timeUs=16486.82`、`bestTimeUs=3750.12`、PASS。`线上结果/R31A/V011/result.json` 对同一 ID 给出 `16603.94us`，Official 总分 `42.18`，低于其父版 `44.09`。两个结果都没有 rows、D 或 dtype；`线上结果/R31A/V016/problem-full.json` 对该 ID 也只保留 ID/type，没有输入 shape。
- 竞赛 wrapper `线上结果/R31A/V016/judge-main-template.asc` 以 `run_kernel(..., availableCoreNum, stream, epsilon)` 调用候选；父版 `线上结果/R31B/V011/submission.asc` 使用同一 host ABI，当前只发出一个 kernel launch，且没有从 wrapper 收到 workspace 指针。
- workspace 在该 ABI 内可由候选 host 侧分配：归档精确提交源 `归档/历史工作区/C001/kernel.txt` 中的 `run_kernel` 调用 `aclrtMalloc` 分配 tiling 与 workspace，再把 workspace 作为 kernel 参数传入；同一 stream 完成后同步并释放。多 kernel 也能从同一 `run_kernel` 向同一 stream 连续发出：精确提交源 `归档/历史工作区/R31B/R31B-V008-DSLICE-SMALL-R_kernel.asc` 发出四个有序 launch。两种写法都没有证明性能或正确性；V008 Official 为 0/15 Runtime Error。

### D-slice 历史证据的边界

- R008 状态文件 `归档/phase3-before-reset-20260920/管理/路线状态/R008.json` 仍是 `PLANNED`。`FULL-R008-TILE-CROSS-CORE/V001` 的提交源 `归档/phase3-before-reset-20260920/实验/online/FULL-R008-TILE-CROSS-CORE/V001/6aae33a6b0477ec41ec3e2f1/kernel.txt` 明确每行由一个 core 完成，只按 row/tile task 排 core；它不是 D-slice。Official 15/15，case14 为 `118917.6us`。因此该结果说明这个完整实现很慢，不能当作 D-slice 失败证据。
- C001 的 `线上结果/C001/result.json` 与归档 `kernel.txt` 对应同一上传源。Official TLE 只发生在 testcase 1；testcase 14 是 `Skipped`，没有 case14 耗时。仓内保留 C001 源码、编译记录和 Judge JSON，没有单独的运行时 TLE 控制台日志。因此 C001 是 D-slice 方案的负面 Official 结果，不是 case14 的失败样本。
- R31A V011 源码 `归档/历史工作区/R31A/R31A-V011-submission.asc` 的 D-slice 分支条件为 FP32、`D>8192` 且 host 计算出每行至少两个 slice。其 testcase 14 的耗时高于 best，但 Official 数据没有 shape，不能确认 case14 是否进入该分支，也不能单靠全局退分归因。
- R31B V008 的 exact source 已在 `归档/历史工作区/R31B/R31B-V008-DSLICE-SMALL-R_kernel.asc`，其 Official JSON 记 15/15 Runtime Error（包含 testcase 14）。这证明该四阶段写法未能通过 Official，但结果没有提供输入 shape 或具体 runtime 原因，不能据此判定低-row/超宽条件已被 case14 覆盖。

### 成本与 profile 交叉核对

- 未在仓内找到单独命名为 `SUPPORT-B` 的手交文件。已交叉核对 `worktrees/m1/shape-tiling/研究/SHAPE-TILING-CHAMPION-X/CASE14-BOTTLENECK-ATTRIBUTION.md` 与 `CASE14-SEGMENTED-TIMING.md`。其中 8.28us/row 与约 2000 rows 是用 Official 总耗时除以本地 D=32768 的每行成本得到的规模估计，不是输入元数据。
- 分段数据来自本地 `2x/8x/16x32768 FP32` 和 `2x/8x16384 FP32`；这些行数和 D 未与 testcase 14 对上。`1.23/4` 来自按 kernel duration 分桶的 535 个 msprof task，`>=13us` 只是“与 case14 耗时量级相近”的桶。当前 shape-tiling V002 证据目录没有保留 `op_summary_*.csv` 等原始 profile 导出，故无法从仓内原始 profile 独立复算 `1.23/4` 或 2.16x 理想上限。它们只作为历史本地线索，不是 case14 profile 或本轮新测量。

### MAIN-2 去重交叉核对

- `worktrees/w2/m2/interpass/研究/INTERPASS-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` 将行内 Pass 边界、MTE2/V/MTE3 调度列为范围，并把五个具体 issue 时序与 ASYNC-OVERLAP、ASYNC-TRIPLE、COEFF-LOCALITY、R31A/R31B 记录逐项比对；结论是没有独立未覆盖方向。它同时说明若干 Wave-2 路线的原始包不在该 canonical tree，故这里引用的是已提交 handoff 与共享记录，不把缺失原始包算作本轮测量。
- `worktrees/w2/m2/crossrow/研究/CROSSROW-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` 记录父版 Pass-1 已在 `(row,tile)` task 间双槽预取；相邻行的输入/计算组合可能自然出现。其三个跨行阶段方案均判重复。该路线不切同一 row 的 D，属于相邻去重证据，不可用来证明 case14 的 rowCount。

结论：workspace 分配与同 stream 多 kernel 在 host ABI 层可实现；低-row/超宽 FP32 条件、活跃 core 数、case14 的 stage profile 都未获输入或逐 case trace 证实。现有 D-slice Official 结果为负，但没有一个能单独定位成 case14 的 D-slice 失败测量。

## Parent 执行结构

`submission.asc` 将输入前导维折叠为 rowCount，将末维作为 D。宽行路径在 `D > 8192` 时启用；每行的 tile 数为 `ceil(D / selectedTileWidth)`。tile 宽起始为 4096，UB 预算不够时可递减到 2048。精确 case14 的 D 未知，因此 tiles/row 不能给出数值。

Host 端请求的 block 数为 `min(availableCoreNum, rowCount)`，每个 row 只归一个 vector core。可启动的 core 上限因此是 `min(availableCoreNum, rowCount)`；实际活跃 core 数仍需设备 trace 确认。若 rowCount=1，当前映射最多让一个 core 处理该 case；若 rows 已覆盖可用 core，则单行跨 core 没有空闲并行度可填。

Parent 的宽行实现沿单 core 对 tiles 做平方和，再完成 RMS 归约和输出。全流程用 FP32 reduction；低精度数据有各自的 y 暂存语义。当前 launch 只有一个 kernel，workspace 参数为 `nullptr`，没有跨 core 全局同步。

| 目标量 | 仓内已知 | case14 当前结论 |
|---|---|---|
| rows、D、dtype | Official result 只给 testcase ID 与成绩字段 | 未知；不可由耗时反推 |
| 活跃 core | launch 上限为 `min(availableCoreNum,rowCount)` | 取决于未知 rows 与现场可用 core；无 trace |
| tiles/row | 宽行路径按 D 和运行时所选 tile 宽计算 | D 未知；无数值 |
| Vector 利用率 | 无 case14 原始 trace | 未知 |
| MTE2 / MTE3 利用率 | 无可复算的 case14 原始 msprof | 未知 |
| reduction 与同步 | 每行当前由一个 core 完成；逐 tile 归约后形成 row RMS | case14 的 tile 次数、等待占比未知 |
| launch / workspace | Parent 为单 kernel、无 workspace | 跨 core 分行需要重新确认调用侧 workspace 与多 kernel 可行性 |

MAIN-1 状态材料另记了 Pass1 63–73%、Pass2 26–38%、pipeline sum 1.23/4、约 110 次 sync/barrier per row 与 2.16x full-overlap ceiling。Support-B 已指出原始件不足以重算；这些值只保留为待复验线索。即使暂用 2.16x 作上界，也解释不了完整的 4.40x 时间比，故 pipeline 单因解释不足。

## 与 Main-2 的重复审计

- `ASYNC-OVERLAP-CHAMPION-X` 覆盖 MTE2/V/MTE3 overlap。V001 在 `128x16384 FP16` 有一项合格本地改善，但 Official 44.17 低于 45.16；V002–V004 没形成新收益。Parent V011 的低精度宽行路径也已有跨 `(row,tile)` 的 2-deep MTE2。任何再做同类 tile 流水的想法都与已有工作重合。
- `MULTIROW-DMA-CHAMPION-X` 覆盖 stride 与 multi-row DMA。V001 退步；V002 的对齐 DataCopy 形式与控制组差异在噪声范围内。跨行合批或合并 DMA 属重复方向。
- `SCHED-CHAMPION-X` 覆盖按 32B 对齐的行组所有权；该路线曾有局部收益，但现已只作调度参考。改变多行 row ownership 不等于跨 core 拆分一条 row。
- `REDUCE-HIER-X` 的多种 row 内 reduction 变体未形成可信收益。只改变单 core 内 partial sum 的折叠方式也属于已覆盖方向。
- D-slice 负面证据来自 C001、R31A V011 和 R31B V008；R008 的完整 V001 是不切 D 的 row/tile core mapping。C001 的 TLE 在 testcase 1 且跳过 case14；R31A V011 的 testcase 14 虽变慢，shape 与分支执行状态缺失；R31B V008 的 testcase 14 Runtime Error 也无 shape 与具体原因。它们提高了重试风险，但不能作为 case14 已触发低-row/超宽分支的证据。
- 因此，跨 core 拆分同一 row 已有概念重合与负面历史；另加一层 workspace/双 kernel 仍属相邻实现，不能仅凭 case14 的时间比重新立项。

## 候选假设

### H1：低 rowCount 时将一条 row 切给多个 core

- `MECHANISM`：第一 kernel 按 row/tile 计算 FP32 partial square sum 并写 workspace；同一 stream 上的第二 kernel 读取该 row 的 partials，合并 RMS 后由多个 core 分段重读输入并写输出。
- `BOTTLENECK`：仅当 case14 的 row 数显著小于可用 core，单 row 的 D 足够大，且当前 row 独占映射造成并行度不足。
- `EXPECTED_SHAPES`：少量 rows、很大 D；FP32 优先做精度原型，FP16/BF16 需沿用各 dtype 的 y 语义。具体范围待 Official 元数据给出。
- `WHY_IT_MAY_HELP`：同一 row 的 tiles 可在多个 core 并行，增加向量计算与分布式 MTE 的总并行度。
- `WHY_IT_MAY_FAIL`：多一次 launch；第一阶段读取 x/residual 并写 partials，第二阶段需再次读取 x/residual、gamma/bias 并写 output。若 rows 已足以占满 core，额外同步和流量只会增加时间。
- `ASCEND_FEASIBILITY`：wrapper 的 `run_kernel` 收到 `aclrtStream`；C001 源码证明可在 host 侧分配 workspace，R31B V008 源码证明可顺序发出多个 kernel。API 形态可行，正确性和性能仍未通过；单 kernel 内不可假设有全 grid barrier。
- `UB/CORE/DMA_IMPACT`：每 row 的 workspace 约为 `4 * ceil(D/tileWidth)` bytes；第二阶段每个 core 还要读该 row 的 partials。core 并行度增加，GM 流量也增加。
- `SYNC_IMPACT`：依靠两次有序 launch，不做跨 core 自旋等待。增加一次 launch 边界；partial sum 写入须在第一 kernel 完成后对第二 kernel 可见。
- `PRECISION_RISK`：partial 与最终 sum 保持 FP32；归约顺序改变仍会带来舍入差。必须确认低精度 y 的舍入位置不变，并覆盖宽 FP32 中 Parent 已知的非确定误差现象。
- `DUPLICATE_CHECK`：R008 V001 不切 D；同 row D-slice 已由 C001、R31A V011 与 R31B V008 覆盖，机制重复风险高。C001 exact source 与 TLE JSON 已齐，但没有运行时 TLE 日志且 testcase 14 被跳过；R31A V011 的 case14 shape 缺失；R31B V008 exact source 与全测 Runtime Error JSON 已齐但失败原因未知。两阶段调用的同步组织不同，尚不足以抵消同一 D-slice reduction 的重复性。
- `FALSIFIABLE_TEST`：先取得 Official case14 的 rows、D、dtype 与可用 core 数。若不满足 FP32、`D>8192`、每行至少两个 slice，则“case14 因低 rowCount 使大部分 core 空闲”的 H1 前提不成立，应停止该机制方向；若满足，再由 Main 判断是否允许 exact-shape correctness 与测量。
- `MINIMAL_OFAT_DIFF`：只变 row ownership 与两阶段 partial reduction；tile 算法、epsilon、归一化次序、dtype 算术和输出公式保持 Parent。
- `EXPECTED_LOCAL_PROBES`：先取 Official 输入元数据、目标设备 core 数与 exact testcase14 dispatch/profile；这些资料到位前不创建 Revision、不做设备运行或测量。C001 exact source/TLE JSON 已在仓内，缺的是它的运行时控制台日志。
- `MATURITY`：`NEEDS_MORE_EVIDENCE`；实施机制与历史 D-slice 重复，case14 前提未证实。

### H2：扩大 tile 并调整 UB 驻留以减少逐 tile 周转

- `MECHANISM`：对确认的 case14 dtype 单独改变 wide tile 宽度或同 core 的完整 y 驻留，减少 tiles/row、stage 往返和逐 tile 等待。
- `BOTTLENECK`：只有 trace 证明单 core 执行中的 tile 固定开销占比大，且 tile 增大没有造成 UB 驻留下降时成立。
- `EXPECTED_SHAPES`：必须先证实为 `D > 8192` 的宽行；D 与 dtype 仍未知。
- `WHY_IT_MAY_HELP`：tile 次数减少可能降低循环与同步数量。
- `WHY_IT_MAY_FAIL`：UB 占用可能减少可并行驻留；大 tile 无法修复 rows 不足造成的 core 空闲；Main-1 已有 4096→8192 与多尺度探针，信号在噪声范围。
- `ASCEND_FEASIBILITY`：沿用 Parent 单 kernel 与 row ownership；改 tile 会改变 vector 工作量、DMA 长度和 UB 需求。
- `UB/CORE/DMA_IMPACT`：每核 UB 增量与驻留 core 数需按 dtype 重算；较长 DMA 可能减少命令数，也可能降低并发。
- `SYNC_IMPACT`：预期减少 tile 边界等待；实际等待须以该 shape 的 profile 归因。
- `PRECISION_RISK`：数学顺序维持时较低，但不同 tile reduction 分组会更改 FP32 求和顺序。
- `DUPLICATE_CHECK`：与 Main-1 SHAPE-TILING 的 tile 轴相撞；目前不构成新假设。
- `MINIMAL_OFAT_DIFF`：仅改 tile 宽度或驻留行数其中一个变量，不同时改流水。
- `EXPECTED_LOCAL_PROBES`：取得 shape 后，在 Parent 上采集 per-stage 与 tile/barrier 计数；仅当瓶颈证据与旧探针覆盖不同，再申请单变量验证。
- `FALSIFIABLE_TEST`：仅当官方 D 确认落入多 tile 路径才评估；若 exact-shape 的 tile 数下降而配对差值仍在该形状噪声范围内，则否定 tile 周转是该 case 的主因。V002 的本地 32768 探针已有 8→6 tile、方向不稳的微小差异，不能外推至未识别的 case14。
- `MATURITY`：`DUPLICATE`。

### H3：同一 row 的 tile 流中加深 MTE2/V/MTE3 重叠

- `MECHANISM`：以双缓冲把下一 tile 的 MTE2 与当前 tile 的 Vector 工作重叠，并推迟可安全延后的 MTE3 等待。
- `BOTTLENECK`：case14 的 MTE2 或 MTE3 空隙确为关键路径，且 Vector 运算能够覆盖 DMA 延迟。
- `EXPECTED_SHAPES`：宽 D、多 tile；dtype 和 rowCount 待确认。
- `WHY_IT_MAY_HELP`：降低每 tile 串行阶段时间。
- `WHY_IT_MAY_FAIL`：Parent V011 已在低精度 wide 路径使用 2-deep MTE2；buffer 别名、事件配对和 UB 增压可能引入等待或 race。历史 overlap 方向没有提升 Official。
- `ASCEND_FEASIBILITY`：需逐 buffer 明确 MTE2/V/MTE3 所有权与事件生命周期；不得依赖跨 core 同步。
- `UB/CORE/DMA_IMPACT`：双缓冲增加 UB；DMA 吞吐上限不变，可能挤压驻留度。
- `SYNC_IMPACT`：减少某些 stage wait，同时增加 event 状态管理；需证明每个 buffer 在复用前已释放。
- `PRECISION_RISK`：算术指令与次序不动时较低；异步重排不能改变 reduction 输入完成时点。
- `DUPLICATE_CHECK`：与 Main-2 ASYNC-OVERLAP-CHAMPION-X 和 Main-1 的 pipeline 分析重复。
- `MINIMAL_OFAT_DIFF`：只调整一个 stage 的事件/预取次序，保留 Parent 算术、tile 和 row ownership。
- `EXPECTED_LOCAL_PROBES`：若未来重开，先取 case14 原始 timeline，量出 MTE2/V/MTE3 重叠区间与空隙，再和已有 V001–V004 覆盖核对。
- `FALSIFIABLE_TEST`：需要绑定 testcase14 的原始 timeline；若关键路径没有可覆盖的 MTE2/V/MTE3 空档，或空档已被父版路径覆盖，则否定继续加深该 row 内流水。历史 `1.23/4` profile 不满足这个 case 绑定条件。
- `MATURITY`：`DUPLICATE`。

### H4：多 row 连续搬运与 row-group 分配

- `MECHANISM`：把相邻 row 的输入或输出 DMA 合并成连续段，再由一个 core group 处理多个 row。
- `BOTTLENECK`：case14 有多行、行边界可安全合并，且 DMA 命令或行分配而非算术 / per-tile 等待占主导。
- `EXPECTED_SHAPES`：至少多行；对齐要求与 dtype 相关，具体值待元数据。
- `WHY_IT_MAY_HELP`：摊薄每行 DMA 与调度开销。
- `WHY_IT_MAY_FAIL`：短 row 可能本就可合批；长 row 的主要瓶颈不会因跨行合并消失；行边界、尾块和输出地址必须严格保持。
- `ASCEND_FEASIBILITY`：需按 32B DataCopy 约束验证合并区间及尾部处理。
- `UB/CORE/DMA_IMPACT`：提高连续搬运长度，可能增加每 core 同时驻留 row 数及 UB 占用；若 rowCount 已足够大，也可能减少可并行 block 数。
- `SYNC_IMPACT`：每 row 同步或 DMA 次数可能降低；跨 row buffer 生命周期更复杂。
- `PRECISION_RISK`：只变搬运和归属时算术风险低，越界读写与写重叠风险高。
- `DUPLICATE_CHECK`：与 Main-2 MULTIROW-DMA-CHAMPION-X、SCHED-CHAMPION-X 重复；已有结果未支持继续沿同轴开发。
- `MINIMAL_OFAT_DIFF`：只变连续搬运或 row group 大小中的一个，不同时改变 tile 和 reduction。
- `EXPECTED_LOCAL_PROBES`：确认 rows 与内存对齐后，先以 trace 数出 DMA 命令及 burst 字节，再决定是否值得后续验证。
- `FALSIFIABLE_TEST`：若 rows=1，则跨 row 合并机制不成立；若 rows>1，也需先证实 testcase14 的 DMA 描述符/带宽由行边界主导，并与已有 MULTIROW-DMA、CROSSROW 路线覆盖范围区分。
- `MATURITY`：`DUPLICATE`。

## 结论与待补输入

四项假设针对不同机制：同 row D-slice、tile 粒度、row 内流水、跨 row 搬运/归属；当前都没有可直接创建 Revision 的假设。H1 还未证明目标前提，H2–H4 与既有路线相撞。

## Main 交接建议

建议先做机制证伪交接，再讨论实现：请 Main 获取 testcase14 的权威 rows、D、dtype 和设备可用 core 数，并将 per-case dispatch 与 profile 绑定到该 ID。若条件不满足 FP32、`D>8192`、rowCount 小于可用 core 的 D-slice 触发范围，则记录 H1 前提不成立，停止以低 rowCount/超宽解释 case14；若满足，再由 Main 决定是否安排 exact-shape correctness 与测量。现阶段继续保持 `NEEDS_MORE_EVIDENCE`，不创建 Candidate 或 Revision。

本轮只读历史证据，未运行 Candidate、构建、正确性、设备实验、计时或线上提交。
