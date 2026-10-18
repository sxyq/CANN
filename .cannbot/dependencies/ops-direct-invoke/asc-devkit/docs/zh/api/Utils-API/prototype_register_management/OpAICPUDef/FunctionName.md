# FunctionName

## 功能说明

配置自定义算子调用的kernel函数接口名称，functionName固定为“RunCpuKernel”。

## 函数原型

```
OpAICPUDef &FunctionName(const char *value)
```

## 参数说明

| 参数 | 输入/输出 | 说明 |
| --- | --- | --- |
| value | 输入 | 算子调用的kernel函数接口名称，functionName固定为“RunCpuKernel”。 |

## 返回值说明

OpAICPUDef算子定义，OpAICPUDef请参考[OpAICPUDef](OpAICPUDef.md)。

## 约束说明

无
