# operator\(\)

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/operator_call_res.md#id1 -->

## 功能说明

获取本LocalTensor的第offset个变量的引用。用于左值，相当于[SetValue](SetValue.md)接口，用于右值，相当于[GetValue](GetValue.md)接口。

## 函数原型

```cpp
__aicore__ inline __inout_pipe__(S) __ubuf__ PrimType& operator()(const uint32_t offset) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | LocalTensor下标索引。 |

## 返回值说明

返回指定索引位置的元素的PrimType类型引用。

PrimType定义如下：

```cpp
// PrimT用于从T中提取基础数据类型：T传入基础数据类型，直接返回数据类型；T传入为TensorTrait类型时萃取TensorTrait中的LiteType基础数据类型
using PrimType = PrimT<T>;
```

## 约束说明

无

## 调用示例

```cpp
// operator()使用方法,用于左值，相当于Setvalue接口，用于右值，相当于Getvalue接口
// 输入数据(inputLocal): [100 100 100 ... 100]
inputLocal(2) = 200;
// 输出数据(inputLocal): [100 100 200 ... 100]
auto x = inputLocal(2);
// x显示为200
```
