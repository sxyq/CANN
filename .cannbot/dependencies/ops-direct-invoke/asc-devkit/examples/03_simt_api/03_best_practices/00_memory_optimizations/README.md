# Memory Optimizations样例介绍

## 概述

访存优化相关样例，通过<<<>>>直调的实现方式，介绍SIMT编程方式下的访存调优思路。涵盖全局访存合并、UB等片上缓存中转复用、Bank冲突规避，以及基于短向量half2提升访存带宽利用率等优化方法。

## 样例列表

| 目录名称                                               | 功能描述                                                                                        |
| ------------------------------------------------------ | ----------------------------------------------------------------------------------------------- |
| [matrix_transpose_practice](./matrix_transpose_practice)   | 样例以矩阵转置为例，从直接索引转置出发，逐步引入UB中转、全局访存合并、Thread Block映射与线程数选择、通过padding降低UB Bank冲突以及双缓冲去除循环尾部同步等优化方法，展示SIMT编程方式下矩阵转置的访存调优路径。 |
| [short_vector_add](./short_vector_add)            | 样例以向量加法为例，对比基础half类型逐元素加法与half2短向量化加法，演示SIMT编程方式下基于短向量提升访存带宽利用率的调优方法。 |
| [cache_hint](./cache_hint)   | 样例以查表法计算sin值为例，通过为不同类型数据指定缓存策略，使频繁访问的sin查找表常驻DCache、减少Global Memory访问次数，展示SIMT编程方式下数据缓存优化的调优路径。 |
| [aligned_types](./aligned_types) | 样例基于Ascend C SIMT编程方式，展示结构体类型对齐对Global Memory访存吞吐的影响。 |
