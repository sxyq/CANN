# SetGlobalBuffer

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
- Atlas 200I/500 A2 推理产品：支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/SetGlobalBuffer_res.md#id1 -->

## 功能说明

传入全局数据地址，初始化GlobalTensor。

## 函数原型

-   传入全局数据的指针，并设置存储大小（通过元素个数表达）。

    ```cpp
    __aicore__ inline void SetGlobalBuffer(__gm__ PrimType* buffer, uint64_t bufferSize)
    ```

-   仅传入全局数据的指针，此时通过[GetSize](GetSize.md)获取到的元素个数为0。

    ```cpp
    __aicore__ inline void SetGlobalBuffer(__gm__ PrimType* buffer)
    ```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| buffer | 输入 | Host侧传入的全局数据指针。PrimType类型。
PrimType定义如下：
// PrimT用于从T中提取基础数据类型：T传入基础数据类型，直接返回数据类型；T传入为TensorTrait类型时萃取TensorTrait中的LiteType基础数据类型
using PrimType = PrimT<T>; |
| bufferSize | 输入 | GlobalTensor所包含的类型为PrimType的数据个数，需自行保证不会超出实际数据的长度。如指向的外部存储有连续256个int32_t，则其bufferSize为256。 |

## 返回值说明

无。

## 约束说明

无。

## 调用示例

```cpp
uint64_t dataSize = 256; //设置input_global的大小为256

AscendC::GlobalTensor<int32_t> inputGlobal; // 类型为int32_t
inputGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ int32_t *>(src_gm), dataSize); // 设置源操作数在Global Memory上的起始地址为src_gm，所占外部存储的大小为256个int32_t

AscendC::LocalTensor<int32_t> inputLocal = inQueueX.AllocTensor<int32_t>();
AscendC::DataCopy(inputLocal, inputGlobal, dataSize); // 将Global Memory上的inputGlobal拷贝到Local Memory的inputLocal上
```
