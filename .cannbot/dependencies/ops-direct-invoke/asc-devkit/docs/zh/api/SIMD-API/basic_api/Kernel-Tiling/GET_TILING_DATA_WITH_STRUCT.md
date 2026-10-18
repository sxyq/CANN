# GET\_TILING\_DATA\_WITH\_STRUCT

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/Kernel-Tiling/GET_TILING_DATA_WITH_STRUCT_res.md#id1 -->

## 功能说明

使用该接口指定结构体名称，可获取指定的tiling信息，并填入对应的Tiling结构体中，此函数会以宏展开的方式进行编译。与[GET\_TILING\_DATA](GET_TILING_DATA.md)的区别是：[GET\_TILING\_DATA](GET_TILING_DATA.md)只能获取默认注册的结构体，该接口可以根据指定的结构体名称获取对应的结构体，常用于针对不同的TilingKey注册了不同结构体的情况下。

## 函数原型

```
GET_TILING_DATA_WITH_STRUCT(struct_name, tiling_data, tiling_arg)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| struct_name | 输入 | 指定的结构体名称。 |
| tiling_data | 输出 | 返回指定Tiling结构体变量。 |
| tiling_arg | 输入 | 此参数为算子入口函数处传入的tiling参数。 |

## 约束说明

-   本函数需在算子核函数（Kernel）代码处使用，并且传入的tiling\_data参数不需要声明类型。
-   暂不支持核函数（Kernel）直调工程。

## 调用示例

```
extern "C" __global__ __aicore__ void add_custom(__gm__ uint8_t *x, __gm__ uint8_t *y, __gm__ uint8_t *z, __gm__ uint8_t *tiling)
{
    KernelAdd op;
    if (TILING_KEY_IS(1)) {
        GET_TILING_DATA_WITH_STRUCT(Add_Struct_Special, tilingData, tiling); // 使用算子指定注册的结构体
	op.Init(x, y, z, tilingData.totalLengthSpecial, tilingData.tileNumSpecial);
    } else {
        GET_TILING_DATA(tilingData, tiling);   // 使用算子默认注册的结构体
	op.Init(x, y, z, tilingData.totalLength, tilingData.tileNum);
    }
    if (TILING_KEY_IS(1)) {
        op.Process();
    } else if (TILING_KEY_IS(2)) {
        op.Process();
    } else if (TILING_KEY_IS(3)) {
        op.Process();
    }
}
```
