# SetOpSupportInfo

## 功能说明

注册算子切分信息函数。当算子的切分信息属于特殊类型，可以在算子实现文件里实现算子切分信息函数并注册，开发者需要自己按照一定的规则填写算子输入、输出的可以被切分的轴信息。

## 函数原型

```
OpAICoreDef &SetOpSupportInfo(optiling::OP_CHECK_FUNC func)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| func | 输入 | 算子切分信息函数。OP_CHECK_FUNC类型定义如下：
using OP_CHECK_FUNC = int32_t (*)(const ge::Operator &op, ge::AscendString &result); |

## 返回值说明

OpAICoreDef算子定义，OpAICoreDef请参考[OpAICoreDef](OpAICoreDef.md)。

## 约束说明

无
