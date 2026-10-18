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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/SetValue_res.md#id1 -->

## 功能说明

设置LocalTensor中的某个值。

**该接口仅在LocalTensor的TPosition为VECIN/VECCALC/VECOUT时支持。**

## 函数原型

```cpp
template <typename T1> __aicore__ inline __inout_pipe__(S)
void SetValue(const uint32_t index, const T1 value) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| index | 输入 | LocalTensor索引，单位为元素。 |
| value | 输入 | 待设置的数值。 |

## 返回值说明

无

## 约束说明

不要大量使用SetValue对LocalTensor进行赋值，会使性能下降。若需要大批量赋值，请根据实际场景选择[数据填充基础API接口](../../memory_vector_compute/data_padding/data_padding.md)或数据填充高阶API接口（[Pad](../../../adv_api/tensor_transform/Pad.md)、[Broadcast](../../../adv_api/tensor_transform/Broadcast.md)），以及在需要生成递增数列的场景，选择[Arange](../../../adv_api/index_compute/Arange.md)。

## 调用示例

```cpp
// srcLen = 256, num = 100, M=50
// 示例1
for (int32_t i = 0; i < srcLen; ++i) {
    inputLocal.SetValue(i, num); // 对inputLocal中第i个位置进行赋值为num
}
// 示例1结果如下：
// 数据(inputLocal): [100 100 100  ... 100]

// srcLen = 256, num = 99, M=50
// 示例2
for (int32_t i = 0; i < srcLen; ++i) {
    inputLocal(i) = num; // 对inputLocal中第i个位置进行赋值为num
}
// 示例2结果如下：
// 数据(inputLocal): [99 99 99  ... 99]
```
