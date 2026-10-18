# 构造函数与析构函数

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/cube_group_mgmt_ISASI/KfcWorkspace/constructor_and_destructor_res.md#id1 -->

## 功能说明

用于创建KfcWorkspace对象，Kfc全称为kernel function call，表示核间通信调用。

## 函数原型

```
class KfcWorkspace;
__aicore__ inline KfcWorkspace(GM_ADDR workspace)
__aicore__ inline ~KfcWorkspace()
```

## 参数说明

**表1**  KfcWorkspace构造函数参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| workspace | 输入 | Global Memory上的消息空间地址，用户需保证地址对齐和清零。 |

## 返回值说明

KfcWorkspace对象实例。

## 约束说明

不能和[REGIST\_MATMUL\_OBJ](../../../adv_api/cube_compute/Matmul_Kernel/REGIST_MATMUL_OBJ.md)接口同时使用。使用资源管理API时，用户自主管理AIC和AIV的核间通信，REGIST\_MATMUL\_OBJ内部是由框架管理AIC和AIV的核间通信，同时使用可能会导致通信消息错误等异常。

## 调用示例

```
AscendC::KfcWorkspace desc(workspaceGM);
```
