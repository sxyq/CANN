# ReinterpretCast

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/ReinterpretCast_res.md#id1 -->

## 功能说明

将当前Tensor重解释为用户指定的新类型，转换后的Tensor与原Tensor地址及内容完全相同，Tensor的大小（字节数）保持不变。

## 函数原型

```cpp
template <typename CAST_T>
__aicore__ inline LocalTensor<CAST_T> ReinterpretCast() const
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| CAST_T | 用户指定的新类型。 |

## 返回值说明

重解释转换后的Tensor。

## 约束说明

无

## 调用示例

```cpp
// 示例
// input_local为int32_t类型，包含16个元素(64字节)
for (int32_t i = 0; i < 16; ++i) {
    inputLocal.SetValue(i, i); // 对inputLocal中第i个位置进行赋值为i
}

// 调用ReinterpretCast将input_local重解释为int16_t类型
AscendC::LocalTensor<int16_t> interpreTensor = inputLocal.ReinterpretCast<int16_t>();
// 示例结果如下，二者数据完全一致，在物理内存上也是同一地址，仅根据不同类型进行了重解释
// inputLocal:0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
// interpreTensor:0 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 11 0 12 0 13 0 14 0 15 0
```
