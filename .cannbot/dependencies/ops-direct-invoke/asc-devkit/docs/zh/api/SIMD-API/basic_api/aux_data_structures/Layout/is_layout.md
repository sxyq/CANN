# is\_layout

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/Layout/is_layout_res.md#id1 -->

## 功能说明

判断输入的数据结构是否为Layout数据结构，可通过检查其成员常量value的值来判断。当value为true时，表示输入的数据结构是Layout类型；反之则为非Layout类型。

## 函数原型

```cpp
template <typename T> struct is_layout
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| T | 根据输入的数据类型，判断是否为Layout数据结构。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

```cpp
// 初始化Layout数据结构并判断其类型
AscendC::Shape<int,int,int> shape = AscendC::MakeShape(10, 20, 30);
AscendC::Stride<int,int,int> stride = AscendC::MakeStride(1, 100, 200);

auto layoutMake = AscendC::MakeLayout(shape, stride);
AscendC::Layout<AscendC::Shape<int, int, int>, AscendC::Stride<int, int, int>> layoutInit(shape, stride);

bool value = AscendC::is_layout<decltype(shape)>::value; //value = false
value = AscendC::is_layout<decltype(stride)>::value; //value = false

value = AscendC::is_layout<decltype(layoutMake)>::value;//value = true
value = AscendC::is_layout<decltype(layoutInit)>::value;//value = true
```
