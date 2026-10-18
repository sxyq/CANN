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
<!-- npu="x90" id8 -->
- Kirin X90：支持
<!-- end id8 -->
<!-- npu="9030" id9 -->
- Kirin 9030：支持
<!-- end id9 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQue/AllocTensor_res.md#id1 -->

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

-   同一个TPosition上的所有Queue，连续调用AllocTensor接口申请的Tensor数量，根据AI处理器型号的不同，有数量约束。申请Buffer时，需要满足该约束。
    <!-- npu="950" id10 -->
    - Ascend 950PR/Ascend 950DT不超过8个。
    <!-- end id10 -->
    <!-- npu="A3" id11 -->
    - Atlas A3 训练系列产品/Atlas A3 推理系列产品不超过8个。
    <!-- end id11 -->
    <!-- npu="910b" id12 -->
    - Atlas A2 训练系列产品/Atlas A2 推理系列产品不超过8个。
    <!-- end id12 -->
    <!-- npu="310b" id13 -->
    - Atlas 200I/500 A2 推理产品不超过8个。
    <!-- end id13 -->
    <!-- npu="310p" id14 -->
    - Atlas 推理系列产品AI Core不超过8个。
    <!-- end id14 -->
    <!-- npu="310p" id15 -->
    - Atlas 推理系列产品Vector Core不超过8个。
    <!-- end id15 -->
    <!-- npu="910" id16 -->
    - Atlas 训练系列产品不超过4个。
    <!-- end id16 -->
    <!-- npu="x90" id17 -->
    - Kirin X90不超过8个。
    <!-- end id17 -->
    <!-- npu="9030" id18 -->
    - Kirin 9030不超过8个。
    <!-- end id18 -->
-   non-inplace接口分配的Tensor内容可能包含随机值。
-   non-inplace接口，需要将TQueBind的depth模板参数设置为非零值；inplace接口，需要将TQueBind的depth模板参数设置为0。

## 返回值说明

non-inplace接口返回值为LocalTensor对象，inplace接口没有返回值。

## 调用示例

-   示例一

    ```cpp
    // 使用AllocTensor分配Tensor
    AscendC::TPipe pipe;
    AscendC::TQue<AscendC::TPosition::VECOUT, 2> que;
    int num = 2;
    int len = 1024;
    pipe.InitBuffer(que, num, len); // InitBuffer分配内存块数为2，每块大小为1024Bytes
    AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>(); // AllocTensor分配Tensor长度为1024Bytes
    ```

-   示例二

    ```cpp
    // 连续使用AllocTensor的限制场景举例如下:
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que0;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que1;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que2;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que3;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que4;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> que5;
    // 不建议：
    // 比如，算子有6个输入，需要申请6块buffer
    // 通过6个队列为其申请内存，分别为que0~que5，每个que分配1块,申请VECIN TPosition上的buffer总数为6
    // 假设，同一个TPosition上连续Alloc的Buffer数量限制为4，超出该限制后，使用AllocTensor/FreeTensor会出现分配资源失败
    // 在NPU上可能体现为卡死等异常行为，在CPU Debug场景会出现报错提示
    pipe.InitBuffer(que0, 1, len);
    pipe.InitBuffer(que1, 1, len);
    pipe.InitBuffer(que2, 1, len);
    pipe.InitBuffer(que3, 1, len);
    pipe.InitBuffer(que4, 1, len);
    pipe.InitBuffer(que5, 1, len);

    AscendC::LocalTensor<T> local1 = que0.AllocTensor<T>();
    AscendC::LocalTensor<T> local2 = que1.AllocTensor<T>();
    AscendC::LocalTensor<T> local3 = que2.AllocTensor<T>();
    AscendC::LocalTensor<T> local4 = que3.AllocTensor<T>();
    // 第5个AllocTensor会出现资源分配失败，同一个TPosition上同时Alloc出来的Tensor数量超出了4个的限制
    AscendC::LocalTensor<T> local5 = que4.AllocTensor<T>();

    // 此时建议通过以下方法解决：
    // 如果确实有多块buffer使用,可以将多个buffer合并到一块buffer,通过偏移使用
    pipe.InitBuffer(que0, 1, len * 3);
    pipe.InitBuffer(que1, 1, len * 3);
    /*
     * 分配出3块内存大小的LocalTensor, local1的地址为que0中buffer的起始地址，
     * local2的地址为local1的地址偏移len后的地址，local3的地址为local1的地址偏移
     * len * 2的地址
     */
    int32_t offset1 = len;
    int32_t offset2 = len * 2;
    AscendC::LocalTensor<T> local1 = que0.AllocTensor<T>();
    AscendC::LocalTensor<T> local2 = local1[offset1];
    AscendC::LocalTensor<T> local3 = local1[offset2];
    ```

-   示例三：inplace接口

    ```cpp
    AscendC::TPipe pipe;
    AscendC::TQue<AscendC::TPosition::VECIN, 0> que;
    int num = 2;
    int len = 1024;
    pipe.InitBuffer(que, num, len); // InitBuffer分配内存块数为2，每块大小为1024Bytes
    AscendC::LocalTensor<half> tensor1;
    que.AllocTensor<half>(tensor1); // AllocTensor分配Tensor长度为1024Bytes
    ```
