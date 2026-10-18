# DeQue

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：不支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：不支持
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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TSCM/DeQue_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

将tensor从队列中取出，用于后续处理。

## 函数原型

```cpp
template <typename T>
__aicore__ inline LocalTensor<T> DeQue()
```

## 参数说明

| 参数名 | 说明 |
| --- | --- |
| T | Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

## 约束说明

无

## 返回值说明

从队列中取出的LocalTensor。

## 调用示例

参考[调用示例](AllocTensor.md#section642mcpsimp)。
