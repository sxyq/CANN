# Tiling Selector样例介绍

## 概述

本目录介绍Ascend C SIMD Tiling Selector特性样例，展示根据输入数据量和AI Core数量计算Tiling参数，并选择核间和核内切分策略的典型流程。

## 算子开发样例

| 目录名称 | 功能描述 |
| -------- | -------- |
| [vector_tiling_strategy](./vector_tiling_strategy) | 以两个一维Tensor相加为例，展示矢量计算场景下多核Tiling切分策略的计算与选择流程。 |
