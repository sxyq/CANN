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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/InitBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

用于为TQue等队列和TBuf分配内存。

## 函数原型

-   为TQue等队列分配内存

    ```cpp
    template <class T>
    __aicore__ inline bool InitBuffer(T& que, uint8_t num, uint32_t len)

    // 为TQue等队列分配内存，且开发者可以自定义内存的地址信息，地址信息包含起始地址和长度。
    template <class T, class U, class V, class... Addrs>
    __aicore__ inline bool InitBuffer(T& que, const Std::tuple<U, V>& addr0, const Addrs&... addrs)
    ```

-   为TBuf分配内存

    ```cpp
    template <TPosition bufPos>
    __aicore__ inline bool InitBuffer(TBuf<bufPos>& buf, uint32_t len)
    ```

## 参数说明

**表1**  为TQue等队列分配内存的函数原型-模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| T | 队列的类型，支持取值TQue、TQueBind、TSCM。 |

**表2**  为TQue等队列分配内存的函数原型-接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| que | 输入 | 需要分配内存的TQue等对象。 |
| num | 输入 | 分配内存块的个数。double buffer功能通过该参数开启：num设置为1，表示不开启double buffer；num设置为2，表示开启double buffer。 |
| len | 输入 | 每个内存块的大小，单位为字节。当传入的len不满足32字节对齐时，API内部会自动向上补齐至32字节对齐，后续的数据搬运过程会涉及非对齐处理。 |

**表3**  自定义地址为TQue等队列分配内存的函数原型-模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| T | 队列的类型，支持取值TQue、TQueBind。 |
| U | 起始地址的类型。类型为整型。 |
| V | 长度的类型。类型为整型。 |
| Addrs... | tuple形式的地址信息，包含起始地址和长度。 |

**表4**  自定义地址为TQue等队列分配内存的函数原型-接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| que | 输入 | 需要分配内存的TQue等对象。 |
| addr0 | 输入 | 分配内存块的地址信息，结构为起始地址和长度。 |
| addrs | 输入 | 格式为tuple的地址信息列表，tuple内的元素个数须为2，即起始地址和长度。 |

**表5**  为TBuf分配内存的函数原型-模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| bufPos | TBuf所在的逻辑位置，TPosition类型。 |

**表6**  为TBuf分配内存的函数原型-接口参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| buf | 输入 | 需要分配内存的TBuf对象。 |
| len | 输入 | 为TBuf分配的内存大小，单位为字节。当传入的len不满足32字节对齐时，API内部会自动向上补齐至32字节对齐，后续的数据搬运过程会涉及非对齐处理。 |

## 约束说明

-   InitBuffer申请的内存会在TPipe对象销毁时通过析构函数自动释放，无需手动释放。
-   如果需要重新分配InitBuffer申请的内存，可以调用[Reset](Reset.md)，再调用InitBuffer接口。
-   一个kernel中所有使用的Buffer数量之和不能超过64。
-   自定义地址InitBuffer方式分配不建议与不指定地址的混用，可能会导致内存冲突。
<!-- npu="950" id10 -->
- 针对Ascend 950PR/Ascend 950DT，对Unified Buffer（UB）的内存分配，会基于静态内存的基础上进行分配，即动态内存初始位置，参考[内存层级](../../../../../guide/programming_guide/advanced_programming/advanced_ai_core_programming_model/simd_simt_hybrid_programming/memory_hierarchy.md)。
<!-- end id10 -->
- 当前暂不支持与L1 Buffer, L0A Buffer, L0B Buffer, L0C Buffer, BiasTable静态数组混用。

## 返回值说明

返回Buffer初始化的结果。

## 调用示例

```cpp
// 为TQue分配内存，分配内存块数为2，每块大小为128字节
AscendC::TPipe pipe; // Pipe内存管理对象
AscendC::TQue<AscendC::TPosition::VECOUT, 2> que; // 输出数据队列管理对象，TPosition为VECOUT
uint8_t num = 2;
uint32_t len = 128;
pipe.InitBuffer(que, num, len);

// 为TQue分配内存，开发者自定义内存的地址信息，分别为[0, 1024], [2048, 4096], [8192, 12288]
AscendC::TPipe pipe; // Pipe内存管理对象
AscendC::TQue<AscendC::TPosition::VECOUT, 1> que; // 输出数据队列管理对象，TPosition为VECOUT
auto addr0 = Std::make_tuple(0, 1024);
auto addr1 = Std::make_tuple(2048, 2048);
auto addr2 = Std::make_tuple(8192, 4096);
pipe.InitBuffer(que, addr0, addr1, addr2);

// 为TBuf分配内存，分配长度为128字节
AscendC::TPipe pipe;
AscendC::TBuf<AscendC::TPosition::A1> buf; // 输出数据管理对象，TPosition为A1
uint32_t len = 128;
pipe.InitBuffer(buf, len);
```
