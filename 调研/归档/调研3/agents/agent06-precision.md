# Agent 6：数值精度与验证方法

> 日期：2026-09-11
> 题目：AddRmsNormBias（CANN 9.0.0 / 直调模板）
> 本机状态：macOS，无 CANN、无 Ascend C 编译器、无昇腾 NPU。本文所有实验均为 **CPU 参考计算**，不构成 NPU 精度结论。
> 证据等级：A=官网/官方 API/官方模板源码；B=官方论文/官方样例；C=社区；D=未核验。

---

## 0. 锁定语义与判题阈值

官方模板 golden（`scripts/AddRmsNormBias.py`，A 级）固定为：

```text
orig_dtype = x.dtype
x,r,g,b = cast_to_fp32(x,r,g,b)
y = x + residual
rms = sqrt(mean(y*y, axis=-1) + epsilon)
out = y / rms * gamma + bias
return cast_back(out, orig_dtype)
```

| 项 | 值 | 来源 |
| --- | --- | --- |
| 计算域 | 中间一律 FP32，输入输出一次 cast | 官方 golden（A） |
| epsilon 位置 | `sqrt(mean + eps)`，加在均值上、开方前 | 官方 golden + PyTorch RMSNorm（A） |
| 题面阈值 fp32 | 相对/绝对 < 1e-4 | 题面（A） |
| 题面阈值 fp16/bf16 | 相对/绝对 < 1e-3 | 题面（A） |
| 本地 verify case0 | `np.isclose(rtol=0.001, atol=0.001, equal_nan=True)` + 失配容忍 0.1% | `verify_result.py`（A） |
| 输出 cast 舍入 | `CAST_RINT`（= round-to-nearest-even，与 numpy `astype` 一致）；fp16/bf16→fp32 用 `CAST_NONE` | 项目文档 2026-09-11 核对（A） |
| A2 硬件限制 | Add/Mul 不支持 `bfloat16_t`，必须在 FP32 域计算 | 项目文档（A） |

`np.isclose` 判据（NumPy 官方，A）：`|a-b| <= atol + rtol*|b|`，不对称，以 golden 为参考值。

---

## 1. 误差来源分析

### 1.1 浮点格式本身的量化（不可消除）

| 格式 | 有效尾数位 | 约十进制精度 | 最大有限值 | 最小正规格化数 |
| --- | --- | --- | --- | --- |
| FP32 | 24（含隐含位） | ~7.2 位 | ~3.4e38 | ~1.2e-38 |
| FP16 | 11（含隐含位） | ~3.3 位 | 65504 | 2^-14 ≈ 6.1e-5 |
| BF16 | 8（含隐含位） | ~2.4 位 | ~3.4e38 | ~1.2e-38 |

- FP16：尾数短 → 相对误差量级 2^-11 ≈ 4.9e-4；动态范围窄 → 平方和易溢出（见 1.4）。
- BF16：尾数更短（2^-8 ≈ 3.9e-3 量级）→ 单次乘加相对误差就可能逼近 1e-3 阈值；但指数位与 FP32 相同，不溢出。
- 这解释了为什么「输入输出保留目标 dtype、中间必须 FP32」是题面和框架的共同选择，而非偏好。

### 1.2 归约累加误差（本题最大风险）

归约 `sum(y_i^2)` 有 D 项。若累加器停在 FP16/BF16：

- **顺序累加的吸收（absorption）**：累加器绝对值变大后，ulp（unit in the last place）随之变大，小加数被舍入丢掉。FP16 累加器到 2048 时 ulp=2，到 4096 时 ulp=4，到 8192 时 ulp=8——此后小于半 ulp 的贡献完全消失。
- **相对误差随 D 增长**：朴素界约 `O(D * u)`（u 为单位舍入误差）。FP16 的 u≈2^-11，D=1024 时上界已到 O(0.5)，实际实验见下。
- **CPU 实测（本文实验 1，fp16 输入，outer=1，随机数种子固定）**：

| D | fp16 顺序累加 max_rel | bf16 顺序累加 max_rel | fp16 向量和（分块） max_rel | bf16 向量和 max_rel |
| --- | --- | --- | --- | --- |
| 64 | 5.7e-3 | 1.1e-2 | 8.2e-4 | 9.5e-4 |
| 128 | 6.6e-3 | 9.7e-1 | 6.6e-3 | 6.0e-3 |
| 1024 | 6.7e0 | 1.1e2 | 1.2e-1 | 1.5e-1 |
| 4096 | 2.7e1 | 3.8e2 | 8.4e-3 | 4.4e-2 |
| 32768 | 4.7e3 | 2.7e4 | **7.4e3（溢出）** | 6.2e-1 |

  判定（`np.isclose(rtol=0.001, atol=0.001)`，失配容忍 0.001）：
  - `fp16_seq` 在 D≥129 多个点 FAIL；D=1024/4096/32768 失配率 83%~99.9%。
  - `bf16_seq` 从 D=32 起就出现 FAIL。
  - `bf16_everything`（residual add / mul / div 全 bf16）在几乎所有 D 上 FAIL，失配率 31%~65%。

- **FP32 链 vs float64 参考（实验 6）**：D≤32768 时绝对误差始终 < 5e-7，相对误差在非零输出处 << 1e-4。FP32 链的归约误差相对 1e-3 阈值有约 3 个数量级余量。

### 1.3 residual add 的精度

`y = x + residual` 是第一步。风险点：

- **大数加小数的吸收**：若 `|x| >> |residual|`，在 FP16 下 residual 可能被完全吞掉。例如 x=1024（FP16 可表示）、r=0.3：FP16 下 `1024+0.3 → 1024`，FP32 下保留 0.3 的贡献。后续 `y^2` 与 `mean` 跟着偏。
- **符号相消**：x≈-r 时 y≈0，此时 `mean(y^2)` 极小，`rms ≈ sqrt(eps)`，输出被 eps 主导。CPU 实验：x=-r 精确到 fp16 后 y=0，rms=3.16e-3=sqrt(1e-5)，输出全 0——golden 与任何实现都应给出 0，此处不是失配源；但若 y 仅部分相消（y~1e-3），`mean(y^2)~1e-6` 与 `eps=1e-5` 同量级，eps 的位置和精度会显著影响输出。
- **结论**：residual add 必须在 FP32 完成。官方 golden 与 NVIDIA 混合精度指南（A，§2.3「values computed by large reductions should be left in FP32」）一致。

### 1.4 epsilon 位置

三种放置方式的差异（CPU 实验 4，fp32 输入，D=128，4 行）：

| 输入量级 | `sqrt(mean+eps)`（正确） | `sqrt(mean)+eps` | `mean(y^2+eps)` |
| --- | --- | --- | --- |
| 正常 scale=1 | 参考 | max_rel ≈ 8.0e-4 | max_rel ≈ 6.2e-6 |
| scale=1e-2 | 参考 | **max_rel ≈ 2.0，失配 99.6%** | max_rel ≈ 1.3e-6 |
| scale=1e-3 | 参考 | **max_rel ≈ 1.3e2，失配 100%** | max_rel ≈ 1.4e-6 |

- `sqrt(mean)+eps` 在小激活时把 eps 加到了 rms 上而不是均值上，相对误差可达 `eps/sqrt(mean)` 量级，直接击穿 1e-3。
- `mean(y^2+eps)` 在数学上等于 `mean(y^2)+eps`（eps 为标量），与正确形式一致——这是可接受的等价实现，但要注意必须是 `mean(y^2) + eps`，不能是 `mean(y^2 + eps * D)` 之类的放缩。
- PyTorch `nn.RMSNorm`（A）公式：`RMS(x) = sqrt(eps + (1/n) sum x_i^2)`，与本题一致。

### 1.5 sqrt vs rsqrt

- 用 `rsqrt(mean+eps)` 再乘 `y`，数学上等于 `y / sqrt(mean+eps)`。
- 风险在 **rsqrt 的实现精度**：NPU 上向量 `Sqrt` 通常 0 ulp（正确舍入）；标量 `sqrtf` / 快速 `rsqrt` 可能有 1~2 ulp 甚至更差的近似。
- CPU 实验 3：向 rsqrt 结果注入固定相对误差后——

| 注入相对误差 | D=64 max_rel | D=1024 max_rel | D=4096 max_rel | 失配@0.001 |
| --- | --- | --- | --- | --- |
| 2^-10 ≈ 9.8e-4 | 6.2e-3 | 5.8e-1 | 3.6e-1 | D=1024 起出现 |
| 2^-20 ≈ 9.5e-7 | 0 | 8.7e-4 | 9.3e-4 | 0 |
| 2^-23 ≈ 1.2e-7 | 0 | 0 | 8.4e-4 | 0 |

- **判据**：rsqrt 相对误差 ≤ 2^-20 时，输出相对误差仍在 1e-3 阈值内；≥ 2^-10 时在中大 D 上击穿。提交前必须确认所用指令的误差界（项目文档已记录风险项「标量 sqrtf 精度不足（bf16）→ 改向量 Sqrt（0 ulp）或正确舍入 rsqrt（误差 ≤2^-20）」）。
- 数学关系：`rsqrt` 的相对误差 δ 会 1:1 传到输出（`y * (1±δ) * gamma`），再叠加 gamma/bias 的量化误差。

### 1.6 输出舍入 CAST_RINT / RNE

- IEEE 754 默认舍入为 round-to-nearest-even（RNE）。NumPy `astype` 对 FP32→FP16/BF16 使用 RNE。官方 golden 用 `astype`，因此实现侧的输出 cast 必须同为 RNE（`CAST_RINT`）。
- CPU 实验 8：把每个输出注入 0.5 ulp 的「向零截断」偏差后，max_rel 达 9.8e-4——刚好压在 1e-3 阈值边缘。在元素量大（例如 8192×32768）时，即使单点失配率只有万分之一，也可能超过本地 verify 的 0.1% 失配容忍。
- **建议**：输出 cast 一律 `CAST_RINT`；不要用截断、不要用 `CAST_FLOOR`。
- BF16 注意：A2 上 fp32→bf16 无 `CAST_NONE`，只能 `CAST_RINT`（项目文档 A 级）。

### 1.7 大 D 的分块归约与 padding 分母

**分块归约（推荐）**：把 D 切成 ≤4096 的块，块内 FP32 求和，块间 FP32 合并。CPU 实验（补充 D）：

| D | chunk | max_rel | 失配@0.001 |
| --- | --- | --- | --- |
| 1024 | 4096 | 0 | 0 |
| 4096 | 4096 | 0 | 0 |
| 8192 | 4096 | 0 | 0 |
| 32768 | 4096 | 9.1e-4 | 0 |
| 32768 | 2048 | 3.2e-3 | 0 |

- chunk=4096 在 D=32768 时与一次性 FP32 归约的差来自浮点加法结合律，max_rel 9e-4 仍在阈值内；chunk=2048 的 max_rel 到 3e-3，但因 atol=0.001 兜底，失配率仍为 0。
- **这是半定量依据**：块间合并保持 FP32，误差不随块数线性爆炸；块内若退化为 FP16 则回到 1.2 节的灾难。

**padding 分母（实现陷阱）**：`DataCopyPad` 把尾块补 0。补 0 不改变平方和，但 **归一化分母必须是原始 D，不是 pad 后长度**。CPU 实验（补充 A，fp16，4 行）：

| D | pad 到 | 若分母误用 pad 后长度 max_rel | 失配率 |
| --- | --- | --- | --- |
| 64 | 64 | 0 | 0 |
| 67 | 96 | 20.1 | 100% |
| 100 | 128 | 1.55 | 99.5% |
| 129 | 160 | 7.04 | 99.6% |
| 1000 | 1024 | 37.4 | 91.9% |

理论比值 `sqrt(pad/D)` 与实测 rms 偏差一致。D=1000 时 pad 到 1024 只多 2.4%，但输出相对误差到 37——因为 rms 整体缩放后 gamma 也跟着偏。**这是会直接 WA 的 bug，与精度策略无关，必须在实现里锁死分母=D。**

### 1.8 运算次序

golden 是 `y / rms * gamma + bias`。等价变形 `(y * gamma) / rms + bias` 在 FP32 下因结合律差异可能产生 1 ulp 级差别。CPU 实验（补充 E）：D=1024、16 行时 1/16384 个元素不逐位相同，max_rel=6.0e-4，仍通过 1e-3 阈值。风险低，但为了与 golden 逐位对齐，建议严格按 `y/rms*g+b` 顺序。

### 1.9 rank / outer 展平

2D/3D/4D 在「沿最后一维归约、其余维展平成 outer 行」语义下逐元素等价。CPU 实验（补充 C）：同一数据 reshape 成 12×D / 3×4×D / 2×3×2×D，golden 输出逐元素相同。大规模 outer（8192 行）下行置换再还原，结果逐元素相同（补充 F）——**行间完全独立**，多核按行切分不会引入跨行精度耦合。

---

## 2. 精度策略建议

### 2.1 推荐基线（与 golden 同链）

```text
CopyIn:  DataCopyPad GM→UB，尾块补 0（补 0 不影响 sum）
Cast:    fp16/bf16 → fp32（CAST_NONE，精确）
Add:     y = x + residual                    （FP32）
Square:  y2 = y * y                          （FP32）
Reduce:  sum = Σ y2，分块 ≤4096，块间 FP32 合并
Mean:    mean = sum / D                      （分母=D，不是 pad 长度）
Rms:     rms = sqrt(mean + epsilon)          （FP32；向量 Sqrt，0 ulp）
Norm:    t = y / rms                         （FP32）
Affine:  t = t * gamma + bias                （FP32）
Cast:    fp32 → fp16/bf16，CAST_RINT（RNE）
CopyOut: DataCopyPad UB→GM，长度=有效字节数
```

对应官方训练营「黄金法则」与 NVIDIA 混合精度指南的推荐一致。

### 2.2 不建议「低精度中间计算」的定量/半定量理由

把「中间计算留在 FP16/BF16」作为提交路线，有五条互相独立的否决理由：

1. **归约误差击穿阈值（定量）**：BF16 顺序累加在 D=32 时失配率已达 12.5%，D=64 起稳定 FAIL；FP16 顺序累加在 D=129 起多点 FAIL，D≥1024 失配率 >80%。判题点覆盖 D 到 32768，低精度归约没有安全区。
2. **吸收效应不可用分块消除（定量）**：FP16 累加器在 8192 时 ulp=8，D=32768、x~±2 的平方和实测只剩 8192（真值 ~43566），偏差 81%。分块到 FP32 合并才可救（补充 D 实验：chunk=4096 + FP32 合并，失配率 0）。
3. **BF16 尾数只有 8 位（半定量）**：单次 `y*y` 的相对误差上界 ~2^-8≈3.9e-3，已经等于题面阈值的 4 倍。D=1 的 CPU 实验里 `bf16_vec` 就已 FAIL（max_rel=3.0e-3）。哪怕不做归约，乘法本身就不够。
4. **FP16 溢出窗口真实存在（定量）**：`y^2` 单项在 |y|>256 时溢出 FP16；D=128、|y|~30 时平方和 ~4e4，接近 65504；D 更大或 |y| 更大直接 inf。residual add 后的 y 分布不受实现控制，不能假设不溢出。
5. **A2 硬件不支持 bf16 的 Add/Mul（A 级事实）**：`bfloat16_t` 的加法和乘法在 A2 上不可用，「低精度中间」对 BF16 输入在硬件上就走不通，必须 FP32 域。

**唯一勉强可行的「半低精度」变体**：输入输出 FP16/BF16，residual add / 平方 / 归约 / 除法 / gamma / bias 全部 FP32，仅在 CopyIn 后与 CopyOut 前做一次 cast——这就是推荐基线本身，已经不能称为「低精度中间计算」。

### 2.3 性能向的精度取舍

| 取舍 | 精度影响 | 建议 |
| --- | --- | --- |
| 两遍扫描 vs 单遍暂存 y | 数学等价；单遍需 UB 存 y（D×4 字节） | 精度无差，性能由 UB 预算决定 |
| 大 tile vs 小 tile | 块内 FP32 时均可；块过小会增加块间合并次数 | tile ≥ 使 chunk 效率最优，上限 4096 |
| 向量 Sqrt vs 标量 sqrtf | 向量 0 ulp；标量可能 1~2 ulp | 用向量 Sqrt |
| rsqrt 替代 sqrt | 相对误差 ≤2^-20 可接受 | 若用 rsqrt，必须确认误差界 |
| 多核按行切分 | 行独立，无精度耦合 | 按行分即可 |
| `y/rms*g` vs `y*g/rms` | 可能 1 ulp 差 | 建议与 golden 同序 |

---

## 3. 完整测试矩阵

### 3.1 维度定义

| 维度 | 取值 | 说明 |
| --- | --- | --- |
| dtype | fp32 / fp16 / bf16 | 与 x 一致；输出同型 |
| rank | 2D / 3D / 4D | 最后一维为 D |
| D | 1, 31, 32, 33, 64, 127, 128, 129, 1024, 4096, 32768 | 题面上限 32768；下限按题面 64，D<64 仅作算法自测 |
| outer | 小(1) / 中(8~64) / 大(8192+) / 极大(压 shape 上限) | outer = batch*seq*heads |
| epsilon | 1e-5（默认）/ 1e-6 / 0 | 题面属性 float，默认 1e-5 |
| 数值分布 | 常规均匀 / 小激活 / 大激活 / 相消 / 特殊值 | 见 3.3 |

D 取值理由：
- 1 / 31 / 33 / 127 / 129 / 1000：非 32 倍数，触发 DataCopyPad 尾块与向量 mask。
- 32 / 64 / 128 / 1024 / 4096：32 的幂次倍数，触发对齐快路径与 ReduceSum 分块边界。
- 32768：题面上限，归约误差与 UB 分块压力最大点。

### 3.2 组合矩阵（判题向）

不是全笛卡尔积，按风险分层：

**L0 冒烟（每次改完必跑）**
| case | shape | dtype | eps | 期望 |
| --- | --- | --- | --- | --- |
| 0 | [1, 64] | fp16 | 1e-5 | 与模板 case0 一致 |
| 1 | [2, 3, 4, 8] | fp32 | 1e-5 | 官方样例 shape |
| 2 | [4, 192] | fp16 | 1e-5 | 题面示例 D=192 |

**L1 精度边界（提交前必过）**
| case | shape | dtype | eps | 关注点 |
| --- | --- | --- | --- | --- |
| 3 | [8, 64] | bf16 | 1e-5 | bf16 尾数 |
| 4 | [8, 128] | fp16 | 1e-5 | FP16 归约起点 |
| 5 | [8, 1024] | fp16 | 1e-5 | FP16 大 D |
| 6 | [8, 4096] | bf16 | 1e-5 | BF16 大 D |
| 7 | [4, 32768] | fp16 | 1e-5 | 上限 D |
| 8 | [4, 32768] | bf16 | 1e-5 | 上限 D + BF16 |
| 9 | [4, 32768] | fp32 | 1e-5 | 上限 D + FP32 阈值 1e-4 |
| 10 | [16, 1000] | fp16 | 1e-5 | 非对齐 + 分块 |
| 11 | [16, 1000] | bf16 | 1e-5 | 非对齐 + BF16 |

**L2 形状/多核（提交前必过）**
| case | shape | dtype | eps | 关注点 |
| --- | --- | --- | --- | --- |
| 12 | [8192, 128] | fp16 | 1e-5 | 大 outer 多核 |
| 13 | [1, 32768] | fp16 | 1e-5 | 单行极限 D |
| 14 | [2, 3, 4, 576] | bf16 | 1e-5 | 4D + 题面示例 D |
| 15 | [8, 1, 1, 67] | fp16 | 1e-5 | 4D + 尾块 |
| 16 | [128, 8, 8, 192] | fp32 | 1e-5 | 4D 中等规模 |

**L3 数值分布专项（精度回归）**
见 3.3。

**L4 特殊值（NaN/Inf）**
见第 4 节。

### 3.3 数值分布专项

| 分布 | 构造 | 验证什么 |
| --- | --- | --- |
| 常规 | x,r ~ U(-2,2)，g~U(0.8,1.2)，b~U(-0.3,0.3) | 主路径 |
| 小激活 | x,r ~ U(-0.02,0.02) | eps 是否被正确加上；`sqrt(mean+eps)` vs 错误位置 |
| 极小激活 | x,r ~ U(-2e-3,2e-3) | eps 完全主导时输出 ≈ y/sqrt(eps)*g+b |
| 大激活 | x,r ~ U(-30,30) | FP16 平方和接近/超过 65504 |
| 相消 | x = -r（fp16 量化后） | y=0，rms=sqrt(eps)，输出=b |
| 近相消 | x = -r + δ，δ~U(-1e-3,1e-3) | mean(y^2) 与 eps 同量级 |
| 单侧偏置 | x~U(1000,1002)，r~U(-1,1) | 大数加小数的吸收（FP16） |
| gamma≈0 | g~U(0,1e-3) | 输出被 bias 主导 |
| gamma 含负 | g~U(-1.2,1.2) | 符号处理 |
| bias 大 | b~U(-10,10) | 输出量级抬高后 rtol/atol 行为 |

### 3.4 判定函数（与本地 verify 对齐）

```python
def judge(out, golden, rtol=0.001, atol=0.001, tol=0.001, fp32=False):
    if fp32:
        rtol, atol = 1e-4, 1e-4   # 题面 fp32 阈值
    o = out.astype(np.float32) if out.dtype == bfloat16 else out
    g = golden.astype(np.float32) if golden.dtype == bfloat16 else golden
    close = np.isclose(o, g, rtol=rtol, atol=atol, equal_nan=True)
    mism = np.sum(~close) / g.size
    return mism <= tol
```

注意：
- bf16 比较前先转 fp32（`verify_result.py` 原文如此）。
- `equal_nan=True`：golden 为 NaN 的位置，输出也必须是 NaN。
- 题面阈值 fp32 为 1e-4，本地模板 case0 是 fp16 故用 1e-3；跑 fp32 用例时把 rtol/atol 换成 1e-4。

### 3.5 可复现数据生成伪代码

```python
def gen_case(shape, dtype, dist='normal', seed=0, eps=1e-5):
    rng = np.random.default_rng(seed)
    D = shape[-1]
    if dist == 'normal':   xr, rr = (-2, 2), (-2, 2)
    elif dist == 'small':  xr, rr = (-0.02, 0.02), (-0.02, 0.02)
    elif dist == 'tiny':   xr, rr = (-2e-3, 2e-3), (-2e-3, 2e-3)
    elif dist == 'large':  xr, rr = (-30, 30), (-30, 30)
    elif dist == 'cancel':
        r = rng.uniform(-1, 1, size=shape).astype(dtype)
        return (-r).astype(dtype), r, ones_g(D, dtype), zeros_b(D, dtype)
    x = rng.uniform(*xr, size=shape).astype(dtype)
    r = rng.uniform(*rr, size=shape).astype(dtype)
    g = rng.uniform(0.8, 1.2, size=(D,)).astype(dtype)
    b = rng.uniform(-0.3, 0.3, size=(D,)).astype(dtype)
    return x, r, g, b
```

种子固定（显式表驱动，例如 `seed = 20260911 + D`），保证跨次运行可复现。本文实验统一使用 `default_rng(20260911 + D)` 或等价固定种子。

---

## 4. NaN / Inf 行为

### 4.1 期望语义（按 IEEE 754 与官方 golden）

| 输入 | y = x+r | mean(y^2) | rms | 输出 |
| --- | --- | --- | --- | --- |
| 任一分量 NaN | 对应位置 NaN | 整行 NaN | 整行 NaN | **整行 NaN** |
| 任一分量 +Inf | 对应位置 +Inf | +Inf | +Inf | 该位置 Inf/Inf=**NaN**；其余有限/Inf=**0**；再乘 gamma、加 bias |
| 任一分量 -Inf | 对应位置 -Inf | (-Inf)^2=+Inf | +Inf | 同上 |
| +Inf 与 -Inf 同行 | 两者皆 Inf | +Inf | +Inf | 两个位置 NaN，其余 0（再 affine） |
| 全 0 | 0 | 0 | sqrt(eps) | 0 * g + b = b |
| y 有限但极大 | 有限 | 可能 +Inf（fp32 溢出） | +Inf | 全 0（再 affine），或 NaN |

CPU 实验 5（D=64，fp32，golden）：
- 行含 1 个 NaN：输出整行 64 个 NaN。
- 行含 +Inf 与 -Inf：输出 2 个 NaN（Inf/Inf）、其余 0（有限/Inf），再 `*1 + 0` 后保持。

### 4.2 实现要求

- **禁止 clamp、禁止 if 分支把 NaN/Inf 改写成有限值**。题面明确要求按数学公式传播。
- 归约遇到 NaN：IEEE 传播规则是「任一操作数为 NaN 则结果 NaN」。向量 ReduceSum 必须保持该语义，不能用「跳过非有限值」的重写。
- `sqrt(NaN) = NaN`，`sqrt(+Inf) = +Inf`，`x/NaN = NaN`，`NaN * gamma = NaN`——硬件向量指令默认行为应已满足，无需额外处理。
- 验证：专门构造 L4 用例，期望输出用官方 golden 生成，不要手写期望值。
- 性能上，NaN/Inf 分支若用标量逐元素判断会显著变慢；正确做法是依赖 IEEE 语义，不做特殊分支。

---

## 5. CPU 参考验证能覆盖什么、不能覆盖什么

本机：macOS，NumPy 2.0.2 + ml_dtypes 0.5.4 + PyTorch 2.4.1，**无 CANN / 无 Ascend C 编译器 / 无昇腾 NPU**。

### 5.1 能覆盖（已用本文实验验证）

| 能力 | 说明 |
| --- | --- |
| 语义锁定 | golden 链逐步对照题面公式，确认 eps 位置、bias 时机、dtype 流 |
| 低精度路线否决 | 定量测出 fp16/bf16 中间计算在各 D 的失配率（第 1.2、2.2 节） |
| 分块归约可行性 | chunk≤4096 + FP32 合并 vs 一次性 FP32 归约，失配率 0 |
| 分母误用风险 | pad 后长度当分母的定量偏差（第 1.7 节） |
| eps 位置敏感性 | 小激活下的相对误差（第 1.4 节） |
| rsqrt 误差预算 | 注入 2^-k 相对误差后的输出偏差（第 1.5 节） |
| NaN/Inf 传播形态 | 按 IEEE 规则的期望输出（第 4 节） |
| rank/outer 等价性 | reshape 与行置换不变性 |
| 测试矩阵设计 | 第 3 节矩阵可在真机直接落地 |
| 数据生成可复现 | 固定种子的 gen_case 伪代码 |

### 5.2 不能覆盖（必须真机）

| 项 | 原因 |
| --- | --- |
| Ascend C 指令实际精度 | 向量 Sqrt/Div/Cast 的 ulp 行为以真机实测为准，文档声明可能与实现有出入 |
| `CAST_RINT` 在目标 SoC 的真实舍入 | A 级文档称 =RNE，仍需真机用「刚好中点」样本核对 |
| `ReduceSum` 的结合律与分块行为 | 硬件归约树顺序未知，CPU 的 pairwise sum 不能代替 |
| DataCopyPad 搬出是否覆盖相邻行 | 写方向语义在项目文档中已标为「真机第一验证项」（D=67/129/1000 逐字节核对） |
| 多核 32B Cache Line 撕裂 | 仅在多核并发写回时出现，CPU 串行无法复现 |
| 大张量 uint32 偏移溢出 | 与地址空间布局相关 |
| 性能（计分维度） | 本题性能是唯一计分维度，CPU 无参考价值 |
| 判题端 15 个测试点的真实 shape/dtype/eps | 平台不开放 |
| 真机 NaN/Inf 是否触发硬件异常 | 需真机确认不崩溃 |

### 5.3 CPU 参考的使用方式

- **现在**：用本文矩阵生成输入 + golden，作为后续真机验证的输入包与期望输出。探针脚本在 `临时/precision_probe.py` 与 `临时/precision_probe2.py`（本轮新建，主线未改）。
- **真机后**：跑同一输入包，用 3.4 节 judge 函数比对；再单独跑 L4 特殊值与 D∈{67,129,1000} 的逐字节核对。
- CPU 通过 ≠ NPU 通过。CPU 只能把「语义错误、分母错误、低精度路线」这类问题在上板前排掉。

---

## 6. 已确认 / 未找到 / 无法确认

### 6.1 已确认（有证据）

1. 官方 golden 为「先 cast FP32 → 计算 → 末尾一次 cast 回原 dtype」，eps 加在 `mean` 上、开方前，bias 在归一化之后。证据：模板 `AddRmsNormBias.py`（A）。
2. 本地 verify case0 为 `np.isclose(rtol=0.001, atol=0.001, equal_nan=True)` + 失配容忍 0.1%，bf16 先转 fp32 再比。证据：`verify_result.py`（A）。
3. PyTorch `nn.RMSNorm` 公式与 eps 默认值（fp16/bf16 输入用 `finfo(float32).eps`）与本题语义一致；opmath 为 FP32。证据：PyTorch 2.14 文档（A）。
4. NVIDIA 混合精度指南明确要求「大归约的结果保留在 FP32」，并以 batch-norm 的 mean/var、Softmax 为例；Tensor Core 为 FP16 输入 + FP32 累加。证据：NVIDIA Train With Mixed Precision、Matrix Multiplication Background（A）。
5. RMSNorm 原文（Zhang & Sennrich, NeurIPS 2019）给出 `RMS(x) = sqrt(eps + (1/n) sum x_i^2)`。证据：arXiv 1910.07467（B）。
6. Mixed Precision Training（Micikevicius et al., ICLR 2018）主张权重 FP32 主副本、低精度只做存储与部分运算。证据：arXiv 1710.03740（B）。
7. 8-bit FP 训练（Wang et al., NeurIPS 2018）证明要把累加精度从 32 位降到 16 位，必须引入 chunk-based accumulation 和 stochastic rounding——侧面印证「低精度累加不能裸用」。证据：arXiv 1812.08011（B）。
8. `np.isclose` 的不对称判据与 `equal_nan` 语义。证据：NumPy 2.5 文档（A）。
9. CPU 定量：低精度中间计算在题面 D 范围内系统性 FAIL；FP32 链 vs float64 绝对误差 <5e-7；分块≤4096 + FP32 合并失配率 0；分母误用 pad 长度在 D=1000 时 max_rel=37。证据：本文实验（本地 CPU，可复现）。
10. rank/outer 展平等价、行间独立。证据：本文补充实验 C/F。
11. A2 上 Add/Mul 不支持 `bfloat16_t`；fp32→bf16 只有 `CAST_RINT`。证据：项目文档 2026-09-11 核对（A）。

### 6.2 未找到

- Ascend C 官网 API 页（Cast/ReduceSum/Sqrt）在本次抓取中返回站点导航而非 API 正文，未直接引用原文。相关结论以项目文档已核对记录为准。
- MindSpore `nn.RMSNorm` 的稳定版文档 URL 未命中（尝试的 2.0.0-alpha 路径 404）。
- 社区中「AddRmsNormBias」本题的历史精度踩坑帖子未检索到（题目较新）。

### 6.3 无法确认（需真机或平台）

1. 判题端 15 个测试点的实际 shape / dtype / epsilon 配置。
2. 判题端是否与本地 `verify_result.py` 完全一致（rtol/atol/tol 与 equal_nan）。
3. 目标 SoC 上向量 Sqrt / 标量 sqrtf / rsqrt 的真实 ulp。
4. `CAST_RINT` 在目标 SoC 是否严格 RNE。
5. `ReduceSum` 的内部结合律、count 上限（文档「受 UB 限制」、官方博客 ≈4096、源码注释 <255 repeat≈16320 的三方不一致）。
6. DataCopyPad UB→GM 搬出的写方向 padding 是否覆盖相邻行。
7. 真机 NaN/Inf 输入是否稳定不崩溃。
8. 本机所有 CPU 实验结论在 NPU 上的对应表现（**不得声称 NPU 精度已验证**）。

---

## 7. 来源

| # | 标题 | URL / 路径 | 等级 | 访问日期 | 用途 |
| --- | --- | --- | --- | --- | --- |
| 1 | AddRmsNormBias 官方模板 golden `AddRmsNormBias.py` | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts/AddRmsNormBias.py` | A | 2026-09-11 | 语义、计算链、dtype 流 |
| 2 | 同模板 `verify_result.py` | 同上目录 `scripts/verify_result.py` | A | 2026-09-11 | 判定阈值、equal_nan、bf16 比较方式 |
| 3 | 同模板 `gen_data.py` | 同上目录 `scripts/gen_data.py` | A | 2026-09-11 | case0 shape/dtype/eps/随机范围 |
| 4 | PyTorch `torch.nn.RMSNorm` 文档 | https://docs.pytorch.org/docs/2.14/generated/torch.nn.RMSNorm.html | A | 2026-09-11 | 公式、eps 位置、opmath FP32 |
| 5 | NVIDIA《Train With Mixed Precision》 | https://docs.nvidia.com/deeplearning/performance/mixed-precision-training/index.html | A | 2026-09-11 | 「大归约保 FP32」、loss scaling、FP16 动态范围 |
| 6 | NVIDIA《Matrix Multiplication Background User's Guide》 | https://docs.nvidia.com/deeplearning/performance/dl-performance-matrix-multiplication/index.html | A | 2026-09-11 | FP16 输入 + FP32 累加的 Tensor Core 模型 |
| 7 | NumPy `numpy.isclose` 文档 | https://numpy.org/doc/stable/reference/generated/numpy.isclose.html | A | 2026-09-11 | 判据公式、不对称性、equal_nan |
| 8 | Root Mean Square Layer Normalization (Zhang & Sennrich) | https://arxiv.org/abs/1910.07467 | B | 2026-09-11 | RMSNorm 原始定义与 eps |
| 9 | Mixed Precision Training (Micikevicius et al., ICLR 2018) | https://arxiv.org/abs/1710.03740 | B | 2026-09-11 | FP32 主副本、低精度边界 |
| 10 | Training DNNs with 8-bit Floating Point Numbers (Wang et al., NeurIPS 2018) | https://arxiv.org/abs/1812.08011 | B | 2026-09-11 | chunk-based accumulation 的必要性 |
| 11 | 本项目题目分析文档 | `/Users/sunyiyang/Desktop/Project/cann/文档/problem-add-rms-norm-bias.md` | A | 2026-09-11 | CAST_RINT、A2 bf16 限制、风险表 |
| 12 | 本轮 CPU 探针脚本与输出 | `临时/precision_probe.py`、`临时/precision_probe2.py` | 本地实测 | 2026-09-11 | 定量实验 1~8 与补充 A~G |

---

## 8. 对其他 Agent 的可迁移结论

1. **精度基线已锁定**：任何实现方案（两遍/单遍/大 tile/小 tile）都必须走「FP32 中间 + 末尾一次 CAST_RINT」。低精度中间路线可直接从方案表中标为「不建议」。
2. **分母必须是原始 D**：DataCopyPad 补 0 只影响存储，不影响计数。这是与精度无关的独立 WA 源，实现时用常量/参数锁死。
3. **分块 ≤4096 + FP32 合并已在 CPU 侧验证**：与项目文档「ReduceSum count 上限争议 → 统一 ≤4096」的处置一致，可作为 tiling 默认值。
4. **rsqrt 若采用，误差预算 ≤2^-20**：否则中大 D 击穿阈值。优先向量 Sqrt。
5. **测试矩阵可直接落地**：第 3 节 L0~L4 可生成固定种子输入包 + golden，供真机阶段复用。
6. **CPU 通过 ≠ NPU 通过**：真机必须补测 DataCopyPad 写方向、Cast 舍入、ReduceSum 行为、NaN/Inf 稳定性。
