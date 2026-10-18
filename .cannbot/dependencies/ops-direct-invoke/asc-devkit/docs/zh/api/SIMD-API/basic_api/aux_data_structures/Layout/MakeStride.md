# MakeStride

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：不支持
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
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/Layout/MakeStride_res.md#id1 -->

## 功能说明

将传入的数据打包成Stride数据结构。

## 函数原型

```cpp
template <typename... Ts>
__aicore__ inline constexpr Stride<Ts...> MakeStride(const Ts&... t)
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| Ts... | 输入 | 表示输入类型的形参包，使用方法和约束说明同Std::tuple。 |

## 返回值说明

Stride结构类型（[Std::tuple](../../../../Utils-API/cpp_stdlib/container_functions/container_functions.md)类型的别名），用于定义各维度在内存中的步长，即同维度相邻元素在内存中的间隔，与Shape的维度信息一一对应。定义如下：

```cpp
template <typename... Strides>
using Stride = Std::tuple<Strides...>;
```

## 约束说明

同[Std::tuple](../../../../Utils-API/cpp_stdlib/container_functions/container_functions.md)。

## 调用示例

参见[调用示例](MakeLayout.md#zh-cn_topic_0000002078447573_zh-cn_topic_0000001576806829_zh-cn_topic_0000001339187720_section320753512363)。
