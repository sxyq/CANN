# HasIdleBuffer

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TQueBind/HasIdleBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

查询Que中是否有空闲的内存块。

## 函数原型

```cpp
__aicore__ inline bool HasIdleBuffer()
```

## 参数说明

无

## 约束说明

该接口不支持[Tensor原地操作](../../../../../guide/programming_guide/appendix/common_operations/tensor_inplace_performance.md)，即TQue的depth设置为0的场景。

## 返回值说明

-   true - 表示Queue中存在空闲内存
-   false - 表示Queue中不存在空闲内存

## 调用示例

```cpp
// 当前Que中已经分配了4块内存
AscendC::TPipe pipe;
AscendC::TQueBind<AscendC::TPosition::VECOUT, AscendC::TPosition::GM, 4> que;
int num = 4;
int len = 1024;
pipe.InitBuffer(que, num, len);
bool ret = que.HasIdleBuffer(); // 没有AllocTensor的操作，返回值为true
AscendC::LocalTensor<half> tensor1 = que.AllocTensor<half>();
ret = que.HasIdleBuffer(); // AllocTensor了一块内存，返回值为true
AscendC::LocalTensor<half> tensor2 = que.AllocTensor<half>();
AscendC::LocalTensor<half> tensor3 = que.AllocTensor<half>();
AscendC::LocalTensor<half> tensor4 = que.AllocTensor<half>();
ret = que.HasIdleBuffer(); // AllocTensor了四块内存，当前无空闲内存，返回值为false，继续AllocTensor会报错
```
