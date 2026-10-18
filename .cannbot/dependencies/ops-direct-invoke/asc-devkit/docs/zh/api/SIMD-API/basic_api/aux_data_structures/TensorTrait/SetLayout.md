# SetLayout

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
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/TensorTrait/SetLayout_res.md#id1 -->

## 功能说明

设置Layout对象至TensorTrait结构中。

## 函数原型

```cpp
__aicore__ inline void SetLayout(const LayoutType& t)
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| t | 输入 | 输入的Layout对象。输入的数据类型LayoutType，需满足约束说明。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

参见[调用示例](TensorTrait_constructor.md#zh-cn_topic_0000002078486173_zh-cn_topic_0000001576727153_zh-cn_topic_0000001389787297_section320753512363)。
