# GET\_TILING\_DATA

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
- Atlas 推理系列产品Vector Core：支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/Kernel-Tiling/GET_TILING_DATA_res.md#id1 -->

## 功能说明

用于获取算子kernel入口函数传入的Tiling信息，并填入注册的TilingData结构体中，此函数会以宏展开的方式进行编译。对应的算子host实现中需要定义TilingData结构体，实现并注册计算TilingData的Tiling函数。如果用户通过[TilingData结构注册](../../../Utils-API/Tiling_data_structure_register/TilingData_struct_register.md)注册了多个TilingData结构体，使用该接口返回默认注册的结构体。

## 函数原型

```
GET_TILING_DATA(tiling_data, tiling_arg)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| tiling_data | 输出 | 返回默认TilingData结构体变量。 |
| tiling_arg | 输入 | 此参数为算子入口函数处传入的Tiling参数。 |

## 约束说明

-   本函数需在算子kernel代码处使用，并且传入的tiling\_data参数不需要声明类型。
-   暂不支持kernel直调工程。

## 调用示例

```
extern "C" __global__ __aicore__ void add_custom(__gm__ uint8_t *x, __gm__ uint8_t *y, __gm__ uint8_t *z, __gm__ uint8_t *tiling)
{
    GET_TILING_DATA(tilingData, tiling);// 反序列化SaveToBuffer生成的数据，并填入注册的TilingData结构体中
    KernelAdd op;
    op.Init(x, y, z, tilingData.numBlocks, tilingData.totalSize, tilingData.splitTile);
    op.Process();
}
```

配套的host侧Tiling函数示例：

```
ge::graphStatus TilingFunc(gert::TilingContext* context)
{
    // 其他代码逻辑
    ...
    TilingData tiling;  // 与算子host实现中定义TilingData结构体的对应
    tiling.set_blkDim(numBlocks);  // 与算子host实现中定义TilingData结构体中的成员的对应
    tiling.set_totalSize(totalSize);
    tiling.set_splitTile(splitTile);
    tiling.SaveToBuffer(context->GetRawTilingData()->GetData(), context->GetRawTilingData()->GetCapacity());
    ...
    // 其他代码逻辑
}
```
