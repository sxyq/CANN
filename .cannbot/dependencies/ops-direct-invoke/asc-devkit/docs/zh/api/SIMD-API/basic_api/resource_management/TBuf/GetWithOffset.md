# GetWithOffset

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
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- npu="x90" id8 -->
- Kirin X90：支持
<!-- end id8 -->
<!-- npu="9030" id9 -->
- Kirin 9030：支持
<!-- end id9 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TBuf/GetWithOffset_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

以TBuf为基地址，向后偏移指定长度，将偏移后的地址作为起始地址，提取长度为指定值的Tensor。

## 函数原型

```cpp
template <typename T>
__aicore__ inline LocalTensor<T> GetWithOffset(uint32_t size, uint32_t bufOffset)
```

## 参数说明

**表1**  模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| T | 待获取Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

**表2**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| size | 输入 | 需要获取的Tensor元素个数。 |
| bufOffset | 输入 | 从起始位置的偏移长度，单位是字节，且需32字节对齐。 |

## 约束说明

size的数值是Tensor中元素的个数，size\*sizeof\(T\) + bufOffset不能超过TBuf初始化时的长度。

bufOffset需满足32字节对齐的要求。

## 返回值说明

获取到的[LocalTensor](../../data_structures/LocalTensor/LocalTensor.md)。

## 调用示例

```cpp
// 为TBuf初始化分配内存，分配内存长度为1024字节
AscendC::TPipe pipe;
AscendC::TBuf<AscendC::TPosition::VECCALC> calcBuf; // 模板参数为TPosition中的VECCALC类型
uint32_t byteLen = 1024;
pipe.InitBuffer(calcBuf, byteLen);
// 从calcBuf偏移64字节获取Tensor,Tensor为128个int32_t类型元素的内存大小，为512字节
AscendC::LocalTensor<int32_t> tempTensor1 = calcBuf.GetWithOffset<int32_t>(128, 64);
```
