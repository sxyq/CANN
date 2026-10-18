# GetValue

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/GetValue_res.md#id1 -->

## 功能说明

获取LocalTensor指定索引的数值。

**该接口仅在LocalTensor的TPosition为VECIN/VECCALC/VECOUT时支持。**

## 函数原型

```cpp
__aicore__ inline __inout_pipe__(S) PrimType GetValue(const uint32_t index) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| index | 输入 | LocalTensor索引，单位为元素。 |

## 返回值说明

LocalTensor指定索引的数值，PrimType类型。

PrimType定义如下：

```cpp
// PrimT用于从T中提取基础数据类型：T传入基础数据类型，直接返回数据类型；T传入为TensorTrait类型时萃取TensorTrait中的LiteType基础数据类型
using PrimType = PrimT<T>;
```

## 约束说明

无

## 调用示例

```cpp
// 示例1
// 数据(inputLocal): [0 100 200  ... ]
auto element = inputLocal.GetValue(2); // 获取inputLocal中第i个位置的数值

// 示例1结果如下：
// element为200

// 示例2
// 数据(inputLocal): [0 100 200  ... ]
auto element = inputLocal(1); // 获取inputLocal中第i个位置的数值
// 示例2结果如下：
// element为100
```
