# membase 基础 API - AIC (Cube 核心) UT 生成指南

## 1. API 概述

AIC (AI Cube) 核心 API 是 membase 基础 API 中的矩阵计算类接口，用于执行矩阵乘法、数据格式转换等操作。

### 1.1 目录位置

| 类型 | 路径 |
|------|------|
| **头文件** | `{ASC_DEVKIT_PATH}/include/basic_api/kernel_operator_*.h`（排除 `reg_compute/` 子目录） |
| **实现文件** | `{ASC_DEVKIT_PATH}/impl/basic_api/dav_xxx/` |
| **测试目录** | `{ASC_DEVKIT_PATH}/tests/api/basic_api/ascendc_case_{arch}/ascendc_case_{arch}_aic/` |

### 1.2 典型 API 示例

| API 类别 | 典型 API | 功能说明 |
|---------|---------|---------|
| 矩阵乘法 | `Mmad` | 矩阵乘法累加 |
| 数据输出 | `Fixpipe` | L0C 数据输出转换 |
| 数据加载 | `LoadData` | 数据加载到 L0A/L0B |

---

## 2. API 特点

### 2.1 AIC 与 AIV 对比

AIC（AI Cube）核心 API 与 AIV（AI Vector）核心 API 有显著差异：

| 特性 | AIC API | AIV API |
|-----|---------|---------|
| 核心类型 | `g_coreType = AIC_TYPE` | `g_coreType = AIV_TYPE` |
| 典型 API | Mmad, Fixpipe, LoadData | Add, Mul, DataCopy |
| 数据位置 | L0A, L0B, L0C (CO1), L1 | UB (VECCALC) |
| Tensor 类型 | `TQue<TPosition::A1/A2/B1/B2/CO1>` | `TQue<TPosition::VECIN/VECOUT/VECCALC>` |
| 测试目录 | `ascendc_case_*_aic` | `ascendc_case_*_aiv` |

### 2.2 AIC 核心说明

AIC (AI Cube) 核心专门用于矩阵计算：

| 特性 | 说明 |
|------|------|
| **核心类型** | 矩阵计算核心 (Cube Core) |
| **设置方法** | `g_coreType = AIC_TYPE` |
| **数据位置** | L0A, L0B, L0C (CO1), L1 |
| **Tensor 类型** | `TQue<TPosition::A1/A2/B1/B2/CO1/CO2>` |

### 2.3 架构适配

AIC UT 不在本 guide 中维护完整芯片映射表。芯片调用名、`__NPU_ARCH__`、`SocVersion` 和 `arch_dir`
统一从 [`asc-npu-arch`](../../../asc-npu-arch/SKILL.md) 及
[`npu-arch-facts.json`](../../../asc-npu-arch/references/npu-arch-facts.json) 获取。

AIC 场景只保留测试目录推导规则：

```text
tests/api/basic_api/${arch_dir}/${arch_dir}_aic/
```

生成 UT 时还需要从目标 API 的实际 `impl/basic_api/dav_*` 实现文件和已有同类 UT 确认：
- 目标 API 在该架构下是否存在实现；
- 目标 API 是否有架构条件分支或特殊参数结构；
- 目标架构已有 AIC UT 使用的目录、target 和编译宏模式。

### 2.5 自动生成边界

当前通用生成器只覆盖**已验证的 MMAD-like 形态**。`Mmad` 这类可复用
`A1 + B1 -> CO1` 骨架的 API 可以走自动模板；`LoadData`、`Fixpipe`
这类参数结构、数据流和目标位置明显不同的 API，必须先读实际 impl 和同类 UT，
再编写 API-specific UT。

不要把非 MMAD-like API 仅靠改类名套进 MMAD 模板。若生成结果的类名是目标 API，
但计算体仍然调用 `Mmad(...)`，这不是有效 UT，必须视为生成失败。

### 2.4 数据类型支持

**必须从目标架构 impl 文件确认数据类型支持：**

```cpp
// impl 文件中的 SupportType 定义
using SupportType = std::tuple<half, int8_t>;
```

---

## 3. 核心类型设置

### 3.1 SetUp/TearDown 配置

```cpp
class TEST_Fixpipe : public testing::Test {
protected:
    void SetUp() {
        g_coreType = AscendC::AIC_TYPE;  // 切换到 Cube 核心
    }
    void TearDown() {
        AscendC::CheckSyncState();       // 检查同步状态
        g_coreType = AscendC::MIX_TYPE;  // 恢复混合模式
    }
};
```

---

## 4. TPosition 选择

| 位置 | 适用场景 | 说明 |
|------|----------|------|
| **A1** | 矩阵编程 | A 矩阵 L1 缓冲区 |
| **A2** | 矩阵编程 | A 矩阵 L0 缓冲区 |
| **B1** | 矩阵编程 | B 矩阵 L1 缓冲区 |
| **B2** | 矩阵编程 | B 矩阵 L0 缓冲区 |
| **C1** | 矩阵编程 | C 矩阵 L1 缓冲区 |
| **C2** | 矩阵编程 | C 矩阵 L0 缓冲区 |
| **CO1** | 矩阵编程 | C 矩阵输出缓冲区 (L0C) |
| **CO2** | 矩阵编程 | C 矩阵输出缓冲区 |

---

## 5. 参数结构初始化（重要）

### 5.1 关键经验

**AIC API 的 Params 结构体不支持花括号初始化列表！**

```cpp
// ❌ 错误：花括号初始化列表不支持
FixpipeParamsV220 fixpipeParams = {n, m, srcStride, dstStride, quantMode, reluEn};

// ✅ 正确：使用成员赋值
FixpipeParamsV220 fixpipeParams;
fixpipeParams.nSize = n;
fixpipeParams.mSize = m;
fixpipeParams.srcStride = static_cast<uint16_t>(AlignUp(m, BLOCK_CUBE));
fixpipeParams.dstStride = n;
fixpipeParams.quantPre = quantMode;
fixpipeParams.reluEn = reluFlag;
fixpipeParams.ndNum = 1;
fixpipeParams.srcNdStride = 0;
fixpipeParams.dstNdStride = 0;
```

### 5.2 其他 Params 结构体

**对于所有 Params 结构体，优先使用成员赋值：**

```cpp
SomeParams params;
params.field1 = value1;
params.field2 = value2;
// ...
```

### 5.2 FixpipeParamsV220 / FixpipeParamsC310

**定义文件**: `include/basic_api/kernel_struct_fixpipe.h`

```cpp
struct FixpipeParamsV220 {
    uint16_t nSize = 0;         // N方向大小
    uint16_t mSize = 0;         // M方向大小
    uint16_t srcStride = 0;     // 源stride
    uint32_t dstStride = 0;     // 目的stride
    QuantMode_t quantPre = QuantMode_t::NoQuant;  // 量化模式
    uint64_t deqScalar;         // 反量化标量值
    uint16_t ndNum = 1;         // ND数量
    uint16_t srcNdStride = 0;   // 源ND stride
    uint16_t dstNdStride = 0;   // 目的ND stride
    bool reluEn = false;        // 是否使能ReLU
    uint8_t unitFlag = 0;       // 单元标志
    bool isChannelSplit = false;// 是否通道切分
};
```

**使用示例**:
```cpp
FixpipeParamsV220 params;
params.nSize = 16;
params.mSize = 16;
params.reluEn = true;
params.quantPre = QuantMode_t::DEQF16;
params.deqScalar = 0x3C00;  // half 格式的 1.0
```

---

## 6. QuantMode 量化模式

### 6.1 QuantMode_t 枚举定义

```cpp
enum QuantMode_t {
    NoQuant,          // 不使能量化功能
    F322F16,          // float量化成half, scalar量化
    F322BF16,         // float量化成bfloat16_t, scalar量化
    DEQF16,           // int32_t量化成half, scalar量化
    VDEQF16,          // int32_t量化成half, tensor量化
    QF322B8_PRE,      // float量化成int8_t/uint8_t, scalar量化
    VQF322B8_PRE,     // float量化成int8_t/uint8_t, tensor量化
    QF162B8_PRE,      // half量化成int8_t/uint8_t, scalar量化
    VQF162B8_PRE,     // half量化成int8_t/uint8_t, tensor量化
    REQ8,             // int32_t量化成int8_t/uint8_t, scalar量化
    VREQ8,            // int32_t量化成int8_t/uint8_t, tensor量化
    QF162S4_PRE,      // half量化成int4, scalar量化
    VQF162S4_PRE,     // half量化成int4, tensor量化
    REQ4,             // int32_t量化成int4, scalar量化
    VREQ4,            // int32_t量化成int4, tensor量化
    DEQS16,           // int32_t量化成int16_t, scalar量化
    VDEQS16,          // int32_t量化成int16_t, tensor量化
    QF162S16_PRE,     // half量化成int16_t, scalar量化
    VQF162S16_PRE,    // half量化成int16_t, tensor量化
};
```

### 6.2 QuantMode 分类

| 分类 | QuantMode 值 | 需要 workspace |
|------|-------------|----------------|
| 无量化 | NoQuant | 否 |
| Scalar 量化 | DEQF16, QF322B8_PRE, REQ8, F322F16, F322BF16, QF162B8_PRE, QF162S4_PRE, REQ4, DEQS16, QF162S16_PRE | 否 |
| Tensor 量化 | VDEQF16, VQF322B8_PRE, VREQ8, VQF162B8_PRE, VQF162S4_PRE, VREQ4, VDEQS16, VQF162S16_PRE | **是** |

### 6.3 判断函数

```cpp
// 判断是否为 Tensor 量化模式
__aicore__ inline bool IsVectorQuantMode(QuantMode_t quantPre) {
    return (quantPre == QuantMode_t::VDEQF16 || quantPre == QuantMode_t::VQF162B8_PRE ||
            quantPre == QuantMode_t::VREQ8 || quantPre == QuantMode_t::VQF162S4_PRE ||
            quantPre == QuantMode_t::VREQ4 || quantPre == QuantMode_t::VDEQS16 ||
            quantPre == QuantMode_t::VQF162S16_PRE);
}

// 判断是否为 Scalar 量化模式
__aicore__ inline bool IsScalarQuantMode(QuantMode_t quantPre) {
    return (quantPre == QuantMode_t::DEQF16 || quantPre == QuantMode_t::QF162B8_PRE ||
            quantPre == QuantMode_t::REQ8 || quantPre == QuantMode_t::QF162S4_PRE ||
            quantPre == QuantMode_t::REQ4 || quantPre == QuantMode_t::DEQS16 ||
            quantPre == QuantMode_t::QF162S16_PRE);
}
```

### 6.4 Workspace 使用规则

**需要 workspace 的 QuantMode**（以 `V` 开头）：
- `VDEQF16`, `VQF322B8_PRE`, `VREQ8`
- `VQF162B8_PRE`, `VQF162S4_PRE`, `VREQ4`
- `VDEQS16`, `VQF162S16_PRE`

```cpp
// workspace 必须是 uint64_t 类型
LocalTensor<uint64_t> cbufWorkspace;

// workspace 大小通常与输出通道数相关
uint32_t deqSize = cout;  // 输出通道数
```

---

## 7. 常用枚举类

### 7.1 CO2Layout - L0C数据布局

**定义文件**: `include/basic_api/kernel_struct_fixpipe.h`

```cpp
enum class CO2Layout : uint8_t {
    NZ = 0,          // NZ格式
    ROW_MAJOR,       // ND Row格式
    COLUMN_MAJOR     // ND Column格式
};

// 预定义配置
constexpr FixpipeConfig CFG_NZ = {CO2Layout::NZ};
constexpr FixpipeConfig CFG_ROW_MAJOR = {CO2Layout::ROW_MAJOR};
constexpr FixpipeConfig CFG_COLUMN_MAJOR = {CO2Layout::COLUMN_MAJOR};
```

### 7.2 FmatrixMode - F矩阵模式

**定义文件**: `include/basic_api/kernel_struct_mm.h`

```cpp
enum class FmatrixMode : uint8_t {
    FMATRIX_LEFT = 0,
    FMATRIX_RIGHT = 1,
};
```

---

## 8. Kernel 类封装模式

推荐使用类封装 AIC Kernel 逻辑：

```cpp
class KernelMmad {
public:
    __aicore__ inline KernelMmad() {}
    __aicore__ inline void Init(__gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* c,
        uint16_t mVal, uint16_t kVal, uint16_t nVal)
    {
        this->m = mVal;
        this->k = kVal;
        this->n = nVal;

        // 设置 GlobalTensor
        aGM.SetGlobalBuffer(reinterpret_cast<__gm__ half*>(a));
        bGM.SetGlobalBuffer(reinterpret_cast<__gm__ half*>(b));
        cGM.SetGlobalBuffer(reinterpret_cast<__gm__ half*>(c));

        // 初始化 Buffer
        pipe.InitBuffer(inQueueA1, 1, m * k * sizeof(half));
        pipe.InitBuffer(outQueueCO1, 1, m * n * sizeof(float));
    }

    __aicore__ inline void Process() {
        CopyIn();
        Compute();
        CopyOut();
    }

private:
    __aicore__ inline void CopyIn() { /* ... */ }
    __aicore__ inline void Compute() { /* Mmad 调用 */ }
    __aicore__ inline void CopyOut() { /* Fixpipe 调用 */ }

    TPipe pipe;
    TQue<TPosition::A1, 1> inQueueA1;
    TQue<TPosition::CO1, 1> outQueueCO1;
    GlobalTensor<half> aGM, bGM, cGM;
    uint16_t m, k, n;
};
```

---

## 9. Fixpipe 分支覆盖

### 9.1 数据路径分支

| 路径 | 模板参数 | 说明 |
|-----|---------|------|
| L0C -> GM | `Fixpipe(GlobalTensor, LocalTensor, ...)` | 输出到全局内存 |
| L0C -> L1 | `Fixpipe(LocalTensor, LocalTensor, ...)` | 输出到 L1 缓冲 |
| L0C -> UB | `Fixpipe(LocalTensor, LocalTensor, ...)` | 输出到 UB |

### 9.2 Workspace 分支

```cpp
// 分支1: 无 workspace（普通量化）
template <typename T, typename U, const FixpipeConfig& config>
void Fixpipe(const GlobalTensor<T>& dst, const LocalTensor<U>& src,
    const FixpipeParamsV220& intriParams);

// 分支2: 有 workspace（Tensor 量化）
template <typename T, typename U, const FixpipeConfig& config>
void Fixpipe(const GlobalTensor<T>& dst, const LocalTensor<U>& src,
    const LocalTensor<uint64_t>& cbufWorkspace, const FixpipeParamsV220& intriParams);
```

### 9.3 FixpipeConfig 分支

| Config | 说明 | 测试用例 |
|--------|------|---------|
| `CFG_ROW_MAJOR` | 行主序输出 | enNz2nd = true |
| `CFG_NZ` | NZ 格式输出 | enNz2nd = false |
| `CFG_COLUMN_MAJOR` | 列主序输出 | 特定场景 |

### 9.4 分支覆盖测试策略

| 分支类型 | 测试策略 |
|---------|---------|
| 数据类型组合 | 9 种组合全覆盖 |
| QuantMode | NoQuant, F322F16, DEQF16, VDEQF16 等 |
| reluEn | true / false |
| enNz2nd (Layout) | ROW_MAJOR / NZ |
| workspace | 有 / 无 |

### 9.5 Fixpipe 分支覆盖检查清单

- [ ] **数据类型组合**: 9 种组合全覆盖
- [ ] **QuantMode**: NoQuant, F322F16, DEQF16, VDEQF16 等
- [ ] **reluEn**: true / false
- [ ] **enNz2nd (Layout)**: ROW_MAJOR / NZ
- [ ] **workspace**: 有 / 无

---

## 10. 测试模板引用

通用 gtest、参数化测试和结果校验骨架见 [测试模板参考](../foundations/test-templates.md)。AIC guide 维护 Kernel 类封装、Fixpipe、QuantMode 和 workspace 等 AIC 特有约束：

- `g_coreType` 切换到 `AscendC::AIC_TYPE`，结束时执行 `CheckSyncState()` 并恢复 `MIX_TYPE`。
- 非目标核心路径需要显式跳过，例如 `if ASCEND_IS_AIV { return; }`。
- Fixpipe 类型组合、QuantMode 和 workspace 分支以本 guide 的 Fixpipe 章节为准。

---

## 11. 常见错误

### 11.1 QuantMode 与类型不匹配

```cpp
// ❌ 错误: F322F16 需要 float L0C -> half GM
Fixpipe<half>(cGM, c1Local, fixpipeParams);  // 类型错误

// ✅ 正确
Fixpipe<float, half>(cGM, c1Local, fixpipeParams);
```

### 11.2 忘记传递 workspace

```cpp
// ❌ 错误: VDEQF16 需要 workspace
Fixpipe(cGM, c1Local, fixpipeParams);

// ✅ 正确
Fixpipe(cGM, c1Local, cbufWorkspace, fixpipeParams);
```

### 11.3 workspace 类型错误

```cpp
// ❌ 错误: workspace 必须是 uint64_t
LocalTensor<half> cbufWorkspace;

// ✅ 正确
LocalTensor<uint64_t> cbufWorkspace;
```

### 11.4 参数结构初始化失败

```cpp
// ❌ 错误: 花括号初始化不支持
FixpipeParamsV220 params = {n, m, stride, ...};

// ✅ 正确: 使用成员赋值
FixpipeParamsV220 params;
params.nSize = n;
params.mSize = m;
```

---

## 12. 常见问题

AIC guide 只保留 AIC 特有约束。以下公共排障项统一查看 [常见问题与解决方案](../troubleshooting/faq.md)：

- [参数结构初始化问题](../troubleshooting/faq.md#1-参数结构初始化问题)
- [Mock/Stub 函数参数不匹配](../troubleshooting/faq.md#2-mockstub-函数参数不匹配)
- [核心类型不匹配](../troubleshooting/faq.md#3-核心类型不匹配)
- [QuantMode 与数据类型组合](../troubleshooting/faq.md#4-quantmode-与数据类型组合)
- [LocalTensor / TPipe Buffer 初始化问题](../troubleshooting/faq.md#9-localtensor--tpipe-buffer-初始化问题)

---

## 13. 检查清单

### 13.1 分析阶段

- [ ] 已从目标架构 impl 文件确认数据类型支持 (SupportType)
- [ ] 已确认 API 是 AIC 类型
- [ ] 已分析 impl 文件中的所有分支

### 13.2 编写阶段

- [ ] SetUp 中正确设置 `g_coreType = AIC_TYPE`
- [ ] **Params 结构体使用成员赋值（非花括号初始化）**
- [ ] TearDown 中调用 `CheckSyncState()` 和恢复 `MIX_TYPE`
- [ ] 使用类封装测试逻辑
- [ ] 使用 `if ASCEND_IS_AIV { return; }` 跳过非目标核心

### 13.3 QuantMode 检查

- [ ] QuantMode 与输入类型匹配
- [ ] QuantMode 与 L0C 类型匹配
- [ ] QuantMode 与输出类型匹配
- [ ] 需要 workspace 的 QuantMode 已传递 workspace
- [ ] workspace 类型为 uint64_t
- [ ] deqScalar 参数已设置（Scalar 模式）

### 13.4 验证阶段

- [ ] 编译通过无错误
- [ ] 测试执行通过
- [ ] 结果验证正确

---

## 14. 相关文件索引

| 文件路径 | 说明 |
|---------|------|
| `include/basic_api/kernel_struct_fixpipe.h` | Fixpipe 相关结构体 |
| `include/basic_api/kernel_struct_mm.h` | MM 相关结构体 |
| `include/basic_api/kernel_type.h` | 基础类型定义 |
| `impl/basic_api/utils/kernel_utils_mode.h` | 枚举模式定义 |

---

## 15. 相关参考

| 文档 | 说明 |
|------|------|
| [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md) | 内存管理详细说明 |
| [分支覆盖分析指南](../foundations/branch-coverage-guide.md) | 分支分析方法 |
| [AIV API UT 指南](membase-api-aiv-ut-guide.md) | AIV (Vector 核心) API 测试指南 |
| [常见问题汇总](../troubleshooting/faq.md) | 常见问题与解决方案汇总 |
| [测试模板参考](../foundations/test-templates.md) | 通用测试骨架与模板选择索引 |
