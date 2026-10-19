# Agent 02：官方 Ascend C API 文档证据

> 日期：2026-09-11
> 范围：CANN Community Edition 9.0.0（`zh/CANNCommunityEdition/900`，页面标注 `CANN9.0.X开发文档`）
> 对照代码：`源码/op_kernel/add_rms_norm_bias.cpp`、`提交/V002/kernel.asc`、比赛直调模板 `kernel.asc`
> 本机无 CANN / 无 NPU：下列内容全部来自官网文档原文抓取，**未做编译或运行验证**。

抓取方式：`webfetch` / `curl` 直接访问 hiascend.com 900 文档页；HTML 内嵌 JSON 已解码后抽取正文。原始页面缓存于 `临时/api-pages/`。

---

## 1. API 清单表

页 ID 均相对前缀：
`https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/`

| API | 页 ID | 9.0.0 函数原型（节选） | 关键约束 / dtype（A2/A3） | 与本题代码对应 |
| --- | --- | --- | --- | --- |
| **DataCopyPad** | `atlasascendc_api_07_0265.html` | GM→UB: `DataCopyPad(LocalTensor<T>& dst, GlobalTensor<T>& src, DataCopyExtParams, DataCopyPadExtParams<T>)`；UB→GM: `DataCopyPad(GlobalTensor<T>& dst, LocalTensor<T>& src, DataCopyExtParams)`（无 pad 参数） | LocalTensor 起始地址 **32B 对齐**；**GlobalTensor 起始地址无对齐约束**。A2/A3 dtype: half / **bfloat16_t** / float / int32 等。A2/A3 **不支持** `PaddingMode` 模板参数 | V001/V002 尾块搬入/搬出均用 DataCopyPad；源码注释“Global 目的地址无对齐约束”与文档一致 |
| **DataCopyExtParams** | 同上页表4 | `blockCount∈[1,4095]` (uint16)；`blockLen` 单位**字节** `[1,2097151]` (uint32)；`srcStride`/`dstStride`：GM 侧单位字节，VECIN/VECOUT 侧单位 dataBlock(32B) | 结构体定义见 `${INSTALL_DIR}/include/ascendc/basic_api/interface/kernel_struct_data_copy.h` | 两版均用 `{1, len*sizeof(T), 0, 0, 0}`（单块、块长=len*类型大小、无 stride） |
| **DataCopyPadExtParams&lt;T&gt;** | 同上页表6 | 字段（文档表序 + 官方示例）：**`isPad, leftPadding, rightPadding, paddingValue`** | `isPad=true` 填自定义值；`false` 填随机值。left/rightPadding 单位为**元素个数**，且**所占字节数均不能超过 32B**。64 位类型 paddingValue 只能为 0。官方示例：`DataCopyPadExtParams<half> padParams{true, 0, 2, 0};` | **见 §2 陷阱 1**。V002 写法 `{isPad, 0, right_padding, 0}` 与官方字段序一致；V001 `{true,0,0,0}` 实际未补右 padding |
| **DataCopy**（基础对齐） | `atlasascendc_api_07_0101.html`（系列简介） | 简介页：支持 LM↔GM 基础/增强/切片搬运；基础搬运“数据保持原始格式和内容不变” | 对齐块使用；具体基础 `DataCopy(dst, src, len)` 子页未单独抓到原文（见 §4 未找到） | V001 对齐块用 `DataCopy(dst, src, len)`；V002 全程改用 DataCopyPad |
| **ReduceSum** | `atlasascendc_api_07_0078.html` | tensor 前 n 个：`ReduceSum(dst, src, sharedTmpBuffer, count)`；高维：`..., mask, repeatTime, srcRepStride` | **参数名在 9.0.0 为 `sharedTmpBuffer`**（不是 workLocal）。A2/A3 dtype: **half / float**（**无 bfloat16_t**）。dst 对齐 half=2B / float=4B；src 与 sharedTmpBuffer 需 **32B 对齐**。count 受 UB 上限约束 | 两版均 `ReduceSum(sum, yF, work, len)`，work 即 sharedTmpBuffer；源码注释仍写 “workLocal” 为旧称 |
| **sharedTmpBuffer 大小** | 同上页 | 方式一最小空间：`firstMaxRepeat = count/elementsPerRepeat`（count&lt;elementsPerRepeat 时为 1）；float: elementsPerRepeat=64, elementsPerBlock=8；`finalNeed = RoundUp(firstMaxRepeat, elementsPerBlock)*elementsPerBlock` | 方式二：传任意大小，sharedTmpBuffer 的值不会被改变 | tile=2048 float → firstMaxRepeat=32 → RoundUp 到 8 的倍数=32；`WORK_LEN=1024` 有余量 |
| **GetReduceRepeatSumSpr** | `atlasascendc_api_07_0225.html` | `template <typename T> __aicore__ inline T GetReduceRepeatSumSpr()` | 直接读 ReduceSum（tensor 前 n 个接口）结果；T 支持 half、float | 早期 V002 草稿曾用；**当前 V002 已改为 `sum.GetValue(0)` + V_S 同步** |
| **Cast** | `atlasascendc_api_07_0073.html` | `template <typename T, typename U> __aicore__ inline void Cast(const LocalTensor<T>& dst, const LocalTensor<U>& src, const RoundMode& roundMode, const uint32_t count)` | half→float、bf16→float：**无精度损失，无舍入**。float→half / float→bf16：按 roundMode 舍入，**溢出默认饱和**。A2/A3 支持与 bf16 互转 | 两版：上行 `CAST_NONE`，下行 `CAST_RINT` |
| **RoundMode** | 同上页 | `enum class RoundMode { CAST_NONE=0, CAST_RINT, CAST_FLOOR, CAST_CEIL, CAST_ROUND, CAST_TRUNC, CAST_ODD, CAST_HYBRID }` | `CAST_NONE`：**有精度损失时等同 CAST_RINT，无损时不舍入**。`CAST_RINT`：四舍六入五成双 | 上行 CAST_NONE 合法；下行 CAST_RINT 与“无损上行 / 显式下行”策略匹配 |
| **Add** | `atlasascendc_api_07_0035.html` | `Add(dst, src0, src1, count)` | A2/A3: **half, int16_t, int32_t, float** — **无 bfloat16_t**。Atlas 350 才有 bf16 | FP32 链使用；BF16 输入必须先 Cast |
| **Mul** | `atlasascendc_api_07_0037.html` | `Mul(dst, src0, src1, count)` | A2/A3: **half, int16_t, int32_t, float** — **无 bfloat16_t** | 平方、gamma 缩放均在 FP32 上做 |
| **Muls** | `atlasascendc_api_07_0055.html` | `Muls(dst, src, scalarValue, count)` | A2/A3: **half, int16_t, float, int32_t** — **无 bfloat16_t** | `Muls(yF, yF, scale, len)` 标量为 float，匹配 |
| **Sqrt** | `atlasascendc_api_07_0029.html` | `Sqrt(dst, src, count)` | 全系列产品：**half、float** — **无 bfloat16_t**。另有 Atlas 350 专用 `SqrtConfig` | 本题 rms 用标量 `sqrtf`，未调用向量 Sqrt |
| **Rsqrt** | `atlasascendc_api_07_0030.html` | `Rsqrt(dst, src, count)` | 全系列产品：**half、float** — **无 bfloat16_t** | 可用向量 Rsqrt 替代标量 `1/rms`；当前实现用标量 scale + Muls |
| **TPipe** | `atlasascendc_api_07_0108.html` | 简介：统一管理 Device 端内存与同步事件 | **一个 Kernel 函数必须且只能初始化一个 TPipe**。`InitBuffer` 给 TQue/TBuf 分配内存；`AllocEventID`/`ReleaseEventID` 管理事件 | 两版均一个 `TPipe pipe/tpipe` 成员 |
| **TQue** | `atlasascendc_api_07_0136.html` | `TQue<QuePosition::VECIN/VECOUT, BUFFER_NUM>` + `pipe.InitBuffer(que, num, len)` | 同一 TPosition 上 TQue Buffer 数量受 eventID 限制：**训练系列=4，A2/A3/AI Core=8**。`AllocTensor`/`EnQue`/`DeQue`/`FreeTensor` | 4×VECIN + 1×VECOUT，BUFFER_NUM=1，远低于 8 |
| **TBuf** | `atlasascendc_api_07_0160.html` | 简介页；用法 `TBuf<TPosition::VECCALC>` + `pipe.InitBuffer(buf, len)` + `buf.Get<float>()` | 临时计算缓冲，无队列同步 | xF/rF/yF/work/sum 均为 TBuf |
| **LocalTensor::GetValue** | `atlasascendc_api_07_0006.html` | `__inout_pipe__(S) PrimType GetValue(const uint32_t offset) const` | 标量读，走 **S 流水**；读向量结果前需 V→S 同步 | 源码与当前 V002：`sum.GetValue(0)` 前后插 V_S / S_V |
| **LocalTensor::SetValue** | 同上页 | `template <typename T1> __inout_pipe__(S) void SetValue(const uint32_t index, const T1 value) const` | 标量写，S 流水 | 本题未用 |
| **PipeBarrier** | `atlasascendc_api_07_0271.html` | `template <pipe_t pipe> __aicore__ inline void PipeBarrier()` | **`PipeBarrier<PIPE_S>()` 会引发硬件错误**。直调/自定义算子工程**默认开启自动同步**，编译器可自动插 PIPE_V | 两版均有 `PipeBarrier<PIPE_V>()`（多余但无害） |
| **SetFlag / WaitFlag** | `atlasascendc_api_07_0270.html` | `template <HardEvent event> void SetFlag(int32_t eventID)`；`WaitFlag` 同形 | HardEvent 枚举含 **`V_S`、`S_V`**。TPipe 场景 **eventID 必须用 `FetchEventID` 获取，禁止自指定**。静态 Tensor 场景 eventID 不能用 6、7 | 两版均 `GetTPipePtr()->FetchEventID(HardEvent::V_S)` |
| **GetBlockIdx** | `atlasascendc_api_07_0185.html` | `__aicore__ inline int64_t GetBlockIdx()` | 返回 `[0, 用户配置的 NumBlocks)`；AIC/AIV 1:2 时 AIV 范围翻倍 | 两版多核按行均分 |
| **GetBlockNum** | `atlasascendc_api_07_0184.html` | `__aicore__ inline int64_t GetBlockNum()` | 返回当前任务配置的核数 | 同上 |
| **REGISTER_TILING_DEFAULT** | `atlasascendc_api_07_00003.html` | `REGISTER_TILING_DEFAULT(TILING_STRUCT)` | 约束原文：**「暂不支持 Kernel 直调工程」**。若结构体在命名空间内需带作用域符 | 源码（标准工程）使用；**直调模板不用** |
| **GET_TILING_DATA_WITH_STRUCT** | `atlasascendc_api_07_0215.html` | `GET_TILING_DATA_WITH_STRUCT(struct_name, tiling_data, tiling_arg)` | 约束原文：**「暂不支持 Kernel 直调工程」** | 同上；V002 直调改为 host 侧解析 TensorGroupInfo 后传标量参数 |
| **GetTPipePtr** | `atlasascendc_api_07_0120.html` | 取当前 TPipe 指针，用于 FetchEventID 等 | — | 两版同步代码均使用 |

### 直调启动与入口属性（模板证据，非 API 页）

| 项 | 证据 | 说明 |
| --- | --- | --- |
| `extern "C" void run_kernel(...)` | 比赛模板 `kernel.asc` L20 | 参数：五组 `GM_ADDR + TensorGroupInfo`，`int64_t availableCoreNum`，`aclrtStream stream`，`float epsilon` |
| `__global__ __vector__` | 模板注释 L13；V002 L381 | 直调 vector kernel 入口属性 |
| `__global__ __aicore__` | 源码 L273；REGISTER/GET_TILING 示例 | 标准 msopgen / 自定义算子入口属性 |
| `<<<blocks, nullptr, stream>>>` | 模板注释 L24；V002 L403 | 直调启动语法；blocks 由 `availableCoreNum` 截断到 `outer` |
| `TensorInfo.dtype` | 模板注释 L10 | `0=fp32 1=fp16 2=bf16`（与源码自定义 `DT_FP16=1/DT_BF16=2/DT_FP32=3` 不同，直调侧用模板编码） |

---

## 2. 关键陷阱

### 2.1 DataCopyPadExtParams 字段顺序（高风险）

任务背景里流传的顺序是 `isPad, paddingValue, leftPadding, rightPadding`。  
**CANN 9.0.0 官方表6 + 官方调用示例不支持这个顺序。**

文档表6顺序：

```text
isPad → leftPadding → rightPadding → paddingValue
```

官方示例（DataCopyPad 页）：

```cpp
AscendC::DataCopyPadExtParams<half> padParams{ true, 0, 2, 0 };
// isPad=true, leftPadding=0, rightPadding=2, paddingValue=0
```

对应代码判断：

| 代码 | 初始化列表 | 按官方字段序的含义 | 结论 |
| --- | --- | --- | --- |
| 源码 V001 | `{true, 0, 0, T(0)}` | isPad=true, left=0, **right=0**, value=0 | 右 padding 未补；tail 仅靠 `blockLen=len*sizeof(T)` 非对齐搬入，不把 LocalTensor 尾部补到 32B |
| 提交 V002 | `{right!=0, 0, right, T(0)}` | isPad, left=0, **right=right_padding**, value=0 | 与官方字段序一致，尾部补 0 到 32B 边界 |

**风险**：若有人按 `isPad, paddingValue, leftPadding, rightPadding` 改写，会把 right_padding 写进 leftPadding、把 0 写进 rightPadding，尾块语义静默出错。  
文档另注：结构体 C++ 声明在 `kernel_struct_data_copy.h`；本机无安装包，**无法用头文件做字节级复核**，但文档表序与 brace-init 示例已互相印证。

### 2.2 BF16：搬运/转换可以，矢量算术不行（A2/A3）

| API | A2/A3 是否支持 bfloat16_t |
| --- | --- |
| DataCopyPad | **支持** |
| Cast（float↔bf16） | **支持**（文档精度转换表有专节） |
| Add / Mul / Muls | **不支持**（列表为 half/int16/int32/float） |
| Sqrt / Rsqrt | **不支持**（仅 half/float） |
| ReduceSum | **不支持**（仅 half/float） |

结论：BF16 路径必须 **搬入 bf16 → Cast 到 FP32 → Add/Mul/Muls/ReduceSum（FP32）→ Cast 回 bf16 → 搬出**。  
两版实现都遵循这一点；**禁止**直接 `Add(bf16, bf16, bf16)` 或 `ReduceSum<bfloat16_t>`。

### 2.3 ReduceSum 参数名与结果读取

- 9.0.0 参数名是 **`sharedTmpBuffer`**。源码注释里的 “workLocal” 是旧叫法，不影响编译（参数位置相同），但检索旧教程时不要当成另一套 API。
- 结果两种读法都有官方依据：
  1. `dst.GetValue(0)`（LocalTensor 成员，S 流水）—— 需要 `HardEvent::V_S`；
  2. `GetReduceRepeatSumSpr<T>()`（专页 0225）—— 官方示例直接跟在 ReduceSum 后。
- 当前 V002 用的是 (1)，同步写法与 SetFlag/WaitFlag 页示例同构，可保留；若去掉 GetValue，可改 (2) 并简化 V_S/S_V。

### 2.4 直调工程禁用 Tiling 宏

`REGISTER_TILING_DEFAULT`、`GET_TILING_DATA_WITH_STRUCT` 两页**约束说明均写明「暂不支持 Kernel 直调工程」**。

因此：

- 标准 msopgen 工程（源码/）可用 tiling 宏；
- 比赛直调（提交/V00x/kernel.asc）**不能**依赖 tiling 宏，必须走 `run_kernel` + `TensorGroupInfo` + 启动参数下发 outer/dim/eps/dtype。V002 这样做是正确方向。

### 2.5 `__vector__` vs `__aicore__`

- 直调模板要求入口：`extern "C" __global__ __vector__ void ...`
- 标准自定义算子 / tiling 示例：`extern "C" __global__ __aicore__ void ...`
- 成员函数（Init/Process/CopyIn…）统一 `__aicore__ inline`，两版一致。
- 官方 API 页本身不展开 `__vector__` 属性说明；**以比赛模板注释为准**（A 级：赛方模板）。

### 2.6 同步与流水

| 规则 | 文档依据 | 代码含义 |
| --- | --- | --- |
| `PipeBarrier<PIPE_S>()` 会硬件错误 | PipeBarrier 页 | 不要对 S 流水加 barrier |
| TPipe 场景 eventID 必须 FetchEventID | SetFlag/WaitFlag 页 | 禁止写死 `SetFlag<V_S>(0)` |
| GetValue 走 S 流水 | LocalTensor 页 `__inout_pipe__(S)` | 读向量归约结果前要 V_S |
| 直调工程默认自动同步 | PipeBarrier 页注 | 手写 `PipeBarrier<PIPE_V>` 冗余但不违规 |

### 2.7 对齐与尾块

- DataCopy：对齐块使用（V001 分支）。
- DataCopyPad：非对齐搬入/搬出；**Global 端无对齐约束**；Local 端 32B。
- left/rightPadding 字节数 ≤32：half 一次最多补 16 个元素，float 最多 8 个。D 非对齐尾块只要单块补齐到下一 32B 边界即可，V002 `AlignedLen` 按 `32/sizeof(T)` 向上取整，落在限制内。
- 向量计算 count：文档未给死上限，只写“不能超过 UB 大小限制”。tile：fp16/bf16=4096、fp32=2048 需在真机上按 UB 复核（见 Agent07）。

### 2.8 TQue 与 TPipe 数量

- 一 Kernel 仅一个 TPipe。
- A2/A3 同一 TPosition 上 TQue 有效数量受 eventID=8 限制；BUFFER_NUM=1 时 5 个队列安全。
- 若开 double buffer（BUFFER_NUM=2），可申请队列数减半，仍足够。

### 2.9 dtype 编码不一致

| 侧 | fp32 | fp16 | bf16 |
| --- | --- | --- | --- |
| 比赛模板 `TensorInfo.dtype` | 0 | 1 | 2 |
| 源码 tiling 自定义 `DT_*` | 3 | 1 | 2 |

直调 V002 用模板编码（0/1/2）；标准工程源码用 1/2/3。两套不可混用。

---

## 3. 版本差异（9.0.0 相对旧资料 / 直调相关）

| 主题 | 9.0.0 文档状态 | 对本题影响 |
| --- | --- | --- |
| ReduceSum 第三参数名 | **`sharedTmpBuffer`**（0078 页正文与原型一致） | 旧教程写 `workLocal` 时按位置传入即可；新文档检索用 sharedTmpBuffer |
| Tiling 宏 × 直调 | REGISTER_TILING_DEFAULT / GET_TILING_DATA_WITH_STRUCT **明确写「暂不支持 Kernel 直调工程」** | 直调包不得依赖 tiling 宏 |
| DataCopyPad 模板 `PaddingMode` | A2/A3 **「否」**（是否支持设置 mode）；仅 Atlas 350 为「是」 | 不要写 `DataCopyPad< T, PaddingMode::Compact >` |
| bf16 矢量算术 | Add/Mul/Muls/Sqrt/Rsqrt/ReduceSum 在 A2/A3 **均无 bf16** | BF16 必须 FP32 中间计算 |
| ISASI 标注 | 站点 latest 文档出现 `*_ISASI` 路径（cube 等）；**9.0.0 的 0265/0078/0073 等向量页正文未单独出现 “ISASI” 标记**，支持度按「产品型号表」列出 | 本题只用 VEC 通路，不依赖 ISASI 专有路径 |
| 自动同步 | PipeBarrier 页：Kernel 直调算子工程和自定义算子开发工程**已默认开启自动同步** | 手写 PIPE_V barrier 可简化，但非错误 |
| Cast RoundMode | 9.0.0 含 CAST_NONE/RINT/FLOOR/CEIL/ROUND/TRUNC/ODD/HYBRID | 与两版使用的 NONE/RINT 兼容 |

---

## 4. 已确认 / 未找到 / 无法确认

### 已确认（有 9.0.0 官方页原文）

1. DataCopyPad 两条通路原型、Global 无对齐约束、Local 32B、A2/A3 支持 bf16。
2. DataCopyExtParams 字段含义与取值范围。
3. DataCopyPadExtParams 文档表序与官方 brace-init 示例一致：`isPad, leftPadding, rightPadding, paddingValue`。
4. ReduceSum 原型第三参数名为 `sharedTmpBuffer`；A2/A3 仅 half/float；sharedTmpBuffer 最小空间算法。
5. GetReduceRepeatSumSpr 原型与用法。
6. Cast 原型、RoundMode 枚举、CAST_NONE 语义、bf16 转换规则（含饱和）。
7. Add / Mul / Muls / Sqrt / Rsqrt 在 A2/A3 的 dtype 表（**均无 bf16**）。
8. PipeBarrier 约束（PIPE_S 禁止）、直调默认自动同步。
9. SetFlag/WaitFlag + HardEvent::V_S/S_V + FetchEventID 强制要求。
10. GetBlockIdx / GetBlockNum 原型与返回值范围。
11. LocalTensor::GetValue / SetValue 为 S 流水接口。
12. TPipe 单实例约束；TQue eventID 数量（train=4，A2/A3=8）。
13. REGISTER_TILING_DEFAULT、GET_TILING_DATA_WITH_STRUCT **不支持 Kernel 直调工程**。
14. 直调入口/启动形式来自赛方模板：`run_kernel` + `__global__ __vector__` + `<<<blocks,nullptr,stream>>>`。

### 未找到（本轮未抓到独立官方页原文）

1. **DataCopy 基础对齐版**的独立函数页（0101 为 DataCopy 系列简介；子页未展开抓取）。签名可从简介与大量官方样例反推，但本报告不把它标为“已读原型原文”。
2. **InitBuffer / FetchEventID / AllocEventID** 的独立 API 页（在 TPipe 简介与 SetFlag 页中交叉引用，未单独打开）。
3. **`__vector__` 属性**的官方 API/编程指南专页（900 站内本轮未定位到；以赛方模板为 A 级依据）。
4. `kernel_struct_data_copy.h` 头文件本体（文档指向 `${INSTALL_DIR}/include/...`，本机无 CANN 安装包）。

### 无法确认（缺少真机/安装包）

1. DataCopyPadExtParams 的 **C++ 成员声明顺序**是否与文档表6顺序字节级一致（文档表序 + 官方示例强烈支持，但无头文件）。
2. `workLocal → sharedTmpBuffer` 更名引入的**精确小版本**（9.0.0 文档已用新名；8.x 资料多用旧名）。
3. 直调工程中 tiling 宏是否在某个 9.0.0 补丁解禁（当前 900 文档仍写“暂不支持”）。
4. 任意 API 在目标 SoC 上的**编译、精度、性能**（本机无 CANN/NPU，禁止声称已验证）。
5. `Register/GET_TILING` 示例里的 `__aicore__` 入口与直调 `__vector__` 在毕昇编译器层面的完整差异表（需真机编译器文档或实测）。

---

## 5. 来源清单

全部为 **A 级（昇腾官网 CANN 9.0.0 文档）**，访问日期 **2026-09-11**。

| # | 标题 | URL |
| --- | --- | --- |
| 1 | CANN 社区版 9.0.0 文档首页 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/ |
| 2 | Ascend C API 列表 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0003.html |
| 3 | DataCopyPad | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0265.html |
| 4 | ReduceSum | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0078.html |
| 5 | Cast | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0073.html |
| 6 | Add | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0035.html |
| 7 | Mul | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0037.html |
| 8 | Muls | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0055.html |
| 9 | Sqrt | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0029.html |
| 10 | Rsqrt | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0030.html |
| 11 | GetBlockIdx | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0185.html |
| 12 | GetBlockNum | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0184.html |
| 13 | SetFlag / WaitFlag | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0270.html |
| 14 | PipeBarrier | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0271.html |
| 15 | GetReduceRepeatSumSpr | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0225.html |
| 16 | TPipe 简介 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0108.html |
| 17 | TQue | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0136.html |
| 18 | LocalTensor（含 GetValue/SetValue） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0006.html |
| 19 | REGISTER_TILING_DEFAULT | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_00003.html |
| 20 | GET_TILING_DATA_WITH_STRUCT | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0215.html |
| 21 | DataCopy 系列简介 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0101.html |

本地对照（非官网，项目内）：

| 文件 | 用途 |
| --- | --- |
| `源码/op_kernel/add_rms_norm_bias.cpp` | 标准工程实现（tiling 宏 + `__aicore__` 入口） |
| `提交/V002/kernel.asc` | 直调提交候选（`__vector__` 入口 + GetValue 同步） |
| `~/Downloads/addrmsnormbias_problem_1742_template/kernel.asc` | 赛方直调模板（run_kernel / `__vector__` / dtype 编码） |
| `临时/api-pages/*.html` | 本轮抓取的 900 页 HTML 原文缓存 |

---

## 6. 对后续实现的直接建议（仅 API 层，不改仓库）

1. **BF16 保持 FP32 中间链**：官方 A2/A3 矢量算术与 ReduceSum 均无 bf16，现有 Cast 上行/下行策略必须保留。
2. **DataCopyPadExtParams 按 `isPad, leftPadding, rightPadding, paddingValue` 书写**；若引用其他笔记里的另一种顺序，以本报告 §2.1 为准。
3. **直调包不要加 tiling 宏**；保持 `run_kernel` 解析 `TensorGroupInfo` 后传参。
4. 归约结果读取二选一：`GetValue(0)+V_S` 或 `GetReduceRepeatSumSpr<T>()`；不要同时依赖两者语义。
5. `PipeBarrier<PIPE_V>` 可保留；**禁止** `PipeBarrier<PIPE_S>`。
6. 源码注释中的 “workLocal” 建议在下次改源码时改成 `sharedTmpBuffer`，避免与 9.0.0 文档检索脱节（非功能问题）。
