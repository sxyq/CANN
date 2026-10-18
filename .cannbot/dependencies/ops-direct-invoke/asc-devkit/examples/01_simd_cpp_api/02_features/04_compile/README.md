# Compile样例介绍

## 概述

基于Ascend C SIMD的编译样例，展示SIMD编程的各类编译模式。

## 算子开发样例

| 目录名称                                                | 功能描述                   |
|-----------------------------------------------------|------------------------|
| [00_basic_compile](./00_basic_compile) | 展示Host侧.cpp与Device侧.asc混合文件的基本异构编译流程。 |
| [01_separate_compile](./01_separate_compile) | 展示将Device侧代码拆分到多个.asc文件中，通过单独编译实现跨文件设备函数调用的流程。 |
| [02_dynamic_library_compile](./02_dynamic_library_compile) | 展示将Device侧.asc编译为动态共享库（.so）的编译流程。 |
| [03_static_library_compile](./03_static_library_compile) | 展示将Device侧.asc编译为静态库（.a）的编译流程。 |
