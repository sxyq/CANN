# LocalMemAllocator构造函数

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：不支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/LocalMemAllocator/LocalMemAllocator_constructor_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tensor.h"`。

LocalMemAllocator构造函数。

## 函数原型

```cpp
template <Hardware hard>
__aicore__ inline LocalMemAllocator<hard>::LocalMemAllocator()
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| hard | 用于表示数据的物理位置，Hardware枚举类型，定义如下，合法位置为：Unified Buffer（UB）、L1、L0A、L0B、L0C、BIAS、FIXBUF。
enum class Hardware : uint8_t {
GM,     // Global Memory
UB,     // UB
L1,     // L1 Buffer
L0A,    // L0A Buffer
L0B,    // L0B Buffer
L0C,    // L0C Buffer
BIAS,   // BiasTable Buffer
FIXBUF, // Fixpipe Buffer
MAX }; |

## 返回值说明

无

## 约束说明

同一个物理位置的LocalMemAllocator对象，在算子生命周期内只能存在1个。
