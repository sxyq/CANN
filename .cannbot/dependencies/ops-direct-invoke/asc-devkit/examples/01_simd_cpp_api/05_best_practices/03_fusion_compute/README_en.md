# Fusion Compute Practices Sample Introduction

## Overview

This directory provides best-practice samples for fusion compute, covering QuantGroupMatmul grouped quantized matrix multiplication and Cube-Vector fusion for Matmul+GELU.

## Sample List

| Directory Name | Description | Supported Products |
| ------------------------------------------------------------ | ---------------------------------------------------- | --- |
| [matmul_gelu_high_performance](./matmul_gelu_high_performance) |  This sample demonstrates a high-performance Cube-Vector fusion implementation that fuses Matmul matrix multiplication and the GELU activation function in the same AI Core for parallel execution. | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [quant_group_matmul_high_performance](./quant_group_matmul_high_performance) |  This sample introduces the high-performance implementation of the QuantGroupMatmul operator, supports per-token quantized grouped matrix multiplication and GELU activation computation, and demonstrates performance tuning for Vector-bound scenarios in CV fusion. | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
