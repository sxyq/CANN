# operator\[\]

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/operator_res.md#id1 -->

## 功能说明

获取距原LocalTensor起始地址偏移量为offset的新LocalTensor，注意offset不能超过原有LocalTensor的size大小。

## 函数原型

```cpp
__aicore__ inline LocalTensor operator[](const uint32_t offset) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | 偏移量，单位为元素。 |

## 返回值说明

返回距原LocalTensor起始地址偏移量为offset的新LocalTensor。

## 约束说明

无

## 调用示例

```cpp
// srcLen = 256, num = 100, M=50
// operator[]使用方法, inputLocal[16]为从起始地址开始偏移量为16的新tensor
AscendC::Add(outputLocal[16], inputLocal[16], inputLocal2[16], M);
// 示例结果如下：
// 输入数据(inputLocal): [100 100 100 ... 100]
// 输入数据(inputLocal2): [1 2 3 ... 66]
// 输出数据(outputLocal): [... 117 118 119 ... 166]
```
