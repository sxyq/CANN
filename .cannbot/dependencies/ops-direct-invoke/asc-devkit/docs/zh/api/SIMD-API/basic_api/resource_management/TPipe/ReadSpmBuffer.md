# ReadSpmBuffer

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/ReadSpmBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

从SPM Buffer读回到local数据中。

## 函数原型

-   适用于连续和不连续的数据回读：

    ```cpp
    template <typename T>
    __aicore__ inline void ReadSpmBuffer(const LocalTensor<T>& readBuffer, const DataCopyParams& copyParams, int32_t readOffset = 0)
    ```

-   适用于连续的数据回读：

    ```cpp
    template <typename T>
    __aicore__ inline void ReadSpmBuffer(const LocalTensor<T>& readBuffer, const int32_t readSize, int32_t readOffset = 0)
    ```

## 参数说明

**表1**  模板参数说明

| 参数名称 | 含义 |
| -------- | ---- |
| T | 待获取Tensor的数据类型，支持的类型请见[LocalTensor](../../data_structures/LocalTensor/LocalTensor.md)相关描述。|

**表2**  接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| readBuffer | 输入 | 读回的目标local内存。 |
| copyParams | 输入 | 搬运参数，DataCopyParams类型，DataCopyParams结构定义请参考表3。 |
| readSize | 输入 | 读回的元素个数。 |
| readOffset | 输入 | SPM Buffer的偏移，单位为字节。 |

**表3**  DataCopyParams结构体参数定义

| 参数名称 | 含义 |
| --- | --- |
| blockCount | 待搬运的连续传输数据块个数。uint16_t类型，取值范围：blockCount∈[1, 4095]。 |
| blockLen | 待搬运的每个连续传输数据块长度，单位为DataBlock（32字节）。uint16_t类型，取值范围：blockLen∈[1, 65535]。 |
| srcGap | 源操作数相邻连续数据块的间隔（前面一个数据块的尾与后面数据块的头的间隔），单位为DataBlock（32字节）。uint16_t类型，srcGap不要超出该数据类型的取值范围。 |
| dstGap | 目的操作数相邻连续数据块间的间隔（前面一个数据块的尾与后面数据块的头的间隔），单位为DataBlock（32字节）。uint16_t类型，dstGap不要超出该数据类型的取值范围。 |

## 约束说明

无

## 返回值说明

无

## 调用示例

调用样例请参考[SPMBuffer调用示例](WriteSpmBuffer.md#section642mcpsimp)。
