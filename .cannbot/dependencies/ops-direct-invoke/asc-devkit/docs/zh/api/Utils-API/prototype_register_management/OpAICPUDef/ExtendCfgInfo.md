# ExtendCfgInfo

## 功能说明

为AICPU自定义算子额外添加一组键值对形式的扩展配置。

## 函数原型

```
OpAICPUDef &ExtendCfgInfo(const char *key, const char *value)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| key | 输入 | 扩展配置的“键名”，配置项的名称。 |
| value | 输入 | 扩展配置的“值”，对应键名的配置内容。 |

## 返回值说明

OpAICPUDef算子定义，OpAICPUDef请参考[OpAICPUDef](OpAICPUDef.md)。

## 约束说明

无
