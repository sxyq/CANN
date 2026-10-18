# GetCurAddr

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
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/LocalMemAllocator/GetCurAddr_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tensor.h"`。

返回当前物理位置空闲的起始地址。

## 函数原型

```cpp
template <Hardware hard>
__aicore__ inline uint32_t LocalMemAllocator<hard>::GetCurAddr() const
```

## 参数说明

无

## 返回值说明

当前物理位置空闲的起始地址，范围为\[0，物理内存最大值\)。

## 约束说明

无

## 调用示例

```cpp
LocalMemAllocator allocator;
// 默认的物理位置为Unified Buffer（UB），由于从0地址开始分配，下面的打印结果为0
AscendC::printf("current addr is %u\n", allocator.GetCurAddr());
```
