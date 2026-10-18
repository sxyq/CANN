# FreeMessage

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/cube_group_mgmt_ISASI/CubeResGroupHandle/FreeMessage_res.md#id1 -->

## 功能说明

在自定义的回调函数逻辑中，完成消息处理后，调用该接口，刷新消息状态为FREE；或者待消息状态为指定状态waitState时，刷新消息状态为FREE。消息状态的介绍可以参考[表2](CubeResGroupHandle_usage.md#table77221554135216)中的参数msgState。

## 函数原型

```
__aicore__ inline uint16_t FreeMessage(__gm__ CubeMsgType *msg);
__aicore__ inline uint16_t FreeMessage(__gm__ CubeMsgType *msg, CubeMsgState waitState);
```

## 参数说明

**表1**  接口参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| msg | 输入 | 该CubeResGroupHandle中的消息空间地址。 |
| waitState | 输入 | 需要等待的msgState。 |

## 返回值说明

当前消息空间与该消息队列队首空间的地址偏移。

## 约束说明

指定的消息状态waitState不能为QUIT和FREE。

## 调用示例

```
template <int32_t funcId>
__aicore__ inline static typename IsEqual<funcId, 1>::Type CubeGroupCallBack(
    MatmulApiCfg &mm, __gm__ CubeMsgBody *rcvMsg, CubeResGroupHandle<CubeMsgBody> &handle)
{
       // Cube核上计算逻辑，此处用户自行实现，在一切计算完毕后需要调用FreeMessage，代表rcvMsg已处理完。
       auto tmpId = handle.FreeMessage(rcvMsg);
};
```
