# AllocTensor

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
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQueBind/AllocTensor_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

从Que中分配Tensor，Tensor所占大小为InitBuffer时设置的每块内存长度。

## 函数原型

-   non-inplace接口：构造新的Tensor作为内存管理的对象

    ```cpp
    template <typename T>
    __aicore__ inline LocalTensor<T> AllocTensor()
    ```

-   inplace接口：直接使用传入的Tensor作为内存管理的对象，可以减少Tensor反复创建的开销，具体使用指导可参考[Tensor原地操作](../../../../../guide/programming_guide/appendix/common_operations/tensor_inplace_performance.md)。

    ```cpp
    template <typename T>
    __aicore__ inline void AllocTensor(LocalTensor<T>& tensor)
    ```

## 参数说明

**表1**  模板参数说明

| 参数名 | 说明 |
| --- | --- |
| T | Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

**表2**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| tensor | 输入 | inplace接口需要传入LocalTensor作为内存管理的对象。 |

## 约束说明

-   non-inplace接口分配的Tensor内容可能包含随机值。
-   non-inplace接口，需要将TQueBind的depth模板参数设置为非零值；inplace接口，需要将TQueBind的depth模板参数设置为0。

## 返回值说明

non-inplace接口返回值为LocalTensor对象，inplace接口没有返回值。

## 调用示例

-   non-inplace接口

    ```cpp
    AscendC::TPipe pipe;
    AscendC::TQueBind<AscendC::TPosition::VECOUT, AscendC::TPosition::GM, 2> que;
    int num = 4;
    int len = 1024;
    pipe.InitBuffer(que, num, len); // InitBuffer分配内存块数为4，每块大小为1024Bytes
    AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>(); // AllocTensor分配Tensor长度为1024Bytes
    ```

-   inplace接口

    ```cpp
    AscendC::TPipe pipe;
    AscendC::TQueBind<AscendC::TPosition::VECOUT, AscendC::TPosition::GM, 0> que;
    int num = 2;
    int len = 1024;
    pipe.InitBuffer(que, num, len); // InitBuffer分配内存块数为2，每块大小为1024Bytes
    AscendC::LocalTensor<half> tensor1;
    que.AllocTensor<half>(tensor1); // AllocTensor分配Tensor长度为1024Bytes
    ```
