# Engine

## 功能说明

配置算子调用的引擎，入参固定为“DNN\_VM\_HOST\_CPU”。

## 函数原型

```
OpHostCPUDef &Engine(const char *value)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| value | 输入 | 配置算子调用的引擎名称。 |

## 返回值说明

OpHostCPUDef算子定义，OpHostCPUDef请参考[OpHostCPUDef](OpHostCPUDef.md)。

## 约束说明

无
