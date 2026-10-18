# GetCoreNumVector

## 功能说明

用于获取硬件平台独立的Vector Core的核数。

该接口仅在Atlas 推理系列产品有效，其他硬件平台型号均返回0。

## 函数原型

```
uint32_t GetCoreNumVector(void) const
```

## 参数说明

无

## 返回值说明

返回硬件平台Vector Core的核数。

## 约束说明

<!-- npu="910" id1 -->
Atlas 训练系列产品，不支持该接口，返回0
<!-- end id1 -->
<!-- npu="310p" id2 -->
Atlas 推理系列产品，支持该接口，返回硬件平台Vector Core的核数
<!-- end id2 -->
<!-- npu="910b" id3 -->
Atlas A2 训练系列产品/Atlas A2 推理系列产品不支持该接口，返回0
<!-- end id3 -->
<!-- npu="A3" id4 -->
Atlas A3 训练系列产品/Atlas A3 推理系列产品不支持该接口，返回0
<!-- end id4 -->
<!-- npu="310b" id5 -->
Atlas 200I/500 A2 推理产品不支持该接口，返回0
<!-- end id5 -->
<!-- npu="950" id6 -->
Ascend 950PR/Ascend 950DT不支持该接口，返回0
<!-- end id6 -->

## 调用示例

### 自定义算子工程

```cpp
ge::graphStatus TilingXXX(gert::TilingContext* context) {
    auto ascendcPlatform = platform_ascendc::PlatformAscendC(context->GetPlatformInfo());
    auto aivCoreNum = ascendcPlatform.GetCoreNumAiv();
    auto vectorCoreNum = ascendcPlatform.GetCoreNumVector();
    auto allVecCoreNums = aivCoreNum + vectorCoreNum;
    // ...按照allVecCoreNums切分
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
    auto aivCoreNum = ascendcPlatform->GetCoreNumAiv();
    auto vectorCoreNum = ascendcPlatform->GetCoreNumVector();
    auto allVecCoreNums = aivCoreNum + vectorCoreNum;
    // ... 根据allVecCoreNums自行设计切分策略
}
```
