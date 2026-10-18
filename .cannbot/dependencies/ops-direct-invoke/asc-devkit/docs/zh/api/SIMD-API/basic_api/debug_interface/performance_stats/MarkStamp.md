# MarkStamp

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/debug_interface/performance_stats/MarkStamp_res.md#id1 -->

## 功能说明

推荐使用[asc\_mark\_stamp](../../../../Utils-API/tuning_interface/asc_mark_stamp.md)接口进行特定位置的标记，该接口同时适用于C语言和C++语言编程。

用户通过调用接口，用于在算子执行过程中标记特定位置，便于后期通过流水图分析代码执行路径与性能热点。

## 函数原型

```
template<pipe_t pipe, uint16_t index>
__aicore__ inline void MarkStamp()

template<pipe_t pipe>
__aicore__ inline void MarkStamp(uint16_t index)
```

## 参数说明

| 参数名 | 含义 |
| --- | --- |
| pipe | 指定打点所在的pipeline类型。 |
| index | 用户设置的打点的唯一标识id。 |

## 返回值说明

无

## 约束说明

-   芯片给index预留了12个bit，可使用的取值范围为\[0,256\)。为方便从打点图中找到对应的代码，建议不要重复使用相同的index。
-   如果在循环中增加了一个MarkStamp指令，每次执行到指令时都会输出一个打点，且index是相同的。
-   如果开发者在两个相邻的VF分别打标记，由于编译器可能会对VF A和VF B做融合，MarkStamp1和MarkStamp2则会被优化掉，不会输出打点。

## 调用示例

```
mte2_opt();
//在算子执行开始处打点
MarkStamp<PIPE_V, 0>();
//执行核心计算
vector_opt();
//在算子执行结束处打点
MarkStamp<PIPE_V, 1>();
mte3_opt();
```
