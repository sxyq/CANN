# ReleaseEventID

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
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- npu="x90" id8 -->
- Kirin X90：支持
<!-- end id8 -->
<!-- npu="9030" id9 -->
- Kirin 9030：支持
<!-- end id9 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/ReleaseEventID_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

用于释放HardEvent（硬件类型同步事件）的TEventID，通常与[AllocEventID](AllocEventID.md)搭配使用。

## 函数原型

```cpp
template <HardEvent evt>
__aicore__ inline void ReleaseEventID(TEventID id)
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| evt | HardEvent硬件同步类型。该类型的具体说明请参考SetFlag/WaitFlag(ISASI)中同步类型的说明。 |

**表2**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| id | 输入 | TEventID类型，调用AllocEventID申请获得的TEventID。 |

## 约束说明

AllocEventID、ReleaseEventID需成对出现，ReleaseEventID传入的TEventID需由对应的AllocEventID申请而来。

## 返回值说明

无

## 调用示例

```cpp
AscendC::TEventID eventID = GetTPipePtr()->AllocEventID<AscendC::HardEvent::V_S>(); //需要插入scalar等vector的同步，申请对应的HardEvent的ID
AscendC::SetFlag<AscendC::HardEvent::V_S>(eventID);
......
......
......
AscendC::WaitFlag<AscendC::HardEvent::V_S>(eventID);
GetTPipePtr()->ReleaseEventID<AscendC::HardEvent::V_S>(eventID); //释放scalar等vector的同步HardEvent的ID
......
```
