# Arrive

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/cube_group_mgmt_ISASI/GroupBarrier/Arrive_res.md#id1 -->

## 功能说明

通知其他等待的AIV，本AIV已经完成其依赖的任务。

## 函数原型

```
__aicore__ inline void Arrive(uint32_t arriveIndex)
```

## 参数说明

**表1**  接口参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| arriveIndex | 输入 | 该AIV在Arrive组的序号。范围为[0, arriveSize - 1]。 |

## 返回值说明

无。

## 约束说明

该接口支持在循环中使用，但是受限于多核间通信效率要求，循环最大次数不超过1,048,575次。

## 调用示例

```
if (id >= 0 && id < ARRIVE_NUM) {
  //各种Vector计算逻辑，用户自行实现
  barA.Arrive(id);  // Arrive组中有2个AIV，分别为Block0、1，表示它们已完成任务。
}
```
