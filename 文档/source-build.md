# AddRmsNormBias 源码编译说明

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

1. `op_host` 的新/旧两套注册 API（gert:: vs ge::Operator）。
2. Tiling 读写宏（本实现使用 `GET_TILING_DATA_WITH_STRUCT`）需与 msopgen 生成工程保持一致。
3. `Muls` 标量类型需与操作数同型（本实现 FP32 链中为 float）。
4. `Cast` 的 RoundMode：fp16/bf16 输入 CAST_NONE；输出 CAST_RINT（可换 ROUND 版对照比较精度）。
5. DataCopyPad 参数结构体（DataCopyExtParams/DataCopyPadExtParams）字段顺序各 CANN 版本一致，但以安装头文件声明为准。

## 正确性红线

- 归约与中间计算必须 FP32；输出时一次性 Cast 回原类型。
- 尾块：搬入 DataCopyPad 补 0；搬出 DataCopyPad 非对齐（禁止用对齐 DataCopy 写越界）。
- 严禁写死输入/输出、Host 代算或绕过计算流程。
