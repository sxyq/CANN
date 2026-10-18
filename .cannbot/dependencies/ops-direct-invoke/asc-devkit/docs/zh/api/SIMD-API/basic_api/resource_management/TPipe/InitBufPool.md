# InitBufPool

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/InitBufPool_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

初始化TBufPool内存资源池。本接口适用于内存资源有限时，希望手动指定Unified Buffer（UB）/L1内存资源复用的场景。本接口初始化后在整体内存资源中划分出一块子资源池。划分出的子资源池TBufPool，提供了如下方式进行资源管理：

-   TPipe::InitBufPool的重载接口指定与其他TBufPool子资源池复用;
-   TBufPool::[InitBufPool](../TBufPool/InitBufPool.md)接口对子资源池继续划分；
-   TBufPool::[InitBuffer](../TBufPool/InitBuffer.md)接口分配Buffer；

关于TBufPool的具体介绍及资源划分图示请参考[TBufPool](../TBufPool/TBufPool.md)。

## 函数原型

```cpp
template <class T>
__aicore__ inline bool InitBufPool(T& bufPool, uint32_t len)
template <class T, class U>
__aicore__ inline bool InitBufPool(T& bufPool, uint32_t len, U& shareBuf)
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| T | bufPool的类型。 |
| U | shareBuf的类型。 |

**表2**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| bufPool | 输入 | 新划分的资源池，类型为TBufPool。 |
| len | 输入 | 新划分资源池长度，单位为Byte，非32Bytes对齐会自动补齐至32Bytes对齐。 |
| shareBuf | 输入 | 被复用资源池，类型为TBufPool，新划分资源池与被复用资源池共享起始地址及长度。 |

## 约束说明

-   新划分的资源池与被复用资源池的硬件属性需要一致，两者共享起始地址及长度；
-   输入长度需要小于等于被复用资源池长度；
-   其他泛用约束参考[TBufPool](../TBufPool/TBufPool.md)。

## 返回值说明

无

## 调用示例

由于物理内存的大小有限，在计算过程没有数据依赖的场景或数据依赖串行的场景下，可以通过指定内存复用解决资源不足的问题。完整算子样例参考：[tbufpool\_management样例](../../../../../../../examples/01_simd_cpp_api/03_basic_api/04_memory_management/tbufpool_management)。

```cpp
// 声明一个指向TPipe管道对象的指针
AscendC::TPipe* pipe;
// 定义两个子资源池对象tbufPool1和tbufPool2
AscendC::TBufPool<AscendC::TPosition::VECCALC> tbufPool1, tbufPool2;
// 初始化第一个子资源池tbufPool1
pipe->InitBufPool(tbufPool1, bufSize * 3);
// 初始化第二个子资源池tbufPool2，并指定tbufPool2复用tbufPool1的起始地址及长度；
pipe->InitBufPool(tbufPool2, bufSize * 3, tbufPool1);

// 计算串行，不存在数据踩踏，实现了内存复用及自动同步的能力
tbufPool1.InitBuffer(srcQue0, 1, bufSize);
tbufPool1.InitBuffer(srcQue1, 1, bufSize);
tbufPool1.InitBuffer(dstQue0, 1, bufSize);
CopyIn();
Compute();
CopyOut();
tbufPool1.Reset();
tbufPool2.InitBuffer(srcQue2, 1, bufSize);
tbufPool2.InitBuffer(srcQue3, 1, bufSize);
tbufPool2.InitBuffer(dstQue1, 1, bufSize);
CopyIn1();
Compute1();
CopyOut1();
tbufPool2.Reset();
```
