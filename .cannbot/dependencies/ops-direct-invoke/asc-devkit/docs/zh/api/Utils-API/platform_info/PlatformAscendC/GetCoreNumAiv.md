# GetCoreNumAiv

## 功能说明

获取当前硬件平台AI Core中Vector核数。若AI Core的架构为Cube、Vector分离模式，返回Vector Core的核数；耦合模式返回AI Core的核数。

## 函数原型

```
uint32_t GetCoreNumAiv(void) const
```

## 参数说明

无

## 返回值说明

<!-- npu="910" id1 -->
Atlas 训练系列产品，耦合模式，返回AI Core的核数
<!-- end id1 -->
<!-- npu="310p" id2 -->
Atlas 推理系列产品，耦合模式，返回AI Core的核数
<!-- end id2 -->
<!-- npu="910b" id3 -->
Atlas A2 训练系列产品/Atlas A2 推理系列产品，分离模式，返回Vector Core的核数
<!-- end id3 -->
<!-- npu="A3" id4 -->
Atlas A3 训练系列产品/Atlas A3 推理系列产品，分离模式，返回Vector Core的核数
<!-- end id4 -->
<!-- npu="950" id5 -->
Ascend 950PR/Ascend 950DT，分离模式，返回Vector Core的核数
<!-- end id5 -->

## 约束说明

无

## 调用示例

### 自定义算子工程

```cpp
ge::graphStatus TilingXXX(gert::TilingContext* context) {
    auto ascendcPlatform = platform_ascendc::PlatformAscendC(context->GetPlatformInfo());
    auto aicNum = ascendcPlatform.GetCoreNumAic();
    auto aivNum = ascendcPlatform.GetCoreNumAiv();
    // ...按照aivNum切分
    context->SetSimdNumBlocks(ascendcPlatform.CalcTschNumBlocks(aivNum, aicNum, aivNum));
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
    auto aicNum = ascendcPlatform->GetCoreNumAic();
    auto aivNum = ascendcPlatform->GetCoreNumAiv();
    // ... 根据aicNum和aivNum自行设计切分策略
}
```
