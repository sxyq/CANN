# AddRmsNormBias 源码编译说明

## A. 平台直调模板（首版候选）

本题提供的 `addrmsnormbias_problem_1742_template` 是一个直接调用 `run_kernel` 的本地模板。平台当前唯一明确的可编辑文件是模板根目录下的 `kernel.asc`；首版候选位于 `提交/首版/kernel.asc`。

在有 CANN 9.0.0 和昇腾 NPU 的机器上，按下面流程运行：

```bash
cp /path/to/cann/提交/首版/kernel.asc /path/to/addrmsnormbias_problem_1742_template/kernel.asc
cd /path/to/addrmsnormbias_problem_1742_template
source /usr/local/Ascend/ascend-toolkit/set_env.sh
./run.sh
```

`run.sh` 会使用模板的 `CMakeLists.txt` 编译 `main.asc`，再生成样例输入并执行 `run_kernel`。模板默认只验证 FP16、形状 `[1, 64]`；真机阶段需要另行覆盖 FP32、BF16、2D/3D/4D、跨行非对齐 D 和平台真实测试点。

本机没有 `ASCEND_HOME_PATH`、Ascend C 编译器、`kernel_operator.h` 或 NPU，因此本节命令没有在本机执行。不能把普通 C++ 编译或 CPU 参考计算当成 Ascend C 构建结果。

## 文件

| 文件 | 说明 |
| --- | --- |
| `源码/add_rms_norm_bias.json` | msopgen 算子原型定义（fp16/bf16/fp32；attr epsilon） |
| `源码/op_kernel/add_rms_norm_bias.cpp` | Ascend C 核函数（核心计算全部在 NPU） |
| `源码/op_host/add_rms_norm_bias_tiling.h` | Tiling 数据结构定义 |
| `源码/op_host/add_rms_norm_bias.cpp` | 原型注册 / InferShape / Tiling 下发 |
| `源码/CMakePresets.json` / `源码/CMakeLists.txt` / `源码/build.sh` | 真机编译入口说明（真机以 msopgen 生成版为准） |

## 生成与编译（真机，CANN 9.0.0）

```bash
# 1) 生成工程骨架（判题 SoC 确认后替换 -c 值）
msopgen gen -i 源码/add_rms_norm_bias.json -c ai_core-<soc> -lang cpp -out /tmp/add_rms_norm_bias_gen

# 2) 用本目录实现覆盖骨架
cp 源码/op_kernel/add_rms_norm_bias.cpp     /tmp/add_rms_norm_bias_gen/op_kernel/
cp 源码/op_host/add_rms_norm_bias_tiling.h  /tmp/add_rms_norm_bias_gen/op_host/
cp 源码/op_host/add_rms_norm_bias.cpp       /tmp/add_rms_norm_bias_gen/op_host/

# 3) 编译（使用本目录脚本时传入已生成工程）
ASCENDC_GENERATED_PROJECT_DIR=/tmp/add_rms_norm_bias_gen ./源码/build.sh
```

> 本机没有 msopgen、CANN 或 NPU；上述步骤当前只能作为真机操作说明，不能在本机完成。
> 本目录的 CMakeLists.txt 只用于源码清单配置，不会生成算子库或提交包。

## 已知版本敏感点（真机落地时逐项核对）

1. `kernel.asc` 直调模板和 msopgen 算子工程是两条不同入口，不能把 `run_kernel` 文件直接当成 `op_kernel` 的 tiling 工程入口。
2. `Cast` 的 RoundMode：fp16/bf16 输入使用 `CAST_NONE`；输出使用 `CAST_RINT`，需要在 CANN 9.0.0 真机用非整数值复核舍入行为。
3. `DataCopyPad` 的模板、产品支持矩阵和 `DataCopyExtParams` 以目标机 `kernel_operator.h` 及对应版本手册为准。
4. `ReduceSum` 的 `workLocal` 容量、V-S/S-V 同步和 `GetValue(0)` 结果读取需要目标版本编译与运行确认。
5. 官方 CANN 9.0.X API 文档入口为
   <https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/>；本机未安装 CANN 9.0.0，无法从本机头文件确认最终签名。

## 正确性红线

- 归约与中间计算必须 FP32；输出时一次性 Cast 回原类型。
- 尾块：搬入 DataCopyPad 补 0；搬出 DataCopyPad 只写真实字节数，不能用对齐 DataCopy 越过行尾。
- 严禁写死输入/输出、Host 代算或绕过计算流程。
