# SetParamGeneralize

## 功能说明

注册算子泛化函数，包含每个参数shape的range。在图模式中，同一range下的shape共用二进制。

## 函数原型

```
OpAICoreDef& SetParamGeneralize(optiling::PARAM_GENERALIZE_FUNC func)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| func | 输入 | 算子参数的shape range。PARAM_GENERALIZE_FUNC类型定义如下：
using PARAM_GENERALIZE_FUNC = ge::graphStatus (*)(const ge::Operator &op, const ge::AscendString &generalize_config,
                                      ge::AscendString &generalized_op_params); |

## 返回值说明

OpAICoreDef算子定义，OpAICoreDef请参考[OpAICoreDef](OpAICoreDef.md)。

## 约束说明

无
