# EXTERN\_IMPL\_BUFPOOL宏

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/custom_TBufPool/EXTERN_IMPL_BUFPOOL_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

开发者可以通过[TBufPool类](../TBufPool/TBufPool.md)手动管理Unified Buffer（UB）、L1 Buffer物理内存。

TBufPool类切分的内存块都是连续的，开发者可能有一些自定义的内存块分配需求，比如不连续内存块、内存块在不同TQue之间共享等，这时就需要开发者自定义一个TBufPool的实现。

为了简化开发者的自定义实现，提供EXTERN\_IMPL\_BUFPOOL宏来辅助用户自定义TBufPool。使用自定义TBufPool功能时，需要注意：

-   自定义TBufPool之前，必须通过[TPipe::InitBufPool](../TPipe/InitBufPool.md)接口进行TBufPool内存资源池初始化。
-   自定义TBufPool需要开发者自行实现对TQue/TBuf内存块的分配、初始化、释放等操作。

EXTERN\_IMPL\_BUFPOOL宏内部定义的函数Reset、Init、GetBufHandle、SetCurAddr、GetCurAddr、SetCurBufSize、GetCurBufSize接口参见后续章节描述。使用该宏后，即可使用上述接口完成自定义TBufPool功能。

> [!NOTE]说明
>自定义TBufPool相关接口为试验接口，在后续版本中可能会调整或改进，不保证后续兼容性。请开发者在使用过程中关注后续版本更新。

## 函数原型

```cpp
// 省略宏定义具体内容
#define EXTERN_IMPL_BUFPOOL(EXT_BUFPOOL, POSITION, BUFID_SIZE) ...
```

## 参数说明

**表1**  EXTERN\_IMPL\_BUFPOOL宏原型定义参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| EXT_BUFPOOL | 输入 | 自定义TBufPool类名。 |
| POSITION | 输入 | 自定义TBufPool逻辑位置，可以为VECIN、VECOUT、VECCALC、A1、B1、C1。关于TPosition的具体介绍请参考TPosition。 |
| BUFID_SIZE | 输入 | 自定义TBufPool分配的Buffer块数量，建议不超过16。 |

## 约束说明

无

## 返回值说明

无

## 调用示例

详细示例请参考[tbufpool\_management样例](../../../../../../../examples/01_simd_cpp_api/03_basic_api/04_memory_management/tbufpool_management)。

如下示例中，为tbufPool0划分65536 \* 3大小的内存，然后自定义MyBufPool的InitBuffer函数，实现TQue和Tbuf的内存分配。

```cpp
#include "kernel_operator.h"

class MyBufPool {
public:
    __aicore__ inline MyBufPool() {
        Init();
    }

    template<class T>
    __aicore__ inline bool InitBuffer(T& que, uint8_t num, uint32_t len) {
    }

    template<AscendC::TPosition bufPos>
    __aicore__ inline bool InitBuffer(AscendC::TBuf<bufPos>& buf, uint32_t len) {
    }

    // MyBufPool，自定义TBufPool类名。自定义TBufPool逻辑位置选择VECCALC。
    // 自定义TBufPool分配的Buffer块数量为16
    EXTERN_IMPL_BUFPOOL(MyBufPool, AscendC::TPosition::VECCALC, 16);
};
```
