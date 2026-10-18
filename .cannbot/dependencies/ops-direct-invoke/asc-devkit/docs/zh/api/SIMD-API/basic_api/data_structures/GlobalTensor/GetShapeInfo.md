# GetShapeInfo

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/GetShapeInfo_res.md#id1 -->

## 功能说明

获取GlobalTensor的shape信息。注意：Shape信息没有默认值，只有通过SetShapeInfo设置过Shape信息后，才可以调用该接口获取正确的ShapeInfo。

## 函数原型

```cpp
__aicore__ inline ShapeInfo GetShapeInfo() const
```

## 参数说明

无。

## 返回值说明

GlobalTensor的shape信息，[ShapeInfo](../../aux_data_structures//ShapeInfo.md)类型。

## 约束说明

无。

## 调用示例

```cpp
// 示例获取Tensor的ShapeInfo信息，获取通过SetShapeInfo设置过Shape信息
AscendC::ShapeInfo maxShapeInfo = tglobal.GetShapeInfo();
// 获取原始的shape各个维度的值
uint32_t orgShape0 = maxShapeInfo.originalShape[0];
uint32_t orgShape1 = maxShapeInfo.originalShape[1];
uint32_t orgShape2 = maxShapeInfo.originalShape[2];
uint32_t orgShape3 = maxShapeInfo.originalShape[3];
// 获取现有的shape的2维的值
uint32_t shape2 = maxShapeInfo.shape[2];
```
