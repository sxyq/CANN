# ToFile

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/LocalTensor/ToFile_res.md#id1 -->

## 功能说明

只限于CPU调试，将LocalTensor数据Dump到文件中，用于精度调试，文件保存在执行目录。

## 函数原型

```cpp
int32_t ToFile(const std::string& fileName) const
```

## 参数说明

**表1**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| fileName | 输入 | 保存的文件名称。 |

## 返回值说明

返回0表示数据Dump成功，非0值表示失败。

## 约束说明

无

## 调用示例

```cpp
// 示例只限于CPU调试，将LocalTensor数据Dump到文件中，用于精度调试，文件保存在执行目录
AscendC::LocalTensor<float> tmpTensor = softmaxMaxBuf.template Get<float>();
tmpTensor.ToFile("tmpTensor.bin");
```
