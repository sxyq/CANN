# Fusion Compute Practices样例介绍

## 概述

本目录提供融合计算相关的最佳实践样例，覆盖QuantGroupMatmul分组量化矩阵乘和Matmul+GELU的Cube-Vector融合实现。

## 样例列表

| 目录名称 | 功能描述 | 支持的产品 |
| ------------------------------------------------------------ | ---------------------------------------------------- | --- |
| [matmul_gelu_high_performance](./matmul_gelu_high_performance) |  本样例展示Cube-Vector融合的高性能实现，将Matmul矩阵乘法与GELU激活函数融合到同一AI Core中并行执行。 | Ascend 950PR/Ascend 950DT
Atlas A3 训练系列产品/Atlas A3 推理系列产品
Atlas A2 训练系列产品/Atlas A2 推理系列产品 |
| [quant_group_matmul_high_performance](./quant_group_matmul_high_performance) |  本样例介绍QuantGroupMatmul算子的高性能实现，支持per-token量化的分组矩阵乘法与GELU激活计算，并展示CV融合时Vector bound场景下的性能调优方法。 | Ascend 950PR/Ascend 950DT
Atlas A3 训练系列产品/Atlas A3 推理系列产品
Atlas A2 训练系列产品/Atlas A2 推理系列产品 |
