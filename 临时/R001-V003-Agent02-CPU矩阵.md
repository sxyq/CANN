# R024 CPU 验证矩阵

> 本报告由独立 CPU 参考链生成。所有结果来自本机 Python/numpy/ml_dtypes，不能当作 CANN 编译、NPU 精度或平台结果。

## 结论

- 主矩阵完成 324 个用例，覆盖 3 种 dtype、2D/3D/4D、全部指定 D，以及 `outer=1/4/8192`；大矩阵按块流式处理，未一次性保存全量张量。
- 主矩阵 R017（FP32 中间、末次转换）严格逐元素通过 319/324；最大绝对误差为 1.562e-02，最大稳健相对误差为 5.960e-02。BF16 有 5 个分块归约用例落在输出舍入边界，最高失配比例仍为 3.052e-05。
- R004 低精度中间路径在专门反例中出现明显失配：FP16 大值平方溢出，BF16 大 D 低精度逐步累加也超过本轮容差；不进入默认计算链。
- R024 结论：CPU 数学链、尾块 D、形状展平和 gamma/bias 末维广播均已得到参考级证据；仍缺少真实 Kernel 输出，不能替代服务器 CANN/NPU 验证。

## 运行信息

- 运行时间：`2026-09-15T05:06:56`
- 命令：`python3 /Users/sunyiyang/Desktop/Project/cann/临时/R001-V003-Agent02/cpu_matrix.py`
- Python：`3.9.6`；numpy：`2.0.2`；ml_dtypes：`0.5.4`
- 随机种子：`1742`；参考块长：`4096`；流式行块上限：按 `min(32, 1048576 // D)` 计算。
- 本次是否为快速子集：`否`。正式结果应使用不带 `--quick` 的命令。

## 参考链

```text
x/residual/gamma/bias 先以目标 dtype 生成
        -> 转 FP32
y = x + residual
rms = sqrt(sum(y*y) / D + epsilon)
out = y / rms * gamma + bias
        -> 只在输出处转换回目标 dtype
```
`golden_direct` 使用直接 FP32 求和；`candidate_r017` 使用固定 `4096` 元素块在 FP32 中累加，再执行相同归一化和广播。两者比较仅表示 CPU 参考链内部的一致性，不表示 NPU 输出误差。

## 容差口径

逐元素条件为 `abs(actual - golden) <= atol + rtol * abs(golden)`，同时记录失配比例。相对误差使用 `abs(actual-golden) / max(abs(actual), abs(golden), 1e-6)`，避免零值分母放大数值。

| dtype | rtol | atol | ratio |
| --- | --- | --- | --- |
| fp32 | 1.000e-04 | 1.000e-04 | 1.000e-03 |
| fp16 | 1.000e-03 | 1.000e-03 | 1.000e-03 |
| bf16 | 1.000e-03 | 1.000e-03 | 1.000e-03 |

`strict_pass` 要求所有有限元素满足逐元素容差；`ratio_pass` 只要求失配比例不超过 0.1%，用于和历史矩阵保持同一统计口径。

## 覆盖范围

| 项目 | 覆盖 |
| --- | --- |
| dtype | fp32 / fp16 / bf16 |
| rank | 2D / 3D / 4D；3D=(1, outer, D)，4D=(1, 1, outer, D) |
| D | 1, 31, 32, 33, 67, 127, 128, 129, 1000, 4096, 8192, 32768 |
| outer | one=1 / small=4 / large=8192 |
| epsilon | 0 / 1e-8 / 1e-5 / 1e-3（辅助矩阵，D=1/67/4096） |
| gamma/bias | (D,) 逐通道向量广播；随机和 channel_ramp 两种 |
| 特殊值 | 全零、NaN、Inf；另有 FP16 大值和 BF16 大 D 反例 |

主矩阵共 324 个用例，每个指定 D 都有 3 dtype × 3 rank × 3 outer 组合；每个用例处理元素数为 `outer × D`。

## 主矩阵结果

按 dtype 汇总：

| dtype | cases | strict_passes | ratio_passes | max_abs_error | max_relative_error | max_mismatch_ratio | total_elements |
| --- | --- | --- | --- | --- | --- | --- | --- |
| bf16 | 108 | 103 | 108 | 1.562e-02 | 1.075e-02 | 3.052e-05 | 1146038964 |
| fp16 | 108 | 108 | 108 | 1.953e-03 | 5.960e-02 | 0.000e+00 | 1146038964 |
| fp32 | 108 | 108 | 108 | 9.537e-07 | 1.490e-02 | 0.000e+00 | 1146038964 |

按 rank 汇总：

| rank | cases | strict_passes | ratio_passes | max_abs_error | max_relative_error | max_mismatch_ratio | total_elements |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 108 | 106 | 108 | 1.562e-02 | 5.960e-02 | 3.052e-05 | 1146038964 |
| 3 | 108 | 106 | 108 | 1.562e-02 | 5.960e-02 | 3.052e-05 | 1146038964 |
| 4 | 108 | 107 | 108 | 1.562e-02 | 5.960e-02 | 2.686e-06 | 1146038964 |

按 dtype × rank × outer 汇总：

| dtype | rank | outer_label | cases | strict_passes | max_abs_error | max_relative_error | max_mismatch_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- |
| bf16 | 2 | large | 12 | 11 | 1.562e-02 | 1.075e-02 | 2.578e-06 |
| bf16 | 2 | one | 12 | 11 | 3.906e-03 | 5.405e-03 | 3.052e-05 |
| bf16 | 2 | small | 12 | 12 | 9.766e-04 | 6.289e-03 | 0.000e+00 |
| bf16 | 3 | large | 12 | 11 | 1.562e-02 | 7.968e-03 | 2.764e-06 |
| bf16 | 3 | one | 12 | 11 | 7.812e-03 | 7.463e-03 | 3.052e-05 |
| bf16 | 3 | small | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| bf16 | 4 | large | 12 | 11 | 1.562e-02 | 7.752e-03 | 2.686e-06 |
| bf16 | 4 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| bf16 | 4 | small | 12 | 12 | 7.629e-06 | 5.208e-03 | 0.000e+00 |
| fp16 | 2 | large | 12 | 12 | 1.953e-03 | 5.960e-02 | 0.000e+00 |
| fp16 | 2 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp16 | 2 | small | 12 | 12 | 9.766e-04 | 9.166e-04 | 0.000e+00 |
| fp16 | 3 | large | 12 | 12 | 1.953e-03 | 5.960e-02 | 0.000e+00 |
| fp16 | 3 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp16 | 3 | small | 12 | 12 | 9.766e-04 | 7.163e-04 | 0.000e+00 |
| fp16 | 4 | large | 12 | 12 | 1.953e-03 | 5.960e-02 | 0.000e+00 |
| fp16 | 4 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp16 | 4 | small | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp32 | 2 | large | 12 | 12 | 9.537e-07 | 7.451e-03 | 0.000e+00 |
| fp32 | 2 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp32 | 2 | small | 12 | 12 | 4.768e-07 | 1.100e-04 | 0.000e+00 |
| fp32 | 3 | large | 12 | 12 | 9.537e-07 | 1.490e-02 | 0.000e+00 |
| fp32 | 3 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp32 | 3 | small | 12 | 12 | 4.768e-07 | 5.255e-05 | 0.000e+00 |
| fp32 | 4 | large | 12 | 12 | 9.537e-07 | 7.451e-03 | 0.000e+00 |
| fp32 | 4 | one | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| fp32 | 4 | small | 12 | 12 | 0.000e+00 | 0.000e+00 | 0.000e+00 |

最坏主矩阵用例：

| dtype | rank | D | outer | max_abs | max_rel | mismatch | strict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| bf16 | 2 | 32768 | large | 1.562e-02 | 1.075e-02 | 2.578e-06 | 否 |
| bf16 | 3 | 32768 | large | 1.562e-02 | 7.968e-03 | 2.764e-06 | 否 |
| bf16 | 4 | 32768 | large | 1.562e-02 | 7.752e-03 | 2.686e-06 | 否 |
| bf16 | 3 | 32768 | one | 7.812e-03 | 7.463e-03 | 3.052e-05 | 否 |
| bf16 | 2 | 32768 | one | 3.906e-03 | 5.405e-03 | 3.052e-05 | 否 |
| fp16 | 2 | 32768 | large | 1.953e-03 | 5.960e-02 | 0.000e+00 | 是 |
| fp16 | 3 | 32768 | large | 1.953e-03 | 5.960e-02 | 0.000e+00 | 是 |
| fp16 | 4 | 32768 | large | 1.953e-03 | 5.960e-02 | 0.000e+00 | 是 |

主矩阵的 `actual` 与 `golden` 都使用 FP32 中间，差异只来自直接求和与 `4096` 元素分块累加的顺序；输出仍在目标 dtype 末次转换。

## epsilon 与广播

| section | dtype | cases | strict_passes | ratio_passes | max_abs_error | max_relative_error | max_mismatch_ratio |
| --- | --- | --- | --- | --- | --- | --- | --- |
| broadcast_matrix | bf16 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| broadcast_matrix | fp16 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| broadcast_matrix | fp32 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| epsilon_matrix | bf16 | 36 | 36 | 36 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| epsilon_matrix | fp16 | 36 | 36 | 36 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| epsilon_matrix | fp32 | 36 | 36 | 36 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| nan_inf_matrix | bf16 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| nan_inf_matrix | fp16 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| nan_inf_matrix | fp32 | 3 | 3 | 3 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| zero_matrix | bf16 | 6 | 6 | 6 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| zero_matrix | fp16 | 6 | 6 | 6 | 0.000e+00 | 0.000e+00 | 0.000e+00 |
| zero_matrix | fp32 | 6 | 6 | 6 | 0.000e+00 | 0.000e+00 | 0.000e+00 |

辅助矩阵中的 `channel_ramp` 让每个通道的 gamma/bias 不同，再在 2D/3D/4D 中复用同一 `(D,)` 向量；这验证了最后一维广播的参考语义。全零用例还验证了 `epsilon=0` 和默认 epsilon 下的输出传播。

## R004 低精度反例

下列候选在每个算术阶段都量化回 FP16/BF16；golden 仍按 R017 的 FP32 中间链计算。

| dtype | shape | input | max_abs | max_rel | mismatch | strict |
| --- | --- | --- | --- | --- | --- | --- |
| bf16 | [4, 32768] | bf16_pattern | 1.309e+01 | 1.688e+00 | 1.000e+00 | 否 |
| bf16 | [4, 32768] | uniform | 9.672e+00 | 2.000e+00 | 9.988e-01 | 否 |
| fp16 | [4, 64] | large_fp16 | 1.000e+00 | 1.000e+00 | 1.000e+00 | 否 |
| fp16 | [64, 1024] | uniform | 2.734e-02 | 1.788e+00 | 7.849e-01 | 否 |

- FP16 `[4, 64]` 反例令 `x=residual=128`，所以 `y=256`；FP16 中 `y*y` 溢出为 `inf`，低精度路径的归一化结果失真。
- BF16 `[4, 32768]` 反例采用大 D、通道变化和低精度逐元素累加；低精度累加的舍入会改变均方和与归一化尺度，出现逐元素失配。
- 该结果只否定低精度中间计算的候选，不代表 Ascend C 指令的具体舍入误差；硬件指令仍需真机测量。

## R024 / R017 / R004 判断

| 路线 | 判断 | 依据 |
| --- | --- | --- |
| R024 | CPU 参考链完成 | 主矩阵 324 用例 + epsilon/广播/特殊值辅助矩阵；无 NPU 结论 |
| R017 | 计算域策略保留；BF16 舍入边界待真机核对 | 输入、加法、平方、归约、epsilon、开方、除法、gamma/bias 全在 FP32，末次转换；BF16 严格全元素通过未达成 |
| R004 | 仅作反例，不采用 | FP16 溢出和 BF16 大 D 累加失配均可复现 |

## 未覆盖与限制

- 本机没有 CANN 编译器和 Ascend NPU；没有运行 `kernel.asc`，也没有产生真实 NPU 输出。
- 题面平台的真实 15 个 shape、数据分布和最终逐元素阈值没有公开；本矩阵按仓库已记录的公式和本地阈值执行。
- 常规输入使用 `U(-2, 2)`，辅助用例使用零值、通道变化、NaN/Inf 和大值；未遍历所有可能的随机分布。
- NaN/Inf 用例用于传播行为核对；相对误差只统计有限元素，不能用它们推导普通精度结论。
- CPU 结果不能证明 DataCopyPad 写回安全、Ascend C 归约 API 语义、硬件 rsqrt 舍入、并行写回或性能。

## 复现与产物

```bash
cd /Users/sunyiyang/Desktop/Project/cann
python3 /Users/sunyiyang/Desktop/Project/cann/临时/R001-V003-Agent02/cpu_matrix.py
```

- 脚本：`/Users/sunyiyang/Desktop/Project/cann/临时/R001-V003-Agent02/cpu_matrix.py`
- 逐用例 JSON：`/Users/sunyiyang/Desktop/Project/cann/临时/R001-V003-Agent02/cpu_matrix_results.json`
- 本报告：`/Users/sunyiyang/Desktop/Project/cann/临时/R001-V003-Agent02-CPU矩阵.md`
- 本轮只创建上述临时目录内的脚本/JSON，以及用户指定的 CPU 矩阵文档；未改源码、版本目录、服务器文件或提交平台。
