# 运算符重载

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：不支持
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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/Layout/operator_overloading_res.md#id1 -->

## 功能说明

重载运算符\(\)，内部实现为调用[Crd2Idx](../Coordinate/Crd2Idx.md)接口，将多维坐标（Coordinate）通过布局（Layout）转换为内存位置索引（Index）。

## 函数原型

```cpp
template <typename CoordType>
__aicore__ inline constexpr auto operator()(const CoordType& coord) const {}
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| coord | 输入 | Coord结构类型，用于表示张量在不同维度上的坐标值。 |

## 返回值说明

返回根据Coordinate信息转换之后的索引值。

## 约束说明

无

## 调用示例

```cpp
...
auto layout = AscendC::MakeLayout(shape, stride);
auto index = layout(coord);
```
