# TILING\_KEY\_LIST

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/Kernel-Tiling/TILING_KEY_LIST_res.md#id1 -->

## 功能说明

TILING\_KEY\_LIST函数用于在核函数（Kernel）中判断当前执行的TilingKey是否与Host侧配置的指定TilingKey匹配，从而标识满足TilingKey == key1或TilingKey == key2条件的分支逻辑。

## 函数原型

```
TILING_KEY_LIST(key1,key2)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| key | 输入 | key表示某个核函数（Kernel）的分支，必须是非负整数。 |

## 约束说明

-   TILING\_KEY\_LIST运用于if和else if分支，不支持else分支，即用TILING\_KEY\_LIST函数来表征N个分支，必须用N个TILING\_KEY\_LIST\(key1,key2\)来分别表示。
-   支持传入两个TilingKey，每个TilingKey具备唯一性。
-   使用该接口时，必须设置默认的核函数（Kernel）类型，也可以为某个TilingKey单独配置核函数（Kernel）类型，该配置会覆盖默认核函数（Kernel）类型。核函数（Kernel）类型仅支持配置为KERNEL\_TYPE\_MIX\_AIC\_1\_1、KERNEL\_TYPE\_MIX\_AIC\_1\_2。
-   暂不支持核函数（Kernel）直调工程。

## 调用示例

```
extern "C" __global__ __aicore__ void add_custom(__gm__ uint8_t *x, __gm__ uint8_t *y, __gm__ uint8_t *z, __gm__ uint8_t *workspace, __gm__ uint8_t *tiling)
{
    GET_TILING_DATA(tilingData, tiling);
    if (workspace == nullptr) {
        return;
    }
    KernelAdd op;
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_MIX_AIC_1_1);
    op.Init(x, y, z, tilingData.numBlocks, tilingData.totalLength, tilingData.tileNum);
    // 当TilingKey为1或2时，执行Process1；为3或4时，执行Process2
    if (TILING_KEY_LIST(1,2)) {
        op.Process1();
    } else if (TILING_KEY_LIST(3,4)) {
        KERNEL_TASK_TYPE(4, KERNEL_TYPE_MIX_AIC_1_2);
        op.Process2();
    }
    // 其他代码逻辑
    ...
    // 此处示例当TilingKey为3或4时，会执行ProcessOther
    if (TILING_KEY_LIST(3,4)) {
        op.ProcessOther();
    }
}
```

配套的Host侧Tiling函数示例（伪代码）：

```
ge::graphStatus TilingFunc(gert::TilingContext* context)
{
    // 其他代码逻辑
    ...
    if (context->GetInputShape(0) > 10) {
        context->SetTilingKey(1);
    } else if (some condition) {
        context->SetTilingKey(2);
    } else if (some condition) {
        context->SetTilingKey(3);
    } else if (some condition) {
        context->SetTilingKey(4);
    }
}
```
