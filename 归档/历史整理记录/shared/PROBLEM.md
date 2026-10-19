# AddRmsNormBias 共同题目事实

## 数学语义

```text
u      = x + residual
rms    = sqrt(mean(u * u, dim=-1, keepdim=True) + epsilon)
norm   = u / rms * gamma
output = norm + bias
```

`gamma` 和 `bias` 沿最后一维广播。

## 输入输出

- `x`、`residual`、`output`：形状 `(..., D)`，x 与 residual 形状完全一致。
- `gamma`、`bias`：形状 `(D,)`。
- 输入 rank：2D、3D、4D。
- 输入 dtype：FP16、BF16、FP32。
- 输出 dtype 与输入一致。
- 输出 shape 与输入 x 一致。
- epsilon 为合法浮点属性，常见默认值为 `1e-5`。

## 合法范围

- 所有维度为正整数。
- `D` 范围为 `64..32768`。
- `D` 可能不是 32B 对齐。
- 需要处理 tail rows、D tail、NaN、Inf 和确定性执行。

将 leading dimensions 的乘积记为 `R`，候选可以把问题视为 `R` 行、每行 `D` 个元素，但不得改变题目语义。

