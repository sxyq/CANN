# GetVecRegLen

## 功能说明

获取当前硬件平台芯片架构Vec计算单元位宽。

## 函数原型

```
uint32_t GetVecRegLen(void) const
```

## 参数说明

无

## 返回值

-   对于RegBase硬件平台芯片架构，返回当前硬件平台Vector计算单元位宽。
-   对于非RegBase硬件平台芯片架构，返回0 。

## 约束说明

无

## 调用示例

### 自定义算子工程

```cpp
ge::graphStatus TilingXXX(gert::TilingContext* context)
{
    auto ascendcPlatform = platform_ascendc::PlatformAscendC(context->GetPlatformInfo());
    const uint32_t vecLenB32 = ascendcPlatform.GetVecRegLen() / LAYERNORM_SIZEOF_FLOAT;
    const uint32_t vecLenB16 = ascendcPlatform.GetVecRegLen() / LAYERNORM_SIZEOF_HALF;
    // ... 使用vecLenB32和vecLenB16设计Tiling策略
    return ret;
}
```

### Kernel直调

```cpp
void GetLayerNormMaxMinTmpSize(...)
{
    auto* ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (ascendcPlatform == nullptr) {
        return;
    }
    const uint32_t vecLenB32 = ascendcPlatform->GetVecRegLen() / LAYERNORM_SIZEOF_FLOAT;
    const uint32_t vecLenB16 = ascendcPlatform->GetVecRegLen() / LAYERNORM_SIZEOF_HALF;
    // ... 使用vecLenB32和vecLenB16设计切分策略
}
```
