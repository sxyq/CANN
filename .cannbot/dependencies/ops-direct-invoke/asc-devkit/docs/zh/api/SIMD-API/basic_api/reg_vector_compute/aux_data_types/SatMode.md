# SatMode

| 参数名 | 描述 |
| :-- | :-- |
| SatMode | 用于设置Cast类型转换饱和与不饱和模式。
enum class SatMode {
    UNKNOWN = -1,
    NO_SAT,
    SAT
};
&bull;  UNKNOWN：当转换类型不支持SatMode时，选择该模式；
&bull;  NO_SAT：不饱和模式；
&bull;  SAT：饱和模式。 |

```cpp
enum class SatMode {
    UNKNOWN = -1,
    NO_SAT,
    SAT
};
```
