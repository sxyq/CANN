# SetValue

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/SetValue_res.md#id1 -->

## 功能说明

设置GlobalTensor相应偏移位置的值。

-   因为硬件实现不同，其与通用CPU标量赋值操作存在差异。SetValue赋值操作首先改写的是每个AI Core内部的DCache，不会立刻写出到Global Memory，且后续写出时以Cache Line（64B）为单位。使用该接口之前必须了解DCache结构和Cache一致性原理（参见[DataCacheCleanAndInvalid](../../cache_control/DataCacheCleanAndInvalid.md)），否则可能存在误用的情况，**请谨慎使用。**
-   调用SetValue后，首先改写的是每个AI Core内部的DCache，如果需要立即写出到Global Memory，需要在调用此接口后，再调用[DataCacheCleanAndInvalid](../../cache_control/DataCacheCleanAndInvalid.md)，确保DCache与Global Memory的Cache一致性。
-   多核操作GM地址时，要求不同核操作的地址（通过offset参数设置元素偏移，可以转换为地址）至少有Cache Line大小的偏移，否则会出现多核数据随机覆盖。同时需要考虑地址对齐（64B）的问题。详细内容请参考[调用示例](../../cache_control/DataCacheCleanAndInvalid.md#section837496171220)。

## 函数原型

```cpp
__aicore__ inline void SetValue(const uint64_t offset, PrimType value)
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | 偏移offset个元素。 |
| value | 输入 | 设置值。PrimType类型。
PrimType定义如下：
// PrimT用于从T中提取基础数据类型：T传入基础数据类型，直接返回数据类型；T传入为TensorTrait类型时萃取TensorTrait中的LiteType基础数据类型
using PrimType = PrimT<T>; |

## 返回值说明

无。

## 约束说明

无。

## 调用示例

```cpp
// aGlobal数据 [1, 1, 1, 1, 1]
AscendC::GlobalTensor<T> aGlobal;
T val = 0;
aGlobal.SetValue(2, val);

//结果如下
aGlobal数据 [1, 1, 0, 1, 1]
```
