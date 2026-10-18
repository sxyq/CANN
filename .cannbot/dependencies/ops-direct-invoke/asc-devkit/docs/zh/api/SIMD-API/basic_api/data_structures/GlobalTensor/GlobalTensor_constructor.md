# GlobalTensor构造函数

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/GlobalTensor_constructor_res.md#id1 -->

## 功能说明

GlobalTensor构造函数。

## 函数原型

```cpp
__aicore__ inline GlobalTensor<T>() {}
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| T | GlobalTensor的数据类型。 |

## 返回值说明

无。

## 约束说明

无。

## 调用示例

参考[调用示例](SetGlobalBuffer.md#section17531157161314)。
