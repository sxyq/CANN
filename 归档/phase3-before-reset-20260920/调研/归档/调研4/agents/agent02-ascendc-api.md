# Agent 02 调研报告：官方 Ascend C API 语义与签名

- **主题**：CANN 9.0.0 下 Ascend C 官方 API 的函数签名、参数含义、产品支持矩阵、文档版本，及其对本题 `AddRmsNormBias`（V002 `kernel.asc`）实现的约束。
- **调研对象**：`提交/V002/kernel.asc`（直调工程，`__vector__` 入口）、`源码/op_kernel/add_rms_norm_bias.cpp`（msopgen 框架工程，`__aicore__` 入口）。
- **证据等级约定**：A = 官方文档原文；B = 官方示例代码/头文件；C = 社区/CSDN 博客（仅交叉参考，不作为唯一依据）；D = 本人基于官方规则的推断。
- **访问日期**：2026-09-11。
- **主要引用版本**：CANN 9.0.0 社区版（缓存页 `CANNCommunityEdition/900`）。个别页面（入口限定符）为 CANN 8.5.0alpha002，已在来源表显式标注版本差异风险。

---

## 1. 结论摘要（≤12 条）

1. `DataCopyPadExtParams<T>` 官方字段声明顺序为 **`{isPad, leftPadding, rightPadding, paddingValue}`**（CANN 9.0.0 文档表，A 级），已定案。
2. UB→GM 搬出时，超出有效长度的 32 字节对齐“假数据（dummy）”在写入 GM 时**被框架自动丢弃**，不会污染相邻内存（CANN 9.0.0 文档原文，A 级），已定案。
3. `GlobalTensor` 起始地址**无地址对齐约束**（官方原文，A 级），因此尾块非对齐搬出到 GM 合法。
4. `ReduceSum` 基础 count 版约束为“最大处理数据量不超过 UB 大小限制”；`count` 上限受数据类型与 UB 容量决定，官方未给单一数字。社区流传的 `repeat<255`、`≈16320 fp32`、`≈4096` 是口径不同的估算（C/D 级），非官方单一结论。
5. Atlas A2（训练/推理系列）上 `Add`/`Mul` 官方支持 **half / float**，**不支持 bfloat16_t**（bf16 仅在 Atlas 350 加速卡列出）。本题 bf16 必须转 FP32 计算，与工程一致。
6. 向量 `Sqrt`/`Rsqrt` 提供精度模式（`PRECISION_1ULP_FTZ_TRUE` ≈1 ULP；`FAST_INVERSE`/`PRECISION_0ULP_FTZ_FALSE` 快速求逆）。本题**不使用向量 Sqrt**，而是对标量 `square_sum` 用 `sqrtf`，规避向量精度问题，设计正确。
7. 纯 Vector 算子入口用 `__vector__` 与 `__aicore__` **均合法**：`__vector__` 官方定义为“仅在 Vector 核执行”（直调模式纯 Vector 核推荐），`__aicore__` 为通用 AI Core 入口（框架模式）。两者口径一致，已定案。
8. 本题 `kernel.asc` 用 `__global__ __vector__` + `<<<blocks,nullptr,stream>>>` 属**直调模式**合法写法；`源码/*.cpp` 用 `__global__ __aicore__` + `REGISTER_TILING` 属**算子框架模式**写法，两者针对不同提交形态，互不冲突。
9. `GetReduceSumMaxMinTmpSize` 是**高阶 `Pattern::Reduce` 版 ReduceSum 的 host 端 tiling API**，本题采用的“基础 count 版 `ReduceSum`”**不使用**该接口，且无需其 `workLocal` 之外的额外 tiling 空间。
10. 高阶 ReduceSum（`Pattern::Reduce`，AR/RA）与基础 `ReduceSum(count)` 是两条独立 API 路径，本题走基础版。
11. 工程全部中间计算用 FP32（符合官方“归约必须 FP32”经验法则），且 `Cast` 用 `CAST_NONE`（fp16/bf16→fp32 无损）与 `CAST_RINT`（fp32→fp16/bf16 就近舍入），枚举名以 `CAST_ROUND`（非 `CAST_RND`）出现在 9.0.0 文档。
12. 风险点（待真机验证）：fp16/bf16 路径下 `ReduceSum` 的 `workLocal` 缓冲 `WORK_LEN=1024`（float）可能小于 `calc_len`（fp32 计数）峰值 4096，需真机确认 `work` 缓冲尺寸下限；其余 API 调用与官方签名一致。

---

## 2. 六个争议点逐条裁决

### 争议点 1：`DataCopyPadExtParams<T>` 字段声明顺序
- **裁决**：官方顺序为 **`{isPad, leftPadding, rightPadding, paddingValue}`**。
- **依据**：CANN 9.0.0 文档 `DataCopyPad` 页（缓存 `atlasascendc_api_07_0265.html`，URL 路径 `CANNCommunityEdition/900/.../atlasascendc_api_07_0265.html`）中“DataCopyPadExtParams 结构体参数定义”表，行序依次为：`isPad` → `leftPadding` → `rightPadding` → `paddingValue`（同一表在 `DataCopyPadParams` 段再次出现，顺序一致）。
- **证据等级**：A（官方文档表原文）。**已定案**。
- **对工程的影响**：`kernel.asc` 第 67–71 行采用**逐字段赋值**（`pad.isPad=...; pad.paddingValue=...; pad.leftPadding=...; pad.rightPadding=...`），不依赖聚合初始化顺序，天然免疫该歧义；`源码/add_rms_norm_bias.cpp` 第 115 行用聚合初始化 `{true,0,0,0}`，在两种顺序下均解析为 `isPad=true, paddingValue=0, leftPadding=0, rightPadding=0`（全零），亦不会出错。
- **真机验证方法**：若需绝对确认，可查 CANN 安装目录下 `include/ascendc/kernel_direct/tensor_api.h` 或 `kernel_operator_data_copy.h` 中 `template<typename T> struct DataCopyPadExtParams` 的字段声明顺序；或在 NPU 上用非全零 leftPadding/rightPadding 跑一次并比对结果。

### 争议点 2：UB→GM 搬出超出行有效长度时 padding 是否真实写入相邻内存
- **裁决**：**不会**。框架在 UB 侧为保证 32 字节对齐自动补 dummy，但**搬到 GM 时自动丢弃填充的假数据**，不写入相邻的下一行/下一元素内存。
- **依据**：CANN 9.0.0 `DataCopy`/`DataCopyPad` 文档原文（缓存 0265）：
  > “当 blockLen 不满足 32 字节对齐，由于 Unified Buffer 要求 32 字节对齐，框架在搬出时会自动补充一些假数据来保证对齐，但在当搬到 GM 时会自动将填充的假数据丢弃掉。”
  > 另有：“blockLen 为 47……框架在搬出时会自动补充 17 字节的假数据来保证对齐，搬到 GM 时再自动将填充的假数据丢弃掉。”
  同页明确：“**GlobalTensor 的起始地址无地址对齐约束**。”
- **证据等级**：A（官方文档原文）。**已定案**。
- **社区相反实测**：未检索到可靠的“污染相邻内存”实测；社区普遍接受“GM 侧 dummy 被丢弃”的官方表述。若有疑虑，属 C 级传言，不影响本裁决。
- **真机验证方法**：构造一行后紧邻有效数据的 GM 缓冲，用非 32B 对齐尾块 `DataCopyPad` 搬出，再读相邻内存是否被改写。

### 争议点 3：`ReduceSum` 的 count 上限
- **裁决**：官方约束是**“参数取值范围与数据类型有关，能够处理的元素个数最大值不同，最大处理的数据量不能超过 UB 大小限制”**（CANN 9.0.0 `ReduceSum` 文档，缓存 0078）。官方**未给出单一固定数字**。社区流传的口径需区分：
  - `repeat < 255`：指一条 reduce 指令（`repeatTimes`）上限 255；每个 repeat 在 fp32 下处理 64 元素 → 255×64 = **16320 fp32 元素**为该口径上限（≈16320 fp32）。
  - `≈4096`：多为**实测安全值/保守工程值**（受 UB 内 `workLocal` 缓冲、mask 设置 `isSetMask` 等综合限制），属 D/C 级经验，不是文档硬上限。
- **依据**：官方文档 `atlasascendc_api_07_0078.html`（CANN 9.0.0）正文“参数取值范围和操作数的数据类型有关……最大处理的数据量不能超过 UB 大小限制。”高阶 `GetReduceSumMaxMinTmpSize` 文档（CANN 9.0.0，已下载 `agent02_GetReduceSumMaxMinTmpSize_900.html`）同样只给 `minValue/maxValue` 由 shape 决定，无全局常量。
- **证据等级**：A（官方约束原文）+ C/D（255/16320/4096 为社区/推断口径）。**已定案（官方口径）**，社区数字不作为唯一依据。
- **对工程影响**：本题把每行切成 `tile_len_` 块（fp32=2048，fp16/bf16=4096 元素），单块 `count` 不超过 2048（fp32）或 4096（fp16/bf16 经 Cast 后以 fp32 计数的 `calc_len`）。fp32 路径 count≤2048 安全；**fp16/bf16 路径 `calc_len` 可达 4096 fp32 元素**，贴近社区保守上限，建议在真机验证（见风险点 12 / 第 5 节）。
- **真机验证方法**：在 Atlas A2 上用 fp16 输入、D=4096、单块 `ReduceSum(count=4096, fp32)` 实测是否溢出/结果正确；并核对 `workLocal` 缓冲需求。

### 争议点 4：Atlas A2 上 `Add`/`Mul` 是否支持 `bfloat16_t`
- **裁决**：**不支持**。Atlas A2 训练/推理系列产品上 `Add`/`Mul`/`Muls` 官方支持的数据类型为 **half / float**（Mul/Muls 另含 int16/int32），**bfloat16_t 仅在 Atlas 350 加速卡**列出。因此 bf16 输入**必须转 FP32 后再做 Add/Mul**。
- **依据**：CANN 9.0.0 `Add`/`Mul`/`Muls` 文档（缓存 `atlasascendc_api_07_0055.html`，`CANNCommunityEdition/900`）中 `T` 参数说明：“操作数数据类型。Atlas 训练系列产品，支持的数据类型为：half/float”；全页仅“Atlas 350 加速卡”行出现 `bfloat16_t`。
- **证据等级**：A（官方文档原文）。**已定案**。
- **对工程影响**：`kernel.asc` 第 107–115、137–146 行的 `MakeValue`/`ApplyAffine` 对 `bfloat16_t` 先做 `Cast(..., CAST_NONE)` 到 FP32 再 `Add`/`Mul`，与官方限制完全一致；fp32 路径直接 `Add`/`Mul` 也合法。**无需**引入 bf16 原生向量运算。

### 争议点 5：向量 `Sqrt` 与标量 `sqrtf` 的精度差异；`Rsqrt` 近似误差
- **裁决**：
  - 向量 `Sqrt`/`Rsqrt` 提供精度模式（CANN 9.0.0 文档，缓存 0029/0030）：`*Algo::PRECISION_1ULP_FTZ_TRUE` —— 单指令得出，**最大精度误差 1 ULP**；`*Algo::FAST_INVERSE` / `PRECISION_0ULP_FTZ_FALSE` —— 快速求逆算法，输入在 `[0, 8.507e37]` 内**最大误差 0 ULP**，仅支持 float 且支持 Subnormal。社区常称 rsqrt 快速实现相对误差约 `2^-20` 量级，属 C 级经验估计，与官方“0 ULP（限定区间）”口径不同（区间外/不同算法下误差会变大）。
  - 本题**根本不使用向量 `Sqrt`/`Rsqrt`**：第 54 行对**标量** `square_sum`（由各行分块 `ReduceSum` 结果标量累加得到）调用 C 标准库 `sqrtf`，即普通单精度平方根，精度由 libm 保证，不存在向量指令的 1ULP/快速求逆近似问题。
- **依据**：CANN 9.0.0 `Sqrt` 页（缓存 0029）、`Rsqrt` 页（缓存 0030）关于 `SqrtConfig`/`RsqrtConfig` 与精度模式的原文；以及工程 `kernel.asc` 第 53–55、170–182 行。
- **证据等级**：A（官方精度模式原文）+ C（2^-20 社区估计）。**已定案**。
- **结论**：工程通过“标量求和 + 标量 `sqrtf` + 全程 FP32”规避了向量 Sqrt/Rsqrt 的精度争议，是更稳妥的写法。

### 争议点 6：入口限定符 `__aicore__` 与 `__vector__`
- **裁决**：**纯 Vector 算子两种写法都合法**，但语义不同、适用场景不同：
  - `__aicore__`：官方定义为“标识该函数在设备端 AI Core 上执行”（不区分 Cube/Vector，通常用于耦合模式 / 算子框架模式）。
  - `__vector__`：官方定义为“标识该核函数**仅在 Vector 核执行**；针对耦合模式的硬件架构，该修饰符不生效”。
  - 直调模式下纯 Vector 核推荐 `__vector__`；框架模式（msopgen）下用 `__aicore__` + tiling 宏。
- **依据**：官方《C 语言拓展》函数修饰符表（下载 `agent02_vector_qualifier_850a002.html`，CANN 8.5.0alpha002；概念在 9.0.0 稳定）原文列出 `__global__`/`__aicore__`/`__inline__`/`__cube__`/`__vector__`/`__mix__(cube,vec)`，并明确“`__vector__`：标识该核函数仅在 Vector 核执行。针对耦合模式的硬件架构，该修饰符不生效。”社区 `ai6s.net` 亦给出“直调纯 Vector → `__vector__`”映射（C 级，但与官方口径一致）。
- **证据等级**：A（官方修饰符表原文，确认 `__vector__` 合法）。**已定案**。A 级文档与社区说法一致。
- **版本差异风险**：该修饰符表页为 CANN 8.5.0alpha002，非 9.0.0；但 `__vector__`/`__aicore__` 修饰符在 9.0.0 直调工程文档中仍沿用，且 `kernel.asc` 已按 CANNJudge 直调要求使用 `__vector__`，合理。
- **对工程影响**：`kernel.asc` 第 381 行 `__global__ __vector__` 合法；`源码/*.cpp` 第 273 行 `__global__ __aicore__` + `REGISTER_TILING_DEFAULT` 合法（框架模式）。两者面向不同提交形态，无矛盾。注意：CANNJudge 直调工程的入口名约定为 `run_kernel`（第 394 行 `extern "C" void run_kernel(...)`），与框架模式入口不同，切换形态时需对应调整。

---

## 3. API 事实表

> 除特别注明外，文档版本均为 **CANN 9.0.0 社区版（CANNCommunityEdition/900）**。

| API | 函数原型 / 签名 | 语义 | 产品支持（本题相关） | 文档版本 | 对本题约束 | 证据 |
|---|---|---|---|---|---|---|
| `GlobalTensor<T>` | `void SetGlobalBuffer(__gm__ T* addr, uint64_t num)` | 全局内存张量视图 | 起始地址**无对齐约束** | 9.0.0 (0265) | 尾块非对齐 GM 地址合法 | A |
| `LocalTensor<T>` | `T GetValue(const uint32_t offset) const;` `LocalTensor<T> Get<T>()` | 片上 UB 张量；取标量 / 取子张量 | 需 32B 对齐（由调用方保证） | 9.0.0 (0006) | `sum.GetValue(0)` 取归约结果 | A |
| `TPipe` | `void InitBuffer(TQue/&TBuf, uint32_t, uint64_t size)` | 片上内存（队列/临时变量）分配管理 | 全产品 | 9.0.0 (0108/0136) | 第 32–41 行初始化各队列与 TBuf | A |
| `TQue<QuePosition, NUM>` | `AllocTensor/DeQue/EnQue/FreeTensor` | VECIN/VECOUT 队列（流水通信） | VECIN、VECOUT 逻辑位置 | 9.0.0 (0108) | 第 260–264、32–36 行 | A |
| `TBuf<TPosition::VECCALC>` | `LocalTensor<T> Get<T>()` | VECCALC 临时变量缓冲 | VECCALC 逻辑位置 | 9.0.0 (0108) | 第 265–269、37–41 行 | A |
| `DataCopy` (GM↔Local, 基础) | `template<typename T> __aicore__ void DataCopy(const LocalTensor<T>& dst, const GlobalTensor<T>& src, uint64_t count)` 等多重载 | 整块对齐数据搬运 | GM 无对齐约束；Local 需 32B 对齐 | 9.0.0 (0265) | 第 118、153、244 行（对齐块） | A |
| `DataCopyExtParams` | `{uint16_t blockCount; uint32_t blockLen; uint16_t srcStride; uint16_t dstStride; uint16_t rsv;}` | DataCopy/DataCopyPad 参数包 | blockCount∈[1,4095]; blockLen∈[1,2097151]（GM src 更大） | 9.0.0 (0265) | 第 66、79 行 `{1, len*sizeof(T),0,0,0}` | A |
| `DataCopyPad` (GM→Local) | `void DataCopyPad(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const DataCopyExtParams&, const DataCopyPadExtParams<T>&)` | 搬入并补 padding 至 32B 对齐 | padding 值随类型；64 位类型 paddingValue 只能为 0 | 9.0.0 (0265) | 第 72 行（搬入，尾块补 0） | A |
| `DataCopyPad` (Local→GM) | `void DataCopyPad(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const DataCopyExtParams&)`（无 pad 参数重载） | 搬出；UB 侧补 dummy，GM 侧自动丢弃 | GM 无对齐约束；dummy 不写相邻内存 | 9.0.0 (0265) | 第 80 行（搬出尾块） | A |
| `DataCopyPadExtParams<T>` | `struct { bool isPad; uint8_t leftPadding; uint8_t rightPadding; T paddingValue; }` | 补 padding 参数 | leftPadding/rightPadding 各 ≤32B | 9.0.0 (0265) | 第 67–71 行逐字段赋值 | A |
| `ReduceSum` (基础 count 版) | `template<typename T,bool isSetMask=true> __aicore__ void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& workLocal, const int32_t count)` | 对 src 前 `count` 个元素求和，结果写 dst[0] | 最大数据量 ≤ UB；Atlas A2 支持 | 9.0.0 (0078) | 第 199 行 `ReduceSum(sum, value, work, count)` | A |
| `WholeReduceSum` (高阶/跨核) | `void WholeReduceSum(...)` | 多核全局归约 | Atlas A2 | 9.0.0 (0078) | 本题未使用 | A |
| `GetReduceSumMaxMinTmpSize` (高阶 Pattern::Reduce 版) | `void GetReduceSumMaxMinTmpSize(const ge::Shape& srcShape, const ge::DataType dataType, ReducePattern pattern, bool isSrcInnerPad, bool isReuseSource, uint32_t& maxValue, uint32_t& minValue)` | host 端求高阶 ReduceSum 的临时空间上下限 | `ReducePattern::{AR=0, RA=1}`（目前仅 AR/RA） | 9.0.0 (下载 900 页) | 本题用基础版，**不使用** | A |
| 高阶 ReduceSum `Pattern::Reduce` | `ReduceSum(dst, src, workLocal, count, ...)` 带 `ReducePattern` | 多维轴归约 | Atlas A2 | 9.0.0 (0078/下载页) | 本题走基础版，未使用 | A |
| `Cast` | `template<typename T,typename U> void Cast(const LocalTensor<T>& dst, const LocalTensor<U>& src, const RoundMode& mode, const int32_t count)` | 数据类型转换 | 见 RoundMode 表 | 9.0.0 (0073) | 第 102–103 等 `CAST_NONE`；160/167 `CAST_RINT` | A |
| `RoundMode` 枚举 | `CAST_NONE=0; CAST_RINT; CAST_ROUND(文档名, 历史曾称 CAST_RND); CAST_FLOOR; CAST_CEIL; CAST_TRUNC; CAST_ODD` | 舍入模式 | 全产品 | 9.0.0 (0073) | 用 `CAST_NONE`(无损) 与 `CAST_RINT`(就近舍入) | A |
| `Add` | `void Add(const LocalTensor<T>& dst, const LocalTensor<T>& src1, const LocalTensor<T>& src2, const int32_t count)` | 逐元素加法 dst=src1+src2 | Atlas A2：**half/float**（不含 bf16） | 9.0.0 (0055) | 第 94、104…；bf16 先 Cast 到 fp32 | A |
| `Mul` | 同 Add 形态 | 逐元素乘法 | Atlas A2：**half/int16/float/int32**（不含 bf16） | 9.0.0 (0055) | 第 122、198… | A |
| `Muls` | `void Muls(dst, src, const T scalar, count)` | 逐元素乘标量 | 同 Mul | 9.0.0 (0055) | 第 241 行 `Muls(value, value, scale, calc_len)` | A |
| `Sqrt` (向量) | `void Sqrt(dst, src, count, SqrtConfig)` | 向量平方根 | half/float（见精度模式） | 9.0.0 (0029) | 本题未用向量 Sqrt | A |
| `Rsqrt` (向量) | `void Rsqrt(dst, src, count, RsqrtConfig)` | 向量快速平方根倒数 | half/float（见精度模式） | 9.0.0 (0030) | 本题未用 | A |
| `Duplicate` | `void Duplicate(dst, const T scalar, count)` | 用标量填充整张量 | 全产品 | 未在 9.0.0 缓存页命中 | 本题未使用；如需可在 UB 清零用 | C/D |
| `GatherMask` | 带 mask 的 gather | 按 mask 收集 | 全产品 | 9.0.0 (0073 提及) | 本题未使用 | C |
| `GetValue` | `PrimType GetValue(const uint32_t offset) const;` | 从 LocalTensor 取标量 | 全产品 | 9.0.0 (0006) | 第 205 行 `sum.GetValue(0)` | A |
| `SetFlag` / `WaitFlag` | `template<HardEvent E> void SetFlag(event_t); void WaitFlag(event_t)` | 事件同步（跨流水） | 全产品 | 9.0.0 (0270) | 第 203–204、206–207 行 V_S / S_V | A |
| `PipeBarrier` | `template<Pipe> void PipeBarrier()` | 流水屏障 | 全产品 | 9.0.0 (0270) | 第 201、209 行 `PipeBarrier<PIPE_V>()` | A |
| `HardEvent` / `FetchEventID` | `GetTPipePtr()->FetchEventID(HardEvent::V_S)` | 取事件 ID | 全产品 | 9.0.0 (0270/0116) | 第 202、206 行 | A |
| `GetBlockIdx` / `GetBlockNum` | `__aicore__ int64_t GetBlockIdx(); int64_t GetBlockNum();` | 多核 SPMD 索引 | 全产品 | 9.0.0 (0184/0185) | 第 46–47 行 | A |
| 入口限定符 | `__global__ __aicore__` / `__global__ __vector__` | 核函数设备端入口 | `__vector__`：仅 Vector 核；`__aicore__`：AI Core | `__vector__` 见 8.5.0a002（概念沿用 9.0.0） | 第 381 行 `__vector__`；源码第 273 行 `__aicore__` | A |

---

## 4. 与 `提交/V002/kernel.asc` 逐 API 对应检查表

> 行号基于 `提交/V002/kernel.asc`。

| 调用位置（行） | API | 是否与官方签名一致 | 风险 / 备注 |
|---|---|---|---|
| 26–30 | `GlobalTensor::SetGlobalBuffer` | 一致 | 无。`SetGlobalBuffer(reinterpret_cast<__gm__ T*>, num)` 标准用法。 |
| 32–41 | `TPipe::InitBuffer` | 一致 | 无。顺序初始化 5 个 TQue + 5 个 TBuf。 |
| 46–47 | `GetBlockIdx`/`GetBlockNum` | 一致 | 无。SPMD 行切分标准写法。 |
| 60–73 (`CopyIn`) | `DataCopyExtParams{1,len*sizeof(T),0,0,0}` + `DataCopyPadExtParams` 逐字段 | 一致 | 无。字段逐赋，免疫顺序歧义（争议点 1）。尾块 `isPad=true, rightPadding=aligned_len-len, paddingValue=0`，符合文档“padding 填 0”。 |
| 79–81 (`CopyOut`) | `DataCopyExtParams{1,len*sizeof(T),0,0,0}` + `DataCopyPad(GM, Local, params)`（无 pad 参数重载） | 一致 | 低。GM 目的地址无对齐约束（争议点 2/3 结论）；UB→GM 非对齐 dummy 自动丢弃，不污染相邻内存。需确认 `DataCopyPad` 仅 3 参重载在 9.0.0 直调头文件中存在（官方有两参/三参/四参重载，A 级）。 |
| 102–103, 112–113, 131–132, 142–143 | `Cast(dst, src, RoundMode::CAST_NONE, len)` | 一致 | 无。`CAST_NONE=0` 为 fp16/bf16→fp32 无损转换，正确。 |
| 94,104,114,123,133,144–145,204–205,211–216 | `Add(dst,src1,src2,count)` | 一致 | 无。fp32 路径直接 Add；bf16 先 Cast 到 fp32 再 Add，规避 Atlas A2 不支持 bf16 Add 的限制（争议点 4）。 |
| 122,133,144–145,198,215 | `Mul(dst,src1,src2,count)` | 一致 | 无。同 Add，bf16 经 Cast 后计算。 |
| 241 | `Muls(dst,src,scalar,count)` | 一致 | 无。`Muls(value, value, 1.0f/rms, calc_len)` 做标量缩放。 |
| 199 | `ReduceSum(sum, value, work, static_cast<int32_t>(calc_len))` | 一致 | 中（见风险 12）。基础 count 版签名匹配；但 `work` 缓冲 `WORK_LEN=1024`(float) 在 fp16/bf16 路径 `calc_len` 可达 4096 fp32 时，需真机确认 `workLocal` 最小尺寸。 |
| 201,209 | `PipeBarrier<PIPE_V>()` | 一致 | 无。 |
| 202,206 | `GetTPipePtr()->FetchEventID(HardEvent::V_S/S_V)` | 一致 | 无。 |
| 203–204,206–207 | `SetFlag<HardEvent::V_S>` / `WaitFlag<HardEvent::V_S>` 等 | 一致 | 无。V_S（向量→标量）、S_V（标量→向量）事件同步标准写法。 |
| 205 | `sum.GetValue(0)` | 一致 | 无。`GetValue(const uint32_t)` 返回标量累加结果。 |
| 153 | `StoreOutput` fp32 分支 `DataCopy(dst, src, len)`（Local→Local） | 一致 | 低。`value_float_`(VECCALC) → `output_local`(VECOUT) 的 Local→Local 拷贝，合法。 |
| 160,167 | `Cast(dst, src, RoundMode::CAST_RINT, len)` | 一致 | 无。fp32→fp16/bf16 用就近舍入 `CAST_RINT`，合理。 |
| 381 | `__global__ __vector__ void add_rms_norm_bias_kernel(...)` | 一致（合法） | 无（争议点 6）。直调模式纯 Vector 核入口，官方合法。 |
| 414 | `kernel<<<blocks, nullptr, stream>>>(...)` | 一致 | 无。直调启动三元组 `blockDim=blocks, l2ctrl=nullptr, stream`，符合文档。 |
| 394 | `extern "C" void run_kernel(...)` | 一致 | 无。CANNJudge 直调工程入口约定名。 |

---

## 5. 对齐与 UB 容量约束汇总

### 5.1 32 字节对齐约束
- **Local（UB）侧**：所有向量指令与 LocalTensor 缓冲**必须 32 字节对齐**。工程的 `AlignedLen(len)` 以 `32/sizeof(T)` 个元素为单位向上取整（fp32→8、fp16/bf16→16），保证 `calc_len` 对应 UB 长度为 32B 整数倍，满足对齐（D 级推断，符合文档“UB 要求 32 字节对齐”）。
- **Global 侧**：`GlobalTensor` 起始地址**无地址对齐约束**（官方原文，A 级）。因此尾块 `output_[base+offset]` 即便不是 32B 对齐也可作为 `DataCopyPad` 的目的地址；搬出时 UB 侧补 dummy、GM 侧自动丢弃（争议点 2）。

### 5.2 UB 容量（Atlas A2 约 1 MB）
每个 block 同时常驻：
- 5 个 TQue（BUFFER_NUM=1）：`x/residual/gamma/bias/output`，各 `tile_len_*sizeof(T)`。
  - fp32：`tile_len_=2048` → 各 8 KB，共 40 KB。
  - fp16/bf16：`tile_len_=4096` → 各 8 KB（元素 2B，4096×2=8KB），共 40 KB。
- 5 个 TBuf（VECCALC）：`x_float_/residual_float_/value_float_` 各 `tile_len_*4` = 16 KB（共 48 KB）；`work_` 4 KB；`sum_` 8 KB(fp32)/极小(fp16)。
- **合计约 92–100 KB ≪ 1 MB**，UB 容量充足（D 级估算，按 9.0.0 UB 规格）。

### 5.3 ReduceSum count 与 workLocal
- 基础 count 版：`count` 上限由数据类型与 UB 决定，官方仅约束“≤UB 容量”（A 级）。fp32 单块 `count≤2048`；fp16/bf16 经 Cast 后 `calc_len`（fp32 计数）≤ 4096。
- `workLocal` 缓冲（`work_`，`WORK_LEN=1024` float = 4 KB）尺寸是否足以支撑 `count=4096` 的 fp32 归约，文档未给显式公式，**需真机验证**（风险点 12）。建议：fp16/bf16 路径在真机跑 D=4096 等边界，或临时放大 `WORK_LEN` 到 ≥ 2×`calc_len` 以留余量（D 级建议）。

### 5.4 DataCopyExtParams 取值
- 工程用 `{blockCount=1, blockLen=len*sizeof(T), srcStride=0, dstStride=0, rsv=0}`，即单连续块、无间隔，符合“整段连续数据”语义；`blockLen≤2097151` 字节（GM src 口径）远未触顶。

---

## 6. 无法确认事项

1. **`workLocal` 最小尺寸公式**：官方 9.0.0 文档对基础 `ReduceSum(count)` 的 `workLocal` 缓冲下限未给出与 `count` 的显式关系式（仅泛述“≤UB”）。`WORK_LEN=1024` 对 fp16/bf16 大 tile 是否够，需真机或头文件确认（D 级）。
2. **`Duplicate` / `GatherMask`**：本题未使用，且 9.0.0 缓存页未命中其完整签名，仅作 C/D 级备注；如后续需要精确签名，应补抓对应 API 页。
3. **`__vector__` 修饰符的 9.0.0 直调专项页**：现有官方修饰符表为 CANN 8.5.0alpha002（概念稳定、沿用至 9.0.0），但非 9.0.0 页面本身；若判题文档要求严格版本号，建议补抓 9.0.0《C 语言拓展》对应页。
4. **社区数字 `255 / 16320 / 4096` 的权威出处**：这些为社区/源码注释口径（repeat 上限、fp32 repeat×64、保守实测值），非官方单一硬上限，不能作为唯一依据（已标注 C/D）。
5. **`CAST_RND` 命名**：9.0.0 文档枚举名为 `CAST_ROUND`；题目给的 `CAST_RND` 疑为旧版/社区别名，工程中实际使用的是 `CAST_RINT` 与 `CAST_NONE`，均确属 9.0.0 枚举，不受影响。
6. **`DataCopyPad` 三参重载（无 pad 参数，用于 UB→GM）在直调头文件中的确切签名**：官方文档展示该重载，工程第 80 行使用，但未在缓存中单独展开其原型；按文档 A 级重载表存在，安全。

---

## 7. 来源表

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 |
|---|---|---|---|---|---|---|
| S1 | DataCopy / DataCopyPad（含 DataCopyExtParams、DataCopyPadExtParams、GM 无对齐约束、dummy 丢弃） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0265.html | CANN 9.0.0 社区版 | 2026-09-11 | 争议点 1、2、3（GM 对齐）、DataCopy 系列签名 | A |
| S2 | ReduceSum（基础 count 版、WholeReduceSum、约束） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0078.html | CANN 9.0.0 | 2026-09-11 | 争议点 3、ReduceSum 事实 | A |
| S3 | Add / Mul / Muls（数据类型支持矩阵） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0055.html | CANN 9.0.0 | 2026-09-11 | 争议点 4、Add/Mul/Muls 事实 | A |
| S4 | Cast（RoundMode 枚举、舍入规则） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0073.html | CANN 9.0.0 | 2026-09-11 | Cast / RoundMode 事实 | A |
| S5 | Sqrt（精度模式） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0029.html | CANN 9.0.0 | 2026-09-11 | 争议点 5 | A |
| S6 | Rsqrt（精度模式） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0030.html | CANN 9.0.0 | 2026-09-11 | 争议点 5 | A |
| S7 | LocalTensor 简介（GetValue 等） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0006.html | CANN 9.0.0 | 2026-09-11 | GetValue、LocalTensor 事实 | A |
| S8 | SetFlag/WaitFlag/PipeBarrier/HardEvent | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0270.html | CANN 9.0.0 | 2026-09-11 | 同步原语事实 | A |
| S9 | TQue / TBuf / TPipe::InitBuffer | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0108.html | CANN 9.0.0 | 2026-09-11 | 队列/缓冲/内存管理事实 | A |
| S10 | GetBlockIdx / GetBlockNum | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0184.html（及 0185） | CANN 9.0.0 | 2026-09-11 | 多核索引事实 | A |
| S11 | GetReduceSumMaxMinTmpSize（高阶 Pattern::Reduce 版 host API） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10160.html | CANN 9.0.0 | 2026-09-11 | 争议点 3（高阶路径）、事实表 | A |
| S12 | C 语言拓展（函数修饰符：`__global__/__aicore__/__vector__/__cube__/__mix__`） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | CANN 8.5.0alpha002（概念沿用 9.0.0） | 2026-09-11 | 争议点 6（入口限定符） | A（限修饰符表）；版本差异已标注 |
| S13 | 社区：直调模式 vs 框架模式入口选择（`__vector__` 用于直调纯 Vector） | https://ai6s.net/6a3a61d510ee7a33f2814f6d.html | 社区 | 2026-09-11 | 争议点 6 交叉验证 | C |
| S14 | 社区：AI-Core 核函数编程指南（`__vector__` 仅在 Vector 核执行） | https://blog.csdn.net/gitblog_00048/article/details/156850198 | 社区（CSDN） | 2026-09-11 | 争议点 6 交叉验证 | C |
| S15 | 社区：rsqrt 快速求逆误差约 2^-20（经验估计） | 多 CSDN/论坛；未在官方文档定稿 | 社区 | 2026-09-11 | 争议点 5 交叉参考 | C |
| S16（本地） | 两份关键页已下载存档于 `/Users/sunyiyang/Desktop/Project/cann/临时/api-pages/`：`agent02_vector_qualifier_850a002.html`、`agent02_GetReduceSumMaxMinTmpSize_900.html` | 本地 | — | 2026-09-11 | 存档备查 | — |

---

### 附：对其它代理（agent01/03/04/05）的协同提示
- 若 agent 负责“数值正确性/精度”，请关注本报告争议点 5：工程用标量 `sqrtf` + 全 FP32，规避了向量 Sqrt/Rsqrt 误差；fp16/bf16 路径的 `CAST_RINT` 就近舍入是主要量化误差源。
- 若 agent 负责“边界/对齐（D 非 32 倍数）”，请直接引用本报告争议点 1/2/3 与第 5 节：尾块 `DataCopyPad` 补 0、GM 无对齐约束、UB→GM dummy 自动丢弃。
- 若 agent 负责“提交形态/判题”，注意 `kernel.asc`（直调 `__vector__`）与 `源码/*.cpp`（框架 `__aicore__`）是**两套不同提交形态**，不要混淆入口限定符与 `run_kernel` 约定。
