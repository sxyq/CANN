# GetCurNpuArch

## 功能说明

获取当前硬件平台芯片架构版本号。

## 函数原型

```
NpuArch GetCurNpuArch(void) const
```

## 参数说明

无

## 返回值

当前硬件平台架构号的枚举类。该枚举类和AI处理器型号的对应关系请通过CANN软件安装后文件存储路径下include/platform/soc\_spec.h头文件获取。

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：DAV\_3510
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：DAV\_2201
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：DAV\_2201
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：DAV\_3002
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：DAV\_2002
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：DAV\_2002
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：DAV\_1001
<!-- end id7 -->

## 约束说明

无

## 调用示例

### 自定义算子工程

```cpp
ge::graphStatus TilingXXX(gert::TilingContext* context) {
    auto ascendcPlatform = platform_ascendc::PlatformAscendC(context->GetPlatformInfo());
    auto npuArch = ascendcPlatform.GetCurNpuArch();
    // 根据所获得的版本型号自行设计Tiling策略
    // DAV_XXX请替换为实际的架构号
    if (socVersion == NpuArch::DAV_XXXX) {
        // ...
    }
    return ret;
}
```

### Kernel直调

```cpp
void GetInfoFun()
{
    auto* ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (ascendcPlatform == nullptr) {
        return;
    }
    auto npuArch = ascendcPlatform->GetCurNpuArch();
    // ... 根据npuArch自行设计切分策略
}
```
