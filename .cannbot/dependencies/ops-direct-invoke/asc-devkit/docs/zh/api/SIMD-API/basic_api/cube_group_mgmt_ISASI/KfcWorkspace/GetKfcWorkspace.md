# GetKfcWorkspace

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：不支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：不支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/cube_group_mgmt_ISASI/KfcWorkspace/GetKfcWorkspace_res.md#id1 -->

## 功能说明

获取用于CubeResGroupHandle消息通信区的内存地址。用户使用[CubeResGroupHandle](../CubeResGroupHandle/CubeResGroupHandle_constructor.md)接口时，需要用此接口自主管理空间地址。

## 函数原型

```
__aicore__ inline GM_ADDR GetKfcWorkspace()
```

## 参数说明

无

## 返回值说明

workspace地址。

## 约束说明

本接口不能和[CreateCubeResGroup](../CubeResGroupHandle/CreateCubeResGroup.md)接口同时使用。

## 调用示例

```
AscendC::KfcWorkspace desc(workspaceGM);
GM_ADDR workspace = desc.GetKfcWorkspace();
```
