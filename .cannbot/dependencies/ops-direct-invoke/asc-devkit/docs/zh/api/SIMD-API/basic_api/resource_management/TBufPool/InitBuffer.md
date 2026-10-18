# InitBuffer

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TBufPool/InitBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

调用TBufPool::InitBuffer接口为TQue/TBuf进行内存分配。

## 函数原型

-   为TQue分配内存
    ```cpp
    template <class T>
    __aicore__ inline bool InitBuffer(T& que, uint8_t num, uint32_t len)
    ```
-   为TBuf分配内存
    ```cpp
    template <TPosition pos>
    __aicore__ inline bool InitBuffer(TBuf<pos>& buf, uint32_t len)
    ```

## 参数说明

**表1**  为TQue分配内存的函数原型-模板参数说明

| 参数名称 | 含义 |
| -------- | ---- |
| T | que参数的类型。|

**表2**  为TQue分配内存的函数原型-接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| que | 输入 | 需要分配内存的TQue对象 |
| num | 输入 | 分配内存块的个数 |
| len | 输入 | 每个内存块的大小，单位为Bytes，非32Bytes对齐会自动向上补齐至32Bytes对齐 |

**表3**  为TBuf分配内存的函数原型-模板参数说明

| 参数名称 | 含义 |
| -------- | ---- |
| pos | Buffer逻辑位置，可以为VECIN、VECOUT、VECCALC、A1、B1、C1。关于TPosition的具体介绍请参考[TPosition](../../aux_data_structures/TPosition.md)。|

**表4**  为TBuf分配内存的函数原型-接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| buf | 输入 | 需要分配内存的TBuf对象 |
| len | 输入 | 为TBuf分配的内存大小，单位为Bytes，非32Bytes对齐会自动向上补齐至32Bytes对齐 |

## 约束说明

声明TBufPool时，可以通过bufIDSize指定可分配Buffer的最大数量，默认上限为4，最大为16。TQue或TBuf的物理内存需要和TBufPool一致。

## 返回值说明

无

## 调用示例

参考[InitBufPool](InitBufPool.md)。
