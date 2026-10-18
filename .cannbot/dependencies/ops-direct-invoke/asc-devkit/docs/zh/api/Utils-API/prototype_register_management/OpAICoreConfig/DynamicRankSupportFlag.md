# DynamicRankSupportFlag

## 功能说明

标识算子是否支持dynamicRank（动态维度）。

## 函数原型

```
OpAICoreConfig &DynamicRankSupportFlag(bool flag)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| flag | 输入 | <br/>- true：表示算子支持dynamic rank，算子支持shape包含（-2），用于判断是否进行动态编译；<br/>- false：表示算子不支持dynamic rank。 |

## 返回值说明

OpAICoreConfig类，请参考[OpAICoreConfig](OpAICoreConfig.md)。

## 约束说明

无
