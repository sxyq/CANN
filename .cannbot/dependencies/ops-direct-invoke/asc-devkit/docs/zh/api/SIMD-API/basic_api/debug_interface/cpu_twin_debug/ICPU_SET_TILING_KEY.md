# ICPU\_SET\_TILING\_KEY

## 功能说明

头文件路径为：`"tools/cpudebug/include/kern_fwk.h"`。

用于指定本次CPU调测使用的tilingKey。调测执行时，将只执行算子核函数（Kernel）中该tilingKey对应的分支。

## 函数原型

```cpp
#define ICPU_SET_TILING_KEY(tilingKey)
```

## 参数说明

**表1**  模板参数说明

| 参数名称 | 输入/输出 | 描述 |
| ------ | ------ | ------ |
| tilingKey | 输入 | 指定本次CPU调测使用的tilingKey。参数类型为uint64_t。|

## 返回值说明

无

## 约束说明

- 为了保留接口兼容，推荐[<<<>>>](../../../../../guide/programming_guide/debug_and_tuning/functional_debug/cpu_twin_debug.md)编译使用。
- tilingKey建议传入正整数，如果设置为负数或者0，将会告警并继续调测。如果传入0，将会执行tilingKey为0的分支；tilingKey传入负数，将导致未定义的行为。
- 该接口需要在[ICPU_RUN_KF](ICPU_RUN_KF.md)前调用。

## 调用示例

```cpp
ICPU_SET_TILING_KEY(10086);
ICPU_RUN_KF(sort_kernel0, coreNum, (uint8_t*)x, (uint8_t*)y);
```
