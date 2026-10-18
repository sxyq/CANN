# MaskMergeMode

| 参数名 | 描述 |
| :-- | :-- |
| MaskMergeMode | 用于指定写入寄存器数据模式。
enum class MaskMergeMode {
    UNKNOWN,
    MERGING,
    ZEROING
};
&bull;  UNKNOWN：当转换类型不支持MaskMergeMode时，选择该模式；
&bull;  ZEROING：mask未选择的元素在dst中置零；
&bull;  MERGING：mask未选择的元素对应dst元素中保留dst原值。 |

```cpp
enum class MaskMergeMode {
    UNKNOWN,
    MERGING,
    ZEROING
};
```

以Duplicate接口为例，ZEROING和MERGING模式图示如下：

**图1** ZEROING/MERGING对比<a></a>

![](../../../../figures/zeroing_merging_comparison.png "ZEROING/MERGING对比")
