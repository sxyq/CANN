# REGISTER\_TILING\_DEFAULT

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/Kernel-Tiling/REGISTER_TILING_DEFAULT_res.md#id1 -->

## 功能说明

头文件路径为：`"utils/base/sys_macros.h"`。

用于在kernel侧注册用户使用标准C++语法自定义的默认TilingData结构体。

注册TilingData结构体用于告知框架侧用户使用标准C++语法来定义TilingData，同时告知框架TilingData结构体类型，用于框架做tiling数据解析。

## 函数原型

```
REGISTER_TILING_DEFAULT(TILING_STRUCT)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| TILING_STRUCT | 输入 | 用户注册的默认自定义TilingData结构体。 |

## 约束说明

-   若TilingData结构体在命名空间内，注册时需要携带对应的命名空间作用域符。
-   暂不支持核函数（Kernel）直调工程。

## 调用示例

```
extern "C" __global__ __aicore__ void add_custom(__gm__ uint8_t *x, __gm__ uint8_t *y, __gm__ uint8_t *z, __gm__ uint8_t *tiling)
{
    REGISTER_TILING_DEFAULT(optiling::TilingData); // 用于在kernel侧注册用户使用标准C++语法自定义的默认TilingData结构体
    GET_TILING_DATA(tilingData, tiling);
    KernelAdd op;
    op.Init(x, y, z, tilingData.blkDim, tilingData.totalSize, tilingData.splitTile);
    op.Process();
}
```
