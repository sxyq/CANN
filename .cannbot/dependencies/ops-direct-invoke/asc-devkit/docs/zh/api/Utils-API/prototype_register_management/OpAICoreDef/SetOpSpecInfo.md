# SetOpSpecInfo

## 功能说明

注册算子额外信息供FE解析。

## 函数原型

```
OpAICoreDef &SetOpSpecInfo(optiling::OP_CHECK_FUNC func)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| func | 输入 | 记录算子额外信息。OP_CHECK_FUNC类型定义如下：
using OP_CHECK_FUNC = int32_t (*)(const ge::Operator &op, ge::AscendString &result); |

## 返回值说明

OpAICoreDef算子定义，OpAICoreDef请参考[OpAICoreDef](OpAICoreDef.md)。

## 约束说明

无
