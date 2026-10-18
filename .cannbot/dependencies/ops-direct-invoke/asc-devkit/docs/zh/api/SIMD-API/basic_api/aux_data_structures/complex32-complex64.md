# complex32/complex64

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：不支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：不支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：不支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/complex32-complex64_res.md#id1 -->

复数类型，其中complex32表示实部和虚部都是half类型的复数，位宽为32位；complex64表示实部和虚部都是float类型的复数，位宽为64位。

具体定义如下：

```cpp
namespace AscendC {
template<class T>
struct Complex {
    T real;
    T imag;
};
} // namespace AscendC
using complex32 = AscendC::Complex<half>;
using complex64 = AscendC::Complex<float>;
```

**表1**  Complex模板参数说明

| 参数名称 | 描述 |
| --- | --- |
| T | 实部和虚部的数据类型，仅支持half/float。 |

**表2**  Complex结构体参数说明

| 函数名称 | 入参说明 |
| --- | --- |
| real | 实部，类型为T，仅支持half/float。 |
| imag | 虚部，类型为T，仅支持half/float。 |

使用示例如下：

```cpp
// value0代表实部为1，虚部为2的复数，即1+2j
complex32 value0(1, 2);
// value1代表实部为3，虚部为0的复数，即3+0j
complex32 value1(3);
// value2代表实部为4，虚部为0的复数，即4+0j
complex64 value2 = 4;
```
