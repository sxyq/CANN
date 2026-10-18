# COPY\_TILING\_WITH\_ARRAY

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/Kernel-Tiling/COPY_TILING_WITH_ARRAY_res.md#id1 -->

## 功能说明

拷贝指定大小的数组内容到目标数组中，并返回指向拷贝后数组的指针。适用于拷贝一个结构体的数组成员变量的场景。该宏将指定数组拷贝至栈上，适用于频繁访问Tiling数据的场景，能够加快数据访问速度。

## 函数原型

```
COPY_TILING_WITH_ARRAY(arr_type, arr_count, src_ptr, dst_ptr)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| arr_type | 输入 | 指定要拷贝的数组类型。 |
| arr_count | 输入 | 指定要拷贝的数组大小。 |
| src_ptr | 输入 | 指向tiling_struct结构体的指针。 |
| dst_ptr | 输出 | 返回拷贝后的指向类型为arr_type、大小为arr_count的数组指针。 |

## 约束说明

-   该宏需在算子核函数（Kernel）代码处使用，并且传入的dst\_ptr参数无需声明类型。
-   该宏需要和[GET\_TILING\_DATA\_PTR\_WITH\_STRUCT](GET_TILING_DATA_PTR_WITH_STRUCT.md)配合使用，输入参数src\_ptr为GET\_TILING\_DATA\_PTR\_WITH\_STRUCT获取到的指针。
-   该宏获取到的dst\_ptr指针指向的数组是局部变量，请确保在合理作用域范围内使用。
-   暂不支持核函数（Kernel）直调工程。

## 调用示例

```
extern "C" __global__ __aicore__ void add_custom(__gm__ uint8_t *x, __gm__ uint8_t *y, __gm__ uint8_t *z, __gm__ uint8_t *tiling)
{
    KernelAdd op;

    GET_TILING_DATA_PTR_WITH_STRUCT(AddCustomTilingData, tilingDataPtr, tiling);

    if ASCEND_IS_AIV {
        COPY_TILING_WITH_ARRAY(uint64_t, 2, tilingDataPtr->vectorTilingArray, vTilingArrayPtr);
        op.Init(x, y, z, (*vTilingArrayPtr)[0], (*vTilingArrayPtr)[1]);
        op.Process();
    } else {
        COPY_TILING_WITH_ARRAY(uint64_t, 2, tilingDataPtr->cubeTilingArray, cTilingArrayPtr);
	op.Init(x, y, z, (*cTilingArrayPtr)[0], (*cTilingArrayPtr)[1]);
	op.Process();
    }
}
```
