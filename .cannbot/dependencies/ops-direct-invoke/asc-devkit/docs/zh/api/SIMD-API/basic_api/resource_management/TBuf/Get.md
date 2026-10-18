# Get

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TBuf/Get_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

从TBuf上获取指定长度的Tensor，或者获取全部长度的Tensor。

## 函数原型

-   获取全部长度的Tensor

    ```cpp
    template <typename T>
    __aicore__ inline LocalTensor<T> Get()
    ```

-   获取指定长度的Tensor

    ```cpp
    template <typename T>
    __aicore__ inline LocalTensor<T> Get(uint32_t len)
    ```

## 参数说明

**表1**  模板参数说明

| 参数名称 | 含义 |
| --- | --- |
| T | 待获取Tensor的数据类型，支持的类型请见LocalTensor相关描述。 |

**表2**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| len | 输入 | 需要获取的Tensor元素个数。 |

## 约束说明

len的数值是Tensor中元素的个数，len\*sizeof\(T\)不能超过TBuf初始化时的长度。

## 返回值说明

获取到的[LocalTensor](../../data_structures/LocalTensor/LocalTensor.md)。

## 调用示例

```cpp
// 为TBuf初始化分配内存，分配内存长度为1024字节
AscendC::TPipe pipe;
AscendC::TBuf<AscendC::TPosition::VECCALC> calcBuf; // 模板参数为TPosition中的VECCALC类型
uint32_t byteLen = 1024;
pipe.InitBuffer(calcBuf, byteLen);
// 从calcBuf获取Tensor,Tensor为pipe分配的所有内存大小，为1024字节
AscendC::LocalTensor<int32_t> tempTensor1 = calcBuf.Get<int32_t>();
// 从calcBuf获取Tensor,Tensor为128个int32_t类型元素的内存大小，为512字节
AscendC::LocalTensor<int32_t> tempTensor2 = calcBuf.Get<int32_t>(128);
```

对同一个TBuf对象连续调用Get接口，获取到的Tensor首地址是相同的，不会依次向后偏移。如果需要获取偏移之后的Tensor，可以使用如下方法：

```
AscendC::TPipe pipe;
// 模板参数为TPosition中的VECCALC类型
AscendC::TBuf<AscendC::TPosition::VECCALC> calcBuf;
// 分配一个2048字节的连续空间
uint32_t byteLen = 2048;
pipe.InitBuffer(calcBuf, byteLen);
// 从calcBuf获取tensor1，tensor1为pipe分配的所有内存大小，为2048字节
AscendC::LocalTensor<int32_t> tensor1 = calcBuf.Get<int32_t>();
// 用户指定tensor1的第256个int32_t的偏移位置为tensor2的首地址，实际上tensor2可以使用的内存大小为1024字节，两个tensor首地址相差256 * sizeof(int32_t)字节
auto tensor2 = tensor1[256];
```
