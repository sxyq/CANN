# DynamicShapeSupportFlag

## 功能说明

用于标识该算子是否支持入图时的动态Shape场景。

## 函数原型

```
OpAICoreConfig &DynamicShapeSupportFlag(bool flag)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| flag | 输入 | <br/>- true：表示算子支持入图时的动态Shape场景。<br/>- false：表示算子不支持入图时的动态Shape场景。 |

## 返回值说明

OpAICoreConfig类，请参考[OpAICoreConfig](OpAICoreConfig.md)。

## 约束说明

无
