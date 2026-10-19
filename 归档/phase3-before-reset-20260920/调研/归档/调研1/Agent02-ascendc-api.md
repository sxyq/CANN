# Agent02：官方 Ascend C API 调研报告（CANN 9.0.0 / 判题模板 Direct Invocation 直调）

> 调研人：Agent 2（官方 Ascend C API 调研）
> 日期：2026-09-11
> 目标版本：CANN 9.0.0 社区版（文档入口 `CANNCommunityEdition/900`）
> 结论适用范围：**本机为 macOS、无 CANN、无 NPU，本报告全部结论均为文档核验，未在任何真实 NPU 上做编译/精度/性能验证**。
> 证据等级：A=官方文档原文；B=官方仓库/官方样例；C=社区；D=仅摘要未核验。

---

## 0. 结论速览（给实现 Agent 的三句话）

1. 判题模板的「Direct Invocation 直调」在官方文档中的名称是 **「Kernel 直调」**（KernelLaunch / Kernel 直调工程），入口函数限定符 `__global__ __vector__` 与 `__aicore__` 均为官方定义（A 级）；`run_kernel` + `<<<blocks, nullptr, stream>>>` 是该模式的 Host 侧启动写法。
2. **DataCopyPad 的支持矩阵按「产品系列」划分**：Atlas A2 训练/推理系列、Atlas A3 训练/推理系列、Atlas 200I/500 A2、Ascend 950（含 mode/Compact 重载）**支持**；**「Atlas 训练系列产品」（老 910A/910B 训练系列）与「Atlas 推理系列产品」（310P 等）不支持**——即「910B 不支持 DataCopyPad」的说法只对老训练系列成立，A2 架构（910B 的 A2 系列产品形态）支持。判题平台必须确认是 A2 系产品，否则要走「无 DataCopyPad 降级路线」。
3. **A2 系产品上 Add/Mul 等双目算术不支持 bfloat16_t（只支持 int16_t/half/int32_t/float）**，因此 BF16 输入必须先 Cast 到 float 计算（首版 kernel 正是此策略，正确）；float→bfloat16_t 输出 Cast 支持 CAST_RINT/FLOOR/CEIL/ROUND/TRUNC，**不支持 CAST_NONE**，首版用 CAST_RINT 正确。

---

## 1. API 对照表

> 「产品」列缩写：950=Ascend 950PR/950DT；A3=Atlas A3 训练/推理系列；A2=Atlas A2 训练/推理系列（含 Atlas 800I A2 推理）；200/500A2=Atlas 200I/500 A2 推理产品；训=Atlas 训练系列产品（老 910A/B）；推=Atlas 推理系列产品 AI Core/Vector Core；Kirin=Kirin X90/9030。
> 「9.0 vs 8.x」列只在有文档差异时填写。

### 1.1 数据结构与搬运

#### GlobalTensor / LocalTensor
- **签名**：模板类；`SetGlobalBuffer(__gm__ T* addr, uint32_t len)`；`operator[]` 返回带偏移的 tensor；`GetValue/SetValue`（标量读写，仅调试用，性能极低）。
- **产品**：所有型号（A 级，9.0 手册「基础数据结构」目录 + asc.gitcode.com 基础API列表）。
- **与当前用法**：首版 `x_.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(x), outer*dim)` 与 `src[offset]` 偏移访问符合官方用法；官方文档明确 GlobalTensor 仅作批量搬运源/目的，**不建议**在 Kernel 内用 GetValue/SetValue 逐元素读写（无 DataCopyPad 文档也强调此点）。
- **未确认**：`SetGlobalBuffer` 的 len 上限与非法地址行为未在文档中单独核验。

#### DataCopy（GM↔UB 连续搬运）
- **签名**：
  - `template<typename T> void DataCopy(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const uint32_t count)`（GM→UB）
  - `template<typename T> void DataCopy(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const uint32_t count)`（UB→GM）
- **参数**：count=元素个数；**count\*sizeof(T) 必须 32 字节对齐，未对齐时搬运量向下取整到 32B 对齐**；UB 侧地址 32B 对齐，GM 侧按 dtype 字节数对齐。
- **产品**：950/A3/A2/200-500A2/训/推/Kirin 全部支持（A 级，9.x master DataCopy_GMAndUB_continuous）。
- **与当前用法**：首版仅在 UB→UB 用 `DataCopy(dst, src, len)`（VECCALC→VECOUT 局部拷贝），GM 侧全部走 DataCopyPad，符合「GM 侧非对齐用 DataCopyPad」的官方建议。
- **9.0 vs 8.x**：8.0 手册中 DataCopy 约束一致；9.x 新增跨卡（HCCS）说明，与本算子无关。

#### DataCopyPad（GM→UB，非对齐+填充）
- **签名**：
  - `template<typename T> void DataCopyPad(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const DataCopyExtParams&, const DataCopyPadExtParams<T>& padParams)`（9.x master；8.0 相同）
  - 950 专属重载：`template<typename T, PaddingMode mode = PaddingMode::Normal> ...`（Normal/Compact 模式）
- **通路**：GM→VECIN / GM→VECOUT / GM→VECCALC（仅 950）。
- **DataCopyExtParams 字段语义**（A2/A3/200-500A2 版）：
  - `blockCount` uint16_t，[0,4095]（950 为 [0,65535]；8.0 为 [1,4095]），默认 1；**blockCount 或 blockLen 任一为 0 时接口为 NOP**（A2/A3/950）。
  - `blockLen` uint32_t，单位 **1 字节**，必须为 sizeof(T) 整数倍，不能超过 UB 空间。
  - `srcStride` uint32_t，单位 Byte（源在 GM 侧）；语义=前一块「结束地址」到后一块「起始地址」的间隔。
  - `dstStride` uint32_t，单位 dataBlock（32B）（目的在 UB 侧）。
  - `rsv` 保留位，必须为 0。
- **DataCopyPadExtParams<T> 字段语义**：
  - `isPad`：true=用 paddingValue 填充；false=不指定，未调用 SetPadValue 时硬件自动在每块右侧补 dummy 至 32B 对齐。
  - `leftPadding/rightPadding`：单位=元素个数；**左右 padding 字节数均不能超过 32B**。
  - `paddingValue`：类型 T；**64 位类型只能为 0**。
- **对齐**：`dst`（UB）起始地址必须 32B 对齐；`src`（GM）**起始地址仅需 1 字节对齐（即无对齐约束）**（A 级原文，8.0 与 9.x 均如此）。
- **产品**（9.x master）：950（需 mode 重载）√；A3 √；**A2 √**；200/500A2 √；**训 ×；推(AI Core/Vector Core) ×**。8.0 页面仅列 A2 训练/800I A2、200/500 A2。
- **dtype**（A2/A3/950）：int8/uint8/int16/uint16/half/**bfloat16_t**/int32/uint32/float/int64/uint64/double（**含 bfloat16_t**；200/500A2 到 float 为止）。
- **与当前用法**：首版 CopyIn 用 `DataCopyExtParams{1, len*sizeof(T), 0,0,0}` + `DataCopyPadExtParams<T>{isPad, 0, rightPadding, 0}` 补 0，字段语义完全一致（blockCount=1 整行搬、rightPadding 为补齐到 32B 的元素数）。**注意**：首版在 `right_padding != 0` 时 isPad=true 并显式补 0，可避免 dummy 随机值/首元素复制的行为，符合文档。
- **未确认**：8.0 文档中 blockLen 上界写 [1,2097151]，9.x master 写 [0, ]——上界应以安装版头文件为准。

#### DataCopyPad（UB→GM，非对齐搬出）
- **签名**：`template<typename T> void DataCopyPad(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const DataCopyExtParams&)`（无 padParams）；950 有 mode 重载。
- **对齐**：`dst`（GM）无对齐约束；`src`（UB）32B 对齐。
- **语义**：blockLen 非 32B 对齐时，框架自动补 dummy 对齐，搬到 GM 时自动丢弃 dummy，实现精确长度的非对齐写回。
- **产品/dtype**：与 GM→UB 相同（950/A3/A2/200-500A2 支持；训/推不支持）。
- **与当前用法**：首版 CopyOut 用 `DataCopyPad(dst[offset], src, {1, len*sizeof(T), 0,0,0})`，正确。

#### DataCopy（UB→UB，含 VECCALC→VECOUT）
- **签名**：`template<typename T> void DataCopy(const LocalTensor<T>& dst, const LocalTensor<T>& src, const uint32_t count)`（连续搬运；另有高维切分/mask 版）。
- **与当前用法**：首版 `StoreOutput(float)` 分支用 `DataCopy(dst, src, len)` 做 VECCALC→VECOUT 拷贝。9.x master 的 DataCopy_UBToUB_continuous 页面为同一 API 族，A2 支持；UB 两侧地址均须 32B 对齐，count\*sizeof(T) 须 32B 对齐（float 版 calc_len 为 32B 对齐长度，满足）。

### 1.2 矢量计算

#### Cast
- **签名**：`template<typename T, typename U> void Cast(const LocalTensor<T>& dst, const LocalTensor<U>& src, const RoundMode& roundMode, const uint32_t count)`；另有高维切分 mask/repeat 版。
- **RoundMode**：`CAST_NONE=0`（有损时等价 CAST_RINT）、`CAST_RINT`（四舍六入五成双）、`CAST_FLOOR`、`CAST_CEIL`、`CAST_ROUND`（四舍五入）、`CAST_TRUNC`、`CAST_ODD`、`CAST_HYBRID`（仅 hifloat8 平台）。
- **产品**：950/A3/A2/200-500A2/训（仅有限组合）/推 AI Core/Kirin 支持；推 Vector Core 不支持。
- **A2 关键组合**：half→float: CAST_NONE；bfloat16_t→float: CAST_NONE；float→half: RINT/FLOOR/CEIL/ROUND/TRUNC/ODD/NONE；**float→bfloat16_t: RINT/FLOOR/CEIL/ROUND/TRUNC（无 NONE/ODD）**；half→int32: RINT/FLOOR/CEIL/ROUND/TRUNC。
- **约束**：mask 按 sizeof 较大者筛选；大小端位宽不同时按大者算；int32→half 的 roundMode 不生效（需 SetDeqScale）；count/repeatTime=0 时 A2/A3 为 NOP。
- **与当前用法**：首版入 Cast(CAST_NONE)（half/bf16→float）、出 Cast(CAST_RINT)（float→half/bf16），均在 A2 支持组合内，正确。
- **未确认**：CAST_HYBRID 与 hifloat8 与本题无关；A2 上 float→bf16 的 RINT 实际舍入行为需真机验证（文档未给误差界）。

#### Add / Mul
- **签名**：`template<typename T> void Add(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, const int32_t& count)`；Mul 同构；另有高维切分 mask/repeat 版。Muls（矢量×标量）同属「双目标量」族。
- **产品**：950/A3/A2/200-500A2/训/推 AI Core/Kirin 支持；推 Vector Core 不支持。
- **A2 dtype**：**int16_t/half/int32_t/float**（**不含 bfloat16_t**）；老「训」系列仅 half/int32_t/float。
- **与当前用法**：首版全部在 float 上做 Add/Mul/Muls，符合 A2 支持表。
- **未确认**：Add/Mul 的别名（dst 与 src 重叠）约束沿用「通用地址重叠约束」，未逐一核验页内细节。

#### Sqrt / Rsqrt
- **签名**：单目 `template<typename T> void Sqrt(const LocalTensor<T>& dst, const LocalTensor<T>& src, const int32_t count)`（Rsqrt 同构），产品全支持（推 Vector Core 除外）。
- **与当前用法**：首版用标量 `sqrtf`（Kernel 内 scalar 指令）而非矢量 Sqrt——因 rms 是标量且每行只算一次；文档未禁止，但需注意标量除法/开方为 PIPE_S，与 PIPE_V 计算之间有 V_S/S_V 同步需求（首版已插）。

#### ReduceSum（count 版本 / mask 高维切分版本）
- **签名**：
  - tensor 前 n 个数据（count 版）：`template<typename T, bool isSetMask=true> void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& sharedTmpBuffer, const int32_t count)`（8.x 文档参数名 workLocal，9.x 改名 sharedTmpBuffer，含义相同）
  - 高维切分 mask 连续版：`template<typename T> void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& workLocal, const int32_t mask, const int32_t repeatTime, const int32_t srcRepStride)`（另 mask 逐 bit 版传 `uint64_t mask[2]`）
- **dtype/产品**：950: half/float/uint64/int64；A3: half/float；**A2: half/float**；200/500A2: half/float；推 AI Core: half/float；**训: 仅 half**；Kirin: half/float。
- **对齐**：dstLocal 起始 2B 对齐（half）/4B 对齐（float）；srcLocal、workLocal 起始 **32B 对齐**。
- **workLocal 最小空间公式**（count 版，8.0/8.0.RC3 文档原文）：
  ```
  typeSize = sizeof(T)              // half=2, float=4
  elementsPerBlock  = 32/typeSize
  elementsPerRepeat = 256/typeSize  // half=128, float=64
  firstMaxRepeat = count/elementsPerRepeat   // count<elementsPerRepeat 时为 1
  iter1OutputCount = firstMaxRepeat
  finalWorkLocalNeedSize = RoundUp(iter1OutputCount, elementsPerBlock) * elementsPerBlock  // 单位：元素个数
  ```
  （高维切分版：firstMaxRepeat = repeatTimes）
  - 或「方式二」：传入任意大小 workLocal，workLocal 值不会被改变（8.x 文档）。
- **累加方式（重要，影响精度）**：**A2/800I A2 的 count 版为「方式二」（同 repeat 二叉树、不同 repeat 顺序累加）**；A2 高维切分版为「方式一」（不同 repeat 也二叉树累加）。训/推/200-500A2/950 为方式一。
- **约束**：src 与 workLocal/dst 可 100% 完全重叠（workLocal 需满足最小空间）；部分重叠不支持；**内部为软件仿真实现，性能可能低于 BlockReduceSum/WholeReduceSum**；count 上限受 UB 大小限制。
- **与当前用法**：首版 `ReduceSum(sum_.Get<float>(), value, work, calc_len)` 使用 count 版 + `GetValue(0)` 取回标量——签名、work 空间（WORK_LEN=1024 float，calc_len≤4096 时 firstMaxRepeat=64，所需 64 元素向上取 32B 对齐=64，1024 远超）、V_S/S_V 同步（ReduceSum 是 PIPE_V，结果经 scalar 读回需 V→S 同步）均符合文档。**注意**：A2 count 版按「方式二」累加，与训练系列（方式一）的求和顺序不同，误差特征不同，需在精度验证中留意。
- **未确认**：9.x master 的 asc.gitcode.com 页面 URL 未直接抓到（经 asc-devkit 仓库转述核实，B 级），但 8.0/8.0.RC3 官方页面为 A 级且一致。

#### Duplicate（填充）
- **签名**：`template<typename T> void Duplicate(const LocalTensor<T>& dst, const T scalarValue, const int32_t count)`（另有 mask/repeat 版）。
- **用途（官方「无 DataCopyPad 的处理方式」）**：搬入后逐行清零脏数据（mask 模式）、归约前清零 dst。
- **与当前用法**：首版未用 Duplicate（靠 DataCopyPad 补 0）；若走降级路线（推理系列/训练系列）则必须用。

#### GatherMask / UnPad / atomic（无 DataCopyPad 降级路线）
- **出处**：官方「无 DataCopyPad 的处理方式」（8.0.RC3.alpha002，A 级）：适用对象=**未提供 DataCopyPad 的 Atlas 推理系列产品**。
  - 冗余数据参与计算（elemwise 场景）；
  - 归约场景用 mask 掩掉脏数据（先 Duplicate 清零 dst）；
  - 逐行 Duplicate 填 0；
  - Pad 一次性清零；
  - 搬出>32B：UnPad 去冗余 + DataCopy；或用 **GatherMask** 借位搬运；
  - 搬出<32B：目标 GM 清零 + 冗余清零 + **SetAtomicAdd<half>() 原子累加**写回（避免踩踏）。
- **产品确认**：GatherMask 为 PIPE_V（9.x API 流水表）；`SetAtomicAdd` 为寄存器配置接口（C API 原子操作列表）；SIMT 级 `asc_atomic_add` 支持 half/bfloat16_t 但 **950 之外不支持**（A2/A3 为 ×）——降级路线用的是 MTE 原子累加模式（SetAtomicAdd + DataCopy），与 SIMT asc_atomic_add 不同，需在文档中区分。
- **未确认**：SetAtomicAdd 在各型号的可用性（9.x C-API 列表有 asc_set_atomic_add，但具体型号矩阵未逐页核验）；UnPad 的签名未抓到（仅降级文档提及）。

### 1.3 资源管理 / 同步 / 系统变量

#### TPipe / InitBuffer（TQue 与 TBuf）
- **签名**：`template<class T> bool InitBuffer(T& que, uint8_t num, uint32_t len)`（TQue/TQueBind/TSCM）；`template<TPosition bufPos> bool InitBuffer(TBuf<bufPos>& buf, uint32_t len)`。
- **语义**：len 单位字节，**非 32B 对齐时内部自动向上补齐**；num=1 单缓冲、num=2 双缓冲；**单个 kernel 所有 Buffer 数量之和不能超过 64**；内存随 TPipe 析构自动释放；重新分配需 Reset 后 InitBuffer。
- **产品**：全支持（推 Vector Core 除外）。
- **与当前用法**：首版 5 个 TQue（各 BUFFER_NUM=1）+ 5 个 TBuf，共 10 个 buffer < 64；tile 尺寸由 sizeof(T) 决定，UB 占用估算见 §4。

#### TQue（AllocTensor / EnQue / DeQue / FreeTensor）
- **语义**（官方 TPipe-TQue 编程原理，A 级）：EnQue 底层发 `Set` 通知下游数据就绪（先写后读）；DeQue 底层发 `Wait` 等待上游就绪；AllocTensor 发 `Wait` 等待该内存读完成；FreeTensor 发 `Set` 通知可覆写（先读后写）。**EnQue/DeQue、AllocTensor/FreeTensor 必须配对**。
- **与当前用法**：首版 EnQue 后立即 DeQue 再算（单缓冲串行），FreeTensor 收尾，符合官方范式；BUFFER_NUM=1 时无双缓冲，性能上可评估开双缓冲。

#### PipeBarrier
- **签名**：`template<pipe_t pipe> void PipeBarrier()`；pipe ∈ {PIPE_V, PIPE_MTE2, PIPE_MTE3, ..., PIPE_ALL}，**不支持 PIPE_S**（标量流水同步由硬件保证）。
- **用途**：同流水内指令串行化（如同 MTE3 目的 GM 重叠时插 `PipeBarrier<PIPE_MTE3>()`）。
- **注意**：Kernel 直调工程默认开启编译器自动同步，PIPE_V 依赖自动插入；显式插入仍合法。
- **与当前用法**：首版在 ReduceSum 前后插 `PipeBarrier<PIPE_V>()`，合法（虽自动同步下可冗余）。

#### SetFlag/WaitFlag（HardEvent，V_S / S_V）
- **签名**：`template<HardEvent event> void SetFlag(int32_t eventID)` / `void WaitFlag(int32_t eventID)`。
- **eventID**：必须用 `GetTPipePtr()->FetchEventID(HardEvent)`（或 AllocEventID/ReleaseEventID 成对）获取，**禁止自行指定**，否则可能与框架事件冲突卡死；SetFlag/WaitFlag 必须成对。
- **eventID 数量**：训系列 0-3；推 AI Core 0-7；**A2 训练系列/800I A2 推理 0-7**。
- **HardEvent 枚举语义**：名称=源流水_目标流水，如 V_S = PIPE_V 为源、PIPE_S 为目标；含 MTE2_V/V_MTE2/MTE3_V/V_MTE3/S_V/V_S 等。
- **与当前用法**：首版 `FetchEventID(HardEvent::V_S)` + SetFlag/WaitFlag 成对（归约结果 V→S 读回），再 `V_S`/`S_V` 两次配对，语义正确；建议真机检查 eventID 数量是否在 0-7 内（首版每行分配 2 个事件，循环内反复 FetchEventID，若 Fetch 不占用计数则安全，需真机确认 Fetch/Alloc 语义差异）。
- **未确认**：9.x master 中 FetchEventID 的确切返回语义（是否耗用事件槽）未在 A 级页面核验（8.0 页面为 A 级）。

#### GetBlockIdx / GetBlockNum
- **签名**：`__aicore__ inline int64_t GetBlockIdx()` / `int64_t GetBlockNum()`（头文件 kernel_operator_sys_var_intf.h）。
- **语义**：GetBlockIdx 返回当前逻辑核 ID（仅 AIC 或 AIV 时范围 [0, numBlocks)）；GetBlockNum 返回 numBlocks（即 <<<>>> 第一个参数）。
- **产品**：全支持（推 Vector Core 除外）。
- **与当前用法**：首版用它做行级负载均衡（each+extra），符合官方多核 Tiling 模式。

#### bfloat16_t
- **定义**：内置 16bit 类型（符号 1 + 指数 8 + 尾数 7）（A 级，内置数据类型页）。
- **搬运**：DataCopy/DataCopyPad 在 A2/A3/950 均支持 bfloat16_t；200/500A2 也支持。
- **计算**：**Add/Mul 等双目算术在 A2 不支持 bfloat16_t**（只能 half/int16/int32/float）；Cast 支持 bfloat16_t→float（CAST_NONE）、float→bfloat16_t（RINT/FLOOR/CEIL/ROUND/TRUNC）。
- **与当前用法**：首版 BF16 路径 = DataCopyPad(bfloat16_t) 搬入 → Cast(CAST_NONE) 转 float 计算 → Cast(CAST_RINT) 转回，符合 A2 支持矩阵。
- **未确认**：SIMT 数学函数（__hmax 等）仅 950 支持 bfloat16_t，与本题无关。

---

## 2. Direct Invocation 直调模式说明（判题模板）

官方文档中该模式的正式名称是 **「Kernel 直调」**（或 KernelLaunch / 基于 Kernel 直调工程 / 核函数直调），区别于「算子框架模式（aclnnXxx / 自定义算子工程）」。

- **入口函数定义**（A 级，asc.gitcode.com 核函数页 + 8.3.RC1 指南 3.2/6.6 节）：
  - 必须 `__global__`（可被 `<<<>>>` 调用）+ 执行空间限定符之一：`__aicore__`（不区分核型，耦合模式）、`__vector__`（**仅 Vector 计算**）、`__cube__`、`__mix__`。
  - **判题模板用 `__global__ __vector__` 对应「纯向量算子」**；`__global__ __aicore__` 为通用写法，两者对纯向量算子均可用（CANNBot 直调指南也注明：矩阵类用 __aicore__，纯向量类用 __vector__，C 级佐证）。
  - 指针入参用 `__gm__`（GM_ADDR 宏）；核函数必须 void 返回；参数只能是指针或内置标量类型。
- **调用语法**（A 级）：`kernel_name<<<numBlocks, dynUBufSize, stream>>>(args)`；**第二个参数是 dynUBufSize（Dynamic UB Size，单位 bytes，默认 0），不是 CUDA 的共享内存**——判题模板传 `nullptr` 会被编译器当作 0 处理（需真机确认判题编译器接受 nullptr 写法）。numBlocks=核数（即 GetBlockNum），stream 为 aclrtStream。
- **Host 侧**：`run_kernel(...)` 这类导出函数内做参数校验 + `<<<blocks, nullptr, stream>>>` 启动；调用是异步的，等待用 `aclrtSynchronizeStream(stream)`。
- **工程形态**：官方样例 `KernelLaunch/AddKernelInvocationNeo`（Ascend/samples）；直调工程编译产物为可执行文件/动态库，`<<<>>>` 语法只在 `.asc` 文件有效。
- **与框架模式差异**（C 级总结）：直调无 KFC 自动调度、无 MIX（Cube+Vector 并行）支持、Tiling 与 Workspace 全手动；`__vector__` 直调在判题平台（A2）上应可行。
- **未确认**：判题平台对 `run_kernel` 签名（含 TensorGroupInfo/可执行核数/stream）的具体编译与链接方式，只能以 CANNJudge 模板为准（不在本 Agent 范围）。

---

## 3. CANN 9.0.0 vs 8.x 差异（与本题相关部分）

| 项 | 8.x | 9.0/9.x（master 文档） | 证据 |
| --- | --- | --- | --- |
| DataCopyPad 文档定位 | 基础搬运接口 | 手册中标为 **DataCopyPad(ISASI)**（跨硬件不保证兼容的指令体系接口） | A（9.0 手册目录） |
| DataCopyPad 支持产品 | A2 训练/800I A2、200/500 A2 | 新增 **Atlas A3**、**Ascend 950（mode 重载、Compact 模式、负 srcStride）**；老训练系列/推理系列仍不支持 | A（8.0 vs 9.x master） |
| DataCopyExtParams.blockCount | [1,4095] | A2/A3/200-500A2: [0,4095]；950: [0,65535]；**blockCount/blockLen=0 → NOP** | A |
| DataCopyPad 补 0 语义 | 8.0：isPad=false 时默认随机值 | 9.x 增加 **SetPadValue 寄存器接口**（isPad=false 时外部配置填充值） | A |
| Cast RoundMode | 8.x 常见 RINT/FLOOR/CEIL/ROUND/TRUNC/NONE | 新增 **CAST_ODD、CAST_HYBRID**（仅 hifloat8 平台） | A（9.x master Cast） |
| Cast 产品 | 训系列组合极少 | A2/A3/950 组合大幅扩展（含 bfloat16_t↔float） | A |
| ReduceSum | 训: half；A2: half/float；无 950/A3/Kirin | 新增 950（half/float/uint64/int64）、A3、Kirin；A2 count 版仍方式二 | A/B |
| 归约性能指引 | 无专门文档 | 「选择低延迟指令，优化归约操作性能」：ReduceRepeat 延迟约为 Add 的 2-5 倍；**数据量大时二分累加(Add+ReduceRepeat) > ReduceRepeat > ReduceSum**；ReduceSum 为软仿实现 | A（9.x guide） |
| 版本公告 | — | CANN 9.0.1（2026-07-02）：`aclnnMatmulAllReduceAddRmsNorm` 等废弃，拆分出 **`aclnnAddRmsNorm`**；CANN 9.1.0（2026-07-31）：商用/社区归一化 | A（product bulletin） |
| 每核 UB | 未统一记载 | **Atlas A2/A3：每核 UB = 192KB**（16 bank group × 3 bank × 4KB） | A（Vector 逻辑架构页） |

> 说明：9.0 社区版 API 页面（`CANNCommunityEdition/900/API/ascendcopapi/...`）目录已可访问；具体 API 页内容（DataCopyPad 等）在 900 路径下抓取失败（URL 结构 404/空），故 9.0 本体差异用 9.x master（asc.gitcode.com，构建于 2026-09）与 8.x 对比，并在「未确认清单」中标注。

---

## 4. 尾块与归约 API 组合建议（基于文档）

面向 A2 系（800I A2 推理 / Atlas A2 训练，每核 UB 192KB）：

1. **尾块搬入**：维持 DataCopyPad（GM→UB）+ rightPadding 补 0（isPad=true, paddingValue=0），使 UB 内整行 32B 对齐；文档明确 GM 侧 1B 对齐即可，无需担心行首地址对齐。
2. **归约**：维持 count 版 `ReduceSum(float, work)`，work 空间按公式 `RoundUp(count/64, 8)*8`（float 下 elementsPerBlock=8）取整即可；首版 WORK_LEN=1024 富余。注意 A2 count 版为「方式二」顺序累加；若精度不达标可评估 WholeReduceSum/BlockReduceSum（PIPE_V 硬件指令，性能更优）或二分累加方案。
3. **标量读回**：ReduceSum 结果在 VEC 流水，用 `GetValue(0)` 读回需 V_S/S_V 成对同步；事件 ID 必须 FetchEventID，禁止手写。
4. **归一化**：`rms=sqrtf(sum/dim+eps)`、`scale=1/rms` 在标量流水算（首版做法）；若想用矢量 Sqrt/Rsqrt 需先把标量 broadcast 成 tensor（Duplicate），本题每行一次标量更省。
5. **输出**：float 直接 DataCopy(UB→UB)+DataCopyPad(UB→GM)；half/bf16 先 Cast(CAST_RINT)→目标类型再搬出；bf16 输出必须用 RINT 等（无 CAST_NONE）。
6. **无 DataCopyPad 的降级路线**（仅当判题平台为老训练/推理系列时）：搬入用 DataCopy 整块 + mask 归约/逐行 Duplicate 清 0；搬出>32B 用 UnPad/GatherMask；搬出<32B 用目标清零 + SetAtomicAdd 原子累加。**A2 系无需此路线**。
7. **UB 预算**（首版）：5 队列 × 8KB（half tile 4096×2B）+ 3 个 float tile × 8KB + work 4KB + sum 64B ≈ 68KB < 192KB；若开双缓冲（×2 队列）≈ 108KB，仍有余量，可评估。

---

## 5. 明确未确认清单

1. **判题平台 SoC/产品系列**：文档无法确认 CANNJudge 判题硬件是 Atlas A2（800I A2 推理）还是老训练/推理系列——这决定 DataCopyPad 可用性，是最高优先级待确认项（不在本 Agent 范围，需 Agent 1/主办方信息）。
2. **9.0.0 本体 API 页内容**：`CANNCommunityEdition/900/API/ascendcopapi/` 下的 DataCopyPad/ReduceSum/Cast 页面 URL 结构抓取失败（404/空），9.0 差异基于 9.x master（2026-09 构建）推断，安装版头文件为准。
3. **`<<<blocks, nullptr, stream>>>`**：官方文档第二参数为 dynUBufSize（默认 0），判题编译器是否接受 nullptr 未核验。
4. **FetchEventID 是否耗用事件槽**：A2 事件槽 0-7，首版循环内反复 Fetch，若耗用则可能溢出——8.0 文档未明示，需真机/头文件确认。
5. **ReduceSum A2「方式二」实际数值误差**：顺序累加与二叉树累加误差差异无官方数据，需真机精度验证。
6. **SetAtomicAdd / UnPad 的型号矩阵与签名**：仅降级文档提及（A 级但为指南非 API 手册），未逐页核验。
7. **910B（Atlas 训练系列）相关能力**：DataCopyPad 不支持（A 级）、ReduceSum 仅 half（A 级）、Cast 组合极少（A 级）——若判题平台确为老训练系列，现有首版（float 归约 + DataCopyPad）不可直接编译，需降级改造。
8. **UB 容量官方数据**：A2/A3=192KB 为 A 级；910B 每核 UB（社区数据 256KB/1.5MB/2MB 互相矛盾，C 级）无法从官方文档确认。
9. **9.0.0 相对 8.x 的 DataCopyPad/ReduceSum/Cast 专项变更公告**：未找到逐 API 的官方 changelog；9.0.1/9.1.0 公告只涉及 aclnn 层算子接口（AddRmsNorm 相关），与 Kernel API 无关。

---

## 6. 本轮新增来源清单

| # | 名称 | URL | 版本/日期 | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | CANN 9.0.X 社区版文档首页 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/index/index.html | 9.0.X，更新 2026/07/30 | 2026-09-11 | 9.0 文档入口确认 | A |
| 2 | CANN 9.0.X Ascend C API 手册（目录，含 DataCopyPad(ISASI)/Cast/ReduceSum 章节） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0003.html | 9.0.X | 2026-09-11 | API 结构与 ISASI 标注 | A |
| 3 | DataCopyPad（社区版 8.0.0 alpha001 手册） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0253.html | 8.0.0 alpha001 | 2026-09-11 | DataCopyExtParams/DataCopyPadExtParams 字段、对齐、型号矩阵 | A |
| 4 | DataCopyPad（GM→UB 非对齐搬运，9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopyPad_GMToUB.html | master，源文档 2026-09-09 | 2026-09-11 | 9.x 字段/产品/dtype/mode/NOP | A |
| 5 | DataCopyPad（UB→GM 非对齐搬运，9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopyPad_UBToGM.html | master | 2026-09-11 | 搬出语义与约束 | A |
| 6 | ReduceSum（商用 8.0 手册） | https://www.hiascend.cn/document/detail/zh/canncommercial/800/apiref/ascendcopapi/atlasascendc_api_07_0078.html | 商用 8.0 | 2026-09-11 | workLocal 公式、mask/count 版、累加方式、dtype 矩阵 | A |
| 7 | ReduceSum（社区 8.0.RC3.alpha001 手册） | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/apiref/opdevgapi/atlasascendc_api_07_0099.html | 8.0.RC3.alpha001 | 2026-09-11 | 交叉确认（同 6） | A |
| 8 | ReduceSum（asc-devkit 仓库文档转述） | https://blog.csdn.net/gitblog_00089/article/details/151635178 | asc-devkit 仓库 | 2026-09-11 | 9.x 产品矩阵（新增 950/A3/Kirin） | B |
| 9 | Cast（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/type_conversion/Cast.html | master | 2026-09-11 | RoundMode 枚举与各产品 dtype 组合 | A |
| 10 | DataCopy（GM↔UB 连续搬运，9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopy_GMAndUB_continuous.html | master | 2026-09-11 | count 对齐约束、产品 | A |
| 11 | Add（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/basic_arithmetic/Add.html | master | 2026-09-11 | 签名、A2 dtype（无 bf16） | A |
| 12 | TPipe::InitBuffer（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/resource_management/TPipe/InitBuffer.html | master | 2026-09-11 | InitBuffer 语义、≤64 buffer、双缓冲 | A |
| 13 | TPipe-TQue 编程原理（9.x guide） | https://asc.gitcode.com/guide/programming_guide/programming_model/ai_core_simd_programming/tpipe_tque_programming/tpipe_tque_principles.html | master | 2026-09-11 | EnQue/DeQue/AllocTensor/FreeTensor 同步语义 | A |
| 14 | 核函数（Kernel）定义与调用（9.x guide） | https://asc.gitcode.com/guide/programming_guide/programming_model/ai_core_simd_programming/kernel_function.html | master | 2026-09-11 | __global__ __vector__、<<<numBlocks,dynUBufSize,stream>>>、block_idx/block_num | A |
| 15 | 核函数（HarmonyOS CANN Kit 镜像文档） | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-kernel-function | 更新 2026-08-18 | 2026-09-11 | 核函数定义规则交叉确认 | A |
| 16 | GetBlockIdx / GetBlockNum（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/tool_interface/system_resources_and_variables/GetBlockIdx.html | master | 2026-09-11 | 签名与语义 | A |
| 17 | PipeBarrier(ISASI)（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.html | master | 2026-09-11 | 签名、不支持 PIPE_S、自动同步说明 | A |
| 18 | SetFlag/WaitFlag（社区版 8.0.0 alpha001） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0257.html | 8.0.0 alpha001 | 2026-09-11 | HardEvent 枚举、eventID 范围（A2 0-7） | A |
| 19 | 无 DataCopyPad 的处理方式 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha002/devguide/opdevg/ascendcopdevg/atlas_ascendc_10_0037.html | 8.0.RC3.alpha002 | 2026-09-11 | 降级路线（Duplicate/mask/GatherMask/UnPad/SetAtomicAdd） | A |
| 20 | Vector 逻辑架构（UB 大小，9.x） | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/Vector逻辑架构/Vector逻辑架构.html | master | 2026-09-11 | A2/A3 每核 UB=192KB | A |
| 21 | 选择低延迟指令，优化归约操作性能（9.x guide） | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | master | 2026-09-11 | ReduceSum 性能定位与二分累加方案 | A |
| 22 | API 流水类型汇总（9.x） | https://asc.gitcode.com/api/appendix/api_pipeline_type_summary.html | master | 2026-09-11 | ReduceSum/Cast/DataCopyPad 流水类型 | A |
| 23 | 昇腾 CANN 9.0.1 版本发布公告 | https://www.hiascend.com/productbulletins/detail/804 | 9.0.1，2026-07-02 | 2026-09-11 | aclnnAddRmsNorm 拆分、废弃接口 | A |
| 24 | 昇腾 CANN 9.1.0 版本发布公告 | https://www.hiascend.com/productbulletins/detail/806 | 9.1.0，2026-07-31 | 2026-09-11 | 商用/社区归一化 | A |
| 25 | CANN 9.1.0 社区版下载页（910b-ops 对应 Atlas A2 说明） | https://www.hiascend.com/en/software/cann/community/ | 9.1.0 | 2026-09-11 | 910b-ops 包与产品对应关系 | A |
| 26 | CANN 8.3.RC1.alpha001 Ascend C 算子开发指南 PDF（3.2/6.6 Kernel 直调） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/83RC1alpha001/opdevg/Ascendcopdevg/CANN社区版%208.3.RC1.alpha001%20Ascend%20C算子开发指南%2001.pdf | 8.3.RC1.alpha001，2025-11-12 | 2026-09-11 | Kernel 直调工程/算子开发章节 | A |
| 27 | CANN 9.0.0 商用版文档下载页（Ascend C 算子开发接口参考 PDF） | https://www.hiascend.com/document/detail/zh/canncommercial/download | 9.0.0 | 2026-09-11 | 9.0 手册 PDF 入口（未下载全文） | A |
| 28 | asc-devkit DataCopy 内存访问最佳实践样例 | https://blog.csdn.net/gitblog_00924/article/details/157925888 | asc-devkit 仓库 | 2026-09-11 | 搬运性能/非对齐场景佐证 | B |
| 29 | CANNBot Ascend C 直调开发指南 | https://blog.csdn.net/gitblog_00909/article/details/151507556 | CANN 官方 skills 仓 | 2026-09-11 | 直调工程代码顺序/核数获取规范 | C |
| 30 | SuperKernel 核函数直调算子额外适配说明 | https://asc.gitcode.com/guide/编程指南/高级编程/SuperKernel/核函数直调算子额外适配说明.html | master | 2026-09-11 | 普通 Kernel 直调 = __global__ __vector__ 佐证 | A |
| 31 | 昇腾 910 系列 UB 容量社区数据 | https://arxiv.org/pdf/2607.20120 ; https://github.com/mov20/ascend-dsl-research/blob/main/pyasc2-design.md | 2026 | 2026-09-11 | 910B 每核 UB（互相矛盾，仅记录） | C |
| 32 | 内置数据类型（bfloat16_t 定义，9.x） | https://asc.gitcode.com/api/SIMT-API/SIMD与SIMT混合编程简介/扩展语法/内置数据类型-144.html | master | 2026-09-11 | bfloat16_t 位宽定义 | A |
| 33 | asc_atomic_add（SIMT，9.x） | https://asc.gitcode.com/api/SIMT-API/原子操作/asc_atomic_add.html | master | 2026-09-11 | SIMT 原子加仅 950 支持（区别于 MTE SetAtomicAdd） | A |
| 34 | 昇腾社区官方博客：HIVM 方言内存优化（910B UB 1.5MB 说法） | https://www.hiascend.com/developer/blog/details/02134205828844777045 | 2026-03 | 2026-09-11 | 910B UB 容量社区说法（C 级矛盾记录） | C |

**交叉核对说明**：本报告与 `sources.md` 已有条目（#4/#5/#6/#7 为 DataCopyPad/ReduceSum/无 DataCopyPad 页面）一致；新增条目以 9.0 与 9.x master 为主。所有「未在真实 NPU 验证」的结论均不得改写为已通过编译/精度/性能验证。
