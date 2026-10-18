# membase 基础 API - AIV (Vector 核心) UT 生成指南

## 1. API 概述

AIV (AI Vector) 核心 API 是 membase 基础 API 中的向量计算类接口，用于执行向量运算、数据搬运、类型转换等操作。

### 1.1 目录位置

| 类型 | 路径 |
|------|------|
| **头文件** | `{ASC_DEVKIT_PATH}/include/basic_api/kernel_operator_*.h`（排除 `reg_compute/` 子目录） |
| **实现文件** | `{ASC_DEVKIT_PATH}/impl/basic_api/dav_xxx/` |
| **测试目录** | `{ASC_DEVKIT_PATH}/tests/api/basic_api/ascendc_case_{arch}/ascendc_case_{arch}_aiv/` |

### 1.2 典型 API 示例

| API 类别 | 典型 API | 功能说明 |
|---------|---------|---------|
| 向量计算 | `Add`, `Mul`, `Sub`, `Div` | 向量加减乘除 |
| 数据搬运 | `DataCopy` | 数据搬运 |
| 类型转换 | `Cast` | 数据类型转换 |
| 归约操作 | `ReduceMax`, `ReduceMin`, `ReduceSum` | 归约计算 |
| 比较操作 | `Compare` | 向量比较 |

---

## 2. API 特点

### 2.1 AIV 核心说明

AIV (AI Vector) 核心专门用于向量计算：

| 特性 | 说明 |
|------|------|
| **核心类型** | 向量计算核心 (Vector Core) |
| **设置方法** | `SetGCoreType(2)` 或 `g_coreType = AIV_TYPE` |
| **数据位置** | UB (Unified Buffer) |
| **Tensor 类型** | `TQue<TPosition::VECIN/VECOUT/VECCALC>` |

### 2.2 架构适配

AIV UT 不在本 guide 中维护完整芯片映射表。芯片调用名、`__NPU_ARCH__`、`SocVersion` 和 `arch_dir`
统一从 [`asc-npu-arch`](../../../asc-npu-arch/SKILL.md) 及
[`npu-arch-facts.json`](../../../asc-npu-arch/references/npu-arch-facts.json) 获取。

AIV 场景只保留测试目录推导规则：

```text
tests/api/basic_api/${arch_dir}/${arch_dir}_aiv/
```

生成 UT 时还需要从目标 API 的实际 `impl/basic_api/dav_*` 实现文件和已有同类 UT 确认：
- 目标 API 在该架构下是否存在实现；
- 目标 API 是否有架构条件分支或特殊参数结构；
- 目标架构已有 AIV UT 使用的目录、target 和编译宏模式。

### 2.3 数据类型支持

**必须从目标架构 impl 文件确认数据类型支持：**

```cpp
// impl 文件中的 SupportType 定义
using SupportType = std::tuple<half, float>;
```

---

## 3. 核心类型设置

### 3.1 SetUp/TearDown 配置

```cpp
class TEST_Add : public testing::Test {
protected:
    void SetUp() {
        AscendC::SetGCoreType(2);  // AIV 使用 2
    }
    void TearDown() {
        AscendC::SetGCoreType(0);
    }
};
```

### 3.2 非二元向量 API 的模板选择

不要把所有 AIV API 都套成 `src0 + src1 -> dst` 的 binary 模板。生成前先从目标接口签名和实现分支确认数据流形态：

| 形态 | 识别方式 | 生成约束 |
|------|----------|----------|
| **binary** | 两个输入 tensor | 可使用 binary AIV 模板 |
| **scalar + tensor** | 既有 `scalar` 重载，又有 `count/mask` 重载 | 生成单输入队列和 scalar 读取逻辑 |
| **tensor trait 分支** | `if constexpr (IsSameType<PrimT<T>, T>::value)` 或同类分支 | raw dtype 与 `TensorTrait<T>` 要分别覆盖 |
| **selector / masked multi-source** | 额外 `selMask` / `SELMODE` / 多源分支 | 先按真实 impl 和同类 UT 分析，当前不能套用通用模板 |

`Duplicate` 是典型的非 binary API：

```cpp
Duplicate(dstLocal, scalar, maskBit, repeatTimes, dstBlockStride, dstRepeatStride);
Duplicate(dstLocal, scalar, maskCounter, repeatTimes, dstBlockStride, dstRepeatStride);
Duplicate(dstLocal, scalar, dataSize);
Duplicate(dstLocal, srcLocal, dataSize);  // TensorTrait 分支
```

因此不能直接使用 binary 模板骨架。raw dtype 用例应覆盖 scalar overload，`TensorTrait<T>` 用例应覆盖 tensor overload 分支。

当前通用生成器中，`binary` 与 `scalar_tensor_dispatch` 是两种独立模板：
- `binary`：双输入 tensor，适用于已经验证的通用二元 API；
- `scalar_tensor_dispatch`：单输入 tensor，raw dtype 走 `API(dst, scalar, count)`，`TensorTrait<T>` 走 `API(dst, src, count)`。

`scalar_tensor_dispatch` 只能表达 count 形态的公共骨架；像 `Duplicate` 的 Level0 `mask` 重载仍需根据实际 impl 和已有 UT 继续补齐，不能把该模板误认为完整覆盖。

`Select` 是另一类常见的非 binary API。它同时依赖 `selMask`、`SELMODE`、Level0/Level2 重载，
还会在 tensor-tensor、tensor-scalar、cmp-mask 等模式间切换。此类 API 不能因为“也有多个输入”
就退化成 binary 模板；在没有经过验证的 selector family 模板前，应直接走 API-specific UT。

### 3.3 `mockcpp` 边界

在 CPU mock UT 中，旧式 membase API 不要默认断言真实向量结果值；优先沿用同目录已有 UT 的做法：
- 用 `MOCKER(...)` 校验 overload 分发和关键参数；
- 如需验证输出路径，再 mock `DataCopyUB2GMImpl` 一类边界函数；
- 对 `TensorTrait<T>` 分支，不要直接按 `DuplicateImpl<TensorTrait<T>>` 注册 mock，这会触发内部实现模板错误实例化。`TensorTrait` 分支保留正常执行覆盖即可。

---

## 4. 内存管理

### 4.1 内存对齐约束

基础 dtype 的 `sizeof` 与 32B 对齐元素数统一见 [`asc-npu-arch` 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md#基础-dtype-大小与-32b-对齐)。本节只保留 AIV UT 的 count 计算规则。

**计算公式：**
```
count >= ceil(min_tensor_size / sizeof(data_type))
count * sizeof(T) % 32 == 0
```

### 4.2 LocalTensor 内存申请

```cpp
// 推荐使用 TPipe::InitBuffer
TPipe pipe;
TQue<TPosition::VECIN, 1> inQueue;
TQue<TPosition::VECOUT, 1> outQueue;

// 分配内存（自动 32B 对齐）
pipe.InitBuffer(inQueue, 1, dataSize * sizeof(T));
pipe.InitBuffer(outQueue, 1, dataSize * sizeof(T));

// 获取 LocalTensor
LocalTensor<T> input = inQueue.AllocTensor<T>();
```

### 4.3 TPosition 选择

| 位置 | 适用场景 | 说明 |
|------|----------|------|
| **VECIN** | 向量编程输入 | Vector Core 输入数据 |
| **VECOUT** | 向量编程输出 | Vector Core 输出数据 |
| **VECCALC** | 向量计算临时空间 | 用于临时变量，别名 LCM |

### 4.4 TmpBuffer 使用

部分 API 需要 sharedTmpBuffer 参数：

TmpBuffer 大小必须从目标 API 注释、impl 内部临时空间计算和已有同类 UT 中确认，不按 dtype 做通用推断。完整申请模式统一参考 [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md#72-需要临时空间的-api)。

---

## 5. 常用结构体参数

### 5.1 DataCopyParams

**定义文件**: `include/basic_api/kernel_struct_data_copy.h`

```cpp
struct DataCopyParams {
    uint16_t blockCount = DEFAULT_DATA_COPY_NBURST;  // 块数量
    uint16_t blockLen = 0;           // 块长度
    uint16_t srcGap = 0;             // 源间隙
    uint16_t dstGap = 0;             // 目的间隙
};
```

### 5.2 DataCopyEnhancedParams

**定义文件**: `include/basic_api/kernel_struct_data_copy.h`

```cpp
struct DataCopyEnhancedParams {
    BlockMode blockMode = BlockMode::BLOCK_MODE_NORMAL;
    DeqScale deqScale = DeqScale::DEQ_NONE;
    uint64_t deqValue = 0;
    uint8_t sidStoreMode = 0;
    bool isRelu = false;
    pad_t padMode = pad_t::PAD_NONE;
    uint64_t padValue = 0;
    uint64_t deqTensorAddr = 0;
};
```

### 5.3 BinaryRepeatParams

**定义文件**: `include/basic_api/kernel_struct_binary.h`

```cpp
struct BinaryRepeatParams {
    uint32_t blockNumber = DEFAULT_BLK_NUM;
    uint8_t dstBlkStride = DEFAULT_BLK_STRIDE;
    uint8_t src0BlkStride = DEFAULT_BLK_STRIDE;
    uint8_t src1BlkStride = DEFAULT_BLK_STRIDE;
    uint8_t dstRepStride = DEFAULT_REPEAT_STRIDE;
    uint8_t src0RepStride = DEFAULT_REPEAT_STRIDE;
    uint8_t src1RepStride = DEFAULT_REPEAT_STRIDE;
    bool repeatStrideMode = false;
    bool strideSizeMode = false;
};
```

### 5.4 Nd2NzParams

**定义文件**: `include/basic_api/kernel_struct_data_copy.h`

```cpp
struct Nd2NzParams {
    uint16_t ndNum = 0;              // ND数量
    uint16_t nValue = 0;             // N值
    uint16_t dValue = 0;             // D值
    uint16_t srcNdMatrixStride = 0;  // 源ND矩阵stride
    uint16_t srcDValue = 0;          // 源D值
    uint16_t dstNzC0Stride = 0;      // 目的NZ C0 stride
    uint16_t dstNzNStride = 0;       // 目的NZ N stride
    uint16_t dstNzMatrixStride = 0;  // 目的NZ矩阵stride
};
```

---

## 6. 常用枚举类

### 6.1 DataFormat - 数据格式

**定义文件**: `include/basic_api/kernel_struct_data_copy.h`

```cpp
enum class DataFormat : uint8_t {
    ND = 0,
    NZ,
    NCHW,
    NC1HWC0,
    NHWC,
    NCDHW,
    NDC1HWC0,
    FRACTAL_Z_3D,
};
```

### 6.2 DataCopyMVType - 数据搬运类型

**定义文件**: `include/basic_api/kernel_struct_data_copy.h`

```cpp
enum class DataCopyMVType : uint8_t {
    UB_TO_OUT = 0,   // UB到外部
    OUT_TO_UB = 1,   // 外部到UB
};
```

### 6.3 RoundMode - 舍入模式

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class RoundMode : uint8_t {
    CAST_NONE = 0,   // 无舍入
    CAST_RINT,       // round to nearest integer
    CAST_FLOOR,      // 向下取整
    CAST_CEIL,       // 向上取整
    CAST_ROUND,      // away-zero rounding
    CAST_TRUNC,      // to-zero rounding
    CAST_ODD,        // Von Neumann rounding
    CAST_HYBRID,     // hybrid round (特定架构支持)
    CAST_EVEN,       // 特定架构支持
    CAST_ZERO,       // 特定架构支持
    UNKNOWN = 0xFF,
};
```

### 6.4 CMPMODE - 比较模式

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class CMPMODE : uint8_t {
    LT = 0,   // 小于
    GT,       // 大于
    EQ,       // 等于
    LE,       // 小于等于
    GE,       // 大于等于
    NE,       // 不等于
};
```

### 6.5 SELMODE - 选择模式

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class SELMODE : uint8_t {
    VSEL_CMPMASK_SPR = 0,
    VSEL_TENSOR_SCALAR_MODE,
    VSEL_TENSOR_TENSOR_MODE,
};
```

### 6.6 DeqScale - 反量化缩放类型

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class DeqScale : uint8_t {
    DEQ_NONE = 0,   // 无反量化
    DEQ,            // 普通反量化
    VDEQ,           // 向量反量化
    DEQ8,           // 8位反量化
    VDEQ8,          // 向量8位反量化
    DEQ16,          // 16位反量化
    VDEQ16,         // 向量16位反量化
};
```

### 6.7 ReduceMode - 归约模式

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class ReduceMode : uint8_t {
    REDUCE_MAX = 0,  // 最大值归约
    REDUCE_MIN,      // 最小值归约
    REDUCE_SUM,      // 求和归约
};
```

### 6.8 ReduceOrder - 归约顺序

**定义文件**: `impl/basic_api/utils/kernel_utils_mode.h`

```cpp
enum class ReduceOrder : uint8_t {
    ORDER_VALUE_INDEX = 0,  // 值-索引顺序
    ORDER_INDEX_VALUE,      // 索引-值顺序
    ORDER_ONLY_VALUE,       // 仅值
    ORDER_ONLY_INDEX,       // 仅索引
};
```

### 6.9 BlockMode - 块模式

**定义文件**: `impl/basic_api/utils/kernel_utils_mode_cpu.h`

```cpp
enum class BlockMode : uint8_t {
    BLOCK_MODE_NORMAL = 0,       // 普通模式
    BLOCK_MODE_MATRIX,           // 矩阵模式
    BLOCK_MODE_VECTOR,           // 向量模式
    BLOCK_MODE_SMALL_CHANNEL,    // 小通道模式
    BLOCK_MODE_DEPTHWISE,        // 深度卷积模式
};
```

---

## 7. 测试模板引用

通用 gtest、参数化测试和结果校验骨架见 [测试模板参考](../foundations/test-templates.md)。AIV guide 只维护 AIV 特有约束：

- `SetGCoreType(2)` 用于 AIV 场景，结束后恢复默认核心类型。
- LocalTensor、TQue、TBuf 和 TmpBuffer 的申请细节回链 [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md)。
- count 和 buffer 大小按目标 API 最小 tensor 要求及 [`asc-npu-arch` dtype 对齐表](../../../asc-npu-arch/references/npu-arch-guide.md#基础-dtype-大小与-32b-对齐) 计算。

---

## 8. 常见问题

AIV guide 只保留 AIV 特有约束。以下公共排障项统一查看 [常见问题与解决方案](../troubleshooting/faq.md)：

- [编译报错找不到底层指令](../troubleshooting/faq.md#7-编译报错找不到底层指令)
- [TmpBuffer / workspace 大小与分支覆盖](../troubleshooting/faq.md#6-tmpbuffer--workspace-大小与分支覆盖)
- [枚举类作用域错误](../troubleshooting/faq.md#10-枚举类作用域错误)

---

## 9. 检查清单

### 9.1 分析阶段

- [ ] 已从目标架构 impl 文件确认数据类型支持 (SupportType)
- [ ] 已确认 API 是 AIV 类型
- [ ] 已分析 impl 文件中的所有分支

### 9.2 编写阶段

- [ ] SetUp 中正确设置 `SetGCoreType(2)`
- [ ] 数据大小满足 32 字节对齐
- [ ] TmpBuffer 大小正确（如需要）
- [ ] 使用参数化测试覆盖分支

### 9.3 验证阶段

- [ ] 编译通过无错误
- [ ] 测试执行通过
- [ ] 结果验证正确

---

## 10. 相关文件索引

| 文件路径 | 说明 |
|---------|------|
| `include/basic_api/kernel_struct_data_copy.h` | DataCopy 相关结构体 |
| `include/basic_api/kernel_struct_binary.h` | Binary 操作结构体 |
| `include/basic_api/kernel_type.h` | 基础类型定义 |
| `impl/basic_api/utils/kernel_utils_mode.h` | 枚举模式定义 |

---

## 11. 相关参考

| 文档 | 说明 |
|------|------|
| [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md) | 内存管理详细说明 |
| [分支覆盖分析指南](../foundations/branch-coverage-guide.md) | 分支分析方法 |
| [AIC API UT 指南](membase-api-aic-ut-guide.md) | AIC (Cube 核心) API 测试指南 |
