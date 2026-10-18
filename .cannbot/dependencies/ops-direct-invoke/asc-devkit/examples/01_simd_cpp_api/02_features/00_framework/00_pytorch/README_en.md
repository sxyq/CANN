# PyTorch Sample Introduction

## Overview

This directory introduces different modes for integrating Ascend C custom operators into the PyTorch framework.

- Integration into PyTorch Eager mode
    - Register an operator into PyTorch through pybind11
    - Register an operator into PyTorch through torch.library
- Integration into PyTorch graph mode
    - Integrate an operator into TorchAir through GE

## Sample List

| Directory Name | Description | Supported Products |
| -------------- | ----------- | ------------------ |
| [pybind](./pybind) | This sample demonstrates how to register a custom operator using pybind11 | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [torch_library](./torch_library) | This sample demonstrates how to register a custom operator using PyTorch's torch.library mechanism | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [ge_torchair](./ge_torchair) | This sample demonstrates how to integrate an Ascend C operator into PyTorch graph mode | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
