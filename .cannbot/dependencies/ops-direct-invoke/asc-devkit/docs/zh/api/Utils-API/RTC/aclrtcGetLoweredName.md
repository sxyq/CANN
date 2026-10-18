# aclrtcGetLoweredName

## 产品支持情况

<!-- npu="950" id3 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id3 -->
<!-- npu="A3" id4 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：支持
<!-- end id4 -->
<!-- npu="910b" id5 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id5 -->
<!-- npu="310b" id6 -->
- Atlas 200I/500 A2 推理产品：不支持
<!-- end id6 -->
<!-- npu="310p" id7 -->
- Atlas 推理系列产品AI Core：不支持
<!-- end id7 -->
<!-- npu="310p" id8 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id8 -->
<!-- npu="910" id9 -->
- Atlas 训练系列产品：不支持
<!-- end id9 -->

## 功能说明

获取已经注册的\_\_global\_\_函数的mangling name。

## 函数原型

```
aclError aclrtcGetLoweredName(aclrtcProg prog, const char *nameExpression, const char **loweredName)
```

## 参数说明

**表1**  接口参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| prog | 输入 | 运行时编译程序的句柄。 |
| nameExpression | 输入 | 一个常量表达式，通常为字符串字面量，用于指明某个__global__函数。 |
| loweredName | 输出 | 与所传入名称表达式对应的底层mangling name；该名称所占用的内存将在通过aclrtcDestroyProg销毁程序对象时自动释放。 |

## 返回值说明

aclError为int类型变量，详细说明请参考[RTC错误码](RTC_error_code.md)。

## 约束说明

使用前必须先通过[aclrtcAddNameExpr](aclrtcAddNameExpr.md)注册原始名称，且两次传入的nameExpression字符串必须完全相同。

## 调用示例

```
aclrtcProg prog;
aclrtcCreateProg(&prog, src, "add_custom", 0, nullptr, nullptr);

const char* kernelNameExpr = "add_custom<float>";
aclrtcAddNameExpr(prog, kernelNameExpr);

const char *options[] = {
    "--npu-arch=dav-2201",
};
int numOptions = sizeof(options) / sizeof(options[0]);
aclrtcCompileProg(prog, numOptions, options);

const char* manglingName = "";
aclrtcGetLoweredName(prog, kernelNameExpr, &manglingName);
```
