# FULL-R011 MANUAL VECTOR REDUCTION · I001

本目录是独立路线 I001 的 Ascend C Vector 直调工程。实现目标是沿最后一维执行
`x + residual`、RMS 归一化、`gamma` 缩放和 `bias` 加法，核心归约采用向量折半累加。

I001 按空 Kernel、tiling/Host、Kernel 核心逻辑三个提交阶段推进。当前第一阶段只验证
工程骨架和直调编译链；不包含本地运行、NPU 运行、精度、性能、profiling 或线上提交。

manual_vector_reduction 算子的 Kernel 直调实现示例，同时支持 PyTorch 对接。

详细代码说明见 `op_kernel/manual_vector_reduction_kernel.asc` 和 `op_kernel/manual_vector_reduction_tiling.h` 中的注释（搜索 `[MODIFY]` 标记）。

## 文件结构

```
├── op_kernel/
│   ├── manual_vector_reduction_tiling.h    Tiling 常量 + 结构体（kernel 和 host 共用）
│   └── manual_vector_reduction_kernel.asc  纯 kernel 代码（KernelManualVectorReduction 类 + manual_vector_reduction_kernel 核函数入口）
├── op_host/
│   ├── manual_vector_reduction.asc         Host + main 入口（#include "manual_vector_reduction_kernel.asc"）
│   └── data_utils.h           数据读写工具
├── op_extension/
│   ├── manual_vector_reduction_torch.cpp   PyTorch host 实现（Tiling 计算 + kernel launch）
│   ├── register.cpp           TORCH_LIBRARY 注册（含 Meta backend）
│   └── ops.h                  函数声明
├── CMakeLists.txt             双 target：可执行文件 + libmanual_vector_reduction_ops.so
└── scripts/                   数据生成和验证脚本
```

## 快速开始

### 方式一：直调验证（可执行文件）

```bash
source ${ASCEND_HOME_PATH}/set_env.sh
bash run.sh
```

或手动执行：

```bash
mkdir -p build && cd build && cmake .. && make -j
cd .. && python3 scripts/gen_data.py
cd build && ./manual_vector_reduction
python3 ../scripts/verify_result.py output/output.bin output/golden.bin
```

### 方式二：PyTorch 调用

```python
import torch
import torch_npu

torch.ops.load_library("build/libmanual_vector_reduction_ops.so")

x1 = torch.randn(8, 2048, dtype=torch.float32).npu()
x2 = torch.randn(8, 2048, dtype=torch.float32).npu()
y = torch.ops.npu.manual_vector_reduction(x1, x2)

assert torch.allclose(y.cpu(), x1.cpu() + x2.cpu())
```
