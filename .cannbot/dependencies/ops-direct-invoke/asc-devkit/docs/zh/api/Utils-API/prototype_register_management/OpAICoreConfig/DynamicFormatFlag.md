# DynamicFormatFlag

## 功能说明

标识是否根据[SetOpSelectFormat](../OpAICoreDef/SetOpSelectFormat.md)设置的函数自动推导算子输入输出支持的dtype和format。设置为“true”，则无需在原型注册时配置固定的dtype与format，会调用推导函数来推导算子输入输出支持的dtype和format。

## 函数原型

```
OpAICoreConfig &DynamicFormatFlag(bool flag)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| flag | 输入 | 标记是否自动推导算子输入输出的dtype和format。 |

## 返回值说明

OpAICoreConfig类，请参考[OpAICoreConfig](OpAICoreConfig.md)。

## 约束说明

无

## 调用示例

请参考[SetOpSelectFormat](../OpAICoreDef/SetOpSelectFormat.md)节调用示例。
