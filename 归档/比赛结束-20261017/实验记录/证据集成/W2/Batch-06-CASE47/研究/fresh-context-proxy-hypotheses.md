# CASE47-SMALL-CLUSTER-CHAMPION-X / Fresh-Context Track-B

DATE: 2026-10-03
WORKTREE: `/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/case47-small-cluster`
BRANCH: `w2/m1/case47-small-cluster`
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_COMMIT: `43a1049a1e08e518c88e354a754fdebb85a96f99`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
MAIN_SELECTED: NONE
REVISION: NONE

## 输入事实与边界

Official 记录给出 case4 ID `6a9a9a99bf41025d6013eb96`、耗时 16.55 us，以及 case7 ID `6a9a9a99bf41025d6013eba2`、耗时 52.34 us。两案的输入 shape、dtype、扁平行数 M、行宽 D 和运行时 `availableCoreNum` 均没有可追溯记录。`研究/OFFICIAL-CASE-ANALYSIS.md` 按耗时归入的类别属于推断，本文不以该类别选择实现路径。

case4 与 case7 是否经过同一个 device path 目前未知。R31B-V011 的 host launch 只按 dtype 选模板；device 内还按 D、dtype、每 block 行数及对齐条件分派。只有取得两案的输入元数据和 `availableCoreNum`，才能分别展开这些条件并判定路径是否相同。若分支条件不同，不把两案合并为同一优化对象。

下文的每个尺寸、dtype 组合均为 **PROXY**，仅代表可控实验输入，不代表 Official 输入。`A` 表示实验时读取的 `availableCoreNum`；`M=2A` 是合成代理行数，确保 Parent 按 `min(A,M)` 启动后每个 block 分到两行。`D=257` 是用于触发非对齐窄中分支的代理宽度，不是对 case4 或 case7 的猜测。

## Parent 路径事实

- `submission.asc:171-176,3532-3545`：Parent 按 `blockCount=min(availableCoreNum,M)` 划分连续行，每个 block 的行数相差至多一行。
- `submission.asc:186-239`：FP32 的短行批处理要求 D 可被 8 整除；FP16/BF16 对应分支要求 D 可被 16 整除。D 在 128 与 4096 之间且未先命中专用批路径时，进入 `ProcessNarrowMidOverlap`。
- `submission.asc:499-616`：`ProcessNarrowMidOverlap` 对每行单独执行 Level-2 `ReduceSum`、两轮 V/S 标量往返及输出；当 block 有多行时，gamma/bias 已驻留。
- `submission.asc:1419-1454,1618-1661,1730-1801`：已存在的小行批处理会按行组读取归约值；FP32 contiguous 路径还会把批量结果一次写回。重复提出对齐行批处理或批量写回不构成新方向。
- `submission.asc:1828-1844`：已有 FP32 gamma/bias 多行广播 helper；窄小路径不能把同一广播方式重新包装为新假设。

相关历史：`MIX-A V005` 已试 tiny `D<=128` 的 FastKernel 分派；`R31B V014` 只完成构建，改动为高行数小 D 的 batch cap，缺少正确性与测时；`ASYNC-OVERLAP V002/V003` 覆盖窄中行 issue 次序与行内 GetValue 路径；`SCHED-CHAMPION V001-V003` 覆盖 ownership、活动 block 和尾组；`MULTIROW-DMA V001/V002` 覆盖宽低精度输入批量 DMA，前者丢失 Parent 的双缓冲收益，后者的结果落在噪声带。以上均不能推导 case4/7 的输入映射。

## H1：非对齐窄中行的标量往返按行组执行

**机制**：仅在 Parent 会进入 `ProcessNarrowMidOverlap` 且同一 block 有至少两行时，先逐行沿用当前 Level-2 `ReduceSum`，将每行平方和写入间隔 8 个 FP32 元素的槽位，并把各行 `x+residual` 保存在现有 value 区。完成一组 K 行的归约后，集中执行一次 `SyncVToS`、读取 K 个平方和、一次 `SyncSToV`；再集中完成每行 `Sqrt`，集中做第二轮 V/S 往返。每行标量公式和输出算术顺序保持 Parent 原样。

**瓶颈与可证伪预测**：Parent 当前每行分别执行两轮 V/S 往返，共 4K 个方向同步调用；本假设预测 K 行组只需 4 个方向同步调用。若编译结果仍保留每行四次往返，或 K>=2 的合格代理形状上延迟改善始终落在 Parent 噪声带内，则假设不成立。

**PROXY shape/dtype**：`M=2A, D=257`，dtype 分别取 FP32、FP16、BF16；三者的 257 元素宽度均不满足 Parent 对齐小行批处理条件，且 `129<=D<=4096`。这些全是 PROXY。case4/case7 的实际 dtype、D、M 仍为 UNKNOWN。

**可能收益 / 失败原因**：多行共用同步边界，可能降低行级固定成本。若 value 与归约槽位增加的占用迫使 K 变小，或标量往返本来被其它工作覆盖，净收益会消失。`ProcessNarrowMidOverlap` 的多行情形已驻留参数；本机制不移动参数 DMA。

**UB / DMA / 同步影响**：需同时保留 K 行 value，峰值数据量约为 `K * D * sizeof(float)`；当前 `valueFp32Buf_` 已有 8192 个 FP32 元素，`M=2A,D=257` 的代理组可在现有分配内复用。只有 K*D 超出现有容量时才需额外 UB。每行归约值另占 8 元素间距；x/residual GM 读取量与逐行 DMA 次数不变。V/S 同步由每行两轮改为每组两轮；输入缓冲复用仍需保留 Parent 的 `V_MTE2` 生命周期等待。

**精度风险**：每行继续用 Parent 的 Level-2 `ReduceSum`、同一计数 D、同一标量公式与 `Sqrt` 次序，预期风险低。填充槽不参与归约；不得把 padded D 作为 count。

**重复审计**：旧 Track-B H1 把 padding、行批处理和 epilogue 混作一个提议；此处只隔离 V/S 同步按行组执行，不改 input DMA、ownership、epilogue 算术或 tile。Parent 已在对齐行路径中批量读取标量，因此新信息仅限未对齐 `ProcessNarrowMidOverlap` 条件。与 ASYNC-OVERLAP 的行内 issue 重排不同；若实现需要改变 MTE2 issue 次序，则越出本假设。

**最小实验定义**：取得 Official 映射后先静态确认 case4/7 是否同入窄中路径，以及各自 `localRows` 是否至少为 2。获 Main 选择后只替换该路径的标量往返编排；以 `M=2A,D=257` 的三种 PROXY 做 Parent/Candidate correctness，再按项目协议完成 same-binary 与交错配对测量。当前不建 Revision、不运行这些实验。

## H2：窄行组使用二维 AR Pattern ReduceSum

**机制**：在 `ProcessNarrowMidOverlap` 的独立替代版本中，把 K 行的 FP32 平方值排为 `{K, paddedD}`，以 `ReduceSum<float, Pattern::Reduce::AR>` 一次生成 K 个行和；归一化、每行标量往返和 epilogue 保留 Parent 编排，以便单独辨别 reduction primitive 的影响。`paddedD` 按 8 个 FP32 元素取整，Pattern 临时区按目标 Toolkit 文档给出的 ReduceSum 要求配置，`srcInnerPad=true`。

**瓶颈与可证伪预测**：当前路径对 K 行调用 K 次 Level-2 `ReduceSum`。本假设预测 AR form 能降低逐行 reduction 的发射成本；若临时区与数据排布开销大于节省，或 NPU 配对结果不超过噪声带，则不成立。

**PROXY shape/dtype**：`M=2A, D=257`，dtype 为 FP32、FP16、BF16；各类型先按 Parent 既有规则转为 FP32 再构造平方值矩阵。Pattern 内部列宽用 264 个 FP32 元素对齐。case4/case7 的确切输入仍为 UNKNOWN；这些输入仅标为 PROXY。

**可能收益 / 失败原因**：一次二维归约可能减少 K 次独立 API 发射。K 较小时收益可能小；Pattern 临时区和二维行距也可能增加本地数据整理成本，抵消收益。

**UB / DMA / 同步影响**：GM 输入字节不变；需额外保存 K 行平方值、K 个输出和 Toolkit 所需临时区。Pattern 列宽需 32B 对齐。Parent 非宽 FP32 `InitBuffer` 最大配置约 176 KiB；若叠加临时区会逼近 910B3 的 192 KiB UB，必须先按实际同时存活区间重算，不能直接追加容量。同步行为由 Pattern API 与现有 scalar handoff 决定，编译前不得假定会减少。

**精度风险**：跨行 Pattern 可能改变每行平方和的树形次序；FP32、FP16、BF16 三条输入路径都需逐案比对。精度风险高于 H1；官方输入映射到手并不代表 correctness 自动通过。

**重复审计**：仓库源码与研究未检索到 `Pattern::Reduce::AR` 的实际算子用法。FULL-R007 采用逐行 ReduceSum；REDUCE-HIER 改同一行的归约组织，与跨行 AR 调用结构不同，但都属于归约方向，存在相邻风险，需 Main 单独审阅。不得与 H1 的跨行 scalar 编排放进同一 Revision。

**最小实验定义**：先以安装 Toolkit 的 API 文档确认 ReduceSum Pattern 的签名、tmp 尺寸接口和 DAV_2201 支持，再由 Main 选择。获选后只把 K 个 Level-2 调用替换为一次 AR 调用；在 `M=2A,D=257` 三 dtype PROXY 完成构建和逐元素 correctness，通过后才进入 same-binary 与交错配对测量。当前不创建独立微基准或 Revision。

## H3：窄中路径专用 device entry

**机制**：为 Parent 已有 `129<=D<=4096` 的窄中通路建立一个按输入元数据选择的专用 kernel entry，使 device entry 直接进入 `ProcessNarrowMidOverlap`，不再经过 `widePath_` 与其它 dtype、对齐小行、整行路径的分支链。Host guard 必须重现 Parent 的全部先行条件，包括 dtype、D、blockCount 和每个 block 的 localRows；命中 Parent 的专用批路径时仍调用 Parent 原 entry。函数内的 M/D ownership、Load、ReduceSum、算术、事件和 Store 全部维持 Parent 原样。仅在 exact case4/7 映射证明它们走同一窄中路径时，才把两案作为共同候选目标。

**瓶颈与可证伪预测**：短行下每 block 的控制指令可能占比可见。预测专用 entry 的 device 指令中不再包含通用路径分支链；若生成代码没有减少相关分支/指令，或 correctness 相同而配对延迟无超噪声改善，则否定该假设。

**PROXY shape/dtype**：`M=2A, D=257`，dtype 为 FP32、FP16、BF16，均为 PROXY；该宽度落在 Parent 窄中范围并绕过对齐短行 batch。case4/case7 的实际分支未知，不以此代理代替 Judge 输入。

**可能收益 / 失败原因**：每个 block 少走多条 device-side 条件分支，可能降低短工作段的控制成本。分支链可能已被编译器简化，或相对 MTE/Vector 工作足够小，专用 entry 也可能只增加 host 侧分支而不改变计时区间。

**UB / DMA / 同步影响**：不改 UB 缓冲、DMA 次数、活动 block 数或事件图；构建产物增加一份专用 entry，源码体积上升。没有跨 block 协作。

**精度风险**：数据通路逐指令保持 Parent 原样，精度风险低；须确认 Host guard 与 Parent 的 dtype、D 条件一致，未命中时仍调用 Parent 通用 entry。

**重复审计**：MIX-A V005 的 tiny dispatch 限于 `D<=128`；R31B V014 改高行数小 D 的 batch cap；SCHED 路线改 ownership/core 数。此项限定为窄中 `129<=D<=4096` 的 device-side 分支裁剪，不改变行分配或批量上限。若反汇编显示 Parent 已裁掉同一分支，假设直接终止。

**最小实验定义**：取得 case4/7 输入映射后展开 Parent 的分支条件，静态比较专用与通用 entry 的设备代码。仅当分支代码确有删除且两案命中同一条件，才建议 Main 考虑一次单变量 Candidate；Main 选择后做 correctness，再按项目协议资格验证与配对测量。当前不编译、不测时。

## C2C：SELECTIVE-FASTPATH H3 机制对照

按本轮 C2C 更新，Main 已选择 `SELECTIVE-FASTPATH H3`：以 STORE V003 为 donor，仅在精确 FP32 shape allowlist 命中时分派 donor，其余输入精确回到 R31B V011。allowlist 只含 `8x16384`、`1x32768`、`1x16384` 三个 PROXY；它们不是 Official case 映射。case4/case7 的输入 shape、dtype 与是否命中该集合依旧 UNKNOWN。

STORE V003 的直接父版为 STORE V002；V003 相对 V002 把每行整段写回切成 K=2 tile 对齐块，在第一块完成时提前发出 Store，让 MTE3 写回与第二块计算交叠，并保留双槽事件环。V003 完整 donor 也带有 V002 的整行合并写回。源码条件还要求 FP32 wide 路径、`tileCount>=4` 与 `D%8==0`；用户提供的三项 allowlist 是更窄的 PROXY 集合。donor 未命中时的 R31B V011 SHA-256 与本 Route Parent 相同：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。

| CASE47 假设 | 机制交集 | 当前判断 |
|---|---|---|
| H1 标量往返按行组执行 | 都保留 V011 计算主体；H1 改 V/S 标量阶段，SELECTIVE H3 改 MTE3 输出发出时点。 | 执行单元和数据阶段不同。CASE47 的 `D=257` PROXY 不进入 V003 wide 路径；Official 输入交集未知。 |
| H2 二维 AR Pattern ReduceSum | H2 改跨行平方和归约；SELECTIVE H3 改归约后的输出写回。 | 机制不同。H2 的额外 UB 与精度风险不因 V003 allowlist 而改变；Official 输入交集未知。 |
| H3 窄中路径专用 device entry | 两者都有按 shape 条件选择执行路径的结构。SELECTIVE H3 以精确 allowlist 选择 donor 并以 V011 作为 fallback；CASE47 H3 原提议也包含 metadata guard 与专用 entry。 | 存在分派机制层面的概念重叠。只有 CASE47 H3 能证明 device 代码实际删除了通用分支链、且不只是再添一层 allowlist，才有独立的代码生成假设；若只增加 shape guard 与 fallback，应与 SELECTIVE H3 归为同一分派机制，不单独提出。 |

以上只作 Route 研究对照，不改变 CASE47 的目标、Parent 或 `MAIN_SELECTED=NONE`，也不读取或修改 SELECTIVE-FASTPATH 的 Candidate。不得从时间、allowlist 或 proxy 反推 case4/case7 的 Official 输入。

## 当前结论

三项假设分别针对 V/S 同步编排、归约 API 组织与 device 分支指令，互不合并。H1 的历史邻接最强但适用域可严格限于未对齐窄中行；H2 有 Toolkit 与 UB 风险，列为较高风险研究项；H3 预期收益较小，且 dispatch 概念与 SELECTIVE-FASTPATH H3 相邻，须由设备代码证据证明独立的分支裁剪收益。没有任何 CASE47 假设获选，`MAIN_SELECTED=NONE`。缺少 case4/7 shape、dtype、M、D 与运行时 `availableCoreNum` 时，结论只适用于上述 PROXY，不对 Official 路径作推断。

本轮只新增此 Route 研究说明；未修改 Candidate、Revision、共享调度、路线成绩、版本记录或任务看板。未运行编译、正确性、设备或性能任务。
