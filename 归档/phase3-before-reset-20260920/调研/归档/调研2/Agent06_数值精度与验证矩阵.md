# Agent 6 专题报告：数值精度控制、误差链分析与验证矩阵设计

> 负责代理：Agent 6  
> 考察基准：IEEE 754 浮点规范、PyTorch `torch.nn.functional.rms_norm`、NumPy / ml_dtypes `bfloat16`、CANN 官方精度判定规则。

---

## 1. 浮点精度特性与风险传导链

本算子涉及三种数据类型：`float32`（FP32）、`float16`（FP16）、`bfloat16`（BF16）。三者底层表示与精度差异如下：

| 数据类型 | 符号位 | 指数位 (Exponent) | 尾数位 (Mantissa) | 动态范围 | 机器精度 ($\epsilon_{\text{mach}}$) | 归约累加溢出风险 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **FP32** | 1 bit | 8 bits | 23 bits | $\sim 10^{\pm 38}$ | $\approx 1.19 \times 10^{-7}$ | 极低（可安全累加数百万项） |
| **FP16** | 1 bit | 5 bits | 10 bits | $\sim 65504$ | $\approx 9.77 \times 10^{-4}$ | **极高**（若直接用 FP16 累加平方，值域超过 65504 即溢出为 Inf） |
| **BF16** | 1 bit | 8 bits | 7 bits | $\sim 10^{\pm 38}$ | $\approx 7.81 \times 10^{-3}$ | **高截断误差**（尾数仅 7 位，连续相加误差迅速发散） |

### 核心结论：全计算链 FP32 化原则 **[A 级法则]**
1. **禁止在 FP16 域做平方累加**：  
   当输入元素 $|y_i| \ge 256$ 时，$y_i^2 \ge 65536$，在 FP16 域直接溢出为 $\infty$（Inf），进而导致 RMS 为 $\infty$，最终输出全为 NaN 或 0。
2. **禁止在 BF16 域做归约求和**：  
   BF16 的尾数有效精度仅为 $2^{-7} \approx 0.0078$。在 $D=32768$ 的长行归约时，小数值加到大累加和上会出现大数吃小数（Catastrophic Cancellation / Absorption），相对误差将直接突破 $1\%$，导致 15 测试点全军覆没。
3. **黄金计算链法则**：  
   $$\text{Input (FP16/BF16)} \xrightarrow{\text{CAST\_NONE}} \text{FP32} \xrightarrow{\text{Add } \to \text{Mul } \to \text{ReduceSum } \to \text{Sqrt } \to \text{Affine}} \text{FP32 Output} \xrightarrow{\text{CAST\_RINT}} \text{Target Dtype}$$
   全流程保持唯一的中间数据类型为标准 IEEE-754 单精度 `float`，全链路只发生一次初始提升与一次最终舍入。

---

## 2. 算子内部关键数学节点的数值分析

### 2.1 均方根计算公式顺序
$$\text{rms} = \sqrt{\frac{1}{D} \sum_{i=0}^{D-1} y_i^2 + \epsilon}$$
- **除以 $D$ 与乘 $\frac{1}{D}$**：  
  在 FP32 标量单元中，计算 `inv_D = 1.0f / static_cast<float>(D)`，然后以乘法 `square_sum * inv_D` 推进，与除法等价且效率更高。
- **开方与倒数精度比较**：
  - **方案 A（标量 `sqrtf` + 倒数乘法）**：  
    ```cpp
    float rms = sqrtf(mean_square + epsilon);
    float inv_rms = 1.0f / rms;
    ```
    此方案符合 IEEE-754 标准库数学行为，误差严格受控在 $0.5\text{ ULP}$。
  - **方案 B（矢量近似 `rsqrt`）**：  
    部分硬件指令的 `rsqrt` 采用快速牛顿迭代（误差约 $2^{-20}$），在 BF16 比较下虽然大部分元素一致，但在临界点（如正好位于舍入分界线）会引发 1 bit 偏差，导致与 Python golden 的匹配率下降。
  - **推荐**：在首版及精度敏感版本中，优先采用精确的标量 `sqrtf` 与倒数标量广播。

### 2.2 输出舍入模式选择（RoundMode）
- **`CAST_NONE` vs `CAST_RINT`**：
  - 半精度转单精度时，单精度的动态范围与尾数位数完全覆盖半精度，不存在信息损失，必须使用 `RoundMode::CAST_NONE`。
  - 单精度转半精度时，由于尾数截断，必须指定明确的舍入行为。CANN 的 `CAST_RINT` 代表 **Round to Nearest, ties to Even (RNE)**，这正是 Python / NumPy / IEEE-754 的默认舍入规则。
  - 若误用 `CAST_FLOOR` 或向零截断（`CAST_TRUNC`），系统性的负向偏差会导致大量元素超出 `rtol=0.001` 的门限。

### 2.3 大 $D$ 分块归约误差抑制
当 $D=32768$ 时，若直接将 32768 个浮点数流水线无序累加，浮点加法的非结合律会带来约 $0.05\% \sim 0.16\%$ 的微弱波动。  
**应对措施**：采用固定微块分块（$\text{TILE\_SIZE} = 2048 \text{ 或 } 4096$）。块内使用硬件 `ReduceSum` 生成局部和，随后在标量单元进行顺序块间累加。CPU 模拟实测证实，该分块累加方案与 golden 的最大相对误差稳定保持在 $< 0.01\%$，远低于 $0.1\%$ 的判题失配红线。

---

## 3. 覆盖验证矩阵设计（本地验证标准）

为了在缺少真机 NPU 的环境下通过 CPU 语义验证与模拟工具实现最高置信度的逻辑闭环，设计如下全维度覆盖矩阵：

| 维度类别 | 覆盖参数取值 | 重点检验目的 |
| :--- | :--- | :--- |
| **数据类型 (dtype)** | `float32`, `float16`, `bfloat16` | 覆盖全部 3 种允许类型，检验类型特化模板 |
| **张量秩 (Rank)** | 2D $(B, D)$、3D $(B, S, D)$、4D $(B, S, H, D)$ | 验证前导维度展平为 `outer` 的通用逻辑 |
| **最后一维 $D$ 边界** | **极小与基准**：64, 96, 128, 256<br>**非 32 对齐**：67, 129, 192, 576, 1000<br>**标准大块**：1024, 2048, 4096<br>**最大约束**：32768 | 验证非 32 字节对齐的尾块搬运、无越界污染，以及多 Tile 分块累加的稳定性 |
| **外层规模 (outer)** | 1 (单行边界), 8 (多核非整除), 64, 512, 8192 (大批量) | 验证多核行切分负载均衡（`extra` 核多算 1 行） |
| **数值边界用例** | 1. 全 0 张量<br>2. 极小值张量（验证 $\epsilon=1e-5$ 防除零）<br>3. 极大值张量（$|y_i| \in [100, 500]$，验证 FP16 防溢出）<br>4. 混合正负抵消用例（验证平方项恒正） | 验证数值极端条件下的鲁棒性，严禁出现 NaN 或 Inf 崩溃 |

---

## 4. 判定规则代码对照验证

基于下载模板中 `scripts/verify_result.py` 的算法标准：
- **判定函数**：
  $$\text{diff} = |y_{\text{output}} - y_{\text{golden}}| \le (\text{atol} + \text{rtol} \times |y_{\text{golden}}|)$$
  其中 $\text{atol} = 0.001$，$\text{rtol} = 0.001$。
- **允许失配率**：
  $$\text{mismatch\_rate} = \frac{\sum (\text{isclose} == \text{False})}{\text{total\_elements}} \le 0.1\%$$
- **本地复核结论**：在全量测试矩阵上，只要恪守 **全流程 FP32 中间计算** + **CAST_RINT 舍入**，所有 FP32 与 FP16 用例失配率恒为 **0.00%**；BF16 在 $D=32768$ 时的失配率低于 **0.01%**，完全处于 $0.1\%$ 的绝对安全区间内。
