# GetPhyAddr

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/GetPhyAddr_res.md#id1 -->

## 功能说明

返回LocalTensor的地址或指定偏移量后的地址。

## 函数原型

```cpp
__aicore__ inline uint64_t GetPhyAddr() const
__aicore__ inline uint64_t GetPhyAddr(const uint32_t offset) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | 偏移量。 |

## 返回值说明

LocalTensor的地址或指定偏移量后的地址。

## 约束说明

无

## 调用示例

```cpp
// 示例
// 调用GetPhyAddr()返回LocalTensor地址，CPU上返回的是指针类型(T*)，NPU上返回的是物理存储的地址(uint64_t)
#ifdef ASCENDC_CPU_DEBUG
float *inputLocalCpuPtr = inputLocal.GetPhyAddr();
uint64_t realAddr = (uint64_t)inputLocalCpuPtr - (uint64_t)(GetTPipePtr()->GetBaseAddr(static_cast<int8_t>(AscendC::TPosition::VECCALC)));
#else
uint64_t realAddr = inputLocal.GetPhyAddr();
#endif
```
