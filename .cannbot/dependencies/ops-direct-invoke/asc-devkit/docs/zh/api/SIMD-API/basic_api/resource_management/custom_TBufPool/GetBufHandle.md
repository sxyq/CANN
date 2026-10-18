# GetBufHandle

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
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/custom_TBufPool/GetBufHandle_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

根据offset取出自定义TBufPool中管理的内存块。

## 函数原型

```cpp
__aicore__ inline TBufHandle GetBufHandle(uint8_t offset)
```

**表1**  参数说明

| 参数名称 | 输入/输出 | 含义 |
| --- | --- | --- |
| offset | 输入 | 内存块的index值 |

## 约束说明

offset值不可超出EXTERN\_IMPL\_BUFPOOL中定义的BUFID\_SIZE大小，用户可根据需要添加校验。

## 返回值说明

指定的内存块，类型为TBufHandle。

## 调用示例

请参考[调用示例](EXTERN_IMPL_BUFPOOL.md#section1234017553610)。
