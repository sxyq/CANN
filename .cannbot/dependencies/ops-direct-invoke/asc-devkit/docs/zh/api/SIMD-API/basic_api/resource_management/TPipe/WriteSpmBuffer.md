# WriteSpmBuffer

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/WriteSpmBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

将因为Unified Buffer（UB）空间不足而需要溢出暂存的数据拷贝到SPM Buffer中。

## 函数原型

-   适用于连续和不连续的数据暂存：

    ```cpp
    template <typename T>
    __aicore__ inline void WriteSpmBuffer(const LocalTensor<T>& writeBuffer, const DataCopyParams& copyParams, int32_t writeOffset = 0)
    ```

-   适用于连续的数据暂存：

    ```cpp
    template <typename T>
    __aicore__ inline void WriteSpmBuffer(const LocalTensor<T>& writeBuffer, const int32_t writeSize, int32_t writeOffset = 0)
    ```

## 参数说明

**表1**  模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| T | 待获取Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

**表2**  接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| writeBuffer | 输入 | 需要溢出暂存的Local内存。 |
| copyParams | 输入 | 搬运参数，DataCopyParams类型，DataCopyParams结构定义请参考表3。 |
| writeSize | 输入 | 拷贝的元素个数。 |
| writeOffset | 输入 | 拷贝到SPM Buffer的偏移，单位为字节。 |

**表3**  DataCopyParams结构体参数定义

| 参数名称 | 含义 |
| --- | --- |
| blockCount | 待搬运的连续传输数据块个数。uint16_t类型，取值范围：blockCount∈[1, 4095]。 |
| blockLen | 待搬运的每个连续传输数据块长度，单位为DataBlock（32字节）。uint16_t类型，取值范围：blockLen∈[1, 65535]。 |
| srcGap | 源操作数相邻连续数据块的间隔（前面一个数据块的尾与后面数据块的头的间隔），单位为DataBlock（32字节）。uint16_t类型，srcGap不要超出该数据类型的取值范围。 |
| dstGap | 目的操作数相邻连续数据块间的间隔（前面一个数据块的尾与后面数据块的头的间隔），单位为DataBlock（32字节）。uint16_t类型，dstGap不要超出该数据类型的取值范围。 |

## 约束说明

-   暂存拷贝到L1时注意writeSize和writeOffset保证32字节对齐。
-   拷贝的内存不要超出初始化的SPM Buffer大小，否则会存在溢出踩踏等问题。

## 返回值说明

无

## 调用示例

-   使用DataCopyParams搬运

    ```cpp
    AscendC::TPipe pipe;
    int offset = 32; // 拷贝到SPM Buffer时偏移32字节
    AscendC::DataCopyParams copyParams{2, 2, 1, 1}; // 搬运两个数据块，一个数据块的长度为2个datablock，一个datablock为32字节，源地址和目的地址相邻数据块间均间隔一个数据块
    // writeLocal为UB上的half类型的LocalTensor
    pipe.WriteSpmBuffer(writeLocal, copyParams, offset); // 将UB上的数据块搬运到SPM Buffer
    ...
    pipe.ReadSpmBuffer(writeLocal, copyParams, offset); // 当UB空间足够时，将暂存在SPM Buffer的数据读回到UB
    ```

-   使用writeSize连续搬运

    ```cpp
    AscendC::TPipe pipe;
    int dataSize = 32;
    int offset = 32; // 拷贝到spmBuffer时偏移32字节

    // writeLocal为UB上的half类型的LocalTensor
    pipe.WriteSpmBuffer(writeLocal, dataSize, offset); // 将UB上的连续传输数据块搬运到SPM Buffer
    ...
    pipe.ReadSpmBuffer(writeLocal, dataSize, offset); // 当UB空间足够时，将暂存在SPM Buffer的数据读回到UB
    ```
