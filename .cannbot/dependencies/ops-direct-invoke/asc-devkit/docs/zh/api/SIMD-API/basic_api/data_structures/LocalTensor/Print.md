# Print

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/Print_res.md#id1 -->

## 功能说明

只限于CPU调试，在调试窗口中打印LocalTensor数据用于精度调试，每一行打印一个DataBlock（32字节）的数据。

## 函数原型

```cpp
inline void Print()
inline void Print(uint32_t len)
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| len | 输入 | 打印元素个数。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

```cpp
// 示例只限于CPU调试，在调试窗口中打印LocalTensor数据用于精度调试，每一行打印一个datablock(32Bytes)的数据
AscendC::LocalTensor<int32_t> inputLocal = softmaxMaxBuf.template Get<int32_t>();
for (int32_t i = 0; i < 16; ++i) {
    inputLocal.SetValue(i, i); // 对input_local中第i个位置进行赋值为i
}
inputLocal.Print();
// 0000: 0 1 2 3 4 5 6 7 8
// 0008: 9 10 11 12 13 14 15
```
