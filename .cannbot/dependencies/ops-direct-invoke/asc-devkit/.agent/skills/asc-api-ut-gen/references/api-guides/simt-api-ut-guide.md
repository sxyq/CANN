# SIMT API UT 生成指南

## 1. API 概述

SIMT (Single Instruction Multiple Threads) API 是 AscendC 提供的 SIMT 编程模型接口，提供类似 CUDA 的线程级编程能力。

### 1.1 目录位置

| 类型 | 路径 |
|------|------|
| **头文件** | `{ASC_DEVKIT_PATH}/include/simt_api/` |
| **实现文件** | `{ASC_DEVKIT_PATH}/impl/simt_api/` |
| **测试目录** | `{ASC_DEVKIT_PATH}/tests/api/simt_api/` |

### 1.2 架构支持限制

**重要：SIMT API 当前仅支持以下架构：**

| 架构 | NPU_ARCH | 芯片名 | 测试目录 |
|------|----------|--------|----------|
| dav_3510 | 3510 | **ascend950pr_9599** | ascendc_case_ascend950pr_9599 |

> **不支持其他架构（如 ascend910、ascend910b1、ascend310p 等）**

### 1.3 典型 API 示例

| API 类别 | 典型 API |
|---------|---------|
| 向量运算 | `vector_add`, `vector_mul` |
| 归约操作 | `block_reduce`, `warp_reduce` |
| 同步操作 | `syncthreads`, `syncwarp` |
| 内存操作 | `shared_memory_load`, `shared_memory_store` |

---

## 2. API 特点

### 2.1 SIMT 编程模型

SIMT API 提供类似 CUDA 的编程范式：

```cpp
// 设备函数声明
__device__ void vector_add(float* dst, float* src0, float* src1, int n);

// 线程索引
int tid = threadIdx.x + blockIdx.x * blockDim.x;

// 并行执行
for (int i = tid; i < n; i += blockDim.x * gridDim.x) {
    dst[i] = src0[i] + src1[i];
}
```

### 2.2 线程级并行

- 每个 thread 执行相同代码
- 通过线程索引区分数据
- 支持线程同步机制

### 2.3 设备函数

SIMT API 通常声明为 `__device__` 函数：

```cpp
__device__ inline void simt_function(...) {
    // 设备端执行代码
}
```

### 2.4 与其他 API 的区别

| 特性 | SIMT API | membase API | regbase API |
|------|---------|-------------|-------------|
| 编程模型 | 线程级 | 向量级 | 寄存器级 |
| 架构支持 | 仅 3510 | 所有架构 | 仅 3510 |
| 并行方式 | SIMT | SIMD | SIMD |
| 抽象层级 | 较低 | 较高 | 较低 |

---

## 3. UT 生成注意事项

### 3.1 架构限制检查

**生成 UT 前必须确认目标架构：**

```bash
# 仅以下架构可生成 SIMT API UT
/ascend950pr_9599    # NPU_ARCH=3510
```

### 3.2 测试目录结构

```
tests/api/simt_api/
└── ascend950pr_9599/
    ├── test_simt_vector_add.cpp
    ├── test_simt_vector_mul.cpp
    └── ...
```

### 3.3 Kernel 声明方式

SIMT API 测试通常需要特殊的 Kernel 声明：

```cpp
// Kernel 声明
extern "C" __global__ __aicore__ void kernel_simt_function(...) {
    // Kernel 实现
    // 调用 SIMT API
}
```

### 3.4 参考现有测试

**必须参考 `tests/api/simt_api/ascend950pr_9599/` 下的现有测试：**

- 查看同类 API 的测试模式
- 了解内存分配和数据初始化方式
- 学习线程同步和结果验证方法

### 3.5 自动生成边界

SIMT API 的函数族差异很大：有直接 host 侧断言的数学函数，也有依赖 thread/block
语义、同步和共享内存的设备函数。某个 family 的可执行模板在真实 UT 中验证前，
通用生成器不得输出带 `TODO` 的 kernel、调用或校验骨架。

当前流程要求先读目标声明、impl 和同 family UT；如果还没有经验证的可复用模板，
就写 API-specific UT，而不是生成占位壳。

---

## 4. 测试模板引用

通用 gtest、参数化测试和结果校验骨架见 [测试模板参考](../foundations/test-templates.md)。SIMT guide 只维护 SIMT 特有约束：

- SIMT Kernel 需要显式覆盖线程索引、block/grid 配置和同步语义。
- 当前仅为 `ascend950pr_9599` 生成 SIMT UT。
- 数据类型和参数组合必须从 SIMT API 声明、impl 分支、设计文档或已有同类 UT 确认。

---

## 5. 分支覆盖要点

### 5.1 架构条件编译

SIMT API 仅支持 3510 架构：

```cpp
// impl 文件中
#if __NPU_ARCH__ == 3510
    // ascend950pr_9599 SIMT 实现
#endif
```

### 5.2 数据类型

根据 SIMT API 声明、impl 分支、设计文档或已有同类 UT 确认真实支持类型，并使用参数化测试覆盖已确认组合。通用基础 dtype 的名称、大小和 32B 对齐要求统一回链到 [`asc-npu-arch` 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md#统一数据类型视图)，本 guide 不维护并行 dtype 表。

### 5.3 线程配置

测试不同线程配置：

- 不同的 Block 大小
- 不同的 Grid 大小
- 边界条件

### 5.4 同步机制

测试同步操作：

- `syncthreads()` - 块内同步
- `syncwarp()` - warp 内同步

---

## 6. 编译与执行

SIMT 的编译、执行、失败修复和验证报告统一按 [自动化验证流程](../workflows/automation-guide.md) 处理。执行前确认 `build/tests/api/simt_api/ascend950pr_9599/` 下的实际测试可执行文件名。

---

## 7. 常见问题

### Q1: 线程同步问题？

**问题**：结果不正确或死锁

**解决**：
- 确保正确使用同步函数
- 检查共享内存访问模式

### Q2: 公共排障索引

SIMT 的架构支持和参考测试查找统一查看 [常见问题与解决方案](../troubleshooting/faq.md)：

- [架构或测试目录不匹配](../troubleshooting/faq.md#5-架构或测试目录不匹配)
- [参考测试找不到](../troubleshooting/faq.md#11-参考测试找不到)

---

## 8. 检查清单

### 8.1 架构确认

- [ ] 已确认目标架构是 ascend950pr_9599 (NPU_ARCH=3510)
- [ ] 已确认 API 在该架构下有实现

### 8.2 分析阶段

- [ ] 已读取 SIMT API 头文件
- [ ] 已确认数据类型支持
- [ ] 已参考 `tests/api/simt_api/ascend950pr_9599/` 下现有测试

### 8.3 编写阶段

- [ ] 使用正确的测试目录
- [ ] Kernel 声明方式正确
- [ ] 线程同步正确
- [ ] 数据大小和对齐正确

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
| [regbase API UT 指南](regbase-api-ut-guide.md) | 同为特定架构 API 参考 |
