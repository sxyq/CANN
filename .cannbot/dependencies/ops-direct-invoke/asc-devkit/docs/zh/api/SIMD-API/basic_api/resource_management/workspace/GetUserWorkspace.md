# GetUserWorkspace

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/workspace/GetUserWorkspace_res.md#id1 -->

## 功能说明

获取用户使用的workspace指针。核函数（Kernel）直调开发方式下，如果未开启HAVE\_WORKSPACE编译选项，框架不会自动设置系统workspace。如果使用了[Matmul核函数（Kernel）侧接口](../../../adv_api/cube_compute/Matmul_Kernel/Matmul_Kernel.md)等需要系统workspace的高阶API，kernel侧需要通过[SetSysWorkSpace](SetSysWorkSpace.md)设置系统workspace，此时用户workspace需要通过该接口获取。

## 函数原型

```
__aicore__ inline GM_ADDR GetUserWorkspace(GM_ADDR workspace)
```

## 参数说明

**表1**  接口参数说明

| 参数名称 | 输入/输出 | 描述 |
| --- | --- | --- |
| workspace | 输入 | 传入workspace的指针，包括系统workspace和用户使用的workspace。 |

## 约束说明

无

## 返回值说明

用户使用workspace指针。
