# Features样例介绍

## 概述

介绍 Ascend C SIMD 特性样例，覆盖编译工程、算子调用、框架接入等场景，用于展示不同特性的典型实现方式和编译运行流程。

## 样例列表

| 目录名称  | 功能描述 |
| --------- | --------- |
| [00_framework](./00_framework) | 本样例介绍了PyTorch、TensorFlow、ONNX、GE和ACLGraph场景的自定义算子实现方法 |
| [02_tiling_selector](./02_tiling_selector) | 展示SIMD矢量计算场景下多核Tiling切分策略和Tiling参数选择方式。 |
| [04_compile](./04_compile) | 展示SIMD编译相关的特性样例。 |
| [05_aclrtc](./05_aclrtc) | 样例主要介绍使用Aclrtc（运行时编译）接口在Host侧编译字符串形式的Ascend C核函数，并完成编译结果的加载和执行的过程 |
| [06_aot_compilation](./06_aot_compilation) | 样例主要介绍基于AOT（Ahead-of-Time）编译的性能优化方法 |
| [99_acl_based](./99_acl_based) | 本样例介绍了自定义算子编译工程和Aclnn/Aclop算子调用的实现方法 |
