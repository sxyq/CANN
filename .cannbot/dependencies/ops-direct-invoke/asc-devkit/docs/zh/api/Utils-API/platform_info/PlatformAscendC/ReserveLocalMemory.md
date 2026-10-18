# ReserveLocalMemory

## 功能说明

该函数用于在Unified Buffer（UB）中预留指定大小的内存空间。调用该接口后，使用[GetCoreMemSize](GetCoreMemSize.md)可以获取实际可用的剩余UB空间大小。

## 函数原型

```
void ReserveLocalMemory(ReservedSize size)
```

## 参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| ReservedSize | 输入 | 需要预留的空间大小。
enum class ReservedSize {
    RESERVED_SIZE_8K,  // 预留8 * 1024B空间
    RESERVED_SIZE_16K, // 预留16 * 1024B空间
    RESERVED_SIZE_32K, // 预留32 * 1024B空间
}; |

## 返回值说明

无

## 约束说明

多次调用该函数时，仅保留最后一次调用的结果。

## 调用示例

### 自定义算子工程

```cpp
ge::graphStatus TilingXXX(gert::TilingContext* context) {
    auto ascendcPlatform = platform_ascendc::PlatformAscendC(context->GetPlatformInfo());
    uint64_t ub_size, l1_size;
    // 预留8KB的UB内存空间
    ascendcPlatform.ReserveLocalMemory(platform_ascendc::ReservedSize::RESERVED_SIZE_8K);
    // 获取UB和L1的实际可用内存大小
    ascendcPlatform.GetCoreMemSize(platform_ascendc::CoreMemType::UB, ub_size);
    ascendcPlatform.GetCoreMemSize(platform_ascendc::CoreMemType::L1, l1_size);
    // ...
    return ret;
}
```

### Kernel直调

```cpp
void ReserveUbMemory()
{
    auto* ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (ascendcPlatform == nullptr) {
        return;
    }
    ascendcPlatform->ReserveLocalMemory(platform_ascendc::ReservedSize::RESERVED_SIZE_8K);
    uint64_t ubSize;
    ascendcPlatform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
    // ... 使用ubSize
}
```

完整样例可参考[与数学库高阶API配合使用的样例](../../../SIMD-API/adv_api/math_compute/more_examples.md#section577043422516)。
