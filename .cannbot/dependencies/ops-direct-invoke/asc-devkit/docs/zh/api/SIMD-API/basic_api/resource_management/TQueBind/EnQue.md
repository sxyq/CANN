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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQueBind/EnQue_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

将Tensor push到队列。

## 函数原型

```cpp
template <typename T>
__aicore__ inline bool EnQue(const LocalTensor<T>& tensor)
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 说明 |
| --- | --- |
| T | Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

**表2**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| tensor | 输入 | 指定的Tensor |

## 约束说明

无

## 返回值说明

-   true - 表示Tensor加入Queue成功
-   false - 表示Queue已满，入队失败

## 调用示例

```cpp
// 接口: EnQue Tensor
AscendC::TPipe pipe;
AscendC::TQueBind<AscendC::TPosition::VECOUT, AscendC::TPosition::GM, 4> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
que.EnQue(tensor1);// 将tensor加入VECOUT的Queue中
```
