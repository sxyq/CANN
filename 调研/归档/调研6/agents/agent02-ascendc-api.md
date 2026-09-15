# Agent 2 调研报告：AddRmsNormBias 官方 Ascend C API（CANN 9.0.0）

> 调研范围：Ascend C 基础 API（SIMD / 矢量计算 / 数据搬运 / 同步 / 系统变量 / 核函数限定符）。
> 平台独占：hiascend.com / hiascend.cn / asc.gitcode.com 的 CANN 9.0.0 / 9.0.X API 参考与开发指南。
> 环境声明：**本机 macOS，无 CANN 工具链、无昇腾 NPU；本报告所有结论均来自官方文档页面与缓存 HTML 的正文提取，未编译、未运行任何 Ascend C 代码。**
> 文档版本约定：标注 `9.0.0`/`9.0.X` 为社区版 9.0.0 文档（含本地缓存页面）；标注 `8.x` 为商用版/社区版旧版，引用时显式标注版本差异。

---

## 1. 执行摘要

### 1.1 总体结论

对 `AddRmsNormBias` 算子（沿最后一维 D 归约，D∈[64,32768] 且可能非 32 字节对齐，dtype∈{fp16,bf16,fp32}，纯矢量计算），CANN 9.0.0 提供完整的基础 API 覆盖：数据搬运（`DataCopy`/`DataCopyPad`）、矢量算术（`Add`/`Mul`/`Muls`/`Div`/`Divs`/`Sqrt`/`Rsqrt`/`Cast`）、归约（`ReduceSum`）、资源管理（`TPipe`/`TQue`/`TBuf`/`LocalTensor`/`GlobalTensor`）、同步（`PipeBarrier`/`SetFlag`/`WaitFlag`+`HardEvent`）、系统变量（`GetBlockIdx`/`GetBlockNum`）。核函数入口可用 `__global__ __vector__`（模板形态）或 `__global__ __aicore__`（通用形态），两者在判题架构 `dav-2201`（Atlas A2，耦合架构）下均合法，且 `__vector__` 在耦合架构"不生效"（等价于 `__aicore__`）。

**最关键的架构约束**：在 Atlas A2 上，`Add`/`Mul`/`Muls`/`Div` **均不支持 `bfloat16_t`**（仅支持 half/float 等）。因此 bf16 路径必须：`bf16 → fp32 (Cast)` → 全程 fp32 计算（含 `ReduceSum` 也仅支持 half/float）→ `fp32 → bf16 (Cast)`。

### 1.2 五个"卡脖子"问题裁决结论（速览）

| # | 问题 | 裁决 |
|---|------|------|
| 1 | `DataCopyPadExtParams<T>` 字段声明顺序 | **裁决 A：`{isPad, leftPadding, rightPadding, paddingValue}`**。官方 9.0.0 文档表 6 参数排列即此顺序，社区示例代码 `padParams{false,0,0,0}` 亦一致。推荐始终按成员名赋值以防版本歧义。 |
| 2 | UB→GM 搬出时 UB 侧 dummy 是否污染相邻 GM | **不污染**。`DataCopyPad` 的 `Local→Global` 重载无 `padParams` 参数；官方文档明确：UB→GM 时框架自动补 dummy 对齐，但**写入 GM 时丢弃 dummy**。前提：用 `blockLen`（可非对齐字节数）让框架处理尾部，不要把 UB 手动圆整到 32B 后整体搬出。 |
| 3 | `ReduceSum` 的 `count` 硬上限与 `sharedTmpBuffer` 空间 | **官方文档未给出 255/4096/16320 这类硬上限**。`count` 为 `int32_t`，文档仅要求"不超过 UB 大小限制"，取值上限随数据类型（half 单次迭代 128 元素、float 64 元素）。`sharedTmpBuffer` 有公式（见 §3.3）。`GetReduceSumMaxMinTmpSize` 是 9.0.0 的 host 侧辅助接口，且"最大临时空间=最小临时空间"。D≤32768 在 UB 预算内可行。 |
| 4 | Atlas A2 上 `Add`/`Mul`/`Muls`/`Div` 是否支持 bf16 | **不支持。** 官方 9.0.0-beta.2 文档明文："Atlas A2 上 Add 接口不支持 bfloat16_t 源操作数"；Muls 9.0.0 文档列 A2 支持 `half/int16_t/float/int32_t`（无 bf16）。bf16 路径必须走 FP32。 |
| 5 | `Cast` fp32→bf16 支持的 roundMode；fp16/bf16→fp32 是否无损 | fp32→bf16 支持 `CAST_RINT`/`CAST_FLOOR`/`CAST_CEIL`/`CAST_ROUND`/`CAST_TRUNC`（示例均给出结果）；`bf16→fp32` 与 `half→fp32` **无精度损失、无舍入**（"不存在精度转换问题，无舍入模式"）。 |

---

## 2. API 清单表（18 项全覆盖）

> 证据等级：A=官方 9.0.0/9.0.X 文档（含本地缓存）；B=官方但旧版（8.x）或官方但非目标产品（如 Kirin）；C=第三方博客/样例；D=未确认。
> "纯 vector 核可用性"指该 API 是否可在 `__global__ __vector__` 纯矢量核函数内调用（A2 耦合架构下 `__vector__` 即 AIV/AI Core 矢量通路）。

| # | API | 函数签名（官方原文摘录） | 参数含义 | 支持产品（9.0.0 文档） | 文档版本 | 纯 vector 核可用性 | 与 AddRmsNormBias 的对应 | 证据 |
|---|-----|------------------------|----------|------------------------|----------|-------------------|--------------------------|------|
| 1 | `GlobalTensor::SetGlobalBuffer` | `void SetGlobalBuffer(__gm__ PrimType* buffer, uint64_t bufferSize);` `void SetGlobalBuffer(__gm__ PrimType* buffer);` | 绑定 GM 地址与长度（bufferSize 可省略） | 全产品（GM 数据结构，与核无关） | 9.0.X | 是（Host/核内均可设） | 运行时从 `TensorGroupInfo` 读 `shape/dtype` 后绑定 xGm/residualGm/gammaGm/biasGm/outputGm | A (S032) |
| 2 | `LocalTensor::GetValue/SetValue/GetSize` | `PrimType GetValue(const uint32_t offset) const;` `void SetValue(const uint32_t index, const T1 value) const;` `uint32_t GetSize() const;`（`GetLength` 已弃用） | 单点读写 UB 元素；GetSize 返回元素个数 | 全产品 | 9.0.X | 是 | 标量初始化 rstd（如 `SetValue(0, ...)`）、读回归约结果 | A (S031,S055) |
| 3 | `DataCopy`（基础搬运） | `template<typename T> void DataCopy(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const uint32_t calCount);`（GM→UB）`template<typename T> void DataCopy(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const uint32_t calCount);`（UB→GM）另有 `DataCopyParams` 0 级重载与 Local→Local 重载 | calCount=元素个数；0 级含 blockCount/blockLen/srcStride/dstStride | GM↔UB 通路 A2 支持（stride 单位：GM=字节，UB=dataBlock） | 9.0.0 签名同 8.5.0 | 是 | CopyIn/CopyOut 主搬运；D 非 32B 对齐时用 `blockLen`（字节）处理尾部 | B (S050) |
| 4 | `DataCopyPad`(+`DataCopyPadExtParams`) | GM→UB：`DataCopyPad(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const DataCopyExtParams&, const DataCopyPadExtParams<T>&)`；UB→GM：`DataCopyPad(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const DataCopyExtParams&)`（**无 padParams**） | 非对齐搬运；padding 仅作用于 GM→UB 方向 | A2 支持（GM↔VECIN/VECOUT） | 9.0.0 | 是 | D 非对齐时 GM→UB 读入补 padding；UB→GM 不需要 padParams | A (S033) |
| 5 | `ReduceSum` | `template<typename T> void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& sharedTmpBuffer, const int32_t count);`（tensor 前 n 个） | count=参与归约元素个数；sharedTmpBuffer=内部临时空间 | A2 训练/推理：half/float（**无 bf16**） | 9.0.0 | 是 | 沿 D 维求 `sum(y^2)` 与 `sum(y)` | A (S034) |
| 6 | `Cast` | `template<typename T,typename U> void Cast(const LocalTensor<T>& dst, const LocalTensor<T>& src, const RoundMode& roundMode, const uint32_t count);` | roundMode 舍入模式；count 元素数 | A2：half/float/int 等；fp32↔bf16 支持 | 9.0.0 | 是 | bf16↔fp32 互转；`y/rms*gamma+bias` 类型对齐 | A (S035) |
| 7 | `Add` | `template<typename T> void Add(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, const int32_t& calCount);` | 逐元素加；calCount 元素数 | A2：half/int16_t/float/int32_t（**无 bf16**） | 8.1RC1 签名；9.0.0-beta.2 明令 A2 不支持 bf16 | 是 | `y = x+residual`（fp32 路径） | A/B (S048,S049) |
| 8 | `Mul` | `template<typename T> void Mul(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, const int32_t& calCount);` | 逐元素乘 | A2：同 Add 家族（half/float，无 bf16） | 8.x 同族 | 是 | `y*y`、`rms*gamma` 等 | B (S038 同族推断) |
| 9 | `Muls` | `template<typename T,bool isSetMask=true> void Muls(const LocalTensor<T>& dst, const LocalTensor<T>& src, const T& scalarValue, const int32_t& count);` | 矢量×标量 | A2：half/int16_t/float/int32_t（**无 bf16**） | 9.0.0 | 是 | `y * (1/rms)`、`* gamma` | A (S038) |
| 10 | `Div`/`Divs` | `Div(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, const int32_t& calCount);` `Divs(const U& dst, const S& src0, const V& src1, const int32_t& count);` | 逐元素除 / 矢量÷标量 | A2：half/float（Div 文档示例）；无 bf16 | 8.5.0/华为 | 是 | `y / rms`（建议用 `Muls(1/rms)` 替代 Div 以避免除零） | B (S052,S053) |
| 11 | `Sqrt` | `template<typename T,const SqrtConfig& config=DEFAULT_SQRT_CONFIG> void Sqrt(const LocalTensor<T>& dst, const LocalTensor<T>& src, const int32_t& count);` | 逐元素开方 | A2：half/float | 9.0.0 | 是 | `sqrt(mean(y^2)+eps)` | A (S036) |
| 12 | `Rsqrt` | `template<typename T,const RsqrtConfig& config=DEFAULT_RSQRT_CONFIG> void Rsqrt(const LocalTensor<T>& dst, const LocalTensor<T>& src, const int32_t& count);` | 开方取倒数 | A2：half/float | 9.0.0 | 是（推荐用 Rsqrt 替代 1/Sqrt） | `rms = rsqrt(mean(y^2)+eps)` | A (S037) |
| 13 | `TPipe::InitBuffer` | `pipe.InitBuffer(TQue& queue, uint32_t depth, uint32_t sizeInBytes);` `pipe.InitBuffer(TBuf& buffer, uint32_t sizeInBytes);` | 为队列/临时缓冲分配 UB | 全产品 | 9.0.0（API 列表） | 是 | 在 Init 中一次性分配所有队列/临时缓冲 | A/B (S058) |
| 14 | `TQue::AllocTensor/EnQue/DeQue/FreeTensor` | `LocalTensor<T> AllocTensor();` `void EnQue(const LocalTensor<T>&);` `LocalTensor<T> DeQue();` `void FreeTensor(LocalTensor<T>&);` | 队列的生产/消费/释放 | 全产品 | 9.0.0 | 是 | 三段式流水（CopyIn/Compute/CopyOut） | A (S040) |
| 15 | `TBuf` | `TBuf<TPosition::VECCALC> buf; pipe.InitBuffer(buf, sizeInBytes); LocalTensor<T> t = buf.Get();` | 不带队列语义的临时 UB 缓冲 | 全产品 | 9.0.0（API 列表） | 是 | `sharedTmpBuffer`、`sqx` 等中间变量 | A/B (S058) |
| 16 | `PipeBarrier` + `PIPE_V/PIPE_S/PIPE_MTE2/PIPE_MTE3` | `template<pipe_t pipe> void PipeBarrier();`（pipe∈{PIPE_S,PIPE_V,PIPE_M,PIPE_MTE1,PIPE_MTE2,PIPE_MTE3,PIPE_FIX,PIPE_ALL}） | 阻塞相同流水；PIPE_MTE2=GM→UB 搬运，PIPE_MTE3=UB→GM 搬运，PIPE_V=矢量计算，PIPE_S=标量 | A2 支持 PipeBarrier | 9.1.0-beta.2 文档（接口稳定） | 是 | 调试期用 `PipeBarrier<PIPE_ALL>` 兜底 | A/B (S044,S051) |
| 17 | `HardEvent`（V_S/S_V/MTE2_V 等） | `enum class HardEvent : uint8_t { MTE2_V, V_MTE2, MTE3_V, V_MTE3, S_V, V_S, ... };` `template<HardEvent e> void SetFlag(int32_t id);` `template<HardEvent e> void WaitFlag(int32_t id);` | 细粒度流水同步（源_目标）；如 MTE2_V=PIPE_V 等 PIPE_MTE2 | A2 支持 SetFlag/WaitFlag；eventID 范围 0–7 | 9.0.0 | 是 | 生产-消费同步：`V_MTE3`（计算完可搬出）、`MTE2_V`（搬入完可算）、`V_MTE2`（释放 input UB） | A (S044) |
| 18 | `GetBlockIdx`/`GetBlockNum`/`GetBlockDim` | `uint32_t GetBlockIdx();` `uint32_t GetBlockNum();` | 当前核编号 / 总核数 | 全产品 | 9.0.0 | 是 | 多核分片：`blockOffset = GetBlockIdx()*GetBlockNum()...` | A (S041,S054,S055) |
| — | 核函数限定符 `__global__ __vector__` vs `__global__ __aicore__` | `__global__`（核入口，须配 `__aicore__`）；`__vector__`（仅 Vector 核，耦合架构不生效）；`__aicore__`（Device 侧函数） | 限定核执行位置 | 全产品（A2 耦合→`__vector__` 不生效） | 9.0.0 | — | 模板用 `__global__ __vector__`（纯矢量）；A2 下等价 `__aicore__` | A (S041,S059) |

---

## 3. 五个卡脖子问题逐条裁决

### 3.1 问题 1：`DataCopyPadExtParams<T>` 字段声明顺序

**裁决：选项 A —— `{isPad, leftPadding, rightPadding, paddingValue}`。**

**官方原文（CANN 9.0.0，`atlasascendc_api_07_0265`，表 6 "DataCopyPadExtParams\<T\> 结构体参数定义"）：**
> 参数名称 / 含义
> - **isPad** —— 是否需要填充用户自定义的数据，取值范围：true，false。true：填充 padding value；false：表示用户不需要指定填充值，会默认填充随机值。
> - **leftPadding** —— 连续搬运数据块左侧需要补充的数据范围，单位为元素个数。
> - **rightPadding** —— 连续搬运数据块右侧需要补充的数据范围，单位为元素个数。
> - **paddingValue** —— 左右两侧需要填充的数据值，需要保证在数据占用字节范围内。

文档中"参数名称"的排列顺序即 **isPad → leftPadding → rightPadding → paddingValue**。

**佐证（社区示例代码，S045/S047）：** 官方/社区示例统一写作
```cpp
AscendC::DataCopyPadExtParams<half> padParams{false, 0, 0, 0};  // isPad, leftPadding, rightPadding, paddingValue
```
与文档表 6 顺序一致。英文 8.5.0 文档（S060，Table 5）同样为 `isPad / leftPadding / rightPadding / paddingValue`。

**反方证据（历史歧义）：** 部分旧版博客曾给出 `{isPad, paddingValue, leftPadding, rightPadding}` 的"结构体真实声明"说法。但本次检索到的 9.0.0 文档表、示例、以及解析源码的博客（S047 标注 `false→isPad; 0U→leftPadding; 0U→rightPadding; 0U→paddingValue`）**均指向选项 A**。本机无 CANN 工具链，无法直接 `cat` 安装包内 `kernel_struct_data_copy.h`（该头文件被文档 defer 引用，属 Agent 3 范畴），故"结构体真实声明"以安装包头文件为准。

**裁决理由与建议：** 以 9.0.0 官方文档表 6 为准 = **选项 A**。但鉴于历史上存在顺序争议，**强烈建议始终按成员名赋值**，避免顺序歧义：
```cpp
AscendC::DataCopyPadExtParams<half> padParams;
padParams.isPad = false;
padParams.leftPadding = 0;
padParams.rightPadding = 0;
padParams.paddingValue = 0;
```
若实测发现按 `{false,0,0,0}` 初始化后 padding 行为异常（左右填充错位），说明所用 CANN 版本头文件实际为选项 B，应改用成员名赋值或对应顺序。

---

### 3.2 问题 2：UB→GM 搬出时，UB 侧补的 dummy 是否会写到 GM 污染相邻内存？

**裁决：不会污染。框架在写入 GM 时丢弃对齐用的 dummy。**

**官方原文（CANN 9.0.0 DataCopyPad 文档，S033；以及技术文章 S046/S047）：**
- DataCopyPad 的 `Local Memory -> Global Memory` 重载**没有 `padParams` 参数**：
  > `template<typename T> __aicore__ inline void DataCopyPad(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const DataCopyExtParams& dataCopyParams);`
  → 即 UB→GM 方向根本不存在"用户填充 padding"的入口。
- 对非对齐搬出的官方说明（S046/S047，引用官方 DataCopyPad 文档）：
  > "对于 Local Memory->Global Memory(VECIN/VECOUT->GM) 的数据搬运，不需要开发者自行填充数据，所以无需关注 DataCopyPadExtParams，仅需要关注 DataCopyExtParams。当每个连续传输数据块长度 blockLen 不满足 32 字节对齐，由于 UB 要求 32 字节对齐，框架在搬出时会自动补充一些假数据来保证对齐，但在当搬到 GM 时会自动将填充的假数据丢弃掉。"
  > "UB→GM 方向：…blockLen 可非对齐（如 47B），框架搬出时自动补 dummy 到 32B、写入 GM 时丢弃。"

**反方证据（陷阱）：** 华为论坛性能优化帖（S047）也指出：若开发者**手动把 UB 圆整到 32B 对齐**后再用 `DataCopy(UB→GM, blockLen×blockCount)`（此时 blockLen 已是 32B 整数倍、覆盖 padding 区），硬件会**如实把 padding 区写入 GM**，从而污染相邻内存。这属于"人为过度补齐"导致，而非 `DataCopyPad` 的自动丢弃行为。

**裁决理由与建议：** DataCopyPad/DataCopy 的**自动对齐 dummy 在写回 GM 时被丢弃**，不污染相邻内存——这是官方明确行为。正确做法是：
1. 搬出时 `blockLen` 设为**真实有效字节数（可非 32B 对齐）**，让框架处理尾部并丢弃 dummy；
2. **不要**手动把 UB 圆整到 32B 后整体搬出（那会写出脏数据）；
3. 若担心边界，可在 Host 侧对输出 GM 多申请 32B 安全余量（业界防御性做法），或在非对齐尾部用 `DataCopyPad` 的 `rightPadding` 处理 GM→UB 读入方向（读入方向的 dummy 只留在 UB，不回写 GM）。

---

### 3.3 问题 3：`ReduceSum` 的 `count` 硬上限、workLocal 空间、`GetReduceSumMaxMinTmpSize` 状态

**裁决：官方文档未给出 255/4096/16320 的硬上限；以 UB 容量与 sharedTmpBuffer 公式为准；`GetReduceSumMaxMinTmpSize` 是 9.0.0 host 侧接口，且"最大临时空间=最小临时空间"。**

**官方原文（CANN 9.0.0 ReduceSum，`atlasascendc_api_07_0078`）：**
- 函数原型（tensor 前 n 个数据计算）：
  > `template<typename T> __aicore__ inline void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& sharedTmpBuffer, const int32_t count);`
- `count` 参数含义：
  > "count 输入，参与计算的元素个数。参数取值范围和操作数的数据类型有关，数据类型不同，能够处理的元素个数最大值不同，**最大处理的数据量不能超过 UB 大小限制**。"
- 数据类型支持（A2 训练/推理）：**half / float（无 bf16）**。
- `sharedTmpBuffer` 大小计算公式（文档原文）：
  > 方式一：`int typeSize = 2`（half）/ `4`（float）；`elementsPerBlock = 32/typeSize`；`elementsPerRepeat = 256/typeSize`；对 tensor 前 n 个数据计算接口，`firstMaxRepeat = count/elementsPerRepeat`（half 下 `count/128`，float 下 `count/64`）；`finalWorkLocalNeedSize = RoundUp(iter1OutputCount, elementsPerBlock) * elementsPerBlock`。
- 约束说明：
  > "该接口内部通过软件仿真来实现 ReduceSum 功能，某些场景下性能可能不及直接使用硬件指令实现的 BlockReduceSum 和 WholeReduceSum 接口。"

**`GetReduceSumMaxMinTmpSize`（CANN 9.0.0，host 侧，S042）：**
> "kernel 侧 ReduceSum 接口的计算需要开发者预留/申请临时空间，本接口用于在 host 侧获取预留/申请的最大最小临时空间大小……**该接口最大临时空间当前等于最小临时空间**。"
> 函数原型：`void GetReduceSumMaxMinTmpSize(const ge::Shape& srcShape, const ge::DataType dataType, ...)`（注意：入参为 `ge::Shape`/`ge::DataType`，是 **host 侧 tiling 接口**，Direct Invocation 无 host tiling 场景时无法直接复用，但其结论"max==min"说明 sharedTmpBuffer 取最小值即可）。

**反方证据（社区 255/4096/16320 说法）：** 社区流传 `count` 上限有 255（repeat 上限？）、4096、16320 等说法。本次检索的 9.0.0 官方文档**未出现这些数字**；文档仅以"UB 大小限制"与"数据类型相关（half 128/迭代、float 64/迭代）"作为约束。16320≈256/2×... 等数值可能是特定硬件 repeat/block 上限的推导，但**非官方文档陈述**。

**裁决理由与建议：**
- `count` **无官方硬上限数字**；D≤32768 在 A2 上：half 下 `count/128=256` 轮、float 下 `512` 轮，UB 预算内可行（UB 通常 256KB，单维 D 归约中间 buffer 远小于此）。
- `sharedTmpBuffer` 必须按公式分配（half：向上取整到 16 元素的整数倍；float：8 元素整数倍）；取最小值即可（max==min）。
- 由于 `ReduceSum` 是软件仿真、A2 仅支持 half/float，**bf16 路径务必先 `Cast` 到 fp32 再归约**（这也呼应问题 4）。
- 若追求性能可考虑 `BlockReduceSum`/`WholeReduceSum`（文档提及），但其为不同 API，需在实现阶段另行评估。

---

### 3.4 问题 4：Atlas A2 上 `Add`/`Mul`/`Muls`/`Div` 是否支持 `bfloat16_t`？

**裁决：不支持。A2 上这四个基础算术接口均不支持 bfloat16_t 源操作数，bf16 路径必须走 FP32。**

**官方原文 1（CANN 9.0.0-beta.2，"Add 自定义算子开发"概述，S048）：**
> "在 Atlas A2 训练系列产品/Atlas 800I A2 推理产品 上，**Add 接口不支持对数据类型 bfloat16_t 的源操作数进行求和计算**。因此，需要先将算子输入的数据类型转换成 Add 接口支持的数据类型，再进行计算。为保证计算精度，调用 Cast 接口将输入 bfloat16_t 类型转换为 float 类型，再进行 Add 计算，并在计算结束后将 float 类型转换回 bfloat16_t 类型。"

**官方原文 2（CANN 9.0.0 Muls，`atlasascendc_api_07_0055`，A2 支持的数据类型）：**
> "Atlas A2 训练系列产品 / Atlas A2 推理系列产品，支持的数据类型为：**half/int16_t/float/int32_t**"（表 1 模板参数 T 说明，**无 bfloat16_t**）。

**官方原文 3（CANN 9.0.0 ReduceSum，`atlasascendc_api_07_0078`，A2 支持 half/float——连归约也不支持 bf16）：** 同上 §3.3。

**反方证据（Built-in Data Types 表，S056）：** 9.0.0 "Built-in Data Types" 文档列 Atlas A2 支持 `bfloat16_t` 作为**内置数据类型**（即可以声明 `LocalTensor<bfloat16_t>`、可做数据搬运与 Cast）。但"类型被支持"≠"所有矢量计算算子都支持该类型做计算"。基础算术/归约算子有独立的产品支持表，A2 上仅为 half/float。

**裁决理由与建议：**
- 结论明确：**A2 上 Add/Mul/Muls/Div/ReduceSum 不支持 bf16 计算**，仅 half/float。
- 对 `AddRmsNormBias` 的 **bf16 路径**：`bf16 → fp32 (Cast)` → 全程 fp32 完成 `x+residual`、`y*y`、`sum`、`sqrt/recip`、`*gamma`、`+bias` → `fp32 → bf16 (Cast)`。
- **fp16 路径**：A2 支持 half 的 Add/Mul/Muls/Div/ReduceSum，**可全程 fp16**（注意归约半精度易溢出/精度差，RmsNorm 建议归约用 fp32 或 fp16 用 `Rsqrt`+标量）。
- **fp32 路径**：原生支持，最稳。
- 注意 `MulDstAdd`（`atlasascendc_api_07_00007`）仅 Atlas 350 支持（A2 为 ×），**不能用于 A2**，用 `Add`+`Mul` 组合替代。

---

### 3.5 问题 5：`Cast` fp32→bf16 支持的 roundMode；fp16/bf16→fp32 是否无损？

**裁决：fp32→bf16 支持全部常见 roundMode（含 CAST_RINT 等）；bf16→fp32 与 half→fp32 均无损、无舍入。**

**官方原文（CANN 9.0.0 Cast，`atlasascendc_api_07_0073`，表 1 精度转换规则）：**
- "float → bfloat16_t" 行：
  > "将 src 按照 roundMode 取到 bfloat16_t 所能表示的数，以 bfloat16_t 格式（溢出默认按照饱和处理）存入 dst 中。示例：…CAST_RINT 模式舍入得尾数 0000001…；CAST_FLOOR…；CAST_CEIL…；CAST_ROUND…；CAST_TRUNC…"
  → **fp32→bf16 支持 CAST_RINT / CAST_FLOOR / CAST_CEIL / CAST_ROUND / CAST_TRUNC**（文档示例逐一给出）。
- "bfloat16_t → float" 行：
  > "将 src 以 float 格式存入 dst 中，**不存在精度转换问题，无舍入模式**。"
- "half → float" 行：
  > "将 src 以 float 格式存入 dst 中，不存在精度转换问题，无舍入模式。"
- `RoundMode` 枚举（文档原文）：
  > `CAST_NONE=0; CAST_RINT; CAST_FLOOR; CAST_CEIL; CAST_ROUND; CAST_TRUNC; CAST_ODD; CAST_HYBRID;`
  > 注：`CAST_HYBRID` 仅 hif8 输出；`double→bfloat16_t` 等少数路径"仅支持 tensor 前 n 个数据计算接口"。

**反方证据/注意：** A2 架构（`__NPU_ARCH__==2201`）下常用 `CAST_RINT`（四舍六入五成双）；社区有针对 2201 用 `CAST_RINT`、2002 用 `CAST_NONE` 的宏分支写法（S041 示例）。`CAST_NONE` 语义："有精度损失时表示 CAST_RINT，无精度损失时不舍入"。

**裁决理由与建议：**
- bf16 路径中 `Cast(fp32→bf16, CAST_RINT)` 与 `Cast(bf16→fp32)` 均可安全使用；bf16→fp32 无损，反向有精度损失但由硬件舍入保证。
- 建议 bf16 路径全程 fp32 计算，仅在出入口做 `bf16↔fp32`，最大化精度。
- `half↔fp32` 同样无损（half→fp32 无舍入），fp16 路径可在归约/累加处提升为 fp32 再降回 half 以保精度。

---

## 4. 「文档未给出 / 无法确认」清单

| 项 | 状态 | 说明 |
|----|------|------|
| `ReduceSum` 的 `count` 硬上限（255/4096/16320） | **文档未给出** | 9.0.0 官方文档仅以"UB 大小限制 / 数据类型相关"约束，未出现上述数字；社区说法无官方背书。 |
| `DataCopyPadExtParams<T>` 头文件**真实结构体声明顺序** | **未能直接读取** | 文档表 6 与示例均指向选项 A，但安装包 `kernel_struct_data_copy.h` 未在本机（无 CANN 工具链），依 Agent 3 边界未抓取源码；建议按成员名赋值。 |
| `Mul` / `Div` 在 A2 的**逐版本产品支持表** | **间接确认** | 未直接抓取 Mul/Div 的 9.0.0 产品表；其签名取自 8.x/华为文档，A2 不支持 bf16 由 Add 9.0.0-beta.2 官方明文 + Muls 9.0.0 表共同佐证（同族一致）。证据等级 B。 |
| `GetBlockDim` 接口 | **不存在** | 官方系统变量访问仅 `GetBlockNum`/`GetBlockIdx`（及 SIMT 模型的 `SetBlockDim`）；`GetBlockDim` 是 CUDA 概念，Ascend C 无此函数（S054）。疑为对 `GetBlockNum` 的误称。 |
| `DataCopy` 基础接口的 **9.0.0 独立页面正文** | **部分** | 9.0.0 文档页 WebFetch 仅返回 API 列表骨架；签名取自 8.5.0 文档与华为云博客（接口稳定）。证据等级 B。 |
| `TPipe`/`TBuf`/`PipeBarrier` 的 **9.0.0 逐页正文** | **部分** | 签名/枚举取自 9.0.0 本地缓存（0270 HardEvent）、8.5.0 API 列表（S058）、华为 PipeBarrier 文档（S051）；PipeBarrier 产品表取自 9.1.0-beta.2（接口稳定）。 |
| `BlockReduceSum`/`WholeReduceSum` 是否在 A2 可用 | **未定案** | 文档仅在 ReduceSum 约束中提及"性能可能不及"，未在本轮展开其 9.0.0 签名与 A2 支持；留待实现阶段评估。 |
| `TensorGroupInfo` / `dtype` 枚举运行期读取的具体字段 | **未覆盖（Agent 1 范畴）** | 属题面/模板数据结构，不在本 Agent API 调研范围。 |

---

## 5. 对实现的 API 级建议（哪些组合最稳、哪些有坑）

### 5.1 推荐 API 组合（按 dtype 路径）

**fp32 路径（最稳，推荐 baseline）：**
- 读入：`DataCopy(xGm→xLocal, count)` / `DataCopy(residualGm→rLocal)` / `DataCopy(gammaGm→gLocal)` / `DataCopy(biasGm→bLocal)`（D 非 32B 对齐时改用 `DataCopyPad` 读入，或 `blockLen` 字节非对齐）。
- 计算：`Add(y, x, residual)` → `Mul(sq, y, y)` → `ReduceSum(sumSq, sq, tmp, D)` + `ReduceSum(sumY, y, tmp, D)`（fp32 支持）→ `Muls(meanSq, sumSq, 1.0f/D)` + `Add(meanSq, meanSq, eps)` → `Rsqrt(rstd, meanSq)` → `Mul(y, y, rstd)` → `Mul(y, y, gamma)` → `Add(y, y, bias)`。
- 写出：`DataCopy(outputGm→yLocal, count)`（同读入对齐处理）。
- 同步：生产-消费用 `SetFlag/WaitFlag<HardEvent::MTE2_V>`（搬入完可算）、`<V_MTE3>`（算完可搬出）、`<V_MTE2>`（释放 input UB）；调试期可整体 `PipeBarrier<PIPE_ALL>()` 兜底。

**fp16 路径（性能优先，注意精度）：**
- A2 支持 half 的 Add/Mul/Muls/Div/ReduceSum，**可全程 fp16**。但 RmsNorm 归约易溢出，建议 `sum(y^2)`/`sum(y)` 用 **fp32 累加**（先把 y `Cast` 到 fp32 做 ReduceSum，再 Cast 回 fp16），或用 fp16 + `Rsqrt` 并放大系数。需精度验证（Agent 6 范畴）。

**bf16 路径（必须 FP32 中转）：**
- `Cast(bf16→fp32)` 入口 → 全程 fp32 用上述 fp32 组合 → `Cast(fp32→bf16)` 出口。
- **坑**：禁止直接用 `Add`/`Mul`/`Muls`/`Div` 处理 `LocalTensor<bfloat16_t>`（A2 不支持，编译/运行期报错或结果错误）。`ReduceSum` 同样需 fp32 输入。
- `Cast(fp32→bf16)` 用 `CAST_RINT`（2201 推荐）。

### 5.2 明确有坑的点

1. **bf16 计算不支持（A2）**——最大坑。所有 bf16 张量必须 Cast 到 fp32 才能进 Add/Mul/Muls/Div/ReduceSum。
2. **`MulDstAdd` 在 A2 为 ×**（仅 Atlas 350）——不能用，用 Add+Mul 替代。
3. **D 非 32B 对齐**——`DataCopy` 用字节级 `blockLen`（可非对齐），让框架丢弃尾部 dummy；不要手动圆整 UB 后整体搬出（会污染 GM，见 §3.2）。
4. **`DataCopyPadExtParams` 顺序歧义**——按成员名赋值（§3.1）。
5. **`ReduceSum` 是软件仿真**——大 D 归约可能慢；UB 中要预留 `sharedTmpBuffer`（公式见 §3.3），且最大==最小。
6. **`__vector__` 在 A2 耦合架构"不生效"**——模板用 `__global__ __vector__` 合法，但 A2 上等价于 `__aicore__`；不要依赖 `__vector__` 做架构分支逻辑。
7. **`PipeBarrier<PIPE_S>()` 会引发硬件错误**——标量流水同步由硬件自动保证，勿手动插（S051）。
8. **同步事件配对**：`SetFlag`/`WaitFlag` 必须成对；TPipe/TQue 场景下 eventID 必须用 `FetchEventID`/`AllocEventID` 获取，禁止自填（静态 Tensor 编程时 eventID 不能用 6/7）。

### 5.3 直接调用形态对应（与下发模板一致）

```cpp
extern "C" void run_kernel(/* 由 TensorGroupInfo 运行时读取 shape/rank/dtype */) {
    // 运行时分支 dtype ∈ {fp32, fp16, bf16}
    add_rms_norm_bias_custom<<<blockNum, nullptr, stream>>>(...);
}
// 核函数：
__global__ __vector__ void add_rms_norm_bias_custom(...) { /* 纯矢量逻辑 */ }
```
- `GetBlockNum()` 对应启动的 `blockNum`；`GetBlockIdx()` 做多核偏移：`offset = GetBlockIdx()*perBlock`。
- 所有 shape/rank/dtype 从 `TensorGroupInfo`（含 `int64_t* shape; int64_t numDims; int32_t dtype`，dtype 0=fp32/1=fp16/2=bf16）运行时读取并分支——非本 Agent 范畴，但与上述 API 选择直接耦合（bf16 分支必走 fp32 中转）。

---

## 6. 来源清单（S031–S060）

> 等级：A=官方 9.0.0/9.0.X；B=官方旧版/非目标产品；C=第三方；D=未确认。状态：verified/partial/unavailable/contradicted。

- [S031] LocalTensor 简介（GetValue/SetValue/GetSize）| file:///.../临时/api-pages/atlasascendc_api_07_0006.html（对应 hiascend 9.0.X）| 平台 hiascend | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：LocalTensor 单点读写与尺寸 API | 可支持结论：LocalTensor 提供 GetValue/SetValue/GetSize（GetLength 已弃用）。
- [S032] GlobalTensor 简介（SetGlobalBuffer）| file:///.../临时/api-pages/atlasascendc_api_07_0007.html | 平台 hiascend | 2026-09-12 | A | verified | 用途：GM 绑定 | 可支持结论：`SetGlobalBuffer(__gm__ T*, uint64_t)` 与无长度重载。
- [S033] DataCopyPad(ISASI) 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0265.html | hiascend | 2026-09-12 | A | verified | 用途：非对齐搬运与 padding 表、UB→GM 无 padParams、Q1/Q2 | 可支持结论：padding 仅 GM→UB；表 6 顺序=选项 A；UB→GM 自动丢弃 dummy。
- [S034] ReduceSum 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0078.html | hiascend | 2026-09-12 | A | verified | 用途：归约 count/sharedTmpBuffer/A2 支持 | 可支持结论：count 无硬上限数字；A2 仅 half/float；sharedTmpBuffer 公式给出。
- [S035] Cast 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0073.html | hiascend | 2026-09-12 | A | verified | 用途：roundMode 与 bf16 互转（Q5） | 可支持结论：fp32→bf16 支持多 roundMode；bf16/fp16→fp32 无损。
- [S036] Sqrt 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0029.html | hiascend | 2026-09-12 | A | verified | 用途：开方 API | 可支持结论：`Sqrt(dst,src,count)` 前 n 个数据计算。
- [S037] Rsqrt 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0030.html | hiascend | 2026-09-12 | A | verified | 用途：开方取倒数（推荐） | 可支持结论：`Rsqrt(dst,src,count)`。
- [S038] Muls 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0055.html | hiascend | 2026-09-12 | A | verified | 用途：矢量×标量、A2 不支持 bf16（Q4） | 可支持结论：A2 支持 half/int16_t/float/int32_t，无 bf16。
- [S039] MulDstAdd 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_00007.html | hiascend | 2026-09-12 | A | verified | 用途：融合乘加（SimD RegTensor） | 可支持结论：仅 Atlas 350 支持（A2 ×），不能用于 A2。
- [S040] TQue 简介 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0136.html | hiascend | 2026-09-12 | A | verified | 用途：队列 AllocTensor/EnQue/DeQue/FreeTensor | 可支持结论：三段式流水 API 完整；队列深度推荐 1。
- [S041] 核函数限定符 / 语言扩展层（__vector__/__aicore__/GetBlockIdx/GetBlockNum/HardEvent V_MTE3）| file:///.../临时/api-pages/agent02_vector_qualifier_850a002.html | hiascend | 2026-09-12 | A | verified | 用途：限定符表、多核变量、同步示例 | 可支持结论：`__vector__` 仅 Vector 核、耦合架构不生效；`__global__` 须配 `__aicore__`；GetBlockIdx/GetBlockNum 可用。
- [S042] GetReduceSumMaxMinTmpSize 9.0.0 | file:///.../临时/api-pages/agent02_GetReduceSumMaxMinTmpSize_900.html | hiascend | 2026-09-12 | A | verified | 用途：host 侧 sharedTmpBuffer 大小（Q3） | 可支持结论：最大临时空间=最小临时空间；host 侧 `ge::Shape` 接口。
- [S043] 什么是 Ascend C（编程指南）9.0.0 | file:///.../临时/api-pages/guide_index.html | hiascend | 2026-09-12 | A | partial | 用途：支持产品、SIMD/SIMT 模型 | 可支持结论：支持 Atlas A2 等；dav-2201 支持完整 Memory 矢量 UB 编程。
- [S044] SetFlag/WaitFlag(ISASI)（含 HardEvent 枚举、PipeBarrier）9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0270.html | hiascend | 2026-09-12 | A | verified | 用途：HardEvent 枚举、eventID 范围、A2 支持 | 可支持结论：`enum HardEvent{MTE2_V,V_MTE2,MTE3_V,V_MTE3,S_V,V_S,...}`；A2 eventID 0–7；SetFlag/WaitFlag 须成对。
- [S045] DataCopyPad（华为开发者联盟/鸿蒙）| https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-datacopypad | huawei | 2026-09-12 | B | verified | 用途：Q1 表 5 顺序佐证 | 可支持结论：顺序 isPad/leftPadding/rightPadding/paddingValue。
- [S046] 如何高效处理 Ascend C 非对齐数据（技术文章）| https://www.hiascend.com/developer/techArticles/20250627-1 | hiascend | 2026-09-12 | C | verified | 用途：Q2 UB→GM dummy 丢弃 | 可支持结论：搬出时框架补 dummy、写 GM 时丢弃。
- [S047] AscendC DataCopyPad 32 字节对齐（掘金/社区解析）| https://juejin.cn/post/7682716879561687050 | 第三方 | 2026-09-12 | C | verified | 用途：Q1 结构体顺序解析、Q2 dummy 丢弃细节 | 可支持结论：结构体字段顺序=选项 A；UB→GM 丢弃 dummy。
- [S048] Add 自定义算子开发概述（CANN 9.0.0-beta.2）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta2/opdevg/Ascendcopdevg/atlas_ascendc_10_0032.html | hiascend | 2026-09-12 | A(β2) | verified | 用途：Q4 Add 不支持 bf16（A2） | 可支持结论：A2 上 Add 不支持 bfloat16_t，须 Cast 到 float。
- [S049] Add（基础算术）8.1RC1 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/apiref/ascendcopapi/atlasascendc_api_07_0035.html | hiascend | 2026-09-12 | B | verified | 用途：Add 签名与 A2 产品表 | 可支持结论：`Add(dst,src0,src1,calCount)`；A2 half/int16_t/float/int32_t（无 bf16）。
- [S050] DataCopy 基础数据搬运（8.5.0 + 华为云博客）| https://www.hiascend.com/document/detail/en/canncommercial/latest/API/ascendcopapi/atlasascendc_api_07_0038.html 与 https://bbs.huaweicloud.com/blogs/436918 | hiascend/华为云 | 2026-09-12 | B | partial | 用途：DataCopy 基础签名 | 可支持结论：`DataCopy(dst,src,calCount)` 与 DataCopyParams 重载；GM↔UB A2 支持。
- [S051] PipeBarrier(ISASI)（PIPE_V/PIPE_S/PIPE_MTE2/PIPE_MTE3）| https://developer.huawei.com/consumer/cn/doc/hiai-References/cannkit-pipebarrier-0000002479486693 | 华为 | 2026-09-12 | B | verified | 用途：流水类型定义 | 可支持结论：PIPE_MTE2=GM→UB，PIPE_MTE3=UB→GM，PIPE_V=矢量，PIPE_S=标量；`PipeBarrier<PIPE_S>()` 报错。
- [S052] Div（基础算术）| https://developer.huawei.com/consumer/cn/doc/hiai-References/cannkit-vector-calculation-binocular-div-0000002158595293 与 8.5.0 atlasascendc_api_07_0038 | 华为/hiascend | 2026-09-12 | B | verified | 用途：Div 签名 | 可支持结论：`Div(dst,src0,src1,calCount)`；支持 half/float（A2 无 bf16）。
- [S053] Divs（双目标量）| http://www.hqwc.cn/a/422816.html（引自 CANN 文档）| 第三方 | 2026-09-12 | C | verified | 用途：Divs 签名 | 可支持结论：`Divs(dst,src0,src1,count)`。
- [S054] GetBlockDim 不存在说明 | https://wenku.csdn.net/answer/6pw24npu8h17 与 8.0.0 API 列表 | CSDN/hiascend | 2026-09-12 | C/B | verified | 用途：澄清 GetBlockDim 非标准 API | 可支持结论：仅 GetBlockNum/GetBlockIdx 标准；GetBlockDim 不存在。
- [S055] 9.0.X 基础 API 列表（LocalTensor/GlobalTensor 方法清单）| https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10432.html | hiascend | 2026-09-12 | A | verified | 用途：LocalTensor/GlobalTensor 方法清单 | 可支持结论：LocalTensor 含 SetValue/GetValue/GetSize；GlobalTensor 含 SetGlobalBuffer/GetValue/SetValue/GetSize。
- [S056] Built-in Data Types 9.0.0 | https://www.hiascend.com/document/detail/en/CANNCommunityEdition/latest/API/ascendcopapi/atlas_ascendc_10_0019.html | hiascend | 2026-09-12 | A | verified | 用途：bf16 是否为内置类型 | 可支持结论：A2 支持 bfloat16_t 作为内置类型（但计算算子另有限制，见 Q4）。
- [S057] Add 样例（asc-devkit，产品与 CANN 版本）| https://gitcode.com/cann/asc-devkit/blob/master/examples/01_simd_cpp_api/00_introduction/01_add/add/README.md | asc.gitcode | 2026-09-12 | B | verified | 用途：A2 支持 ≥ CANN 9.0.0 | 可支持结论：Atlas A2 训练/推理 ≥ CANN 9.0.0 支持 Add 样例。
- [S058] Ascend C API 列表（TPipe/TBuf/TQue/Sync）8.5.0 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha001/API/ascendcopapi/atlasascendc_api_07_10251.html | hiascend | 2026-09-12 | B | verified | 用途：TPipe/TBuf 在 API 列表中 | 可支持结论：TPipe.InitBuffer、TBuf、TQue、PipeBarrier、SetFlag/WaitFlag、GetBlockNum/GetBlockIdx 均为标准 API。
- [S059] Memory 矢量计算编程（静态/动态 Tensor，`__global__ __vector__` 示例，dav-2201）| https://asc.gitcode.com/guide/编程指南/编程模型/AI-Core-SIMD编程/基于Tensor的CPP编程/Memory矢量计算编程.html | asc.gitcode | 2026-09-12 | B | verified | 用途：纯矢量核 `__global__ __vector__` 写法、dav-2201 支持 | 可支持结论：dav-2201 支持完整 Memory 矢量 UB 编程；示例用 `__global__ __vector__`。
- [S060] DataCopyPad（英文 8.5.0，Table 5 顺序）| https://www.hiascend.com/document/detail/en/canncommercial/latest/API/ascendcopapi/atlasascendc_api_07_0265.html | hiascend | 2026-09-12 | B | verified | 用途：Q1 英文表顺序佐证 | 可支持结论：isPad/leftPadding/rightPadding/paddingValue 顺序与选项 A 一致。

---

## 附：版本差异提示（验收强制项）

- 凡标注 `8.x` / `8.5.0` / `8.1RC1` / `9.0.0-beta.2` / `9.1.0-beta.2` 的来源，均**非目标判题版本 9.0.0 正式版**；其签名/接口大多稳定，但产品支持表（尤其 bf16）以 9.0.0-beta.2 与 9.0.0 正式文档（S033/S034/S035/S038/S041/S044/S048）为准。
- 不得把 8.x 结论直接写成 9.0.0 结论：本报告对 Add 不支持 bf16（A2）使用了 9.0.0-beta.2 官方明文（S048）+ 9.0.0 Muls 表（S038）双重佐证；Mul/Div 的 A2 不支持 bf16 为同族推断（B 级），实现时需以 9.0.0 正式文档或编译实测最终确认。
- 本机无 CANN/NPU，**未编译、未运行任何 Ascend C 代码**；所有"可用性"结论来自文档，最终以判题环境编译/运行验证为准。
