# FULL-R011 MANUAL VECTOR REDUCTION · I001

本目录是独立路线 I001 的 Ascend C Vector 直调工程。实现目标是沿最后一维执行
`x + residual`、RMS 归一化、`gamma` 缩放和 `bias` 加法，核心归约采用向量折半累加。

I001 按空 Kernel、tiling/Host、Kernel 核心逻辑三个阶段推进。Kernel 使用 FP32 输入和中间累加，
最后一维按 `TILE_LENGTH` 分块，两遍处理分别完成平方和归约与输出变换；归约主体是向量 `Add`
的折半累加，没有把 `ReduceSum` 作为实现路径。

本目录只完成 server3 CANN 9.0 编译。没有执行本地运行、NPU 正确性、CPU/精度、性能、profiling、
PyTorch 或线上提交。标量归约结果通过 `LocalTensor::GetValue(0)` 取出后继续完成平方根和归一化，
这是当前实现中需要后续性能评估的明确位置。

平台直接提交入口为同目录 `kernel.asc`。该文件自包含手工向量折半归约核心，并实现题目模板要求的
`extern "C" void run_kernel(...)`。在 server3 的实验构建目录中，模板文件与该 `kernel.asc` 放在同一目录，
使用 `cmake . && make -j4` 只编译，不执行生成的程序。
本目录的 `platform-build/CMakeLists.txt` 是同一提交入口的 server3 构建适配，补充了 GCC C++ 头文件目录；
在实验构建目录中复制该 CMake 文件后使用 `cmake . && make -j4`。

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
