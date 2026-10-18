# GetStoreAtomicConfig\(ISASI\)

## 产品支持情况

<!-- npu="950" id8 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id8 -->
<!-- npu="A3" id9 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：支持
<!-- end id9 -->
<!-- npu="910b" id10 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id10 -->
<!-- npu="310b" id11 -->
- Atlas 200I/500 A2 推理产品：支持
<!-- end id11 -->
<!-- npu="310p" id12 -->
- Atlas 推理系列产品AI Core：不支持
<!-- end id12 -->
<!-- npu="310p" id13 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id13 -->
<!-- npu="910" id14 -->
- Atlas 训练系列产品：不支持
<!-- end id14 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/atomic_operations/GetStoreAtomicConfig_ISASI_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_operator_common_intf.h"`。

获取原子操作开启位与原子操作类型的值，详细说明见[表1](SetStoreAtomicConfig_ISASI.md#table622mcpsimp)。

## 函数原型

```cpp
__aicore__ inline void GetStoreAtomicConfig(uint16_t& atomicType, uint16_t& atomicOp)
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
|--------|-----------|------|
| atomicType | 输出 | 原子操作开启位。
0：无原子操作
1：开启原子操作，进行原子操作的数据类型为float
2：开启原子操作，进行原子操作的数据类型为half
3：开启原子操作，进行原子操作的数据类型为int16_t
4：开启原子操作，进行原子操作的数据类型为int32_t
5：开启原子操作，进行原子操作的数据类型为int8_t
6：开启原子操作，进行原子操作的数据类型为bfloat16_t |
| atomicOp | 输出 | 原子操作类型。
0：求和操作 |

## 返回值说明

无

## 约束说明

此接口需要与[SetStoreAtomicConfig\(ISASI\)](SetStoreAtomicConfig_ISASI.md)配合使用，用以获取原子操作开启位与原子操作类型的值。

## 调用示例

```cpp
AscendC::SetStoreAtomicConfig<AscendC::AtomicDtype::ATOMIC_F16, AscendC::AtomicOp::ATOMIC_SUM>();
uint16_t type = 0;       // 原子操作开启位
uint16_t op = 0;         // 原子操作类型
AscendC::GetStoreAtomicConfig(type, op);
```
