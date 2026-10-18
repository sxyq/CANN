# AutoContiguous

## 功能说明

配置该参数，当单算子API（aclnnxxx）接口中的输入（aclTensor类型）是非连续tensor时，框架会自动将其转换为连续tensor。

**该接口仅在如下场景支持：**

-   通过单算子API执行的方式开发单算子调用应用。
-   间接调用单算子API\(aclnnxxx\)接口：Pytorch框架单算子直调的场景。

## 函数原型

```
OpParamDef &AutoContiguous()
```

## 参数说明

无

## 返回值说明

OpParamDef算子定义，OpParamDef请参考[OpParamDef](OpParamDef.md)。

## 约束说明

当一个Input调用了AutoContigous\(\)后，其他未调用IgnoreContiguous\(\)的Input也会在框架中默认进行非连续tensor转连续tensor的处理。
