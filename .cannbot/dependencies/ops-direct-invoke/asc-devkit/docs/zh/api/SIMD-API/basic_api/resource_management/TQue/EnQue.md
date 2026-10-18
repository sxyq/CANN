# EnQue

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQue/EnQue_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

将Tensor push到队列。

## 函数原型

-   无需指定源和目的位置

    ```cpp
    template <typename T>
    __aicore__ inline bool EnQue(const LocalTensor<T>& tensor)
    ```

-   需要指定源和目的位置

    通过[TQueBind](../TQueBind/TQueBind.md)绑定VECIN和VECOUT可实现VECIN和VECOUT内存复用，如下接口用于存在Vector计算的场景下实现复用，在入队时需要指定源和目的位置；不存在Vector计算的场景下可直接调用bool EnQue\(LocalTensor<T\>& tensor\)入队接口。

    ```cpp
    template <TPosition srcUserPos, TPosition dstUserPos, typename T>
    __aicore__ inline bool EnQue(const LocalTensor<T>& tensor)
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
| tensor | 输入 | 指定的Tensor。 |

## 约束说明

无

## 返回值说明

-   true - 表示Tensor加入Queue成功
-   false - 表示Queue已满，入队失败

## 调用示例

```cpp
// 接口: EnQue Tensor
AscendC::TPipe pipe;
AscendC::TQue<AscendC::TPosition::VECOUT, 4> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
que.EnQue(tensor1);// 将tensor加入VECOUT的Queue中
// 接口：EnQue指定特定的src/dst position，加入相应的队列
// template <TPosition srcUserPos, TPosition dstUserPos> bool EnQue(LocalTensor<T>& tensor)
AscendC::TPipe pipe;
AscendC::TQueBind<AscendC::TPosition::VECIN, AscendC::TPosition::VECOUT, 1> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
que.EnQue<AscendC::TPosition::GM, AscendC::TPosition::VECIN, half>(tensor1);// 将tensor加入VECIN的Queue中，实现内存复用
```
