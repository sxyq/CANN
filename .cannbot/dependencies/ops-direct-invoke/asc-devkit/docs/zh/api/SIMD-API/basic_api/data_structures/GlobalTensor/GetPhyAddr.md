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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/GetPhyAddr_res.md#id1 -->

## 功能说明

获取全局数据的地址。

## 函数原型

-   获取全局数据的地址

    ```cpp
    __aicore__ inline const __gm__ PrimType* GetPhyAddr() const
    ```

-   获取全局数据（指定偏移offset个元素）的地址

    ```cpp
    __aicore__ inline __gm__ PrimType* GetPhyAddr(const uint64_t offset) const
    ```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| offset | 输入 | 偏移的元素个数，用于指定数据的位置。 |

## 返回值说明

全局数据的地址。

## 约束说明

无。

## 调用示例

```cpp
template <typename T>
__aicore__ inline void CopyIn(const AscendC::GlobalTensor<T>& input)
{
    AscendC::GlobalTensor<T> output;
    output.SetGlobalBuffer(input.GetPhyAddr(0), input.GetSize());
    // ...
}
```
