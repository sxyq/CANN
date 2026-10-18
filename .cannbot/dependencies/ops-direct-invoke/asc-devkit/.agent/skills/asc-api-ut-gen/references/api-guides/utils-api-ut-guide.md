# 工具类 API (Utils API) UT 生成指南

## 1. API 概述

工具类 API (Utils API) 是 AscendC 提供的辅助工具接口，包括内存管理、对齐检查、类型判断等辅助功能。

### 1.1 目录位置

| 类型 | 路径 |
|------|------|
| **头文件** | `{ASC_DEVKIT_PATH}/include/utils/` |
| **实现文件** | `{ASC_DEVKIT_PATH}/impl/utils/` |
| **测试目录** | `{ASC_DEVKIT_PATH}/tests/api/utils/` |

`include/basic_api/**` 和 `impl/basic_api/**` 下的 helper 仍按 membase/basic API 归类，不属于本 guide 的 utils API 范围。

### 1.2 典型 API 示例

| API 类别 | 典型 API | 功能说明 |
|---------|---------|---------|
| 对齐检查 | `check_align` | 检查地址或大小对齐 |
| 类型判断 | `is_same_type` | 编译期类型判断 |
| 内存计算 | `get_padding_size` | 计算填充大小 |
| 数值操作 | `max`, `min`, `abs` | 数值比较和运算 |
| 位操作 | `bit_extract`, `bit_set` | 位级操作 |

---

## 2. API 特点

### 2.1 辅助功能

工具类 API 主要提供辅助功能：

```cpp
// 对齐检查
__aicore__ inline bool IsAligned(uint64_t addr, uint32_t alignment);

// 获取对齐后的大小
__aicore__ inline uint32_t AlignUp(uint32_t size, uint32_t alignment);

// 类型判断
template<typename T1, typename T2>
struct is_same { static constexpr bool value = false; };
```

### 2.2 编译期计算

部分工具类 API 在编译期执行：

```cpp
// 编译期常量
constexpr uint32_t BLOCK_SIZE = 32;

// 编译期类型判断
if constexpr (std::is_same_v<T, half>) {
    // half 特定处理
}
```

### 2.3 跨架构通用

工具类 API 通常跨架构通用：

- 不依赖特定硬件特性
- 提供通用辅助功能
- 可在多架构复用

### 2.4 内联实现

多数工具类 API 采用内联实现：

```cpp
__aicore__ inline uint32_t AlignUp(uint32_t size, uint32_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}
```

---

## 3. UT 生成注意事项

### 3.1 测试类型分类

工具类 API 测试可分为两类：

| 测试类型 | 说明 | 示例 |
|---------|------|------|
| **运行时测试** | 测试运行时行为 | `AlignUp`, `IsAligned` |
| **编译期测试** | 测试编译期行为 | `is_same_type`, `constexpr` |

### 3.2 运行时测试模式

```cpp
class TEST_AlignUp : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

TEST_F(TEST_AlignUp, AlignUp_Basic) {
    // 测试对齐计算
    EXPECT_EQ(AlignUp(30, 32), 32);
    EXPECT_EQ(AlignUp(32, 32), 32);
    EXPECT_EQ(AlignUp(33, 32), 64);
    EXPECT_EQ(AlignUp(0, 32), 0);
}

TEST_F(TEST_AlignUp, AlignUp_EdgeCases) {
    // 边界条件测试
    EXPECT_EQ(AlignUp(UINT32_MAX - 1, 32), UINT32_MAX - 31);
}
```

### 3.3 编译期测试模式

```cpp
// 使用 static_assert 测试编译期行为
static_assert(is_same<half, half>::value, "should be true");
static_assert(!is_same<half, float>::value, "should be false");

// 使用 constexpr 函数测试
constexpr uint32_t aligned = AlignUp(100, 32);
static_assert(aligned == 128, "alignment calculation should be correct");
```

### 3.4 架构无关性

工具类 API 通常无需区分架构：

```cpp
// 无需设置特定核心类型
TEST_F(TEST_Utils, Utils_Basic) {
    // 直接测试
    EXPECT_EQ(UtilFunction(...), expected_value);
}
```

### 3.5 自动生成边界

utils API 不是单一形状：`Std::remove_pointer` 这类 trait 主要是编译期行为，
`Std::abs` 这类函数会进入 device kernel，`tiling` 和 `context` 又依赖 host
对象、mock 和平台信息。某个 utils family 的可执行模板在真实 UT 中验证前，
通用生成器不得把这些类别塞进同一骨架。

当前流程要求先按 `std`、`tiling`、`context` 等子类读取声明、实现和同类 UT；
如果还没有经验证的可复用模板，就直接写 API-specific UT。

---

## 4. 分支覆盖要点

### 4.1 边界条件

工具类 API 常有边界条件分支：

```cpp
TEST_F(TEST_AlignUp, AlignUp_Boundaries) {
    // 零值
    EXPECT_EQ(AlignUp(0, 32), 0);

    // 已对齐值
    EXPECT_EQ(AlignUp(32, 32), 32);
    EXPECT_EQ(AlignUp(64, 32), 64);

    // 未对齐值
    EXPECT_EQ(AlignUp(1, 32), 32);
    EXPECT_EQ(AlignUp(31, 32), 32);
    EXPECT_EQ(AlignUp(33, 32), 64);

    // 大值
    EXPECT_EQ(AlignUp(1024, 32), 1024);
    EXPECT_EQ(AlignUp(1025, 32), 1056);
}
```

### 4.2 参数组合

测试不同参数组合：

```cpp
INSTANTIATE_TEST_CASE_P(TEST_ALIGN, AlignTestsuite,
    ::testing::Values(
        AlignTestParams { 30, 16, 32 },
        AlignTestParams { 30, 32, 32 },
        AlignTestParams { 30, 64, 64 },
        AlignTestParams { 100, 16, 112 },
        AlignTestParams { 100, 32, 128 }
    ));
```

### 4.3 类型组合

对于模板工具函数，测试不同类型：

```cpp
TEST_F(TEST_Max, Max_DifferentTypes) {
    EXPECT_EQ(Max(1, 2), 2);
    EXPECT_EQ(Max(1.0f, 2.0f), 2.0f);
    EXPECT_EQ(Max(1.0, 2.0), 2.0);
    EXPECT_EQ(Max(int8_t(1), int8_t(2)), int8_t(2));
}
```

### 4.4 错误处理

测试错误输入处理：

```cpp
TEST_F(TEST_CheckAlign, CheckAlign_Invalid) {
    // 非对齐地址
    EXPECT_FALSE(IsAligned(1, 32));
    EXPECT_FALSE(IsAligned(31, 32));
    EXPECT_FALSE(IsAligned(33, 32));

    // 对齐地址
    EXPECT_TRUE(IsAligned(0, 32));
    EXPECT_TRUE(IsAligned(32, 32));
    EXPECT_TRUE(IsAligned(64, 32));
}
```

---

## 5. 测试模板引用

通用 gtest、参数化测试和精度验证骨架见 [测试模板参考](../foundations/test-templates.md)。Utils guide 只维护工具类 API 特有约束：

- 对齐计算类 API 使用边界表驱动测试，覆盖 0、已对齐、差 1、超过 1 和不同 alignment。
- 类型判断、常量计算等编译期工具优先使用 `static_assert`，必要时只保留一个 `SUCCEED()` 占位用例。
- 纯 host 逻辑不要引入不必要的 device kernel 或 LocalTensor。

---

## 6. 编译与执行

Utils API 的编译、执行、失败修复和验证报告统一按 [自动化验证流程](../workflows/automation-guide.md) 处理。执行前按子类确认 `build/tests/api/utils/` 下的实际测试可执行文件名。

---

## 7. 常见问题

### Q1: 编译期测试如何编写？

**问题**：不确定如何测试编译期行为

**解决**：
- 使用 `static_assert` 进行编译期断言
- 使用 `constexpr` 验证编译期计算

### Q2: 工具类 API 需要设置核心类型吗？

**问题**：是否需要 SetGCoreType

**解决**：
- 多数工具类 API 不依赖核心类型
- 参考具体 API 文档确认

### Q3: 如何处理模板函数测试？

**问题**：模板工具函数测试复杂

**解决**：
- 使用显式模板实例化
- 使用参数化测试覆盖类型组合

---

## 8. 检查清单

### 8.1 分析阶段

- [ ] 已读取工具类 API 头文件
- [ ] 已确认 API 功能和行为
- [ ] 已参考现有测试模式

### 8.2 编写阶段

- [ ] 区分运行时测试和编译期测试
- [ ] 边界条件已覆盖
- [ ] 参数组合已覆盖
- [ ] 类型组合已覆盖（模板函数）

### 8.3 验证阶段

- [ ] 编译通过
- [ ] 编译期断言正确
- [ ] 运行时测试通过
- [ ] 边界条件正确处理

---

## 9. 常用工具类 API

### 9.1 对齐相关

```cpp
// 对齐计算
__aicore__ inline uint32_t AlignUp(uint32_t size, uint32_t alignment);
__aicore__ inline uint32_t AlignDown(uint32_t size, uint32_t alignment);

// 对齐检查
__aicore__ inline bool IsAligned(uint64_t addr, uint32_t alignment);
```

### 9.2 数值操作

```cpp
// 最大最小值
template<typename T>
__aicore__ inline T Max(T a, T b);

template<typename T>
__aicore__ inline T Min(T a, T b);

// 绝对值
template<typename T>
__aicore__ inline T Abs(T value);
```

### 9.3 类型判断

```cpp
// 类型相同判断
template<typename T1, typename T2>
struct is_same {
    static constexpr bool value = false;
};

template<typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};
```

---

## 10. 相关参考

| 文档 | 说明 |
|------|------|
| [分支覆盖分析指南](../foundations/branch-coverage-guide.md) | 分支分析方法 |
| [测试模板参考](../foundations/test-templates.md) | 通用测试骨架与模板选择索引 |
