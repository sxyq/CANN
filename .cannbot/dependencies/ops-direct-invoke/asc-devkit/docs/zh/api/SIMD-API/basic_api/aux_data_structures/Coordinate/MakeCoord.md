# MakeCoord

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
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/Coordinate/MakeCoord_res.md#id1 -->

## 功能说明

将传入的数据打包成Coord数据结构。

## 函数原型

```cpp
template <typename... Ts>
__aicore__ inline constexpr Coord<Ts...> MakeCoord(Ts const&... t)
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| t | 输入 | 表示输入数据类型的形参包，具体使用方法和约束说明同Std::tuple。
输入的数据类型支持size_t和Std::Int。 |

## 返回值说明

Coord结构类型（[Std::tuple](../../../../Utils-API/cpp_stdlib/container_functions/container_functions.md)类型的别名）。

## 约束说明

无

## 调用示例

```cpp
auto blockCoordM = Std::Int<11>{};
auto blockCoordN = Std::Int<12>{};
auto coord = AscendC::MakeCoord(blockCoordM, blockCoordN);
```
