# BestPractices Sample Introduction

## Overview
Performance optimization practices based on Ascend C, focusing on tuning key operators and memory access, aimed at improving execution efficiency on the Ascend platform.

## Sample List
| Directory Name | Description |
| --- | --- |
| [00_vector_compute](./00_vector_compute) | Optimization samples based on Vector Compute API usage, introducing performance tuning methods for static Tensor programming through the `<<<>>>` direct call implementation. |
| [01_matrix_compute](./01_matrix_compute) | Optimization samples based on Matrix Compute API usage, introducing performance tuning methods for the Matmul operator. |
| [02_reg_compute](./02_reg_compute) | Performance optimization samples based on VF functions, introducing VF loop optimization, VF instruction dual-issue optimization, VF continuous non-aligned scenario optimization, and VF fusion optimization through the `<<<>>>` direct call implementation. |
| [03_fusion_compute](./03_fusion_compute) | Performance optimization samples for fusion compute, introducing high-performance implementations of Cube-Vector fusion operators such as QuantGroupMatmul and Matmul+GELU. |
| [04_memory_access](./04_memory_access) | Optimization samples based on data copy API usage, introducing methods to reduce invalid data copy and reduce data copy instruction count through the <<<>>> direct call implementation. |
