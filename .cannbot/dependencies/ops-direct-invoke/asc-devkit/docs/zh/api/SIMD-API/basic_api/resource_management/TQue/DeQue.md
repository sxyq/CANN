# DeQue

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQue/DeQue_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

将Tensor从队列中取出，用于后续处理。

## 函数原型

-   无需指定源和目的位置
    -   non-inplace接口：将入队的LocalTensor地址从队列中取出赋值给新创建的Tensor并返回。

        ```cpp
        template <typename T>
        __aicore__ inline LocalTensor<T> DeQue()
        ```

    -   inplace接口：通过出参的方式返回，可以减少Tensor反复创建的开销，具体使用指导可参考[Tensor原地操作](../../../../../guide/programming_guide/appendix/common_operations/tensor_inplace_performance.md)。

        ```cpp
        template <typename T>
        __aicore__ inline void DeQue(LocalTensor<T>& tensor)
        ```

-   需要指定源和目的位置

    通过[TQueBind](../TQueBind/TQueBind.md)绑定VECIN和VECOUT可实现VECIN和VECOUT内存复用，如下接口用于存在Vector计算的场景下实现复用，在出队时需要指定源和目的位置；不存在Vector计算的场景下可直接调用LocalTensor<T\> DeQue\(\)出队接口。

    ```cpp
    template <TPosition srcUserPos, TPosition dstUserPos, typename T>
    __aicore__ inline LocalTensor<T> DeQue()
    ```

## 参数说明

**表1**  模板参数说明

| 参数名 | 说明 |
| --- | --- |
| T | Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |
| srcUserPos | 用户指定队列的src position，当前只支持如下通路：GM->Unified Buffer（UB，VECIN/VECOUT）->GM。 |
| dstUserPos | 用户指定队列的dst position，当前只支持如下通路：GM->UB（VECIN/VECOUT）->GM。 |

**表2**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| tensor | 输出 | inplace接口需要通过出参的方式返回Tensor。 |

## 约束说明

-   对空队列执行DeQue是一种异常行为，会在CPU调测时报错。
-   non-inplace接口和指定源和目的位置的接口，需要将TQueBind的depth模板参数设置为非零值；inplace接口，需要将TQueBind的depth模板参数设置为0。

## 返回值说明

non-inplace接口和指定源和目的位置的接口返回值为从队列中取出的[LocalTensor](../../data_structures/LocalTensor/LocalTensor.md)；inplace接口没有返回值。

## 调用示例

```cpp
// 接口: DeQue Tensor
AscendC::TPipe pipe;
AscendC::TQue<AscendC::TPosition::VECOUT, 4> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
que.EnQue(tensor1);
AscendC::LocalTensor<half> tensor2 = que.DeQue<half>(); // 将tensor从VECOUT的Queue中搬出
// 接口: DeQue Tensor，指定特定的Src/Dst position
AscendC::TPipe pipe;
AscendC::TQueBind<AscendC::TPosition::VECIN, AscendC::TPosition::VECOUT, 1> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
que.EnQue<AscendC::TPosition::GM, AscendC::TPosition::VECIN, half>(tensor1);
// 将tensor从VECIN的Queue中搬出
AscendC::LocalTensor<half> tensor2 = que.DeQue<AscendC::TPosition::GM, AscendC::TPosition::VECIN, half>();
// inplace接口
AscendC::TPipe pipe;
AscendC::TQue<AscendC::TPosition::VECOUT, 0> que;
int num = 2;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1;
que.AllocTensor<half>(tensor1);
que.EnQue(tensor1);
que.DeQue<half>(tensor1); // 将tensor从VECOUT的Queue中搬出
que.FreeTensor<half>(tensor1);
```
