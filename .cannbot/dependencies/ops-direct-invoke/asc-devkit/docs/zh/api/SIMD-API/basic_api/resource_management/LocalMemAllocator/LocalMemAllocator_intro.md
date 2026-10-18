# LocalMemAllocator简介

LocalMemAllocator是在使用[静态Tensor编程](../../../../../guide/programming_guide/programming_model/ai_core_simd_programming/cpp_tensor_programming/static_tensor_programming.md)时用于内存管理的类，用户无需构建TPipe/TQue，而是直接创建LocalTensor对象（也可以直接通过[LocalTensor构造函数](../../data_structures/LocalTensor/LocalTensor_constructor.md)进行构造）并开发算子，从而减少运行时的开销，实现更优的性能。

LocalMemAllocator仅支持在Ascend C静态Tensor编程方式中使用，不可以与TPipe等接口混用。

## 需要包含的头文件

```cpp
#include "kernel_operator.h"
```

## 原型定义

```cpp
template<Hardware hard = Hardware::UB>
class LocalMemAllocator {
public:
    __aicore__ inline LocalMemAllocator();
    __aicore__ inline uint32_t GetCurAddr() const;
    template <class DataType, uint32_t tileSize> LocalTensor<DataType> __aicore__ inline Alloc();
    template <TPosition pos, class DataType, uint32_t tileSize> __aicore__ inline LocalTensor<DataType> Alloc();
    template <class DataType> LocalTensor<DataType> __aicore__ inline Alloc(uint32_t tileSize);
    template <TPosition pos, class DataType> LocalTensor<DataType> __aicore__ inline Alloc(uint32_t tileSize);

    template <class DataType> LocalTensor<DataType> __aicore__ inline Alloc();
};
```

## 模板参数

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
