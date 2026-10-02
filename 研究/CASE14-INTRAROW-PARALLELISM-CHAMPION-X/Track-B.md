# CASE14-INTRAROW-PARALLELISM-CHAMPION-X Track-B

状态：`NEEDS_MORE_EVIDENCE`。本记录没有选定实现方向，不创建 Revision；方向选择留给 Main。

## 证据边界

- Direct Parent 固定为 `线上结果/R31B/V011/submission.asc`，SHA256=`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`；本工作树内复算值一致。
- `线上结果/R31B/V011/result.json` 的 case14 为 `testcaseId=6a9a9a99bf41025d6013ebbe`、`timeUs=16486.82`、`bestTimeUs=3750.12`、PASS，时间比约 4.40x。结果条目没有 rows、D 或 dtype。
- Support-A 交接结论：仓内缺少 Official case 与 shape/dtype 的映射。Support-B 交接结论：`PIPELINE_ONLY_EXPLANATION=INSUFFICIENT`；历史 2.16x 上限与 msprof 数值无法从仓内原始资料复算。本记录未从时间推测 shape，也未把这些历史数值当成重新测得的结果。
- 旧文档 `研究/OFFICIAL-CASE-ANALYSIS.md` 以时间量级推测 case14 可能是宽行或多行。该推测没有输入元数据支撑，不作为本研究事实。

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
- 历史 `R008 Tile 跨核` 记有 D-slice 失败；`C001 协作 D-Slice 归约` 记录为 TLE。`R31A V011` 也记录为 few-row wide D-slice，Official 42.18 低于父版 44.09，但当时没有 per-case shape 映射，不能归因到 case14。
- 因此，跨 core 拆分同一 row 已有概念重合与负面历史；另加一层 workspace/双 kernel 仍属相邻实现，不能仅凭 case14 的时间比重新立项。

## 候选假设

### H1：低 rowCount 时将一条 row 切给多个 core

- `MECHANISM`：第一 kernel 按 row/tile 计算 FP32 partial square sum 并写 workspace；同一 stream 上的第二 kernel 读取该 row 的 partials，合并 RMS 后由多个 core 分段重读输入并写输出。
- `BOTTLENECK`：仅当 case14 的 row 数显著小于可用 core，单 row 的 D 足够大，且当前 row 独占映射造成并行度不足。
- `EXPECTED_SHAPES`：少量 rows、很大 D；FP32 优先做精度原型，FP16/BF16 需沿用各 dtype 的 y 语义。具体范围待 Official 元数据给出。
- `WHY_IT_MAY_HELP`：同一 row 的 tiles 可在多个 core 并行，增加向量计算与分布式 MTE 的总并行度。
- `WHY_IT_MAY_FAIL`：多一次 launch；第一阶段读取 x/residual 并写 partials，第二阶段需再次读取 x/residual、gamma/bias 并写 output。若 rows 已足以占满 core，额外同步和流量只会增加时间。
- `ASCEND_FEASIBILITY`：跨 kernel 的 stream 顺序可提供阶段边界；单 kernel 内没有可供全 grid 使用的安全 barrier。需确认 direct-invoke host ABI 可传 workspace，并确认 workspace 生命周期与第二次 launch 均受支持。
- `UB/CORE/DMA_IMPACT`：每 row 的 workspace 约为 `4 * ceil(D/tileWidth)` bytes；第二阶段每个 core 还要读该 row 的 partials。core 并行度增加，GM 流量也增加。
- `SYNC_IMPACT`：依靠两次有序 launch，不做跨 core 自旋等待。增加一次 launch 边界；partial sum 写入须在第一 kernel 完成后对第二 kernel 可见。
- `PRECISION_RISK`：partial 与最终 sum 保持 FP32；归约顺序改变仍会带来舍入差。必须确认低精度 y 的舍入位置不变，并覆盖宽 FP32 中 Parent 已知的非确定误差现象。
- `DUPLICATE_CHECK`：与 Main-2 interpass overlap、crossrow DMA、行组 ownership、单 core reduction 拓扑机制不同；但跨 core 同 row reduction 已被 R008/C001 覆盖，C001 有 TLE 记录。R31A V011 的全局 Official 回退也不能定位到 case14。当前判为重复；如要重开，须先取得旧 C001 的 exact source / TLE log，并证明 Official case 的 rows、D、dtype 及新两阶段调用机制提供旧结果没有验证的新条件。
- `MINIMAL_OFAT_DIFF`：只变 row ownership 与两阶段 partial reduction；tile 算法、epsilon、归一化次序、dtype 算术和输出公式保持 Parent。
- `EXPECTED_LOCAL_PROBES`：先拿到 case14 的 rows/D/dtype 与目标设备 core 数，再取得旧 C001 的 exact source 和 TLE log；静态确认 workspace/launch ABI。只有 Main 认为新条件足以重开后，才做 exact-shape correctness、Parent same-binary 与单变量配对测量。当前未做设备运行或测量。
- `MATURITY`：`DUPLICATE`；case14 专项是否值得重开仍为 `NEEDS_MORE_EVIDENCE`。

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
- `MATURITY`：`DUPLICATE`。

## 结论与待补输入

当前资料不能确认 case14 是单行、低 rowCount、宽 D 或任一 dtype；也不能给出活跃 core、tiles/row 与 Vector/MTE2/MTE3 利用率。H1 与 Main-2 的 interpass/crossrow 机制有区别，但和早期 D-slice 同 row 归约重合；H2–H4 也已有路线覆盖，暂没有可直接选中的新假设。

Main 若要继续判断，先需 Official testcase 元数据映射（rows、D、dtype）、该 case 的 kernel/profile 原始文件和 profile 指标定义、旧 C001 exact source/TLE log，以及 direct-invoke workspace 与多 kernel 调用侧能力。缺少这些资料时，本 Route 停在 `NEEDS_MORE_EVIDENCE`；本轮没有 Candidate、设备运行、计时或线上操作。
