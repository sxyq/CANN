# regbase 基础 API (Register-based Basic API) UT 生成指南

## 1. API 概述

regbase 基础 API 是 AscendC 中基于 register 的基础接口，提供底层硬件操作能力，仅支持特定架构。

### 1.1 目录位置

| 类型 | 路径 |
|------|------|
| **头文件** | `{ASC_DEVKIT_PATH}/include/basic_api/reg_compute/` |
| **实现文件** | `{ASC_DEVKIT_PATH}/impl/basic_api/reg_compute/` |
| **测试目录** | `{ASC_DEVKIT_PATH}/tests/api/reg_compute_api/` |

### 1.2 架构支持限制

**重要：regbase 基础 API 仅支持以下架构：**

| 架构 | NPU_ARCH | 芯片名 | 测试目录 |
|------|----------|--------|----------|
| dav_3510 | 3510 | **ascend950pr_9599** | ascendc_case_ascend950pr_9599_reg_compute |

> **不支持 ascend910、ascend910b1、ascend310p、ascend310b1 等架构**

### 1.3 典型 API 示例

| API 类别 | 典型 API |
|---------|---------|
| 寄存器操作 | `RegAdd`, `RegMul`, `RegSub` |
| 内存操作 | `RegLoad`, `RegStore` |
| 索引操作 | `CreIndex`, `Arange` |

---

## 2. API 特点

### 2.1 底层硬件操作

regbase API 直接操作寄存器，提供更细粒度的硬件控制：

```cpp
// 基于寄存器的向量操作
__aicore__ inline void RegAdd(const LocalTensor<T>& dst,
                               const LocalTensor<T>& src0,
                               const LocalTensor<T>& src1,
                               uint32_t count);
```

### 2.2 特定架构优化

这些 API 针对特定架构进行优化，利用硬件特性：

- 3510 (ascend950pr_9599): 新一代 AI 核心架构

### 2.3 与 membase API 的区别

| 特性 | membase 基础 API | regbase 基础 API |
|------|-----------------|-----------------|
| 数据位置 | Memory (UB, L1 等) | Register |
| 架构支持 | 所有架构 | 仅 3510 |
| 抽象层级 | 较高 | 较低 |
| 性能 | 通用优化 | 特定架构优化 |

---

## 3. UT 生成注意事项

### 3.1 架构限制检查

**生成 UT 前必须确认目标架构支持：**

```bash
# 确认架构支持
# 仅以下架构可生成 regbase API UT
/ascend950pr_9599    # NPU_ARCH=3510
```

### 3.2 测试目录结构

```
tests/api/reg_compute_api/
└── ascendc_case_ascend950pr_9599_reg_compute/
    ├── test_operator_reg_compute_add.cpp
    ├── test_operator_reg_compute_mul.cpp
    └── ...
```

### 3.3 参考现有测试模式

**必须参考 `tests/api/reg_compute_api/` 下的现有测试文件：**

- 查看同类 API 的测试模式
- 了解内存分配和数据初始化方式
- 学习断言验证方法

### 3.4 数据类型支持

从 impl 文件确认数据类型支持：

```cpp
// impl 文件中的 SupportType 定义
using SupportType = std::tuple<half, float, int32_t>;
```

### 3.5 自动生成边界

regbase API 按 register family 的参数、寄存器类型和 mask 语义差异很大。
在某个 family 的可执行模板经过验证前，通用生成器不得输出带 `TODO` 的
占位测试，也不得把 membase 风格的内存模板伪装成 regbase UT。

当前流程要求先读目标 impl，再读 `tests/api/reg_compute_api/` 下同 family UT；
如果还没有可复用的可执行模板，就直接写 API-specific UT，而不是生成占位壳。

---

## 4. 测试模板引用

通用 gtest、参数化测试和结果校验骨架见 [测试模板参考](../foundations/test-templates.md)。regbase guide 只维护 regbase 特有约束：

- 只为 `ascend950pr_9599` 生成 regbase UT。
- 先从目标架构 impl、设计文档或 `SupportType` 确认真实支持类型。
- 测试目录必须落到对应架构的 reg_compute 目录。

---

## 5. 分支覆盖要点

### 5.1 架构条件编译

```cpp
// impl 文件中可能存在的架构分支
#if __NPU_ARCH__ == 3510
    // ascend950pr_9599 特定实现
#endif
```

**测试策略**：
- 3510 架构测试放在 `ascendc_case_ascend950pr_9599_reg_compute/`

### 5.2 数据类型组合

根据目标架构 impl 文件、设计文档或 `SupportType` 确认真实支持类型，并使用参数化测试覆盖已确认组合。通用基础 dtype 的名称、大小和 32B 对齐要求统一回链到 [`asc-npu-arch` 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md#统一数据类型视图)，本 guide 不维护并行 dtype 表。

### 5.3 特殊参数组合

某些 regbase API 可能有特殊参数：

- 索引模式
- 循环模式
- 边界处理方式

**策略**：使用参数化测试覆盖所有参数组合

---

## 6. 编译与执行

regbase 的编译、执行、失败修复和验证报告统一按 [自动化验证流程](../workflows/automation-guide.md) 处理。执行前根据目标芯片确认 `build/tests/api/reg_compute_api/` 下的实际架构目录和测试可执行文件名。

---

## 7. 常见问题

### Q1: 架构不支持？

**问题**：编译报错找不到 API 定义

**原因**：regbase API 仅支持 3510 架构

**解决**：
- 确认目标芯片是 ascend950pr_9599
- 使用正确的测试目录
- 芯片与目录映射统一参考 [架构或测试目录不匹配](../troubleshooting/faq.md#5-架构或测试目录不匹配)

### Q2: 公共排障索引

regbase 测试目录和参考测试查找统一查看 [常见问题与解决方案](../troubleshooting/faq.md)：

- [架构或测试目录不匹配](../troubleshooting/faq.md#5-架构或测试目录不匹配)
- [参考测试找不到](../troubleshooting/faq.md#11-参考测试找不到)

---

## 8. 检查清单

### 8.1 架构确认

- [ ] 已确认目标架构是 ascend950pr_9599
- [ ] 已确认 API 在该架构下有实现

### 8.2 分析阶段

- [ ] 已读取 regbase API 头文件
- [ ] 已确认数据类型支持
- [ ] 已参考 `tests/api/reg_compute_api/` 下现有测试

### 8.3 编写阶段

- [ ] 使用正确的测试目录
- [ ] SetUp/TearDown 正确设置
- [ ] 数据大小和对齐正确
- [ ] 使用参数化测试覆盖分支

### 8.4 验证阶段

- [ ] 编译通过
- [ ] 测试执行通过
- [ ] 结果验证正确

---

## 9. 相关参考

| 文档 | 说明 |
|------|------|
| [分支覆盖分析指南](../foundations/branch-coverage-guide.md) | 分支分析方法 |
| [测试模板参考](../foundations/test-templates.md) | 通用测试骨架与模板选择索引 |
| [LocalTensor 内存申请指南](../foundations/local-tensor-memory.md) | 内存管理说明 |
