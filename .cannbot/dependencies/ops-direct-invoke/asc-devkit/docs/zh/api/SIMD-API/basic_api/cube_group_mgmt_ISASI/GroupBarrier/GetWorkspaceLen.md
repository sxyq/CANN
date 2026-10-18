# GetWorkspaceLen

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/cube_group_mgmt_ISASI/GroupBarrier/GetWorkspaceLen_res.md#id1 -->

## 功能说明

返回当前GroupBarrier所占用的Global Memory消息空间大小。

## 函数原型

```
__aicore__ inline uint64_t GetWorkspaceLen()
```

## 参数说明

无

## 返回值说明

当前GroupBarrier所占用的Global Memory消息空间大小。

## 约束说明

无

## 调用示例

```
// 6个AIV等3个AIV Arrive后再开始后续业务，总共需要6*512B地址空间，起始地址为用户指定的startAddr。
AscendC::GroupBarrier<AscendC::PipeMode::MTE3_MODE> barA(startAddr, 3, 6);
uint64_t offset = barA.GetWorkspaceLen(); // 返回barA所占用的GlobalMemory空间。
```

结果示例如下：

```
占用的GlobalMemory空间(offset)的大小为: 3072
```
