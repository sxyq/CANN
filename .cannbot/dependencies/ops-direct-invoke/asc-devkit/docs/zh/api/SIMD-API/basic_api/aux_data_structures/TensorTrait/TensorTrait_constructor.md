# TensorTrait构造函数

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/aux_data_structures/TensorTrait/TensorTrait_constructor_res.md#id1 -->

## 功能说明

根据输入的Layout对象，实例化TensorTrait对象。

## 函数原型

```cpp
__aicore__ inline TensorTrait(const LayoutType& t = {})
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| t | 输入 | 输入的Layout对象。输入的数据类型LayoutType，需满足约束说明。 |

## 返回值说明

无

## 约束说明

无

## 调用示例

-   TensorTrait使用示例

    ```cpp
    // TensorTrait使用示例为基于googletest的UT示例

    // MakeTensorTrait方法创建TensorTrait
    AscendC::Shape<int,int,int> shape = AscendC::MakeShape(10, 20, 30);
    AscendC::Stride<int,int,int> stride = AscendC::MakeStride(1, 100, 200);
    auto layoutMake = AscendC::MakeLayout(shape, stride);
    auto tensorTraitMake = AscendC::MakeTensorTrait<float, AscendC::TPosition::VECIN>(layoutMake);

    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitMake.GetLayout().GetShape()), 10);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitMake.GetLayout().GetShape()), 20);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitMake.GetLayout().GetShape()), 30);
    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitMake.GetLayout().GetStride()), 1);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitMake.GetLayout().GetStride()), 100);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitMake.GetLayout().GetStride()), 200);

    // 构造函数方法创建TensorTrait
    using TensorTraitType = AscendC::TensorTrait<half, AscendC::TPosition::VECCALC, AscendC::Layout<AscendC::Shape<int, int, int>, AscendC::Stride<int, int, int>>>;
    TensorTraitType tensorTraitInit(layoutMake);

    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitInit.GetLayout().GetShape()), 10);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitInit.GetLayout().GetShape()), 20);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitInit.GetLayout().GetShape()), 30);
    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitInit.GetLayout().GetStride()), 1);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitInit.GetLayout().GetStride()), 100);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitInit.GetLayout().GetStride()), 200);

    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitInit.GetShape()), 10);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitInit.GetShape()), 20);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitInit.GetShape()), 30);
    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitInit.GetStride()), 1);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitInit.GetStride()), 100);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitInit.GetStride()), 200);

    // SetLayout方法设置TensorTrait
    TensorTraitType tensorTraitSet;
    tensorTraitSet.SetLayout(layoutMake);

    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitSet.GetLayout().GetShape()), 10);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitSet.GetLayout().GetShape()), 20);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitSet.GetLayout().GetShape()), 30);
    EXPECT_EQ(AscendC::Std::get<0>(tensorTraitSet.GetLayout().GetStride()), 1);
    EXPECT_EQ(AscendC::Std::get<1>(tensorTraitSet.GetLayout().GetStride()), 100);
    EXPECT_EQ(AscendC::Std::get<2>(tensorTraitSet.GetLayout().GetStride()), 200);
    ```

-   TensorTrait和API配合使用示例

    ```cpp
    AscendC::LocalTensor<AscendC::TensorTrait<half>> tensor1 = que1.DeQue<AscendC::TensorTrait<half>>();
    AscendC::LocalTensor<AscendC::TensorTrait<half>> tensor2 = que2.DeQue<AscendC::TensorTrait<half>>();
    AscendC::LocalTensor<AscendC::TensorTrait<half>> tensor3 = que3.AllocTensor<AscendC::TensorTrait<half>>();
    Add(tensor3, tensor1, tensor2, tensor3.GetSize());
    ```
