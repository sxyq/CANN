# 调研 2 · Agent 02：官方 Ascend C API 逐项核对报告

- 任务：对 AddRmsNormBias 用到的全部 Ascend C API 逐项核对官方文档（hiascend.com CANNCommunityEdition/900 即 CANN 9.0.X、asc.gitcode.com 官方镜像/官方仓库 cann/asc-devkit），复核上轮三项定案，核实入口限定符、A2 bfloat16 支持、fp32→bf16 Cast RoundMode，并对照本地代码用法。
- 日期：2026-09-12（访问日期均为当日）
- 比对对象：
  - `源码/op_kernel/add_rms_norm_bias.cpp`（下称"op_kernel"）
  - `提交/V002/kernel.asc`（下称"V002"，直调模式，`run_kernel` host 入口 + `__global__ __vector__` 核函数）
- 证据等级：A=官方文档页面/官方文档仓库原文（本地缓存 HTML 已解码正文核读，或 gitcode 官方镜像全文抓取）；B=官方样例源码；C=社区；D=仅摘要。
- 版本口径说明：本地缓存 `临时/api-pages/atlasascendc_api_07_*.html` 页面内嵌元数据均为 `zh/CANNCommunityEdition/900`（版本名 9.0.X，即 CANN 9.0.0 社区版文档线），与比赛目标 CANN 9.0.0 一致，本报告直接按 9.0.X 引用。hiascend.com 对 `/document/detail/zh/CANNCommunityEdition/900/...` 路径有 cookie 墙，无法直接抓取，但本地缓存即该版本页面存档；gitcode `asc.gitcode.com/api/...` 与官方仓库 `gitcode.com/cann/asc-devkit` 为官方最新文档镜像（演进到 9.x 后期），用作交叉验证与增量信息补充，涉及差异处均单独标注。

---

## 一、API 总表

| # | API | 关键签名（9.0.X） | 支持产品（A2/dav-2201） | 文档版本 | 来源 | 证据等级 | 本地代码一致性 |
|---|-----|------------------|------------------------|---------|------|---------|--------------|
| 1 | GlobalTensor | `template<typename T> class GlobalTensor`；成员 `SetGlobalBuffer(__gm__ PrimType* buffer, uint64_t bufferSize)` / `SetGlobalBuffer(__gm__ PrimType* buffer)`、`GetPhyAddr()`、`GetValue(offset)`（`__inout_pipe__(S)`）、`SetValue(offset, value)`、`GetSize()`、`operator()(offset)` | 使用处随指令 | 9.0.X | 缓存 atlasascendc_api_07_0007.html | A | **一致**。op_kernel 用 `SetGlobalBuffer(reinterpret_cast<__gm__ T*>(x), outer*D)`（双参版）；V002 用单参版 `SetGlobalBuffer((__gm__ T*)x)`，均与签名匹配 |
| 2 | LocalTensor | 存放 Local Memory 数据；TPosition 支持 VECIN/VECOUT/VECCALC/A1/A2/B1/B2/CO1/CO2；成员 `GetValue`（`__inout_pipe__(S)`）、`SetValue`、`GetPhyAddr`、`operator[]`、`GetElementNum` 等 | 使用处随指令 | 9.0.X | 缓存 atlasascendc_api_07_0006.html | A | **一致**。`xFbuf.Get<float>()`、`sum.GetValue(0)` 均为文档成员 |
| 3 | SetGlobalBuffer | 见 #1（属 GlobalTensor 成员） | A2 支持 | 9.0.X | 缓存 0007 | A | 一致（op_kernel 双参、V002 单参均合法） |
| 4 | DataCopy（GM↔UB 连续搬运） | `template<typename T> __aicore__ inline void DataCopy(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const uint32_t count)`；UB→GM 对称。约束：`count * sizeof(T)` 需 32 字节对齐，否则搬运量**向下取整**；A2 数据类型含 half/bfloat16_t/float 等 | A2 支持（GM→VECIN、VECOUT→GM） | gitcode 最新（9.x 后期） | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopy_GMAndUB_continuous.html | A | **一致**。op_kernel 仅对 `len == tileLen`（tileLen·sizeof(T) 为 32 倍数）调用 DataCopy；V002 全部走 DataCopyPad，无裸 DataCopy |
| 5 | DataCopyPad（GM→UB 非对齐） | `template<typename T> __aicore__ inline void DataCopyPad(const LocalTensor<T>& dst, const GlobalTensor<T>& src, const DataCopyExtParams& dataCopyParams, const DataCopyPadExtParams<T>& padParams)`（另有 DataCopyParams/DataCopyPadParams 版本）。A2 上不可用 PaddingMode 模板参数（表 1：A2 支持 mode=否） | A2 支持（GM→VECIN/VECOUT 等） | 9.0.X | 缓存 atlasascendc_api_07_0265.html | A | **一致**。op_kernel/V002 均使用无 mode 的 DataCopyExtParams 版本；A2 上不需要也不可用 Compact 模式 |
| 6 | DataCopyPad（UB→GM 非对齐） | `template<typename T> __aicore__ inline void DataCopyPad(const GlobalTensor<T>& dst, const LocalTensor<T>& src, const DataCopyExtParams& dataCopyParams)`（**无 padParams**；最新文档另增 blockLen 必须为 sizeof(T) 整数倍约束） | A2 支持（VECIN/VECOUT→GM） | 9.0.X + gitcode 最新 | 缓存 0265；https://asc.gitcode.com/api/SIMD-API/.../DataCopyPad_UBToGM.html | A | **一致**。op_kernel/V002 的 Store 均用三参原型；blockLen=count·sizeof(T) 天然满足整数倍约束 |
| 7 | DataCopyExtParams | 字段顺序 `{blockCount, blockLen, srcStride, dstStride, rsv}`：blockCount∈[1,4095]（uint16_t）；blockLen∈[1,2097151]（uint32_t，**单位字节**，支持非对齐搬运）；srcStride/dstStride：VECIN/VECOUT 侧单位 dataBlock(32B)、GM 侧单位字节 | — | 9.0.X | 缓存 0265 表 4 | A | **一致**。op_kernel `DataCopyExtParams params{1, len*sizeof(T), 0, 0, 0}`；V002 构造函数同序 5 参。blockLen 单位为字节，两份代码均传 `count * sizeof(T)`，正确 |
| 8 | DataCopyPadExtParams\<T\> | 字段顺序 `{isPad, leftPadding, rightPadding, paddingValue}`；leftPadding/rightPadding 单位为元素个数、**字节数均不得超过 32 字节**；paddingValue 类型与源操作数一致（T），64 位类型时只能为 0 | — | 9.0.X | 缓存 0265 表 6 | A | **一致**。op_kernel `{true, 0, 0, static_cast<T>(0)}`；V002 按成员名赋值（isPad/leftPadding/rightPadding/paddingValue），rightPadding = 对齐补齐元素数（half≤15→30B、float≤7→28B，均 <32B），合规 |
| 9 | ReduceSum（tensor 前 n 个数据计算） | `template<typename T, bool isSetMask = true> __aicore__ inline void ReduceSum(const LocalTensor<T>& dst, const LocalTensor<T>& src, const LocalTensor<T>& sharedTmpBuffer, const int32_t count)`。A2 数据类型：**half/float**（无 bfloat16_t）。A2 相加方式：方式二（repeat 内二叉树、repeat 间顺序累加）；方式二下 sharedTmpBuffer 传入任意大小即可。dst 起始地址 4 字节对齐（float）；src/sharedTmpBuffer 32 字节对齐。count 上限仅受 UB 大小限制。count=0 时 A2 视为 NOP | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0078.html；gitcode ReduceSum 页交叉验证 | A | **一致**。op_kernel/V002 均在 FP32 上调用（A2 不支持 bf16 归约，代码先 Cast 提升，正确规避）；workLocal 预留远超公式最小值 |
| 10 | ReduceSum（高维切分计算） | mask 连续/逐 bit 两版：`ReduceSum(dst, src, sharedTmpBuffer, mask, repeatTime, srcRepStride)`；**repeatTime 支持 int32 范围**（不同于通用高维切分 API 的 uint8_t 0-255 上限，官方明示例外） | A2 支持 | 9.0.X | 缓存 0078；gitcode | A | 未使用（两份代码均用 tensor 前 n 个数据接口），不涉及 |
| 11 | GetReduceSumMaxMinTmpSize | Host 侧：`void GetReduceSumMaxMinTmpSize(const ge::Shape& srcShape, const ge::DataType dataType, ReducePattern pattern, bool isSrcInnerPad, bool isReuseSource, uint32_t& maxValue, uint32_t& minValue)`；预留空间不得小于 minValue；当前 maxValue==minValue；pattern 仅支持 AR/RA | 配合 ReduceSum（A2 适用） | 9.0.X（URL 版本字段 zh/CANNCommunityEdition/900） | 缓存 agent02_GetReduceSumMaxMinTmpSize_900.html；https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10160.html | A | **未采用**。op_host tiling 未调用该接口（静态预留 workLocal 1024 float），对 SIMD 基础版 ReduceSum 非必需；若改用带 pattern 的高阶 ReduceSum 则必须接入 |
| 12 | Cast | `template<typename T, typename U> __aicore__ inline void Cast(const LocalTensor<T>& dst, const LocalTensor<U>& src, const RoundMode& roundMode, const uint32_t count)`。RoundMode 枚举：CAST_NONE=0（有精度损失时等同 CAST_RINT）、CAST_RINT/FLOOR/CEIL/ROUND/TRUNC/ODD/HYBRID | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0073.html | A | **一致**。A2 支持矩阵：half→float=CAST_NONE；float→half=7 种含 CAST_RINT；float→bfloat16_t=CAST_RINT/FLOOR/CEIL/ROUND/TRUNC；bfloat16_t→float=CAST_NONE。op_kernel/V002 用法全部落在支持矩阵内（详见第四节） |
| 13 | Add | `template<typename T> __aicore__ inline void Add(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, const int32_t& count)`（另有高维切分版） | A2 支持，但数据类型仅 **half/int16_t/int32_t/float（无 bfloat16_t）** | 9.0.X | 缓存 atlasascendc_api_07_0035.html | A | **一致（设计正确）**。两份代码均以 float 实例调用 Add；若直接对 bfloat16_t 张量调用 Add，在 A2 上不被支持（见第四节） |
| 14 | Mul | 同 Add 形态：`Mul(dst, src0, src1, count)` | A2 支持；类型 half/int16_t/int32_t/float（**无 bfloat16_t**） | 9.0.X | 缓存 atlasascendc_api_07_0037.html | A | 一致（均 float 调用） |
| 15 | Muls | `Muls(dst, src, scalarValue, count)`（标量乘） | A2 支持；half/int16_t/float/int32_t（**无 bfloat16_t**） | 9.0.X | 缓存 atlasascendc_api_07_0055.html | A | 一致。op_kernel `Muls(yF, yF, scale, len)` 为 float；V002 的 fp32→fp32 拷贝改用 `Adds(dst, src, 0.0f, count)`（Adds 同族接口，A2 支持 float），因 A2 Cast 矩阵无 float→float 行，该替代合理 |
| 16 | Sqrt（向量版） | `template<typename T, const SqrtConfig& config = DEFAULT_SQRT_CONFIG> __aicore__ inline void Sqrt(const LocalTensor<T>& dst, const LocalTensor<T>& src, ...)`（tensor 前 n 个数据计算，默认 INTRINSIC 算法） | A2 支持；类型 half/float | 9.0.X | 缓存 atlasascendc_api_07_0029.html | A | 一致。V002 用 `Sqrt(scalarSlot, scalarSlot, 1)` 求 1/rms（float 槽）；op_kernel 用标量 `sqrtf`（math.h，亦合法） |
| 17 | Rsqrt（向量版） | `template<typename T, const RsqrtConfig& config> __aicore__ inline void Rsqrt(...)` | A2 支持；类型 half/float | 9.0.X | 缓存 atlasascendc_api_07_0030.html | A | 未使用（V002 用 Sqrt+Muls 组合；op_kernel 用标量 1/rms），可作后续优化候选 |
| 18 | TPipe / InitBuffer | `TPipe`：一个 Kernel 必须且只能初始化一个；`template<class T> __aicore__ inline bool InitBuffer(T& que, uint8_t num, uint32_t len)`（TQue，num=2 开 double buffer）；`template<TPosition bufPos> __aicore__ inline bool InitBuffer(TBuf<bufPos>& buf, uint32_t len)`；len 非 32 倍数自动向上补齐；**一个 kernel 全部 Buffer 数量之和 ≤64**；AllocEventID/ReleaseEventID/FetchEventID 管理事件 | A2 支持 | 9.0.X + gitcode 最新 | 缓存 atlasascendc_api_07_0108.html；https://asc.gitcode.com/api/SIMD-API/基础API/资源管理/TPipe/InitBuffer.html | A | **一致**。op_kernel 5×TQue + 5×TBuf；V002 1×TPipe + 11×TBuf（无 TQue，手动事件同步），总量均远低于 64 |
| 19 | TQue（AllocTensor/EnQue/DeQue/FreeTensor） | `template<TPosition pos, int32_t depth, ...> class TQue`；成员 AllocTensor/EnQue/DeQue/FreeTensor；文档注记：**VECIN 位置可申请 buffer 数量最大为 8**（不开 double buffer 时最多 8 个 TQue） | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0136.html | A | **一致**。op_kernel VECIN 上 4 个 TQue + VECOUT 1 个（≤8）；V002 不用 TQue（TBuf 直管 + HardEvent 手动同步，文档允许） |
| 20 | TBuf | `template<TPosition bufPos> class TBuf`；经 TPipe InitBuffer 初始化后 `Get<T>()` 取 LocalTensor；只能参与计算、不能入队出队；多个临时变量需多个 TBuf 分别 InitBuffer | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0160.html | A | 一致（op_kernel 5 个 VECCALC TBuf；V002 11 个） |
| 21 | SetFlag / WaitFlag（含 HardEvent） | `template<HardEvent event> __aicore__ inline void SetFlag(int32_t eventID)`；WaitFlag 同型。HardEvent 枚举含 S_V/V_S/V_MTE3/MTE3_V 等 34 个成员。**必须成对出现**。A2 eventID 范围 0-7；TPipe/TQue 编程场景下 eventID 须经 AllocEventID 或 FetchEventID 获取 | A2 支持（200I/500 A2、Vector Core 不支持） | 9.0.X | 缓存 atlasascendc_api_07_0270.html | A | **一致**。op_kernel 用 `GetTPipePtr()->FetchEventID(HardEvent::V_S/S_V)`；V002 用 `tpipe.AllocEventID<HardEvent::V_MTE3/MTE3_V>()`，两途径均为文档认可；均成对调用 |
| 22 | PipeBarrier | `template<pipe_t pipe> __aicore__ inline void PipeBarrier()`；阻塞相同流水；**Scalar 流水同步由硬件保证，调用 PipeBarrier\<PIPE_S\>() 会引发硬件错误**；PIPE_ALL 阻塞全部。注记：Kernel 直调工程与自定义算子工程**默认开启自动同步**（编译器自动插入 PIPE_V 同步，无需手动插） | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0271.html | A | **一致但冗余**。op_kernel/V002 手动插 PipeBarrier<PIPE_V>() 163 处（V002）；直调工程默认自动同步下为保守写法，无害但非必需（潜在性能冗余点） |
| 23 | GetBlockNum / GetBlockIdx | `__aicore__ inline int64_t GetBlockNum()`；`__aicore__ inline int64_t GetBlockIdx()`（**返回 int64_t**） | A2 支持 | 9.0.X | 缓存 atlasascendc_api_07_0184.html / 0185.html | A | 基本一致。V002 显式 `static_cast<uint64_t>(GetBlockIdx())` ✓；op_kernel `uint32_t coreIdx = GetBlockIdx()` 为隐式收窄（核数≪2^32，实际安全，严格类型口径下建议显式转换） |
| 24 | GetValue（Local/GlobalTensor 成员） | `PrimType GetValue(const uint64_t offset)`，带 `__inout_pipe__(S)` 标量流水注记（LocalTensor/GlobalTensor 各自成员） | 随 Tensor | 9.0.X | 缓存 0006 / 0007 | A | 一致。op_kernel `sum.GetValue(0)`（LocalTensor）；V002 36 处 GetValue 均在事件同步之后读取 |
| 25 | GET_TILING_DATA_WITH_STRUCT | `GET_TILING_DATA_WITH_STRUCT(struct_name, tiling_data, tiling_arg)`，宏展开；**约束：暂不支持 Kernel 直调工程**（9.0.X 明文） | A2 支持（标准算子工程） | 9.0.X | 缓存 atlasascendc_api_07_0215.html | A | **一致（注意适用面）**。op_kernel 属 msopgen 标准算子工程，可用；V002 为直调模式，**未使用**该宏（参数经 run_kernel 直传核函数），正确规避了该约束 |
| 26 | `__global__` / `__aicore__` / `__vector__` 限定符 | 8.5.0.alpha002：__global__ 标识核函数入口、必须返回 void、**同时需用 __aicore__ 修饰**；__vector__ 标识仅在 Vector 核执行、"针对耦合模式的硬件架构，该修饰符不生效"。gitcode 最新（9.x 后期）：核函数须 __global__ + 执行空间限定符四选一（__aicore__ 不区分核类型通常用于耦合模式 / __vector__ 仅 Vector / __cube__ / __mix__(cube,vec)），官方示例 `__global__ __vector__ void add_kernel(...)`；约束页示例含 `__global__ __vector__ __aicore__` 组合 | A2=耦合模式（AI Core 含 Cube+Vector，__NPU_ARCH__=2201） | 850alpha002（缓存）+ 83RC1alpha002（在线快照）+ gitcode 最新 | 缓存 agent02_vector_qualifier_850a002.html；https://asc.gitcode.com/guide/编程指南/编程模型/AI-Core-SIMD编程/核函数.html 等 | A（900 原页未直接取得，按三个版本点演进链佐证） | **一致**。op_kernel `__global__ __aicore__`（文档基准组合）；V002 `__global__ __vector__`（最新规范推荐写法，详见第三节） |
| 27 | kernel_operator.h | Ascend C 统一头文件（SIMD API 各接口头文件路径如 `basic_api/kernel_tpipe.h`、`basic_api/kernel_operator_vec_reduce_intf.h`、`basic_api/kernel_operator_data_copy_intf.h` 均由其聚合）；LocalTensor 简介页明示 `#include "kernel_operator.h"` | — | 9.0.X + gitcode | 缓存 0006（含 include 示例）；gitcode 各 API 页头文件路径 | A | 一致。op_kernel/V002 均 `#include "kernel_operator.h"` |

补充条目（非本地代码直接调用，但属核对范围）：

| # | API | 说明 | 来源 | 等级 |
|---|-----|------|------|------|
| 28 | REGISTER_TILING_DEFAULT | Tiling 默认注册宏，A2 支持 | 缓存 atlasascendc_api_07_00003.html（页面版本字段 8.2.RC1.alpha001 标题、正文为该宏；A2 √） | A |
| 29 | GetSysWorkSpacePtr | 获取系统 workspace 指针，A2 支持（200I/500 A2 不支持） | 缓存 atlasascendc_api_07_00174.html | A |
| 30 | MulDstAdd（复合指令，对照项） | Reg 矢量复合计算；A2 **不支持**（表列 x） | 缓存 atlasascendc_api_07_00007.html（9.0.X） | A |

---

## 二、三项定案复核详情

### 定案 1：DataCopyPadExtParams 字段顺序为 {isPad, leftPadding, rightPadding, paddingValue} —— **维持**

证据链（均为 CANN 9.0.X 官方页面，缓存文件 `临时/api-pages/atlasascendc_api_07_0265.html`，正文已从页面内嵌数据完整解码核读）：

1. **表 6「DataCopyPadExtParams\<T\>结构体参数定义」**按序给出四个字段：
   - `isPad`：是否填充用户自定义数据（true=填 paddingValue；false=默认填随机值）；
   - `leftPadding`：左侧补充的元素个数（字节数 ≤32B）；
   - `rightPadding`：右侧补充的元素个数（字节数 ≤32B）；
   - `paddingValue`：填充值，类型与源操作数一致（T），64 位类型时只能为 0。
2. **调用示例原文**：`AscendC::DataCopyPadExtParams<half> padParams{true, 0, 2, 0};` —— 与四字段顺序完全对应。
3. 文档明示结构体定义参见 `${INSTALL_DIR}/include/ascendc/basic_api/interface/kernel_struct_data_copy.h`（安装目录头文件为最终裁决源，本机无 CANN，未核对，列入未确认清单）。
4. 对照本地代码：
   - op_kernel：`DataCopyPadExtParams<T> pad{true, 0, 0, static_cast<T>(0)};` —— 顺序正确；
   - V002：按成员名逐一赋值（`padParams.isPad/.leftPadding/.rightPadding/.paddingValue`）—— 完全规避顺序风险。
   
结论：**维持**。字段顺序有 9.0.X 官方表格+官方示例双重印证。

### 定案 2：UB→GM 搬出时 UB 侧 padding（dummy）映射到 GM 时被丢弃、不写相邻内存 —— **维持**

证据链：

1. **CANN 9.0.X（缓存 0265）VECIN/VECOUT→GM 场景说明原文**："当每个连续传输数据块长度 blockLen 不满足 32 字节对齐，由于 Unified Buffer 要求 32 字节对齐，框架在搬出时会自动补充一些假数据来保证对齐，但在当搬到 GM 时会自动将填充的假数据丢弃掉。"并给出 blockLen=47 时补 17 字节 dummy、到 GM 丢弃的具体示例。
2. **gitcode 官方镜像 DataCopyPad（UB→GM 非对齐数据搬运）专页**（对应官方仓库 cann/asc-devkit `docs/zh/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopyPad_UBToGM.md`）原文："对于非 32 字节对齐的场景，在读取 Unified Buffer 数据时会填入 dummy 假数据，对齐到 32B，搬入 Global Memory 时会将 dummy 空数据丢弃，从而实现 Unified Buffer 到 Global Memory 的非对齐搬运。"——最新文档口径与 9.0.X 一致且更明确。
3. **结构性佐证**：UB→GM 方向的 DataCopyPad 原型**没有 padParams 形参**（仅 dst/src/dataCopyParams），padding 语义仅存在于 GM→UB 方向；UB 侧 dummy 是框架内部对齐行为，不由用户控制，落 GM 时丢弃。
4. **对齐约束**：UB 侧源地址必须 32 字节对齐；GM 侧目的地址 1 字节对齐（无对齐约束）——支撑"非对齐尾块直接落到 GM 任意地址"的设计。
5. 官方样例佐证：`examples/01_simd_cpp_api/03_basic_api/00_data_movement/data_copy_pad_gm2ub_ub2gm`（cann/asc-devkit 仓库，路径见 DataCopyPad_UBToGM 页脚，B 级）。
6. 对照本地代码：op_kernel `CopyOut` 与 V002 `Store` 均以 `blockLen = len*sizeof(T)`（非 32 倍数时）调用三参 DataCopyPad，依赖"dummy 丢弃"实现非对齐写出——**与文档行为一致**；因此尾块写出不会覆盖 GM 相邻行。

结论：**维持**。9.0.X 与最新官方镜像双版本一致；官方样例路径佐证。

### 定案 3：ReduceSum 无单一 count 硬上限；255/4096/16320 为不同实现路径口径；workLocal 按公式预留 —— **维持（附口径修正）**

证据链：

1. **count 无单一硬上限**：CANN 9.0.X ReduceSum（缓存 0078）count 参数原文："参数取值范围和操作数的数据类型有关，数据类型不同，能够处理的元素个数最大值不同，**最大处理的数据量不能超过 UB 大小限制**。"gitcode 最新版及 8.0.0.alpha001 在线版原文相同。即唯一约束是 UB 容量，**不存在跨版本的单一固定 count 上限**。
2. **"255"口径**：通用高维切分 API 的 repeatTime 为 uint8_t、∈[0,255]（Cast 页参数说明原文"repeatTime∈[0,255]"；WholeReduceMax 等归约指令 repeatTimes 同为 [0,255]）。而 **ReduceSum 高维切分接口是官方明示的例外**："与高维切分中不同的是，repeatTime 可以支持更大的取值范围，保证不超过 int32_t 的最大值即可"（gitcode 最新版明示）。→ 255 属"通用高维切分/硬件归约指令 repeatTime"口径，不约束 ReduceSum。
3. **"16320"口径**：官方文档未出现 16320 字面值。可由官方参数推导：硬件归约指令（WholeReduceSum 类）repeatTimes∈[0,255] × mask 连续模式 float 每迭代 64 元素 = 16320（float 单次调用上限）；half 为 128×255=32640。→ 属"硬件归约指令/高维切分 mask 模式"口径。
4. **"4096"口径**：官方 ReduceSum/Cast 页面均未出现该字面值作为 count 上限。与之相关的官方数字是 DataCopyExtParams blockCount∈[1,4095] 及 DataCopy 32 字节向下取整行为。4096 更可能源自旧版社区资料或 WholeReduce 相关样例口径，**本轮未能从官方文档直接证实**，维持"非 ReduceSum 官方上限"的判断（若上轮把它当作 ReduceSum 上限则应修正）。
5. **workLocal 公式**（9.0.X 原文伪码，方式一）：`typeSize=2/4`；`elementsPerBlock=32/typeSize`；`elementsPerRepeat=256/typeSize`；tensor 前 n 个接口 `firstMaxRepeat=count/elementsPerRepeat`（不足 1 取 1）；`finalWorkLocalNeedSize=RoundUp(firstMaxRepeat, elementsPerBlock)*elementsPerBlock`（元素数）。方式二（A2 的 tensor 前 n 个接口采用方式二）：**传入任意大小 sharedTmpBuffer 即可、其值不被改变**。
6. **本地代码验证**：A2 上 `ReduceSum(sum, yF, work, count)` 走方式二，workLocal 任意大小可用；即便按方式一公式：fp32、count≤2048 → firstMaxRepeat=32 → 需 RoundUp(32,8)×8=32 个 float=128B。op_kernel 预留 WORK_LEN=1024 float=4096B ≫ 128B ✓；V002 各 reduceFp32Buf 同样按整 tile 预留 ✓。dst：SUM_LEN=16 float，起始 4 字节对齐 ✓（dst 与 sharedTmpBuffer 允许地址重叠）。
7. 附带确认：A2 上 ReduceSum 数据类型仅 half/float（无 bfloat16_t）；count=0 时 A2 视为 NOP（不写目的操作数）。

结论：**维持**（"无单一 count 硬上限、workLocal 按公式预留"核心结论成立；255/16320 的口径归属已明确，4096 无官方出处，标注为未证实口径）。

---

## 三、入口限定符结论（__aicore__ 与 __vector__ 在 CANN 9.0.0 纯向量算子中的合法性）

版本演进与证据：

1. **8.5.0.alpha002（本地缓存 `agent02_vector_qualifier_850a002.html`，URL /document/detail/zh/CANNCommunityEdition/850alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html）**：
   - `__global__`：标识核函数入口，必须返回 void，**该函数同时也需要使用 __aicore__ 修饰**；
   - `__aicore__`：标识 Device 侧执行；
   - `__vector__` / `__cube__` / `__mix__(cube, vec)`：分别标识仅在 Vector 核 / Cube 核 / 双核执行；**"针对耦合模式的硬件架构，该修饰符不生效"**。
   - `__NPU_ARCH__` 映射：Atlas A2 训练系列 / Atlas 800I A2 推理产品 / A200I A2 Box → **2201**（dav-2201 即此架构）。
2. **8.3.RC1.alpha002（hiascend 在线快照，搜索摘要全文）**：修饰符表同口径（__global__/__aicore__/__inline__）。
3. **gitcode 官方最新文档（对应 9.x 后期演进，asc.gitcode.com 核函数页 + 算子编译约束页 + 核间同步页）**：
   - 核函数定义规范改为：必须 `__global__` + 从四种**函数执行空间限定符**中选一：`__aicore__`（"不区分具体核类型，通常用于耦合模式"）、`__vector__`（"适用于仅包含 Vector 计算的算子"）、`__cube__`、`__mix__`；
   - 官方示例即 `__global__ __vector__ void add_kernel(...)`；约束页示例含 `__global__ __vector__ __aicore__ void func0(...)`（纯 Scalar 算子推荐加 __vector__ 标记）；
   - 算子类型表：Vector 算子 = `__vector__` = KERNEL_TYPE_AIV_ONLY。
4. **CANN 9.0.0 具体页面**：`/document/detail/zh/CANNCommunityEdition/900/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html` 被 cookie 墙拦截，**未能直接取得 9.0.0 原文**（列入未确认清单）。但 9.0.X（900）API 线的其余全部缓存页面（LocalTensor/GlobalTensor/DataCopyPad/ReduceSum/Cast/SetFlag 等）与 850→gitcode 演进链无冲突。

结论（对 dav-2201 纯向量算子、CANN 9.0.0 直调模式）：

- **`__global__ __aicore__` 合法且是最稳妥组合**：850alpha002 明文要求 __global__ 配 __aicore__；gitcode 最新规范中 __aicore__ 明确"通常用于耦合模式"，A2 为耦合架构，语义完全匹配。op_kernel 采用此写法 ✓。
- **`__global__ __vector__` 合法**：gitcode 最新规范推荐纯 Vector 算子用 __vector__（KERNEL_TYPE_AIV_ONLY），官方示例即此写法；V002 采用此写法 ✓。需要注意的版本差异：850alpha002 时代 __vector__ 在耦合架构"不生效"（被忽略而非报错）；最新规范已改为可选执行空间限定符。9.0.0 处于该演进区间内，两种写法在语义上均不构成编译风险，但**若判题环境严格按 9.0.0 文档口径校验，`__global__ __aicore__` 的证据链最强**；`__global__ __vector__` 依赖"最新规范推荐 + 旧版不报错只忽略"的组合论证。V002 与题面"内部 __global__ __vector__ 核函数"的直调模式说明一致，维持现状是合理选择，风险评级为低。
- 核函数必须 void 返回、指针入参加 `__gm__`、host 侧以 `<<<blockCount, dynUBufSize, stream>>>` 调用（V002 的 `add_rms_norm_bias_custom<T><<<blockCount, nullptr, stream>>>(...)` 与该 ABI 一致，numBlocks 为 uint32_t 语义，V002 已按 UINT32_MAX 夹紧 ✓）。

---

## 四、A2（dav-2201）bfloat16 支持结论与 Cast RoundMode

### 4.1 Add/Mul/Muls/Sqrt/Rsqrt/ReduceSum 在 A2 上均不支持 bfloat16_t

CANN 9.0.X 各 API 页"模板参数说明"表（缓存 0035/0037/0055/0029/0030/0078，正文逐条核读）：

| API | Atlas A2 训练/推理系列支持类型 | bfloat16_t |
|-----|------------------------------|-----------|
| Add | half、int16_t、int32_t、float | **否**（仅 Atlas 350 加速卡含 bf16） |
| Mul | half、int16_t、int32_t、float | **否** |
| Muls | half/int16_t/float/int32_t | **否** |
| Sqrt | half、float | **否** |
| Rsqrt | half、float | **否** |
| ReduceSum | half/float | **否** |

搬运层不受限：DataCopy/DataCopyPad 在 A2 上支持 bfloat16_t（0265 表"模板参数说明"、gitcode DataCopy 数据类型表均列 bfloat16_t）。

**对本地代码的判定**：op_kernel 与 V002 均执行"bf16 输入 → Cast 提升 FP32 → 全链 FP32 计算 → Cast 回 bf16 输出"策略，所有 Add/Mul/Muls/Sqrt/ReduceSum 调用均为 float 实例化——**与 A2 类型支持矩阵完全一致，正确规避了 bf16 计算指令缺失问题**。这是两份代码共同的关键正确性设计。

### 4.2 fp32→bf16 Cast 可用 RoundMode（A2 支持矩阵，缓存 0073 表 6）

| src→dst | A2 支持的 RoundMode |
|---------|--------------------|
| half → float | CAST_NONE |
| float → half | CAST_RINT / CAST_FLOOR / CAST_CEIL / CAST_ROUND / CAST_TRUNC / **CAST_ODD / CAST_NONE** |
| **float → bfloat16_t** | **CAST_RINT / CAST_FLOOR / CAST_CEIL / CAST_ROUND / CAST_TRUNC**（5 种；未列 CAST_NONE/CAST_ODD） |
| bfloat16_t → float | CAST_NONE |

RoundMode 枚举定义：CAST_NONE=0（"在转换有精度损失时表示 CAST_RINT 模式，不涉及精度损失时表示不舍入"）、CAST_RINT（四舍六入五成双）、CAST_FLOOR、CAST_CEIL、CAST_ROUND、CAST_TRUNC、CAST_ODD、CAST_HYBRID（hif8 专用随机舍入）。

**对本地代码的判定**：
- 提升（half/bf16→float）用 CAST_NONE：在 A2 矩阵支持范围内 ✓（无损转换）。
- 量化（float→half / float→bf16）用 **CAST_RINT**：在 A2 矩阵 5 种支持模式内 ✓。注意 CAST_RINT 为"四舍六入五成双"，与 PyTorch 的 round-half-to-even 一致，对 bf16/fp16 输出精度评测通常有利。
- 细节差异记录：A2 矩阵中 float→bfloat16_t 未列 CAST_NONE，而 float→half 列出了 CAST_NONE——两者支持的显式模式集不同；本地代码统一用 CAST_RINT，落在两者交集内，不受该差异影响。若后续想改用 CAST_NONE 走"有损即 RINT"语义，float→half 有显式依据、float→bf16 只能依赖枚举注释的等价说明，建议维持 CAST_RINT 显式写法。
- V002 的 fp32→fp32 用 `Adds(dst, src, 0.0f)` 替代 Cast：A2 矩阵无 float→float 行，该替代规避了"fp32→fp32 Cast 不在支持矩阵"的问题，合理（Adds 在 A2 支持 float）。

---

## 五、风险与未确认清单

1. **9.0.0 版 C++语言拓展原页未直接取得**（hiascend cookie 墙）：__vector__/__aicore__ 结论基于 850alpha002 缓存 + 83RC1alpha002 在线快照 + gitcode 最新规范三点演进链。剩余风险：9.0.0 当页措辞与 850alpha002 相同（__vector__ 耦合模式"不生效"）的概率存在，但即便如此也是"忽略而非报错"，与 V002 现状不冲突。
2. **kernel_struct_data_copy.h 头文件原文未核对**（本机无 CANN 安装目录）：DataCopyExtParams/DataCopyPadExtParams 字段顺序以 9.0.X 官方文档表格+示例为证，头文件为最终裁决源。
3. **"4096"口径无官方出处**：官方文档未出现该数字作为 ReduceSum 或相关接口上限；若上轮调研将其当作 ReduceSum count 上限引用，应修正为"未证实口径"。
4. **16320 为推导值**：255（repeatTimes 上限）×64（float mask/迭代）=16320 有官方参数依据，但文档未直接给出该乘积作为上限表述；引用时应标注"由官方参数推导"。
5. **op_kernel 的 GET_TILING_DATA_WITH_STRUCT**：仅适用于标准算子工程（msopgen）；该宏 9.0.X 明文"暂不支持 Kernel 直调工程"。op_kernel 若被直接搬入直调包会踩此约束——V002 已经以直传参数方式规避，op_kernel 不用于直调即可。
6. **op_kernel GetBlockIdx/GetBlockNum 隐式 int64_t→uint32_t 收窄**：实际安全（核数远小于 2^32），严格类型口径下建议显式 static_cast（V002 已做）。
7. **PipeBarrier<PIPE_V> 手动插入的冗余性**：9.0.X PipeBarrier 页注记"Kernel 直调算子工程和自定义算子开发工程已默认开启自动同步，编译器自动插入 PIPE_V 同步，无需开发者手动插入"。V002 的 163 处 PIPE_V 屏障属保守冗余（无害，性能可再评估）；另注意**严禁 PipeBarrier<PIPE_S>()**（硬件错误），两份代码均未违反。
8. **VECIN 队列数上限 8**（TQue 页注记）：op_kernel 4 个 VECIN TQue，安全；若后续扩队列需留意。
9. **A2 上 DataCopyPad 不支持 PaddingMode（Normal/Compact 选择）**：两份代码均未使用 mode 模板参数，一致；未来优化不得引入 Compact 模式。
10. **bf16 Host 端声明限制**（gitcode 约束页）：bfloat16_t 在 Host 端仅支持以 C++ 模板形式定义与声明（A2 支持 bfloat16_t 本身）。V002 以模板参数 `AddRmsNormBiasKernel<bfloat16_t>` 使用，符合该限制。
11. hiascend.com 的 `/document/detail/.../900/...` 路径全部受 cookie 墙拦截：本次 9.0.X 证据全部来自本地缓存（即该版本页面存档，内嵌版本元数据 zh/CANNCommunityEdition/900 已验证）+ gitcode 官方镜像；对"页面存档完整性"的信任基于内嵌元数据与正文自洽。

---

## 六、来源登记

本地缓存（正文均从页面内嵌数据完整解码核读，verified；缓存目录 `/Users/sunyiyang/Desktop/Project/cann/临时/api-pages/`；页面内嵌版本元数据均为 zh/CANNCommunityEdition/900 / 9.0.X，访问/核读日期 2026-09-12）：

| 缓存文件 | 标题 | 对应 URL（版本路径 zh/CANNCommunityEdition/900） | 用途 | 证据状态 |
|----------|------|------------------------------------------------|------|---------|
| atlasascendc_api_07_0007.html | GlobalTensor简介 | hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0007.html | SetGlobalBuffer/GetValue/GetSize 签名 | verified |
| atlasascendc_api_07_0006.html | LocalTensor简介 | .../atlasascendc_api_07_0006.html | LocalTensor 成员、kernel_operator.h 引用 | verified |
| atlasascendc_api_07_0265.html | DataCopyPad(ISASI) | .../atlasascendc_api_07_0265.html | 定案 1/2 全部证据（表 4/6/7、UB→GM dummy 丢弃、A2 通路与 mode 支持度、示例） | verified |
| atlasascendc_api_07_0078.html | ReduceSum | .../atlasascendc_api_07_0078.html | 定案 3（count 约束、方式一/二、workLocal 公式、A2 类型 half/float） | verified |
| agent02_GetReduceSumMaxMinTmpSize_900.html | GetReduceSumMaxMinTmpSize | .../atlasascendc_api_07_10160.html | Host 侧 tmpSize 公式接口 | verified |
| atlasascendc_api_07_0073.html | Cast | .../atlasascendc_api_07_0073.html | RoundMode 枚举、A2 Cast 支持矩阵（fp32→bf16 5 种模式） | verified |
| atlasascendc_api_07_0035.html | Add | .../atlasascendc_api_07_0035.html | A2 类型支持（无 bf16） | verified |
| atlasascendc_api_07_0037.html | Mul | .../atlasascendc_api_07_0037.html | 同上 | verified |
| atlasascendc_api_07_0055.html | Muls | .../atlasascendc_api_07_0055.html | 同上 | verified |
| atlasascendc_api_07_0029.html | Sqrt | .../atlasascendc_api_07_0029.html | 向量 Sqrt 原型、A2 half/float | verified |
| atlasascendc_api_07_0030.html | Rsqrt | .../atlasascendc_api_07_0030.html | 向量 Rsqrt 原型、A2 half/float | verified |
| atlasascendc_api_07_0101.html | DataCopy简介 | .../atlasascendc_api_07_0102.html（页面内嵌 route 0102） | DataCopy 功能总览（导航页） | verified |
| atlasascendc_api_07_0108.html | TPipe简介 | .../atlasascendc_api_07_00148.html（页面内嵌 route 00148） | TPipe 职责（InitBuffer/AllocEventID/ReleaseEventID） | verified |
| atlasascendc_api_07_0136.html | TQue简介 | .../atlasascendc_api_07_0136.html | TQue 模板参数、VECIN buffer≤8、AllocTensor/EnQue/DeQue/FreeTensor | verified |
| atlasascendc_api_07_0160.html | TBuf简介 | .../atlasascendc_api_07_0160.html | TBuf/Get/InitBuffer 差异 | verified |
| atlasascendc_api_07_0270.html | SetFlag/WaitFlag(ISASI) | .../atlasascendc_api_07_0270.html | HardEvent 枚举全量、A2 eventID 0-7、成对约束、AllocEventID/FetchEventID 要求 | verified |
| atlasascendc_api_07_0271.html | PipeBarrier(ISASI) | .../atlasascendc_api_07_0271.html | PIPE_S 禁令、PIPE_ALL、自动同步注记 | verified |
| atlasascendc_api_07_0184.html | GetBlockNum | .../atlasascendc_api_07_0184.html | int64_t 签名 | verified |
| atlasascendc_api_07_0185.html | GetBlockIdx | .../atlasascendc_api_07_0185.html | int64_t 签名 | verified |
| atlasascendc_api_07_0215.html | GET_TILING_DATA_WITH_STRUCT | .../atlasascendc_api_07_0215.html | "暂不支持 Kernel 直调工程"约束 | verified |
| atlasascendc_api_07_00003.html | REGISTER_TILING_DEFAULT | （页面标题版本 8.2.RC1.alpha001 线，A2 支持） | 对照条目 | verified |
| atlasascendc_api_07_00174.html | GetSysWorkSpacePtr | ... | 对照条目 | verified |
| atlasascendc_api_07_00007.html | MulDstAdd | .../atlasascendc_api_07_0007.html（内嵌） | 对照条目（A2 不支持） | verified |
| agent02_vector_qualifier_850a002.html | C++语言拓展 | hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | 修饰符表（850 时代口径）、__NPU_ARCH__ 2201 映射 | verified |
| guide_tutorial.html / guide_index.html | 什么是Ascend C（编程指南导航） | zh/CANNCommunityEdition/900 | 目录确认（无增量正文） | partial |

在线来源（访问日期均为 2026-09-12）：

| URL | 标题 | 用途 | 证据状态 |
|-----|------|------|---------|
| https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopy_GMAndUB_continuous.html | DataCopy（GM 与 UB 连续数据搬运） | DataCopy 三参原型、count·sizeof(T) 32B 对齐向下取整、A2 数据类型 | verified |
| https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopyPad_UBToGM.html | DataCopyPad（UB→GM 非对齐数据搬运） | 定案 2 最新版原文（dummy 丢弃）、blockLen∈[0,2097151] 且须为 sizeof(T) 整数倍、NOP 约束、官方样例路径 | verified |
| https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/归约计算/ReduceSum.html | ReduceSum（gitcode 官方镜像） | 定案 3 交叉验证：count 受 UB 限制、repeatTime int32 例外、count=0 NOP（A2）、A2 half/float、workLocal 公式 | verified |
| https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/归约计算/WholeReduceSum.html | WholeReduceSum | repeatTime/mask 约束（16320 推导依据） | verified |
| https://asc.gitcode.com/api/SIMD-API/基础API/资源管理/TPipe/InitBuffer.html | InitBuffer | InitBuffer 两种签名、len 自动 32B 补齐、Buffer 总数 ≤64、double buffer 语义 | verified |
| https://asc.gitcode.com/guide/编程指南/编程模型/AI-Core-SIMD编程/核函数.html | 核函数 | __global__ + 四种执行空间限定符规范、`__global__ __vector__` 官方示例、<<<>>> ABI | verified |
| https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/约束说明.html | 约束说明 | Kernel 类型标记建议、bfloat16_t Host 端模板限制（A2 列入支持） | verified |
| https://asc.gitcode.com/api/SIMD-API/基础API/同步控制/核间同步/核间同步能力概述.html | 核间同步能力概述 | __vector__=KERNEL_TYPE_AIV_ONLY、group 配置表 | verified |
| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | C++语言拓展（83RC1alpha002，搜索快照全文） | 修饰符表版本演进中间点 | verified（搜索快照全文） |
| https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0078.html | ReduceSum（8.0.0.alpha001） | count 约束跨版本一致性 | verified |
| https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0079.html | WholeReduceMax（8.0.0.alpha001） | repeatTimes∈[0,255] 佐证 | verified |
| https://gitcode.com/cann/asc-devkit（仓库） | 官方文档/样例仓库 cann/asc-devkit | 文档仓库定位；DataCopyPad 官方样例 examples/01_simd_cpp_api/03_basic_api/00_data_movement/data_copy_pad_gm2ub_ub2gm | verified（仓库页；样例未逐行核读） |
| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | C++语言拓展（900） | 尝试直取——被 cookie 墙拦截 | unavailable |
| https://www.hiascend.com/developer/blog/details/0289201670005562056 | 官方博客：--npu-arch=dav-2201 编译案例 | 910B/A2 ↔ 2201/dav-2201 佐证、bisheng --npu-arch 用法 | verified（C 级辅证） |

---

## 七、总结

- 27 个主条目 + 3 个对照条目完成核对，全部条目均有 9.0.X 官方文档或官方镜像来源（A 级为主）。
- **三项定案全部维持**：① DataCopyPadExtParams 字段顺序 {isPad, leftPadding, rightPadding, paddingValue}（表 6 + 官方示例双证）；② UB→GM 搬出 dummy 假数据落 GM 时被框架丢弃、不写相邻内存（9.0.X 与最新镜像双版本同文）；③ ReduceSum 无单一 count 硬上限、唯一约束为 UB 容量（255=通用 repeatTime 口径、16320=硬件归约指令推导口径、4096 无官方出处待修正）。
- **入口限定符**：`__global__ __aicore__` 与 `__global__ __vector__` 在 A2/CANN 9.0.0 语境下均合法（前者证据链最强，后者为最新规范推荐且与 V002 及直调模式说明一致）；9.0.0 原页未能直取，为唯一留白。
- **A2 bfloat16**：Add/Mul/Muls/Sqrt/Rsqrt/ReduceSum 均不支持 bfloat16_t，本地两份代码的"全 FP32 计算链"设计与此完全一致；fp32→bf16 Cast 在 A2 支持 CAST_RINT 等 5 种模式，代码所用 CAST_RINT 在矩阵内。
- **本地代码不一致处**：无致错级不一致；记录 4 处提示——op_kernel GetBlockIdx 隐式收窄（建议显式转换）、op_kernel 的 GET_TILING_DATA_WITH_STRUCT 不可用于直调工程（V002 已规避）、PipeBarrier<PIPE_V> 手动插入在默认自动同步下冗余、float→bf16 与 float→half 的 CAST_NONE 支持差异（当前 CAST_RINT 写法不受影响）。
