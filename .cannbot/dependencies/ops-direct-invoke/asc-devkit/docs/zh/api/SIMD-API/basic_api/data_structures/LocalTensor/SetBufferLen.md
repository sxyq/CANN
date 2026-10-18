# SetBufferLen

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/SetBufferLen_res.md#id1 -->

## 功能说明

设置Buffer长度。当用户调用operator\[\]函数创建新LocalTensor时，建议调用该接口设置新LocalTensor长度，便于编译器对内存及同步进行自动优化。

## 函数原型

```cpp
__aicore__ inline void SetBufferLen(uint32_t dataLen)
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| dataLen | 输入 | Buffer的元素个数。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

```cpp
// 示例SetBufferLen如下示例将申请的Tensor长度修改为1024(单位为字节)
AscendC::LocalTensor<float> tmpBuffer2 = tempBmm2Queue.AllocTensor<float>();
tmpBuffer2.SetBufferLen(1024);
```
