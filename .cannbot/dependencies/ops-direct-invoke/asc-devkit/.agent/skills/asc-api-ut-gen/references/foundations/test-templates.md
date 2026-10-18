# 测试模板参考

## 概述

本文档只维护跨 API 可复用的 UT 骨架、占位符约定和模板选择索引，避免与各场景 guide 重复维护 API 特有代码。

使用模板前必须先读取目标 API 的声明、impl、设计文档或已有同类 UT。模板中的 dtype、shape、count、kernel 名、核心类型、目录和可执行文件名都必须按真实 API 替换；不要从本文档推断 API 支持范围。

## 1. 模板职责边界

| 内容 | 事实来源 |
|------|----------|
| 通用 gtest fixture、参数化测试、结果比较骨架 | 本文档 |
| AIV 核心类型、TPosition、TmpBuffer 和结构体参数 | [membase AIV API UT 指南](../api-guides/membase-api-aiv-ut-guide.md) |
| AIC 核心类型、Kernel 类封装、Fixpipe、QuantMode 和 workspace | [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md) |
| C API mockcpp、指针空间标记和底层指令断言 | [C API UT 指南](../api-guides/c-api-ut-guide.md) |
| LocalTensor、TPipe、TQue 和临时空间申请 | [LocalTensor 内存申请指南](local-tensor-memory.md) |
| 分支覆盖矩阵和用例组合策略 | [分支覆盖分析指南](branch-coverage-guide.md) |
| 编译、执行、失败修复和报告格式 | [自动化验证流程](../workflows/automation-guide.md) |
| 芯片、`__NPU_ARCH__`、SocVersion、dtype 大小和 32B 对齐事实 | [asc-npu-arch 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md) |

## 2. 通用文件骨架

适用于需要 gtest fixture 的 UT。核心类型设置和 include 列表按目标 API 所属 guide 调整。

```cpp
#include <gtest/gtest.h>
#include "kernel_operator.h"

using namespace AscendC;

class TEST_{API_NAME} : public testing::Test {
protected:
    void SetUp() override
    {
        // 按目标 API 场景设置核心类型。
    }

    void TearDown() override
    {
        // 按目标 API 场景恢复核心类型并做必要同步检查。
    }
};

TEST_F(TEST_{API_NAME}, Level1_{ApiName}_{Scenario})
{
    // 1. 构造输入、输出、workspace 或 LocalTensor。
    // 2. 调用目标 API 或测试 kernel。
    // 3. 用精确断言或 CompareResult 校验结果。
}
```

## 3. 参数化测试骨架

参数结构只放目标 API 的真实变量维度。不要把通用 dtype 列表、固定 shape 或固定 count 写进模板。

```cpp
struct {ApiName}TestParams {
    // 示例字段：shape、count、stride、mode、kernel 函数指针等。
};

class {ApiName}Testsuite : public testing::Test,
                           public testing::WithParamInterface<{ApiName}TestParams> {
protected:
    void SetUp() override
    {
        // 按目标 API 场景设置核心类型。
    }

    void TearDown() override
    {
        // 按目标 API 场景恢复核心类型。
    }
};

INSTANTIATE_TEST_CASE_P(TEST_{API_NAME}, {ApiName}Testsuite,
    ::testing::Values(
        {ApiName}TestParams { /* case 1: confirmed supported branch */ },
        {ApiName}TestParams { /* case 2: confirmed boundary branch */ }
    ));

TEST_P({ApiName}Testsuite, {ApiName}TestCase)
{
    const auto param = GetParam();
    // 使用 param 构造输入并校验目标 API 行为。
}
```

## 4. 结果比较骨架

优先使用目标 API 已有同类 UT 的误差阈值。没有同类参考时，阈值必须结合 dtype、算子语义和累计误差确定。

```cpp
template <typename T>
bool CompareResult(const T* output, const T* golden, uint32_t size, float rtol, float atol)
{
    for (uint32_t i = 0; i < size; i++) {
        const float actual = static_cast<float>(output[i]);
        const float expect = static_cast<float>(golden[i]);
        const float threshold = atol + rtol * std::abs(expect);
        if (std::abs(actual - expect) > threshold) {
            return false;
        }
    }
    return true;
}
```

## 5. AIV（Vector 核心）API 测试模板

AIV UT 使用第 2、3、4 节的通用骨架，再叠加 [membase AIV API UT 指南](../api-guides/membase-api-aiv-ut-guide.md) 中的核心类型、LocalTensor、TPosition、TmpBuffer 和结构体参数规则。

生成时至少确认：

- 目标 API 是 AIV 路径，不是 AIC、regbase、C API 或 SIMT 路径。
- `SetUp` / `TearDown` 使用 AIV guide 指定的核心类型模式。
- dtype、count、mask、repeat、stride、sharedTmpBuffer 等参数来自目标 API 支持范围。
- 输入输出空间、LocalTensor 位置和临时空间大小已从 impl 或已有 UT 验证。

## 6. AIC（Cube 核心）API 测试模板

AIC UT 使用第 2、3、4 节的通用骨架，再叠加 [membase AIC API UT 指南](../api-guides/membase-api-aic-ut-guide.md) 中的核心类型、Kernel 类封装、Fixpipe、QuantMode、workspace 和参数结构初始化规则。

生成时至少确认：

- 非目标核心路径按已有 UT 模式跳过。
- AIC Kernel 的 `Init`、`Process`、`CopyIn`、`Compute`、`CopyOut` 生命周期与目标 API 数据流一致。
- Params 结构体按 AIC guide 使用成员赋值，不使用不兼容的花括号初始化。
- Fixpipe 覆盖真实支持的 dtype、QuantMode、layout、relu 和 workspace 分支。

## 7. C API 测试模板

C API UT 使用第 2、3 节的 gtest 骨架；mockcpp、指针空间标记、底层指令 stub 和断言方式统一参考 [C API UT 指南](../api-guides/c-api-ut-guide.md)。

生成时至少确认：

- include 使用目标 C API 需要的头文件和已有同类 UT 的 stub 头。
- `g_coreType` 选择目标接口实际运行的 AIC 或 AIV 类型。
- Mock 函数签名与底层指令完全一致。
- 指针空间标记、地址、count、stride、repeat、mask、mode 和标量参数逐项断言。

## 8. 高阶、regbase、SIMT 和 Utils API

这些 API 不在本文档维护专用代码块，统一从对应 guide 组合通用骨架和场景约束：

| API 类型 | 参考 guide |
|----------|------------|
| 高阶 API | [高阶 API UT 指南](../api-guides/adv-api-ut-guide.md) |
| regbase API | [regbase 基础 API UT 指南](../api-guides/regbase-api-ut-guide.md) |
| SIMT API | [SIMT API UT 指南](../api-guides/simt-api-ut-guide.md) |
| Utils API | [工具类 API UT 指南](../api-guides/utils-api-ut-guide.md) |

## 9. 检查清单

- [ ] 已确认目标 API 类型、目标芯片和架构限制。
- [ ] 已读取目标 API 声明、impl、设计文档或已有同类 UT。
- [ ] 已从场景 guide 确认核心类型、目录、include 和内存模型。
- [ ] 参数化用例只覆盖真实支持的 dtype、shape、mode 和分支。
- [ ] 结果校验使用目标 API 语义匹配的断言或误差阈值。
- [ ] 编译、执行和报告格式按 [自动化验证流程](../workflows/automation-guide.md) 处理。
