# 题目分析：AddRmsNormBias

> 依据官网题面原文（2026-09-10 抓取）整理。与用户提供的初版理解有两处差异，已用官网原文核对并以官网为准。

## 1. 数学公式

```
Step 1（残差加法）:
    y_i = x_i + residual_i                       ∀ i ∈ [0, D)

Step 2（RMS 归一化，沿最后一维）:
    rms = sqrt( (1/D) * Σ_{i=0..D-1} y_i^2 + epsilon )
    z_i = y_i / rms * gamma_i                    ∀ i ∈ [0, D)

Step 3（逐通道偏置加法）:
    output_i = z_i + bias_i                      ∀ i ∈ [0, D)
```

- 与用户初版题意的差异点：官方归一化的分母是 **mean(y^2)**（除以 D，而不是别的定义），且 `epsilon` 加在 `mean(y^2)` 之后、开方之前，与 PyTorch `rms_norm` 一致。
- 偏置加法在归一化**之后**（`z + bias`），不在归一化内部。

## 2. 输入输出关系

| 参数 | 方向 | 形状 | dtype（与 x 一致） | 说明 |
| --- | --- | --- | --- | --- |
| x | 输入 | (..., D) 2D/3D/4D | fp16 / bf16 / fp32 | 主输入 |
| residual | 输入 | 与 x 完全一致 | 同 x | 残差 |
| gamma | 输入 | (D,) | 同 x | 缩放系数 |
| bias | 输入 | (D,) | 同 x | 逐通道偏置 |
| epsilon | 属性 | — | float | 默认 1e-5 |
| output | 输出 | 与 x 相同 | 同 x | 结果 |

约束：`output.shape == x.shape`；`residual.shape == x.shape`；`gamma.shape == bias.shape == (D,)`。

## 3. 数据类型

- 支持：float16、bfloat16、float32，输出与输入同型。
- 计算精度策略：**归约与中间累加必须使用 FP32**（官方训练营“黄金法则”），f16/bf16 输入先 Cast 到 FP32 再平方求和，最后输出时 cast 回原类型（仅一次量化）。
- 判定阈值：fp32 → 相对/绝对 <1e-4；f16/bf16 → 相对/绝对 <1e-3。
- NaN 输入 → 对应位置输出 NaN，不能崩溃；Inf 输入 → 按数学公式得 Inf/NaN（Inf/Inf=NaN），不能崩溃。

## 4. 张量维度

- 支持 2D（batch, D）、3D（batch, seq, D）、4D（batch, seq, heads, D）。
- batch ∈ [1, 8192]，seq_len ∈ [1, 32768]，**D ∈ [64, 32768]**（官网约束；用户初版写“D<=32768”，一致；官网下限 64）。
- 实现按“沿最后一维 D 归约，其余维度积 outer = batch*seq*heads 作为行数”统一展平处理。

## 5. 广播关系

- gamma、bias 为 (D,)，沿最后一维广播到每个样本；每行（一个 outer 行）共享同一份 gamma/bias。
- residual 与 x 形状完全一致，无广播（逐元素相加）。本题无跨维广播需求，实现上直接逐元素搬运对齐即可。

## 6. epsilon 处理

- 属性类型 float，默认 1e-5；取值范围通常在 1e-5 ~ 1e-6。
- 顺序必须是 `sqrt(mean(y^2) + eps)` —— eps 在开方**之前**、加在均值上（不是开方之后）。
- 防止 rms=0 除零；实现按 float 计算并参与 FP32 计算链，避免低精度截断。

## 7. D 非 32 倍数 / 非对齐边界处理

- 约束：D 可能不是 32 的整数倍（题面示例 D=192、576；更一般地 D 可为 64..32768 任意值）。
- 昇腾 DMA（MTE2/MTE3）对 `DataCopy` 有 32B 对齐/长度约束；本项目使用 **DataCopyPad** 处理：
  - 搬入：GM→UB 任意字节长度，不足部分自动补 0（正好满足归约需要：补 0 不影响平方和）。
  - 搬出：UB→GM 目的地址无对齐约束（手册明确 Global 地址无对齐约束），DataCopyPad 支持非对齐搬出。
  - A2 支持情况：Atlas A2 训练系列/Atlas 800I A2 推理产品支持 DataCopyPad（无 mode 参数版本）；Atlas 200/500 A2 不支持，需 fallback（GatherMask/atomic），当前按 A2 主路径实现。
- 向量计算尾块：Vector 指令的 mask 连续模式（fp16 ∈[1,128]、fp32 ∈[1,64] 每 repeat）可用于限定有效元素，冗余数据已清零则不影响结果。

## 8. 15 个测试点要求

- 用户提供：“共 15 个测试点，全部通过才计分”，平台页面未明文（待确认）。
- 本地验证计划以覆盖矩阵逼近判题组合（见 `submission-checklist.md`）：
  - dtype × {fp32, fp16, bf16}
  - rank × {2D, 3D, 4D}
  - D 边界 × {64, 96, 192, 576, 1024, 4096, 32768} ∪ 非 32 倍数 {67, 129, 1000}
  - outer × {1, 8, 8192} 与 seq 大形状抽查

## 9. 实现风险

| 风险 | 级别 | 对策 |
| --- | --- | --- |
| FP16/FP32 混算导致归约溢出/下溢 | 高 | 一律 FP32 归约与累加，末尾一次 cast |
| ReduceSum 的 dst/work 布局错误或 workLocal 空间不足 | 高 | 按官方 workLocal 空间公式预留；dst 起始 4B 对齐、src 32B 对齐 |
| 尾块复制越界覆盖下一行 | 高 | CopyIn 用带填充 DataCopyPad；CopyOut 用非对齐 DataCopyPad，长度按实际字节 |
| GetValue/标量同步导致性能退化（大 outer 时放大） | 中 | 先用正确性优先版本，性能预优化路线：向量化 rsqrt/scale、减少 GetValue 次数（列在后文） |
| 多核负载不均（outer 不能被核数整除） | 中 | 按行切分，末尾核处理剩余行（每核行数差 ≤1） |
| SoC 判题型号与本地开发型号不一致导致指令缺失 | 高(待确认) | 真机确认 SoC 后以判题配置编译；DataCopyPad 依赖 A2 系 |
| msopgen 模板 API 版本差异（CANN 9.0.0 与 8.x 的 tiling/宏差异） | 中 | 先基于官方模板生成再覆盖核心文件，避免手写全套框架代码 |
| int32 行（精度表）为模板遗留 | 低 | 忽略，正式输入 dtype 不含 int32 |

## 10. 性能优化路线（先正确后性能，供真机阶段实施）

1. v1：按行切分 + 两遍（一遍归约、一遍归一化输出），正确性优先。
2. v2：gamma/bias 一次性驻留 UB（D ≤ UB 预算时），避免每块重复搬运。
3. v3：归约阶段用向量链替代标量 GetValue（如把每块 sum 写入 sum 缓冲后整体再归约，每行仅一次标量）；用 `rsqrt` 向量/复制替代逐块标量除。
4. v4：UB 分块与 TQue 双缓冲重叠搬运与计算（CopyIn/Compute/CopyOut 流水），多核按行均分。
5. 每步在真机用 msopst / npu-smi 记录耗时与误差，防止优化损伤精度（FP32 中间计算链保持）。
