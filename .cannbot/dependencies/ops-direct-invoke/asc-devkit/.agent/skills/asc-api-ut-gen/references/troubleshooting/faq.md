# 常见问题与解决方案

本文件归档跨 API 类型、跨 guide 复用的 UT 排障项。单一 API 类型专属规则仍保留在对应 guide 中。

## 1. 参数结构初始化问题

### 问题现象

```
error: no matching constructor for initialization of 'FixpipeParamsV220'
```

### 根本原因

部分参数结构体（如 `FixpipeParamsV220`, `LoadData2dParams` 等）未定义花括号初始化构造函数。

### 通用解决方案

```cpp
// ❌ 错误：花括号初始化不支持
FixpipeParamsV220 params = {n, m, stride, ...};

// ✅ 正确：使用成员赋值
FixpipeParamsV220 params;
params.nSize = n;
params.mSize = m;
params.srcStride = stride;
params.dstStride = n;
params.quantPre = quantMode;
params.reluEn = reluFlag;
```

**规则：对于所有 Params 结构体，优先使用成员赋值。**

> 详细参数类型说明请参考 [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md)

---

## 2. Mock/Stub 函数参数不匹配

### 问题现象

```
segmentation fault (core dumped)
```

或者 mock 期望不命中、底层指令 stub 运行异常。

### 原因分析

UT 框架中的 mock/stub 函数签名与实际 impl 调用不一致，例如参数数量、参数类型、指针地址空间标记或参数顺序不匹配。

### 解决方法

1. 对照目标 impl 的实际调用点确认参数数量和顺序。
2. 检查 `cce_stub.h`、mock 函数和测试中声明的签名是否完全一致。
3. 指针参数必须保留正确的地址空间标记，如 `__ubuf__`、`__gm__`、`__ca__`、`__cb__`、`__cc__`。
4. 参考同架构下已有成功测试的 mock/stub 写法。

> 详细 AIC 测试模式请参考 [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md)，C API 指针规则请参考 [C API UT 指南](../api-guides/c-api-ut-guide.md)。

---

## 3. 核心类型不匹配

### 问题现象

测试执行失败，设备状态异常。

### 解决方法

```cpp
// 确保 Cube 核心 API 在 SetUp 中切换类型
class TEST_FIXPIPE : public testing::Test {
protected:
    void SetUp() {
        g_coreType = AscendC::AIC_TYPE;  // 必须设置
    }
    void TearDown() {
        AscendC::CheckSyncState();       // 检查同步状态
        g_coreType = AscendC::MIX_TYPE;  // 恢复混合模式
    }
};
```

---

## 4. QuantMode 与数据类型组合

### 问题现象

Fixpipe 输出异常或测试失败。

### 快速参考

| QuantMode | 需要 workspace | 量化类型 |
|-----------|----------------|---------|
| NoQuant | 否 | 无量化 |
| F322F16, F322BF16 | 否 | Scalar 量化 |
| DEQF16, REQ8 | 否 | Scalar 量化 |
| **VDEQF16, VQF322B8_PRE, VREQ8** | **是** | **Tensor 量化** |

**关键规则**：
- QuantMode 名称以 `V` 开头的需要 workspace
- workspace 类型必须是 `LocalTensor<uint64_t>`

> 完整对照表请参考 [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md) 中的 QuantMode 章节

---

## 5. 架构或测试目录不匹配

### 问题现象

编译报错找不到 API 定义，或者测试文件放入目录后没有被目标 target 编译。

### 解决方法

1. 芯片调用名、`__NPU_ARCH__`、`SocVersion` 和 `arch_dir` 统一从 [`asc-npu-arch`](../../../asc-npu-arch/SKILL.md) 与 [`npu-arch-facts.json`](../../../asc-npu-arch/references/npu-arch-facts.json) 获取。
2. 检查 API 头文件、impl 文件和已有 UT，确认目标架构确实支持该 API。
3. 测试目录按 API 类型选择，不要只凭 API 名称判断：
   - membase AIC/AIV：`tests/api/basic_api/${arch_dir}/${arch_dir}_aic/` 或 `${arch_dir}_aiv/`
   - regbase：`tests/api/reg_compute_api/`
   - SIMT：`tests/api/simt_api/ascend950pr_9599/`
   - utils：`tests/api/utils_api/`

---

## 6. TmpBuffer / workspace 大小与分支覆盖

### 问题现象

API 签名中包含 sharedTmpBuffer 参数，如何确定大小？

### 解决方法

1. 查看 API 头文件注释
2. 从 impl 文件分析内部计算
3. 对高阶 API，还需要检查 params/config/tiling 中 workspace 的大小、位置和启用条件
4. 参考现有测试文件

### 覆盖规则

TmpBuffer 和 workspace 大小不是通用 dtype 事实，必须从目标 API 注释、impl 内部临时空间计算、tiling 逻辑和已有同类 UT 中确认。覆盖时至少区分：

- 不走 workspace / sharedTmpBuffer 的普通路径；
- 明确走 `sharedTmpBuffer`、`TBuf`、`PopStackBuffer` 或 workspace 分支的路径。

详细内存申请模式统一参考 [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md#72-需要临时空间的-api)。

---

## 7. 编译报错找不到底层指令

### 问题现象

```
undefined reference to 'vadd'
```

### 解决方法

```cpp
// 检查 cce_stub.h 是否已声明该指令
// 如果没有，需要添加声明
void cce_instruction_name(type1 arg1, type2 arg2);
```

同时确认目标架构、测试目录和编译 target 与该底层指令所在实现匹配。

---

## 8. 数据对齐与内存大小计算

### 问题现象

运行时出现内存访问越界，或 LocalTensor 数据只初始化了一部分。

### Count 值计算

```python
# 公式 1: 最小大小约束
count >= ceil(min_tensor_size / sizeof(data_type))

# 公式 2: 32 字节对齐约束
count * sizeof(T) % 32 == 0

# 推荐值：先按 min_tensor_size 推出最小 count，再向上取满足 32B 对齐的 count
# 具体 dtype 的 sizeof 和 32B 对齐元素数见 asc-npu-arch 架构指南
```

### 对齐约束快速计算

基础 dtype 的 `sizeof` 与 32B 对齐元素数统一见 [`asc-npu-arch` 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md#基础-dtype-大小与-32b-对齐)。FAQ 中的 count 推荐值应按该统一表和具体 API 的最小 tensor 大小共同计算。

### 关键规则

- `InitBuffer` 的字节长度使用 `count * sizeof(T)`。
- host 侧初始化非 byte dtype 时按 `T*` 或 `std::vector<T>` 写入，不要只初始化 `count` 个字节。
- 对需要临时空间的 API，临时空间大小按 API 规则确认，不按 dtype 做通用推断。

---

## 9. LocalTensor / TPipe Buffer 初始化问题

### 问题现象

编译提示 Buffer 数量超限，运行时出现内存冲突、数据损坏，或 TQue 位置设置错误。

### 解决方法

1. 检查一个 kernel 中所有 `InitBuffer` 调用的数量总和，double buffer 会占用两倍 Buffer 数量。
2. 优先复用已分配内存，减少不必要的 TQue/TBuf。
3. 不建议混用自定义地址 `InitBuffer` 与不指定地址的自动分配 `InitBuffer`。
4. 按核心类型选择正确的 `TPosition`：
   - AIV：`VECIN`、`VECOUT`、`VECCALC`
   - AIC：`A1`、`A2`、`B1`、`B2`、`CO1`、`CO2`

---

## 10. 枚举类作用域错误

### 问题现象

编译报错枚举值未定义或类型不匹配。

### 解决方法

```cpp
// 错误：枚举类需要使用完整路径
RoundMode mode = CAST_RINT;

// 正确
RoundMode mode = RoundMode::CAST_RINT;
```

先查看 API 参数类型，如果是 `enum class`，调用处必须带枚举类作用域。

---

## 11. 参考测试找不到

### 问题现象

不知道如何组织某个 API 的 UT，或者不确定 target、目录和 fixture 写法。

### 解决方法

1. 先在对应 API 类型目录下查找同类 API 测试。
2. 优先参考同架构、同核心类型、同 dtype 或同参数结构的测试。
3. 不要跨 API 类型直接套模板；只能复用通用 gtest、参数化测试和内存申请骨架。

---

## 12. UT 执行时间过长

### 问题现象

UT 执行明显变慢，影响本地回归或 CI。

### 解决方法

1. 减少重复 shape 和冗余 dtype 组合，保留能触发分支的最小集合。
2. 大 shape 只保留少量 smoke 或边界用例，普通分支用小 shape 覆盖。
3. UT 的目标是稳定覆盖 API 合约，不是优化目标算子的 NPU 流水性能。

---

## 13. API 类型判断

### 问题现象

不确定一个 API 应按高阶 API、membase basic、regbase、C API、SIMT 还是 utils 写 UT。

### 解决方法

看入口目录和实现目录，不要只凭 API 名称判断：

- `include/adv_api/` 下的 API 按高阶 API。
- `include/basic_api/` 下且不在 `reg_compute/` 子目录的 API 按 membase basic。
- `include/basic_api/reg_compute/` 下的 API 按 regbase。
- `include/c_api/` 下的 API 按 C API。
- `include/simt_api/` 下的 API 按 SIMT API。
- 工具类 API 按 utils guide 处理。

尤其不要把 `Add`、`Sub`、`Mul`、`Div` 这类基础 vector API 放进高阶 API guide。

---

## 14. 测试用例设计建议

### DO（推荐）

```cpp
// 1. 使用类封装，结构清晰
class KernelFixpipe { ... };

// 2. 简单的测试断言
for (int32_t i = 0; i < size; i++) {
    EXPECT_EQ(output[i], expected_value);
}

// 3. 使用 ASCEND_IS_AIV 宏跳过非目标核心
if ASCEND_IS_AIV {
    return;
}
```

### DON'T（避免）

```cpp
// 1. 直接使用 LOCAL_TENSOR_REGISTER 宏（复杂场景易出错）
// 2. 过于复杂的测试配置组合
// 3. 未经验证的 QuantMode 与类型组合
```

---

## 15. 相关文档索引

| 问题类型 | 参考文档 |
|---------|---------|
| AIC 测试模式 | [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md) |
| AIV 测试模式 | [membase AIV API UT 指南](../api-guides/membase-api-aiv-ut-guide.md) |
| 内存申请 | [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md) |
| 分支覆盖 | [分支覆盖分析指南](../foundations/branch-coverage-guide.md) |
| 自动化流程 | [自动化验证流程](../workflows/automation-guide.md) |
| 架构事实 | [Ascend NPU 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md) |
