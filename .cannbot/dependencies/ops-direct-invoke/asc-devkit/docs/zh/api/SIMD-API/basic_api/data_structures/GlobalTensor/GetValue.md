# GetValue

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/GetValue_res.md#id1 -->

## 功能说明

获取GlobalTensor的相应偏移位置的值。

## 函数原型

```cpp
__aicore__ inline __inout_pipe__(S) PrimType GetValue(const uint64_t offset) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | 偏移offset个元素。 |

## 返回值说明

返回PrimType类型的立即数。

## 约束说明

如果GetValue的Global Memory地址内容存在被外部改写的可能，需要先调用[DataCacheCleanAndInvalid](../../cache_control/DataCacheCleanAndInvalid.md)，确保Data Cache与Global Memory的Cache一致性，之后再调用此接口。

## 调用示例

```cpp
// 数据(GlobalTensor): [0 100 200  ... ]

AscendC::GlobalTensor<uint32_t> tiling_global;
tiling_global.SetGlobalBuffer(reinterpret_cast<__gm__ uint32_t *>(tiling), 16);
const uint16_t C1 = (uint16_t)tiling_global.GetValue(0);
const uint16_t H = (uint16_t)tiling_global(1);
// 示例结果如下：
// C1 = 0，H = 100
```
