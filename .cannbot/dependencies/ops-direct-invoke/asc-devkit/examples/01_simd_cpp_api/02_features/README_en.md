# Features Sample Introduction

## Overview

This document introduces Ascend C SIMD feature samples, covering compilation projects, operator invocation, framework integration and other scenarios, demonstrating typical implementation methods and build/run flows for different features.

## Sample List

| Directory Name | Description |
| -------------- | ----------- |
| [00_framework](./00_framework) | This sample introduces custom operator implementation methods for PyTorch, TensorFlow, ONNX, GE, and ACLGraph scenarios |
| [02_tiling_selector](./02_tiling_selector) | Demonstrates multi-core Tiling split strategies and Tiling parameter selection for SIMD vector computing. |
| [04_compile](./04_compile) | Samples demonstrating SIMD compile-related features. |
| [05_aclrtc](./05_aclrtc) | This sample demonstrates using Aclrtc (runtime compilation) interfaces to compile Ascend C kernel functions in string form on the Host side and complete the loading and execution of compilation results |
| [06_aot_compilation](./06_aot_compilation) | This sample introduces AOT (Ahead-of-Time) compilation optimization to achieve performance improvements |
| [99_acl_based](./99_acl_based) | This sample introduces the implementation of custom operator compilation projects and Aclnn/Aclop operator invocation |
