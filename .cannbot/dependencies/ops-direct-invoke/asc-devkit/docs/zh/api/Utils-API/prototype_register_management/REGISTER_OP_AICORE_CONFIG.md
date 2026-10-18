# OpAICoreConfig注册接口（REGISTER\_OP\_AICORE\_CONFIG）

## 功能说明

不同的硬件形态算子原型定义不同的情况，可以通过新增[OpAICoreConfig](OpAICoreConfig/OpAICoreConfig.md)的方式，针对不同的AI处理器型号[注册差异化的算子原型](../../../guide/programming_guide/advanced_programming/aclnn_operator_development/design_and_implementation/operator_prototype_definition.md)。REGISTER\_OP\_AICORE\_CONFIG宏在不改变原有注册的基础上，允许单独新增文件来注册算子在不同硬件形态上的差异化信息。

使用该注册宏需要包含以下头文件：

```
#include "register/op_config_registry.h"
```

## 函数原型

```
REGISTER_OP_AICORE_CONFIG(opType, socVersion, opFunc)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| opType | 输入 | 算子类型。 |
| socVersion | 输入 | 支持的AI处理器型号。 |
| opFunc | 输入 | 返回OpAICoreConfig的回调函数指针，回调函数原型定义为：
OpAICoreConfig (*)() |

## 返回值说明

无

## 约束说明

若算子同时使用[AddConfig](OpAICoreDef/AddConfig.md)注册算子支持的AI处理器型号以及OpAICoreConfig信息，且AI处理器型号相同时，通过AddConfig方式注册的配置优先级更高，会覆盖REGISTER\_OP\_AICORE\_CONFIG宏注册的OpAICoreConfig信息。

## 调用示例

假设，已有原型注册文件op\_host/add\_custom.cpp实现如下，配置了算子支持的AI处理器型号ascendxxx1及算子输入输出原型信息：

```
...
namespace ops {
class AddCustom : public OpDef {
public:
    AddCustom(const char* name) : OpDef(name)
    {
        this->Input("x")
            .ParamType(REQUIRED)
            .DataType({ge::DT_FLOAT16, ge::DT_FLOAT, ge::DT_INT32})
            .Format({ge::FORMAT_ND, ge::FORMAT_ND, ge::FORMAT_ND});
        this->Input("y")
            .ParamType(REQUIRED)
            .DataType({ge::DT_FLOAT16, ge::DT_FLOAT, ge::DT_INT32})
            .Format({ge::FORMAT_ND, ge::FORMAT_ND, ge::FORMAT_ND});
        this->Output("z")
            .ParamType(REQUIRED)
            .DataType({ge::DT_FLOAT16, ge::DT_FLOAT, ge::DT_INT32})
            .Format({ge::FORMAT_ND, ge::FORMAT_ND, ge::FORMAT_ND});
        this->AICore()
            .SetTiling(optiling::TilingFunc);
        // 请替换为实际的AI处理器型号
        this->AICore().AddConfig("ascendxxx1");
    }
};
OP_ADD(AddCustom);
} // namespace ops
```

可新增文件op\_host/add\_custom\_xxx.cpp，使用REGISTER\_OP\_AICORE\_CONFIG单独注册算子支持的AI处理器型号ascendxxx2，示例如下：

```
#include "register/op_config_registry.h"
namespace ops {
REGISTER_OP_AICORE_CONFIG(AddCustom, ascendxxx2, []() {
    ops::OpAICoreConfig config("ascendxxx2");
    return config;
});
}
```
