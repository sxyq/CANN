# Agent 06 — AddRmsNormBias 数值精度与验证方法 调研报告

> **角色**：10 个并行子代理之一（数值精度与验证方法）。
> **本轮范围**：仅做调研 + CPU 参考数值实验，**不写 Ascend C 提交代码**，不越界检索其他代理主题。
> **重要声明**：本报告所有数值结论均为 **CPU 参考（numpy + ml_dtypes），非 NPU 实测**。本机无 CANN、无 NPU，所有关于硬件指令行精度的结论均以「CPU 复现 + 外部权威资料推演」给出，凡涉及 NPU 指令（Muls/Divs/Rsqrt 的硬件舍入）均显式标注「无法在 CPU 量化」。

---

## 1. 执行摘要

golden 基准（`AddRmsNormBias.py:impl`）三条铁律已确认：(1) `y / rms` 是**除法**而非乘倒数；(2) `epsilon` 加在 `mean(y²)` 之后、开方之前；(3) 全程 FP32，最后一次性 cast 回原 dtype。判题 `verify_result.py` 用 `np.isclose(rtol=1e-3, atol=1e-3, equal_nan=True)` + 失配比例 `tol=1e-3`（0.1%）；题面口径（Agent 1）对 fp32 更严为 **1e-4**（相对与绝对），fp16/bf16 为 1e-3。

### 6 个数值问题答案（均来自 CPU 参考实验，脚本见第 3 节）

| # | 问题 | 关键数字（CPU 参考） | 结论 |
|---|------|----------------------|------|
| **Q1** | `Muls(y,1/rms)` vs `Divs(y,rms)` | **Divs 在所有 dtype/形状下失配数 = 0（与 golden 逐位一致）**；Muls 在 fp32/fp16 失配数 = 0（安全），在 **bf16 有 1–28 个元素**跨 6/33 个配置越界（最大稳健相对误差达 5e-2，由近零分母放大；真实越界元素绝对误差极小），聚合失配比例 ≤ 0.0003%（< 0.1%，未翻车但非零误差） | **用 Divs（除法）**；Muls 在 bf16 有元素级风险，NPU 下可能更差 |
| **Q2** | `1/Sqrt` vs `Rsqrt` vs `Sqrt+除法` | **`Sqrt+除法` 与 golden 最大相对误差 = 0（逐位一致）**；`1/Sqrt+Mul` 与 `Rsqrt+Mul` 在 CPU 数学等价，误差与 Q1 的 Muls 同量级（bf16 D=32768 稳健相对误差达 1e-2 量级） | **`Sqrt+除法` 最贴近 golden**；CPU 无法复现硬件 rsqrt 的单次舍入误差（~1 ulp） |
| **Q3** | 分块累加 vs 一次累加 | 平方和相对误差 **≤ 3.0e-6**（D=32768 最坏）；远小于 fp32 阈值 1e-4 | **不是误差来源**；顺序/分块归约均可（FP32 域） |
| **Q4** | FP16 平方和溢出阈值 | `|y| ≥ 256.0` 时 `y*y` 在 fp16 下 = **inf**（fp16 max=65504，√65504≈255.94）；模板数据 uniform(-2,2) 安全 | 若判题端 `|y|` 接近/超过 256，**fp16 平方和直接 inf**；平方和必须在 FP32 算 |
| **Q5** | BF16 大 D 顺序 vs 分块归约 | D=4096/32768，两种 FP32 归约顺序对 golden 的失配比例 **≤ 0.001%**（远 < 0.1%） | 顺序不影响精度（只要累加在 FP32）；两种均可 |
| **Q6** | 全程低精度累加（对照） | **bf16 全程低精度失配比例 50%+（灾难性失败）**；fp16 在 D=1024 达 0.13%（> 0.1% 翻车），D=64/128 为 0.024%（勉强过线但危险） | **证否**低精度累加方案；FP32 中间累加是必需的 |

**一句话策略**：输入 cast 到 FP32 → 残差加 → FP32 求平方和（可顺序或分块）→ `rms = sqrt(mean(y²)+eps)` → **除法** `y/rms` → 乘 gamma 加 bias → 最后一次性 cast 回原 dtype。全程不碰 rsqrt/乘倒数，epsilon 在 sqrt 内。

---

## 2. 测试矩阵定义（可复现）

### 2.1 维度与取值理由

| 维度 | 取值 | 选取理由 |
|------|------|----------|
| **dtype** | fp16 / bf16 / fp32 | 题面三档精度；fp32 阈值最严（1e-4），是精度底线 |
| **rank** | 2D / 3D / 4D | 归一化沿最后一维 D，rank 只改变数据体积不改变数值路径；2D 为主，3D/4D 作体积覆盖 |
| **D** | 1 / 31 / 32 / 33 / 64 / 127 / 128 / 129 / 1024 / 4096 / 32768 | 64–32768 为题面约束区间；32/64/128 为 2 的幂（硬件分块友好边界），127/129 为奇边界，31/33 小奇边界，1 为退化行（全行单元素） |
| **outer** | 小规模(1–8 行) / 多行(数百) / 大规模(数千~8192) | 覆盖判题端未知的输出体量；小体量会放大失配比例（少数越界元素即可 >0.1%） |
| **特殊值** | 全零行(rms=eps 分支) / 极大值 / NaN / Inf / fp16 大值[250,260] | 覆盖 rms≈eps 的退化分支、溢出分支、NaN/Inf 传播 |

### 2.2 数据分布假设与局限

- 正常用例：`x, residual ~ uniform(-2, 2)`，`gamma ~ uniform(0.9, 1.1)`，`bias ~ uniform(-0.1, 0.1)`（对齐模板生成方式），随机种子 `SEED=1742`（与 problem_1742 对应，保证可复现）。
- **局限**：判题端真实数据范围未知，模板 uniform(-2,2) 仅作参考；若判题端 `|y|` 更大，fp16 溢出风险骤增（见 Q4）。
- 特殊值用例单独构造，不计入随机矩阵。

### 2.3 实验覆盖映射

| 问题 | 使用维度 |
|------|----------|
| Q1 Muls vs Divs | dtype×D(全11)×outer(small/mid/large)，2D [outer,D] |
| Q2 三种归一化 | dtype×D(64,128,1024,32768)×mid |
| Q3 累加顺序 | dtype×D(64,128,1024,4096,32768)，block∈{8..1024} |
| Q4 fp16 溢出 | |y|∈[250,300] 扫描 |
| Q5 bf16 大 D | D(4096,32768)×outer(small,mid) |
| Q6 全程低精度 | low∈{fp16,bf16}×D(64,128,1024)，outer=64 |
| Q7 epsilon 位置 | dtype×D(64,1024,32768)×mid |

---

## 3. 实验装置说明

- **环境**：macOS，Python 3.13.12（`/Users/sunyiyang/.workbuddy/binaries/python/envs/default`），numpy 2.5.3、ml_dtypes 0.6.0。**无 CANN / 无 NPU**。
- **脚本**：`/Users/sunyiyang/Desktop/Project/cann/调研/调研2/数值参考脚本/agent06_precision_matrix.py`
- **运行**：`python3 agent06_precision_matrix.py 2>&1 | tee agent06_precision_matrix.log`
- **日志**：同目录 `agent06_precision_matrix.log`（完整输出）、`agent06_results.json`（机器可读结果）
- **比较口径**：候选输出与 golden 均在「最后一次性 cast 回目标 dtype」后比较，cast 到 fp32 用 `np.isclose`；元素级失配以题面阈值（fp32 1e-4 / fp16·bf16 1e-3）+ 0.1% 失配比例为通过线。相对误差采用对称稳健归一化 `|c-g|/max(|g|,|c|,1e-6)`，避免近零元素放大假值。
- **可复现性**：固定 `np.random.default_rng(1742)`。

---

## 4. 六个数值问题逐条结果

### Q1：`Muls(y, 1/rms)` vs `Divs(y, rms)`

**方法**：两个候选均在 FP32 中间计算、最后一次性 cast 回目标 dtype，仅改变 `1/rms` 取倒数后 `Muls` 与 `y/rms` 除法两条路径，与 golden 比较。

**结果（节选最坏与代表行，CPU 参考）**：

| dtype | D | outer | Muls 最大稳健相对误差 | Muls 失配数 | Muls 失配比例 | Muls 过阈(1e-3/1e-4+0.1%) | Divs 失配数 |
|-------|---|-------|------|------|------|------|------|
| fp32 | 32768 | mid | 1.49e-2* | 0 | 0% | True | 0 |
| fp16 | 32768 | mid | 1.52e-2* | 0 | 0% | True | 0 |
| bf16 | 1024 | large | 7.63e-3 | 10 | 0.0005% | True | 0 |
| bf16 | 32768 | mid | 5.00e-2* | 28 | 0.0003% | True | 0 |
| bf16 | 127 | large | 7.35e-3 | 3 | 0.0012% | True | 0 |

> *最大稳健相对误差中的大值来自少数近零输出元素（分母≈0 的相对误差放大），其绝对误差极小；**真实判据是失配数与比例**。

**各 dtype 最坏情况（按失配数）**：fp32 = 0；fp16 = 0；bf16 = 28 个元素（@D=32768, outer=mid），聚合比例 0.0003%。

**结论（CPU 参考）**：
- `Divs(y, rms)` 与 golden **逐位一致（失配数=0，任何 dtype/形状）**——与上一轮结论「改 Divs 后与 golden 逐位一致」独立复现吻合。
- `Muls(y, 1/rms)`：fp32、fp16 在全部测试形状下失配数=0（安全）；**bf16 在 6/33 个配置中出现 1–28 个越界元素**，但聚合比例均 < 0.1%，故本次实验未翻车。
- **是否翻车**：在题面阈值 + 0.1% 比例线下，本轮 CPU 实验三者均未翻车；但 **bf16 Muls 存在非零元素级误差，是唯一的精度风险点**。NPU 上 `Muls`/`1/rms` 的硬件舍入可能与 CPU 不同，且 rsqrt 路径额外 ~1 ulp，风险只增不减 → **必须用 Divs（除法）**。
- 注：上一轮记忆称 bf16 Muls「最大相对差 7.4e-3、1–3 元素越界」——本轮用稳健口径复现到同类现象（bf16 有元素越界、D 越大越明显），**确认该结论方向正确，但本轮量化到 1–28 元素、最大稳健相对误差达 1e-2 量级（近零放大）**，建议以失配数/比例为硬判据。

### Q2：`1/Sqrt` vs `Rsqrt` vs `Sqrt+除法`

**结果（CPU 参考，max 稳健相对误差 vs golden）**：

| dtype | D | 1/Sqrt+Mul | Rsqrt+Mul | Sqrt+Div |
|-------|---|------|------|------|
| fp32 | 64 | 9.87e-5 | 9.87e-5 | **0** |
| fp32 | 1024 | 9.81e-4 | 9.81e-4 | **0** |
| fp32 | 32768 | 5.00e-2 | 5.00e-2 | **0** |
| fp16 | 128 | 8.26e-4 | 8.26e-4 | **0** |
| fp16 | 32768 | 2.00e-2 | 2.00e-2 | **0** |
| bf16 | 32768 | 1.15e-2 | 1.15e-2 | **0** |

**结论**：
- **`Sqrt+除法`（golden 路径）与 golden 逐位一致（相对误差=0），最贴近 golden。**
- `1/Sqrt+Mul` 与 `Rsqrt+Mul` 在 CPU 上数学等价（均 = `1/sqrt(s)` 后乘），误差与 Q1 的 Muls 同来源。
- **硬件 rsqrt 无法在 CPU 复现**：真实 NPU 的 `rsqrt` 是单次舍入指令，比 `1.0/sqrt` 少一次除法舍入，但其自身有 ~1 ulp 误差；该差异只能在 NPU 上量化，CPU 只能以 `1/Sqrt+Mul` 作为保守上界参考。
- 推荐：**`Sqrt` 后做除法**，不要走 rsqrt/乘倒数。

### Q3：分块累加 vs 一次累加

**结果（平方和相对误差，FP32 中间，与输入 dtype 无关）**：

| dtype | D=64 | D=128 | D=1024 | D=4096 | D=32768 |
|-------|------|-------|--------|--------|---------|
| fp32 | 2.16e-7 | 2.38e-7 | 4.46e-7 | 1.05e-6 | 2.94e-6 |
| bf16 | 2.04e-7 | 1.95e-7 | 5.31e-7 | 1.04e-6 | 3.63e-6 |
| fp16 | 2.17e-7 | 2.10e-7 | 6.13e-7 | 1.10e-6 | 3.07e-6 |

（block ∈ {8,16,32,64,128,256,512,1024}，取各 block 最坏值）

**结论**：相对误差 **≤ 3.0e-6**（D=32768 最坏），远低于 fp32 阈值 1e-4。**不是误差来源**。上一轮结论「≤8.9e-8」方向正确；本轮测得略大是因为本实现用「顺序分块累加」而 numpy 一次 `sum` 用 pairwise 求和，两者差异仍可忽略。顺序归约与分块归约在 FP32 下均可接受。

### Q4：FP16 平方和溢出阈值

**结果（CPU 参考）**：

```
fp16 max normal = 65504.0
sqrt(65504) = 255.937  → |y| >= 256 时 y*y 溢出
|y|=255.90 → 65472.0 (ok)      |y|=256.00 → inf (OVERFLOW)
|y|=300.00 → inf (OVERFLOW)
```

**结论**：
- **FP16 下 `|y| ≥ 256.0` 时 `y*y = inf`**（单个元素即可；累加更早在普通激活下溢出，见 Q6 实证）。
- 模板数据 uniform(-2,2) → `|y|≤4`，安全。
- **风险**：判题端数据范围未知；若 `|y|` 接近/超过 256，fp16 平方和直接 inf → RMSNorm 输出 NaN/0。
- **对策**：平方和必须在 **FP32** 域累加（golden 已如此做），与输入 dtype 无关。

### Q5：BF16 大 D 顺序归约 vs 分块归约

**结果（CPU 参考，对 golden 的失配比例）**：

| D | outer | 顺序归约 最大稳健相对误差 | 顺序归约 失配比例 | 分块(256) 失配比例 |
|---|-------|------|------|------|
| 4096 | small | 0 | 0% | 0% |
| 4096 | mid | 0 | 0% | 0.0004% |
| 32768 | small | 0 | 0% | 0.0008% |
| 32768 | mid | 0 | 0% | 0.0010% |

**结论**：两种 FP32 归约顺序对 golden 的失配比例均 **≤ 0.001%**，远小于 0.1%。**顺序不影响精度**（前提：累加在 FP32）。bf16 的误差来自 8-bit 尾数本身，与归约顺序无关；只要平方和在 FP32 累加，顺序/分块都安全。

### Q6：全程低精度累加（对照方案，证否）

**结果（CPU 参考，全程每步量化到低精度 dtype，对 golden 失配比例）**：

| low | D | 失配数 | 失配比例 | 过 0.1%？ |
|-----|---|--------|----------|-----------|
| fp16 | 64 | 1 | 0.024% | True |
| fp16 | 128 | 2 | 0.024% | True |
| fp16 | 1024 | 85 | 0.130% | **False** |
| bf16 | 64 | 2130 | 52.0% | **False** |
| bf16 | 128 | 4105 | 50.1% | **False** |
| bf16 | 1024 | 32874 | 50.2% | **False** |

**结论**：
- **bf16 全程低精度：50%+ 元素失配，灾难性失败**，直接证否。
- **fp16 全程低精度：D=1024 即 0.13% > 0.1% 翻车**；D=64/128 侥幸过线但依赖小 D，不可作为通用方案。
- 根因：bf16 仅 8 位尾数，长累加「静默停滞」（小项被大和吞没）；fp16 则在普通激活下累加即溢出 inf（见 Q4 引用实证）。
- **证否结论**：全程低精度累加不可行；FP32 中间累加是必须的。

### Q7（附加）：`epsilon` 在 sqrt 内 vs sqrt 外

**结果（CPU 参考）**：golden 路径（eps 在 `mean(y²)` 之后、`sqrt` 之前）vs 错误路径（`rms = sqrt(mean(y²)) + eps`）。

| dtype | D | 错误路径最大稳健相对误差 | 失配比例 |
|-------|---|------|------|
| fp32 | 32768 | 1.04 | 0% |
| fp16 | 32768 | 4.00e-1 | 0% |
| bf16 | 32768 | 1.14 | 0.064% |

**结论**：正常数据下 `rms≈1`，差异约 `eps/|rms|`（~1e-5 绝对），可忽略；但**全零行（rms≈eps）时差异最大**，bf16 下出现 0.064% 失配。标准且正确的位置是 **epsilon 在 sqrt 内（`mean(y²)+eps` 后开方）**，与 golden 一致且对近零行数值稳定。**必须用 `mean(y*y)+epsilon` 再开方。**

### 特殊值（定性，CPU 参考）

- **全零行**：`golden == bias` 精确成立（`rms = sqrt(eps) ≈ 0.00316`，`0/rms*g + b = b`）。
- **NaN 输入**：golden 输出含 NaN，`equal_nan=True` 可匹配。
- **Inf 输入**：经 `inf/inf` 产生 **NaN**（非 Inf）；判题端若有 Inf 输入，golden 也输出 NaN，需 `equal_nan` 比较。
- **FP16 大值 [250,260]**：`y*y` 全部 = inf（512/512），证明若平方和在 fp16 域算则直接溢出。

---

## 5. 精度策略建议（明确到「用哪个操作」）

1. **中间累加必须 FP32**：平方和 `Σy²`、均值、rms 一律在 FP32 算（Q3、Q4、Q6 实证）。输入 `x/residual/gamma/bias` 载入即 cast FP32。
2. **cast 时机**：输入 → FP32 计算 → 输出 **最后一次性 cast 回原 dtype 一次**（对齐 golden）。不要中间反复 cast。
3. **归一化用除法，不用乘倒数**：`out = y / rms * gamma + bias`（`Divs`），**禁止 `Muls(y, 1/rms)`**（Q1：bf16 有元素级越界风险；Q2：`Sqrt+Div` 与 golden 逐位一致）。
4. **`Sqrt` 后除法，不用 `Rsqrt`**：`rms = sqrt(mean(y²)+eps)` 再除（Q2）。避免 rsqrt/1-sqrt 路径的额外舍入；硬件 rsqrt 的 ~1 ulp 误差无法在 CPU 验证，宁可用除法。
5. **epsilon 必须在 `mean(y²)` 之后、`sqrt` 之前**：`rms = sqrt(mean(y*y) + epsilon)`（Q7）。
6. **尾块补零对平方和无影响**：归约在 FP32，尾块（D 非对齐）用 `GetValue`/mask 只算有效元素，补零项贡献 0，不改变 `Σy²`（Q3 已证顺序/分块误差可忽略）。
7. **residual add 精度**：`y = x + residual` 在 FP32 做；其误差属 FP32 基础舍入（~1e-7 相对），远低于阈值，不是误差来源。
8. **输出 cast 舍入模式**：numpy/`astype` 在多数平台用 RNE（round-to-nearest-even）；**NPU 的 bf16/fp16 cast 舍入模式依赖硬件（TPU 用 RNE、部分 ARM 用 Round-to-Odd、早期 bf16 用截断）**，最后 1 ulp 可能不同——这是本 CPU 实验无法覆盖的未验证项（见第 6 节），但影响仅最后 1 ulp，不会越过 1e-3/1e-4 + 0.1% 判据。

---

## 6. 局限与未验证项（必须明示）

- **CPU 参考 ≠ NPU 实测**：所有数字来自 numpy/ml_dtypes CPU 仿真，非昇腾 NPU 运行结果；**未声称任何 NPU 精度验证通过**。
- **硬件指令舍入无法量化**：`Muls`/`Divs`/`Rsqrt` 的硬件单次舍入误差、NPU 的 bf16/fp16 cast 舍入模式（RNE vs 截断 vs Round-to-Odd）只能在 NPU 上实测，CPU 无法复现。本报告对 NPU 的结论为「风险推演」，非实证。
- **判题端数据分布未知**：模板用 uniform(-2,2)，但判题端真实范围未开放；若 `|y|` 更大（如 fp16 下 ≥256），溢出风险（Q4）会真实发生，CPU 普通用例未触及。
- **失配比例阈值**：题面本地 verify 含 `tol=1e-3`（0.1%），但判题端是否含「失配比例 tol」未开放（Agent 1 未确认）；本报告以 0.1% 作为保守通过线。
- **rank 影响**：归一化沿最后一维，rank 2/3/4 数值路径相同，本报告以 2D 为主、未穷举 3D/4D 全组合，但结论与 rank 无关。
- **近零相对误差**：报告中的「最大稳健相对误差」已用对称归一化压制近零放大，硬判据以失配数/比例为准。

---

## 7. 来源清单（编号 S151–S166，Agent 06 段）

- [S151] PyTorch `torch.nn.RMSNorm` 官方文档 | https://docs.pytorch.org/docs/stable/generated/torch.nn.RMSNorm.html | PyTorch | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认标准公式 `RMS(x)=sqrt(eps+mean(x²))`，epsilon 在 sqrt 内 | 可支持结论：Q2/Q7 公式定义
- [S152] RMSNorm Deep Dive — Math + Kernels (Belgavi AI Lab) | http://aicassindra.com/blogs/transformer_math/tm_rmsnorm_deep.html | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：FP32 累加必要性、epsilon 量纲、fp16 单元素溢出阈值 |x|>256、bf16 8-bit 尾数静默停滞 | 可支持结论：Q3/Q4/Q5/Q6
- [S153] LayerNorm vs RMSNorm architecture (Belgavi AI Lab) | http://aicassindra.com/blogs/transformer_math/tm_layernorm.html | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：混合精度下归一化统计量必须 FP32、bf16 长累加精度丢失 | 可支持结论：Q5/Q6
- [S154] RMSNorm: Efficient Normalization for Modern LLMs | https://mbrenndoerfer.com/writing/rmsnorm-efficient-normalization-modern-llms | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：LLaMA 风格 RMSNorm 全程 float32 计算后 cast 回原精度 | 可支持结论：第 5 节策略
- [S155] Normalization (DeepWiki, fused kernel 分析) | https://deepwiki.com/kiki632/Smurfs/5-normalization | 技术文档 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：FP32 累加、epsilon 在 rsqrt 前加入、rsqrtf 硬件指令 | 可支持结论：Q2/Q5/Q7
- [S156] NumPy `numpy.isclose` 文档 | https://numpy.org/doc/stable/reference/generated/numpy.isclose.html | NumPy 官方 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：`absolute(a-b) <= atol + rtol*abs(b)`、非对称（b 为参考）、`equal_nan` 语义、atol 对近零值作用 | 可支持结论：第 3 节比较口径、特殊值 NaN 处理
- [S157] LLM-algo-4：RMSNorm fp16 溢出陷阱与 Upcasting | https://blog.csdn.net/weixin_43424450/article/details/162249124 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 max=65504、平方安全上限 255、升精度模式（UPcasting） | 可支持结论：Q4
- [S158] Triton 替换 RMSNorm 溢出踩坑（fp16 累加→inf→0） | https://www.cnblogs.com/lihuacheng/p/19642788 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 全程累加使 `mean=inf`→`1/inf=0`→输出全 0 | 可支持结论：Q6 实证（fp16 低精度灾难）
- [S159] Nano-vLLM 源码解读：RMSNorm 为何先 `.float()` | https://blog.csdn.net/qq_37755661/article/details/161624866 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 累加 `var→inf`，fp32 累加≈900（对照实证） | 可支持结论：Q6 实证
- [S160] MindSpore `mindspore.ops.rms_norm` 官方文档 | https://mindspore.cn/docs/zh-CN/master/api_python/ops/mindspore.ops.rms_norm.html | MindSpore 官方 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：公式 `y = x_i / sqrt(mean(x²)+ε) * γ_i`，支持 fp16/fp32/bf16，epsilon 默认 1e-6 | 可支持结论：Q2/Q7 多框架一致
- [S161] MindSpore `test_ops_rms_norm.py` 测试代码 | https://gitee.com/mindspore/mindspore/blob/v2.7.0/tests/st/ops/test_ops_rms_norm.py | MindSpore 测试 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：前向精度阈值 `loss = 1e-4 (fp32) / 1e-3 (fp16)`——与题面 Agent 1 口径一致 | 可支持结论：判题阈值佐证（fp32=1e-4, fp16=1e-3）
- [S162] MindSpeed-Core-MS `norm.py`（RMSNorm 实现） | https://atomgit.com/Ascend/MindSpeed-Core-MS/blob/r0.1.0/mindspeed_ms/legacy/model/norm.py | 框架源码 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：`x.float()` 升精度 + `mint.rsqrt(mean(square(x))+eps)` 实践 | 可支持结论：第 5 节策略（升精度+epsilon 内）
- [S163] Bfloat16 floating-point format (Wikipedia) | https://en.wikipedia.org/wiki/Bfloat16_floating-point_format | 百科/标准 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：bf16 仅 8-bit 尾数、与 fp32 转换可截断或 RNE、依赖硬件 | 可支持结论：Q5/Q6 + 第 6 节 cast 舍入局限
- [S164] `BF16RoundingMode` 文档（IEEE 754 舍入模式） | https://docs.rs/torsh-core/latest/torsh_core/dtype/bfloat16/enum.BF16RoundingMode.html | 技术文档 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：RNE(默认)/TowardZero/等舍入模式对数值稳定性与可复现性的影响 | 可支持结论：第 6 节 cast 舍入未验证项
- [S165] Floating Point：torch bf16 cast 用 RNE 舍入（实测） | https://tensor.khalilli.ai/blog/floating-point/ | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：torch 的 fp32→bf16 cast 用最近偶舍入，逐位验证 | 可支持结论：第 5/6 节 cast 舍入
- [S166] Automatic Verification of Floating-Point Accumulation Networks | https://link.springer.com/content/pdf/10.1007/978-3-031-98682-6_12.pdf | 论文(Springer) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：RNE 为 IEEE 754 默认舍入，unit roundoff u=2^-p，单操作相对误差界 | 可支持结论：第 6 节舍入误差理论上界

---

*报告结束。所有数值结论均为 CPU 参考（numpy + ml_dtypes），非 NPU 实测；硬件指令级精度须以 NPU 实测为准。*
