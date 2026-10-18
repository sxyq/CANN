# GetLibApiWorkSpaceSize

## 功能说明

获取AscendC API需要的workspace空间大小。

## 函数原型

```
uint32_t GetLibApiWorkSpaceSize(void) const
```

## 参数说明

无

## 返回值说明

返回uint32\_t数据类型的结果，该结果代表当前系统workspace的大小，单位为字节。接口正常返回workspace大小，当返回`std::numeric_limits<uint32_t>::max()`（即`UINT32_MAX`）则接口异常。

## 约束说明

无

## 调用示例

### 自定义算子工程

```cpp
// 用户自定义的tiling函数
static ge::graphStatus TilingFunc(gert::TilingContext* context)
{
    AddApiTiling tiling;
    ...
    size_t usrSize = 256; // 设置用户需要使用的workspace大小。
    // 如需要使用系统workspace需要调用GetLibApiWorkSpaceSize获取系统workspace的大小。
    auto ascendcPlatform = platform_ascendc:: PlatformAscendC(context->GetPlatformInfo());
    uint32_t sysWorkspaceSize = ascendcPlatform.GetLibApiWorkSpaceSize();
    if (sysWorkspaceSize == std::numeric_limits<uint32_t>::max()) {
        return ge::GRAPH_FAILED;
    }
    size_t *currentWorkspace = context->GetWorkspaceSizes(1); // 通过框架获取workspace的指针，GetWorkspaceSizes入参为所需workspace的块数。当前限制使用一块。
    currentWorkspace[0] = usrSize + sysWorkspaceSize; // 设置总的workspace的数值大小，总的workspace空间由框架来申请并管理。
    ...
}
```

### Kernel直调

```cpp
uint32_t GetSysWorkspaceSize()
{
    auto* ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (ascendcPlatform == nullptr) {
        return std::numeric_limits<uint32_t>::max();
    }
    return ascendcPlatform->GetLibApiWorkSpaceSize();
}
```
