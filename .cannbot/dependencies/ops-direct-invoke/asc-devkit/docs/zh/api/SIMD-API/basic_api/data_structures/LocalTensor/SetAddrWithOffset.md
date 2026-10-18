# SetAddrWithOffset

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/SetAddrWithOffset_res.md#id1 -->

## 功能说明

设置带有偏移的Tensor地址。用于快速获取定义一个Tensor，同时指定新Tensor相对于旧Tensor首地址的偏移。偏移的长度为旧Tensor的元素个数。

## 函数原型

```cpp
template <typename S>
__aicore__ inline void SetAddrWithOffset(LocalTensor<S> &src, uint32_t offset)
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| src | 输入 | 基础地址的Tensor，将该Tensor的地址作为基础地址，设置偏移后的Tensor地址。 |
| offset | 输入 | 偏移的长度，单位为元素。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

```cpp
// 示例SetAddrWithOffset，用于快速获取定义一个Tensor，同时指定新Tensor相对于旧Tensor首地址的偏移
// 需要注意，偏移的长度为旧Tensor的元素个数
AscendC::LocalTensor<float> tmpBuffer1 = tempBmm2Queue.AllocTensor<float>();
AscendC::LocalTensor<half> tmpHalfBuffer;
tmpHalfBuffer.SetAddrWithOffset(tmpBuffer1, calcSize * 2);
```
