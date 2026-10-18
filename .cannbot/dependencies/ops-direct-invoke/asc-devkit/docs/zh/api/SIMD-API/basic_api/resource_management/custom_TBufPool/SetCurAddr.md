# SetCurAddr

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/custom_TBufPool/SetCurAddr_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

设置自定义TBufPool已经被分配完的地址，比如初始化时一共申请了32k的内存大小，给某一个TQue分配了8K，则需要调用该接口以保证后续的内存块从8K开始分配。

## 函数原型

```cpp
__aicore__ inline void SetCurAddr(uint32_t curAddr)
```

**表1**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| curAddr | 输入 | 已分配的内存地址。 |

## 约束说明

无

## 返回值说明

无

## 调用示例

请参考[调用示例](EXTERN_IMPL_BUFPOOL.md#section1234017553610)。
