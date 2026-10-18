# PyTorch样例介绍
## 概述
本目录介绍Ascend C自定义算子接入PyTorch框架的不同模式。
- 接入PyTorch Eager模式
    - 算子通过pybind11注册接入到PyTorch
    - 算子通过torch.library注册接入到PyTorch
- 接入PyTorch图模式
    - 算子通过GE接入到torchair

## 样例列表
| 目录名称 | 功能描述 | 支持的产品 |
| ------------------------------------------------------------ | ---------------------------------------------------- | --- |
| [pybind](./pybind) |  本样例展示如何使用pybind11注册自定义算子 | Ascend 950PR/Ascend 950DT
Atlas A3 训练系列产品/Atlas A3 推理系列产品
Atlas A2 训练系列产品/Atlas A2 推理系列产品 |
| [torch_library](./torch_library) |  本样例展示如何使用PyTorch的torch.library机制注册自定义算子 | Ascend 950PR/Ascend 950DT
Atlas A3 训练系列产品/Atlas A3 推理系列产品
Atlas A2 训练系列产品/Atlas A2 推理系列产品 |
| [ge_torchair](./ge_torchair) | 本样例展示Ascend C算子接入PyTorch图模式 | Ascend 950PR/Ascend 950DT
Atlas A3 训练系列产品/Atlas A3 推理系列产品
Atlas A2 训练系列产品/Atlas A2 推理系列产品 |
