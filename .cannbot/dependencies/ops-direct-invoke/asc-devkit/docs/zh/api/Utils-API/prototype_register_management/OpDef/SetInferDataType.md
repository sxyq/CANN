# SetInferDataType

## 功能说明

使用图模式时，需要调用该接口注册DataType推导函数。

## 函数原型

```
OpDef &SetInferDataType(gert::OpImplRegisterV2::InferDataTypeKernelFunc func)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| func | 输入 | DataType推导函数。InferDataTypeKernelFunc类型定义如下：
using InferDataTypeKernelFunc = UINT32 (*)(InferDataTypeContext *); |

## 返回值说明

OpDef算子定义，OpDef请参考[OpDef](OpDef.md)。

## 约束说明

无
