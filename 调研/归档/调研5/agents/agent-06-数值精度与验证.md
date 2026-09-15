# Agent 06 调研报告：AddRmsNormBias 数值精度与验证方法

- 角色：Agent 6（数值精度和验证方法）
- 日期：2026-09-12
- 环境声明：本机为 macOS，无 CANN、无昇腾 NPU。本文所有本地实验均为 **CPU 参考实验**（numpy 2.0.2 / ml_dtypes 0.5.4 / torch 2.4.1，脚本在 `/tmp/agent06/exp.py`、`exp2.py`、`exp3.py`），**不代表 NPU 精度，不构成任何"精度已通过"的证据**。
- 判题容差（题面 A 级）：fp32 相对/绝对误差 < 1e-4；fp16/bf16 < 1e-3；NaN 输入须输出 NaN；多次运行结果确定。
- 本地模板判定（`verify_result.py`）：`np.isclose(rtol=atol=0.001, equal_nan=True)`，失配元素比例容忍 `tol=0.001`（0.1%）。

---

## 1 调研范围与覆盖

| # | 研究主题 | 覆盖情况 | 主要手段 |
| --- | --- | --- | --- |
| 1 | FP32 累加 vs FP16/BF16 直接累加误差（含大 D） | 已覆盖，有实测数字 | CPU 实验 E1/E1b/E2 + Higham 误差界 + PyTorch bf16 bug |
| 2 | residual add 计算域（half 加 vs 升 FP32 加） | 已覆盖，有实测数字 | CPU 实验 E3/E3b + CANN 社区溢出案例 |
| 3 | epsilon 位置差异 | 已覆盖，有实测数字 | CPU 实验 E4/E4b/E4c + PyTorch/MindSpore 官方公式 |
| 4 | sqrt vs rsqrt vs 标量除 | 已覆盖，有实测数字 | CPU 实验 E5/E5b/E11b + CUDA ulp 表 + XLA issue |
| 5 | 输出舍入 CAST_RINT/TRUNC/ROUND 影响 | 已覆盖，有实测数字（决定性实验） | CPU 实验 E6/E6b/E10 + 前序代理 API 结论 |
| 6 | 大 D 与分块两阶段归约误差界 | 已覆盖，有实测数字 | CPU 实验 E1/E2/E11 |
| 7 | NaN/Inf 传播行为 | 已覆盖，有实测数字 | CPU 实验 E7/E7b |
| 8 | 本地判定与判题判定差异风险 | 已覆盖（定性 + 数字外推） | 模板脚本审阅 + Helion/业界容差对照 |

平台覆盖：PyTorch Issue/PR（#189581、#195638、#170758、helion#1983）、PyTorch 官方文档、RMSNorm 原论文/官方仓库、NumPy/求和算法文献（Higham 误差界转述）、CUDA 数学附录、XLA rsqrt issue、MindSpore 官方精度调优指南、昇腾社区文章（CSDN/InfoQ）。未做性能结论，未调用 CANNJudge。

---

## 2 计算链逐环节精度证据

计算链分解（golden 语义，模板 `AddRmsNormBias.py`）：

```
x, residual (原dtype)
  → [环节0 输入cast] 升 FP32
  → [环节1 加法域] y = x + residual
  → [环节2 平方域] y²
  → [环节3 归约域] mean(y²) = sum(y²)/D
  → [环节4 epsilon] rms = sqrt(mean + eps)     ← eps 在 mean 之后、开方之前
  → [环节5 除法/逆] y / rms
  → [环节6 仿射] * gamma + bias（FP32）
  → [环节7 输出cast] 舍入回原 dtype
```

### 2.1 环节 0/2：输入 cast 时机与平方域（溢出与下溢）

**结论：fp16/bf16 输入必须在平方之前（建议在加法之前）cast 到 FP32。**

- fp16 平方溢出阈值 |y| > sqrt(65504) ≈ 255.96。CPU 实测（E1b，x,r~U(-250,250) fp16）：fp16 域平方产生 8.0M 个溢出元素（占 25%），**1024/1024 行全部非有限**；同数据 FP32 链 sum(y²) 相对误差仅 9.8e-6。社区案例（C 级来源，CSDN 昇腾文章）原话："直接在 FP16 张量上计算 x*x 再转 FP32 归约已经晚了——溢出发生在乘法指令，结果已是 INF，正确做法是在乘法前将操作数 cast 到 FP32"。
- fp16 平方下溢（E7b 实测）：|y| < 5.96e-8^(1/2)≈2.4e-4 时 y² 下溢为 0（fp16 最小次正规数 5.96e-8）。x=1e-4 时 fp16 域 x²=0，fp32 域=1.0e-8。若整行都是小值，fp16 域平方会把 mean(y²) 清零，rms 退化为 sqrt(eps)，输出错误。bf16 指数域与 fp32 相同（无下溢问题），但尾数仅 8 位。
- PyTorch issue #170758（bf16 LayerNorm，输入 1e10 输出全错 -114688）：同类"低精度平方/归约链在大数值下灾难失败"的官方仓库实证。

### 2.2 环节 1：residual add 计算域

**结论：模板分布（U(-2,2)）下 fp16 域加法可侥幸通过；大数值分布下必须 FP32 域加法。**

- CPU 实测（E3，其余环节同为 FP32 链）：fp16 域加法 vs FP32 域加法，FP32 中间输出最大绝对差 7.3e-4 ~ 1.06e-3，最终 fp16 输出失配率 **0.00000**（D=64/1024/32768 均为 0）。原因：fp16 加法引入 ≤0.5 ulp 量化，落在输出容差（atol=1e-3）之内。
- 大数值（E3b）：x=r=33000（fp16 可表示）时 fp16 域加法 y=inf（66000 > 65504），后续 inf² / inf 归约全链 NaN；FP32 域加法 y=65984 正常。golden 走 FP32 域（模板先 `astype(np.float32)` 再加），**若判题测试点含大数值，fp16 域加法必然失配**。
- 代价权衡：加法前 cast 与加法后 cast 只差一次 Cast 指令；升 FP32 加法与平方共用一次 cast，是更安全的默认。

### 2.3 环节 3：归约累加域（本研究最核心证据）

**结论：平方与累加都必须在 FP32 域完成（与 AGENTS.md 合规边界一致）。**

CPU 实测（E1，sum(y²) 相对误差 vs float64 参考，y=x+r，x,r~U(-2,2) fp16 输入，outer=2048）：

| D | FP32 顺序累加 | FP32 分块256两阶段 | fp16乘+fp16累 | fp16乘+FP32累 | bf16乘+bf16累 |
| --- | --- | --- | --- | --- | --- |
| 64 | 3.74e-07 | 1.37e-07 | 3.64e-03 | 3.26e-04 | 2.74e-02 |
| 1024 | 2.01e-06 | 1.60e-07 | 2.43e-02 | 8.47e-05 | 2.36e-01 |
| 4096 | 4.99e-06 | 2.33e-07 | 9.75e-02 | 4.53e-05 | 6.47e-01 |
| 32768 | 1.15e-05 | 5.77e-07 | **6.34e-01** | 2.20e-05 | **9.54e-01** |

要点：
- **fp16 累加器误差随 D 近似线性增长**（3.6e-3 → 6.3e-1），D=1024 时已达 2.4%（rms 偏差 ~1.2% > 1e-3 容差），**任何 D 都不可用**。注意 U(-2,2) 下 fp16 累加器并未溢出（sum≈D·4/3≈4.4e4 < 65504），错误纯来自舍入累积——"不溢出"不等于"够准"。
- **bf16 累加器任何 D 下误差 ≥2.7%**，彻底不可用（bf16 尾数 8 位）。
- fp16 乘法+FP32 累加（每项先乘后逐个 cast）误差 2.2e-5~3.3e-4：乘法量化 ~4.9e-4 相对（fp16 ulp/2），对均值统计后收敛，**在容差内但比全 FP32 差一个数量级**；平方也应放 FP32（cast 更早更安全）。
- FP32 累加即使是最坏的单遍顺序，D=32768 时也只有 1.15e-5，比 fp16/bf16 判题容差 1e-3 低近两个数量级。理论依据（Higham，C 级转述）：朴素顺序求和误差界 O(n·ε·cond)，成对/树形 O(log n·ε·cond)，ε_fp32=2^-24≈6e-8。
- 误差增长率旁证：torch 2.4.1 CPU `nn.RMSNorm` 对 fp16 输入直算（内部低精度路径）与 FP32 链对比失配率 **99.86%**（E9 实测）；PyTorch issue #189581 报告 bf16 小方差输入下 `nn.RMSNorm` 输出与 FP32 参考差 1.718 倍。官方框架在"低精度直算 RMSNorm"上栽过的跟头是本题最有力的反面教材。

**分块两阶段归约（大 D 必用，E2 实测，D=32768，FP32）**：

| 归约方式 | sum(y²) 相对误差 |
| --- | --- |
| 单遍顺序 | 1.10e-05 |
| 分块两阶段（块=128/1024/8192，块内块间均顺序） | 8.8e-07 ~ 4.0e-06 |
| numpy pairwise（块8） | 1.42e-07 |

分块两阶段**不劣于**单遍顺序（块内深度被限制，块间合并次数少），UB 分块归约在数值上安全，可放心用于 D=32768 的多段 ReduceSum + 标量合并。E11 进一步验证（fp32 dtype，判题容差 1e-4）：顺序归约/分块1024/rsqrt-乘法式 三种实现 vs numpy pairwise golden 的输出失配率均为 **0.0000%**——fp32 判题容差对归约顺序不敏感。

### 2.4 环节 4：epsilon 位置（语义级红线）

**结论：必须实现 `rms = sqrt(mean + eps)`（题面/golden/PyTorch 官方文档同款）。**

- PyTorch 官方文档（A 级）：`RMS(x) = sqrt(eps + 1/n Σ x_i²)`——epsilon 加在 mean 之后、开方之前，与题面和模板 golden 完全一致。
- CPU 实测（E4，eps=1e-5）三种变体的 rms 相对偏差：

| mean(y²) | A=sqrt(mean+eps) | B=sqrt(mean)+eps 相对偏差 | C=1/(sqrt(mean)+eps) 相对 1/A 偏差 |
| --- | --- | --- | --- |
| 0（全零行） | 3.162e-03 | **-99.7%** | **+315 倍** |
| 1e-8 | 3.164e-03 | -96.5% | +27.8 倍 |
| 1e-6 | 3.317e-03 | -69.6% | +228% |
| 1e-5 | 4.472e-03 | -29.1% | +41.0% |
| 1e-4 | 1.049e-02 | -4.56% | +4.78% |
| 1e-2 | 1.0005e-01 | -4.0e-04 | +4.0e-04 |
| ≥1 | ≈sqrt(mean) | ~5e-6（可忽略） | ~-5e-6（可忽略） |

- 全链实测（E4b，y~N(0,0.001) 小方差行，D=1024，fp16 输出）：B 链与 C 链相对 A 链失配率均为 **99.88%** → 必然 FAIL。mean(y²) ≥ 1e-2（常规 U(-2,2) 数据 mean≈2.7）时三种变体差异 ~4e-4 在容差内，但**测试点只要含一行小方差数据，B/C 链即挂**。
- 全零行（E4c）：y=0 时分子为 0，A/B/C 链输出都等于 bias（rms 差异被 0 分子吞掉），全零行本身**不能**区分 epsilon 位置；区分靠"非零但方差小"的行。
- 注意原论文（arXiv:1910.07467，Zhang & Sennrich）公式本身**没有 epsilon**（RMS(a)=sqrt(1/n Σa²)），epsilon 是工程实现（含 PyTorch 官方实现）为避免除零加的，位置以题面与 golden 为准，不要按第三方论文实现。

### 2.5 环节 5：sqrt/除法 vs rsqrt

**结论：FP32 域内选择 `y/rms`（先开方再除）、`y*(1/rms)`（先除法求逆再乘）或硬件 `Rsqrt`，在判题容差下均不敏感；golden 用的是"sqrt 后除法"。**

- CUDA 数学附录（A 级）给出参考量级：`rsqrtf` 最大 2 ulp，`sqrtf` 0~1 ulp（`-prec-sqrt=true` 时 0），除法 `-prec-div=true` 时 0 ulp。XLA issue #40862（B 级）实证：GPU 上 `rsqrt` 与 `1/sqrt` 的位模式可有 1 ULP 差异（f64 sweep 中 26% 样本 1 ULP off，而 `1/sqrt(x)` 100% 位精确）——即"rsqrt 替代 1/sqrt"**会改变位级结果**，但幅度仅 ulp 级。
- CPU 注入实验（E5b，把 ±N ulp(fp32) 相对扰动注入 1/rms）：±1/2/4/8 ulp 对 |out|≥0.1 元素的最大相对误差分别为 8.98e-07 / 1.49e-06 / 2.51e-06 / 4.19e-06。
- 失配率（E5/E11b）：fp16/bf16 容差 1e-3 下 ±8 ulp 注入失配率 0.000000；fp32 容差 1e-4 下 ±4 ulp 注入失配率 0.0000%。**相对容差余量 ≥ 两个数量级**。
- 唯一未决项：A2 上 `Sqrt`/`Rsqrt`/标量除的实际 ulp 精度未见官方文档数值承诺（前序代理未覆盖），按 CUDA 同类 2 ulp 量级估计无风险，但需真机复核确认（见第 6 节）。

### 2.6 环节 6：gamma/bias 仿射域

- golden 在 FP32 域完成 `y/rms*gamma + bias`。建议 kernel 同样在 FP32 域完成全部仿射后再做输出 cast，避免 bf16 中间乘加（bf16 域乘加直接引入 ~0.4% 级误差，见 E8 bf16 低精度链失配 18~19%）。
- 结合顺序（`(y/rms)*g` vs `y*(1/rms*g)`）差异在 FP32 下 ≤ 数 ulp，E11 的 rsqrt-乘法式实测失配 0%。

### 2.7 环节 7：输出 cast 舍入模式（bf16 的决定性风险）

**结论：fp32→bf16 输出必须用 `CAST_RINT`（四舍五入五成双）；`CAST_TRUNC`/`CAST_FLOOR` 会导致 ~40% 元素失配，必然 FAIL。fp16 输出各模式实测均可过，但 `CAST_RINT` 最稳（与 golden 的 numpy RNE 语义一致）。**

前序代理已确认 A2 上 fp32→bf16 支持 CAST_RINT/FLOOR/CEIL/ROUND/TRUNC，CAST_RINT=四舍六入五成双。numpy `astype`/ml_dtypes cast 默认也是 RNE，故 golden 输出即 RNE 语义。CPU 实验 E6（假设 FP32 中间值与 golden 完全一致，仅输出 cast 模式不同，失配率按 rtol=atol=1e-3、容忍 0.1% 判定）：

| dtype | 输入分布 | CAST_RINT(RNE) | CAST_TRUNC | CAST_ROUND | CAST_FLOOR |
| --- | --- | --- | --- | --- | --- |
| fp16 | U(-2,2)（模板） | 0.000% | 0.000% | 0.000% | 0.000% |
| fp16 | U(-30,30)（输出值域 ±3.6） | 0.000% | 0.000% | 0.000% | 0.000% |
| bf16 | U(-2,2)（模板） | **0.000%** | **40.4~40.7%** | 0.001% | 40.5% |
| bf16 | U(-100,100)（大数值） | **0.000%** | **40.3~40.4%** | 0.001% | 40.6% |

机理：bf16 尾数 7 位，输出值 |v|≈1 时 ulp=2^-7≈7.8e-3，远大于容差 atol+rtol·|v|≈2e-3。TRUNC/FLOOR 与 RNE 相差 1 ulp（被丢尾数 ≥0.5 ulp 的元素，约 50% 概率，其中输出值 >0.128 的元素必失配）→ ~40% 失配，是 0.1% 容忍的 400 倍。fp16 尾数 10 位，|v|<4 时 ulp≤2e-3 ≈ 容差，1 ulp 差被容差吞掉，故各模式都过——但这依赖输出值域，**不构成使用 TRUNC 的理由**。

进一步的鲁棒性（E6 第三组）：即使 FP32 中间值带 ±1e-6 相对扰动（模拟 fp32 实现顺序差异），bf16+RNE 失配率仅 0.0072%（< 0.1% 容忍），bf16+TRUNC 仍 40%+。即 **RNE 输出对微小中间扰动免疫，TRUNC 无药可救**。

业界旁证：PyTorch Helion PR #1983——bf16 rms_norm 在 atol/rtol=1e-3 下出现 14/8.4M（0.00017%）失配、max error 0.03125（正是 1 个 bf16 ulp），社区将容差放宽到 1e-2 才稳定。说明"fp32 累加+RNE 输出"的 bf16 RMSNorm 在 1e-3 容差下出现**极少量**格点失配是普遍现象，本题 0.1% 失配容忍恰好覆盖此风险（我们的 E6 实测 RNE 失配 0~0.007%）。

### 2.8 PyTorch fused RMSNorm 官方数值表（外部基准）

PyTorch PR #195638（B 级）给出 bf16 输入、FP32 累加、输出 cast 回 bf16 的 fused RMSNorm 相对 **fp64 独立参考** 的误差（M=2048,N=1024）：forward max-abs 3.082e-02、rel-L2 1.661e-03——这正是"bf16 输出 dtype 的固有量化误差"量级（~2^-9 相对）。它同时说明：与 fp64 相比 bf16 输出必然有 ~0.2% L2 误差，但**与同为 bf16-RNE-cast 的 golden 比较**则可逐格点一致（本题判定模式）。两者不矛盾：判题比的是"kernel 输出 vs golden 输出（同 dtype）"，不是 vs 无限精度。

---

## 3 CPU 参考实验结果汇总

实验设置：macOS CPU，numpy 2.0.2 + ml_dtypes 0.5.4（bf16）+ torch 2.4.1；随机源 `default_rng(42)`（模板 case0 用 `np.random.seed(42)` 复刻）；判定函数复刻 `verify_result.py`（`np.isclose(rtol=atol, equal_nan=True)` + 失配率）。脚本：`/tmp/agent06/exp.py`（E1-E3b）、`exp2.py`（E4-E10）、`exp3.py`（E5b/E6b/E11-E12）。

| 实验 | 内容 | 关键数字 | 结论 |
| --- | --- | --- | --- |
| E1 | 累加域 × D（U(-2,2)） | fp16累 D=32768 误差 6.34e-1；FP32 顺序 1.15e-5；bf16累 ≥2.7e-2 | 累加必须 FP32 |
| E1b | 大数值 U(-250,250) | fp16 域平方 25% 元素溢出，全部行非有限；FP32 链 9.8e-6 | 平方必须 FP32 |
| E2 | 分块两阶段 vs 单遍（FP32, D=32768） | 分块 8.8e-7~4.0e-6，不劣于单遍 1.1e-5 | 分块归约数值安全 |
| E3 | fp16 加法域 vs FP32 加法域 | 输出失配率 0.00000（U(-2,2)）；最大 absdiff 1.06e-3 | 模板分布下可过，但不推荐 |
| E3b | fp16 加法域大数值 | x=r=33000 → fp16 加法 inf → NaN；FP32 域 65984 正常 | 大数值必须 FP32 加法 |
| E4 | epsilon 位置（rms 级） | mean=1e-6 时 B 链偏差 -69.6%、C 链 +228% | 变体在 mean<1e-4 时致命 |
| E4b | epsilon 位置（全链，小方差行） | B/C 链失配率均 99.88% | 变体必然 FAIL |
| E5/E5b | rsqrt ±N ulp 注入 | ±8 ulp → 输出相对误差 4.19e-6，失配 0% | rsqrt/1/sqrt 不敏感 |
| E6 | 输出 cast 模式 | bf16：RNE 0% vs TRUNC 40.4%+；fp16 全 0% | bf16 必须 CAST_RINT |
| E7 | NaN/Inf/全零行传播 | 见第 5 节行为表 | 与 golden 链行为一致 |
| E8 | 低精度中间链整体 | fp16 链失配 0%（侥幸）；bf16 链 18~19.4% | bf16 中间链不可用 |
| E9 | torch 对照 | torch fp16 直入 RMSNorm 失配 99.86%；torch-fp32 vs numpy-golden 最大相对差 2.5e-4（out≈0 元素伪相对误差） | 官方框架前车之鉴；fp32 链间顺序差在容差内 |
| E10 | 模板 case0 复刻 | 模拟 NPU 链（FP32 全中间+CAST_RINT）失配率 0.000000 | 与 golden 完全一致 |
| E11 | fp32 容差 1e-4 下的实现差异 | 顺序/分块/rsqrt 式失配均 0.0000% | fp32 判题对实现顺序不敏感 |
| E12 | rank 等价性 | 2D/3D/4D 展平后 golden 输出逐位一致 | 数值与 rank 无关，只需搬运正确 |

---

## 4 测试矩阵表

矩阵维度：dtype × D（数值核心）为主表；rank、outer、数值分布为辅表。判定容差：fp16/bf16 rtol=atol=1e-3+0.1% 失配容忍；fp32 rtol=atol=1e-4。

### 4.1 主矩阵：dtype × D（rank 2D/3D/4D 任一，outer 中等规模，U(-2,2)）

| D | fp16 | bf16 | fp32 | 真机复核 |
| --- | --- | --- | --- | --- |
| 1 | rms=\|y\|，out=±gamma+bias；无归约误差；风险=尾块/边界（D<32 搬运补齐） | 同左 | 同左 | 必须（搬运边界） |
| 31 | 非 32 倍数尾块；数值无特异（E1 D=64 内误差均小） | 同左 | 同左 | 必须（DataCopyPad 尾块） |
| 32 | 单块整除；基准点 | 基准点（cast 模式敏感） | 基准点 | 必须 |
| 33 | 尾块 1 元素，越界写会毁下一行 | 同左 | 同左 | 必须（输出边界） |
| 64 | 模板 case0 同款；低精度累加误差 3.6e-3 已超容差（fp16累） | bf16累 2.7e-2 | FP32 安全 | 必须（与模板对齐） |
| 127 | 尾块 31；数值同 128 | 同左 | 同左 | 必须（尾块） |
| 128 | 2 块整除；常规 | 常规 | 常规 | 必须 |
| 129 | 尾块 1；跨块归约合并首次出现 | 同左 | 同左 | 必须（块合并） |
| 1024 | fp16 累加误差 2.4e-2（若误用）；FP32 顺序 2.0e-6 | bf16 中间链失配 18% | FP32 安全 | 必须 |
| 4096 | fp16 累加误差 9.8e-2；UB 单块放不下（需分段） | 同左 | 分块两阶段误差 9e-7 级 | 必须（分段归约） |
| 32768 | fp16 累加误差 6.3e-1；UB 必分段；FP32 分块 5.8e-7 | bf16累 9.5e-1 | FP32 顺序 1.15e-5，分块更优 | 必须（多段+合并） |

### 4.2 辅矩阵 1：rank × outer（数值上 rank 无关，E12 逐位一致；风险全在搬运与并行切分）

| 配置 | 预期风险 | 真机复核 |
| --- | --- | --- |
| 2D [outer, D] | 最低，基准 | 必须 |
| 3D 如 [8, 128, D] | 展平正确性；stride 计算 | 必须 |
| 4D 如 [2, 8, 128, D] | 展平正确性；多维索引 | 必须 |
| outer 小（1-8 行） | 单核/少核路径；tile 均摊 | 必须 |
| outer 数千行 | 多核行切分；行间无依赖（每行独立 rms） | 必须 |
| outer 数万行 | 循环上限/uint32 溢出风险（outer×D > 2^31 时索引溢出） | 必须（若题面含此规模） |

### 4.3 辅矩阵 2：数值分布（每分布 × {fp16, bf16, fp32}）

| 分布 | 预期风险 | 实验依据 | 真机复核 |
| --- | --- | --- | --- |
| U(-2,2)（模板同款） | 基准；各环节 FP32 链 0 失配 | E3/E6/E10 失配 0% | 必须 |
| 大数值（接近 fp16 上限 65504，如 U(-30000,30000) 或定点 60000） | fp16 加法溢出（>65504→inf）、fp16 平方溢出（\|y\|>256）、输出超 fp16 范围 | E1b 全行非有限；E3b inf | 必须（若测试点含） |
| 小数值（1e-4~1e-3 量级行） | fp16 域平方下溢为 0；mean(y²)≈eps 时 epsilon 位置敏感 | E7b 下溢实测；E4b 失配 99.88% | 必须（若测试点含） |
| 含全零行 | rms=sqrt(eps)=3.16e-3，out=bias；golden 同 | E7 probe | 建议 |
| 含 NaN 行 | 全行 NaN 传播（须输出 NaN） | E7：out 全 NaN | 必须（题面明示 NaN 要求） |
| 含 ±Inf 行 | Inf/Inf=NaN（该位置）+ 其余位置=bias；判题 equal_nan 只救 NaN 位置 | E7：out 含 NaN 1/64 | 必须（若测试点含） |
| gamma/bias 极值（gamma=0 或大 bias） | 输出值域扩张→fp16 ulp 超容差边界 | E6b 值域 ±3.6 仍 0 失配 | 建议 |

### 4.4 必须包含的判定口径用例（本地 verify 与判题一致性）

1. 模板 case0 原样（fp16 [1,64] seed=42）——与平台 golden 链路对齐的锚点。
2. 上述矩阵每 dtype 至少一个 0 失配用例（目标不是"压线过 0.1%"，是全对）。
3. NaN 用例按 `equal_nan=True` 语义核对 NaN 位置逐位对应（NaN≠NaN，位置错即失配）。
4. 确定性用例：同一输入连续跑 3 次逐位一致（题面要求多次运行一致；Ascend 侧注意确定性开关，见 InfoQ 案例：`HCCL_DETERMINISTIC` 等是通信归约的，单算子内 Vector 归约顺序固定则天然确定）。

---

## 5 NaN/Inf 与边界行为（CPU 实验 E7，golden FP32 链实测）

| 输入情形 | y=x+r | mean(y²)/rms | 输出 | 判定 |
| --- | --- | --- | --- | --- |
| x 含单个 NaN | 含 NaN 1/D | NaN | **全行 NaN** | equal_nan=True 通过（位置须对应） |
| x 含单个 +Inf（r 有限） | 含 Inf | rms=Inf | **Inf 位置 Inf/Inf=NaN；其余位置 y/Inf=0 → out=bias** | 混合行，须与 golden 逐位一致 |
| x 位置 +Inf，r 对应位置 -Inf | 该位置 NaN（Inf-Inf） | NaN | 全行 NaN | 通过 |
| x 全零行（r 也零） | 0 | rms=sqrt(eps)=3.1622777e-3 | out=bias | 常规 |
| fp16 x=r=60000 | FP32 域：120000（ok）；fp16 域：inf | FP32 域 rms=120000 | FP32 域 out≈gamma+bias；fp16 域 inf/inf=NaN | **加法域选择决定成败** |
| fp16 小值 x=1e-4 | 2e-4 | fp16 域平方=0 → rms=sqrt(eps)；FP32 域平方=1e-8 | fp16 域平方输出偏大（rms 被高估） | **平方域选择决定成败** |

实现要点：kernel 中不做任何 NaN/Inf 特判即可复现 golden 行为（IEEE 运算自然传播）；**切忌**在 rms 计算中做 `max(mean, eps)` 或 `if mean==0` 之类的"稳定化"补丁——golden 没有，补丁会造成失配。唯一需要保证的是中间环节不引入额外溢出（FP32 域计算天然满足：y≤131008，y²≤1.7e10 << 3.4e38）。

---

## 6 必须真机复核清单

CPU 实验只能证明"FP32 全中间 + CAST_RINT 输出"这条链在数学上与 golden 一致；以下事项 CPU 无法验证，必须上 CANN 9.0.0 / dav-2201 真机确认：

1. **A2 `Sqrt`/`Rsqrt`/标量除法的实际 ulp 精度**：文档未见 ulp 承诺。若实际精度远差于 CUDA 同类（2 ulp 量级），需重估第 2.5 节余量（当前余量约 240 倍：容差 1e-3 / 注入误差 4.2e-6）。
2. **`Cast` fp32→bf16 的 `CAST_RINT` 是否严格 RNE**（含 tie 情形与上溢饱和行为）：E6 显示 bf16 输出对 cast 模式是生死攸关的（RNE 0% vs TRUNC 40%）。需真机用"已知 FP32 中间值 → cast → 与 numpy RNE 对照"验证。
3. **`ReduceSum`(fp32) 的内部累加顺序与 sharedTmpBuffer 行为**：前序代理已确认对齐约束（dst 4B、src/tmp 32B）；数值上任何顺序都在容差内（E2/E11），但需确认 count 为 0/尾块时无脏数据污染。
4. **`DataCopyPad` 尾块搬运与输出边界**（D=1/31/33/127/129）：越界写毁相邻行是数值实验测不到的内存错误；需真机 4.1 节全部尾块 D 用例。
5. **大 D（32768）多段归约 + 标量合并**的实现正确性：UB 分段、段间合并顺序、以及 mean 的除法（`sum/D` 用 fp32 除还是先乘 1/D）——E11 显示 rsqrt-乘法式 0 失配，但需真机确认除法/乘法指令组合后的实际结果。
6. **NaN/Inf 真机传播**：A2 Vector 指令对 NaN/Inf 的实际行为（尤其 Inf/Inf、0*Inf）应与 IEEE 一致，但需用第 5 节表格逐行核对真机输出。
7. **确定性**：同输入多次运行逐位一致（题面硬性要求）。Vector/Reduce 指令应为固定数据路径，但需实测（尤其多核行切分时核间不交互，理论确定）。
8. **bf16 输出的 0.1% 失配容忍是否平台同款**：本地 verify_result.py 是模板脚本；平台判定参数（tol 比例、isclose 语义、是否 equal_nan）未公开确认。E6 显示 RNE 链即使有 1e-6 级中间扰动也只有 0.007% 失配，但若平台 tol=0（严格全对），bf16 需要做到 FP32 中间逐位对齐 golden 链（同为 RNE cast，理论可位一致）。**按 0 失配目标实现**是唯一稳妥策略。
9. **15 个测试点的真实数据范围**：本地 gen_data 只有 fp16 [1,64] U(-2,2) 单点；大数值/小数值/NaN 行是否在测试点内未知（第 4.3 节按"可能存在"做防御）。

---

## 7 来源登记表

分级：A=官方文档/官方仓库/论文原文；B=官方样例/源码/测试/Issue；C=社区；D=仅摘要。状态：verified=已读取原始内容。

| # | URL | 标题 | 版本/commit | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | https://docs.pytorch.org/docs/main/generated/torch.nn.modules.normalization.RMSNorm.html | PyTorch RMSNorm 官方文档 | main（含 c712ec6 引用） | 2026-09-12 | epsilon 位置官方定义 sqrt(eps+mean)；opmath eps 默认 | A | verified |
| 2 | https://github.com/pytorch/pytorch/issues/189581 | bf16 RMSNorm 小方差归一化常数错误（偏差 1.718x） | 2026-07-10 开 | 2026-09-12 | bf16 低精度累加实证（D=32, std=0.058） | B | verified |
| 3 | https://github.com/pytorch/pytorch/pull/195638 | fused RMSNorm mixed-dtype（数值表） | e8a736e | 2026-09-12 | bf16 前向 rel-L2 1.661e-3 vs fp64；两条路径均 FP32 累加 | B | verified |
| 4 | https://github.com/pytorch/pytorch/issues/170758 | bf16 LayerNorm 大数值输出全错 | 2025-12-18 开 | 2026-09-12 | 大数值低精度链灾难案例（1e10→-114688） | B | verified |
| 5 | https://github.com/pytorch/helion/pull/1983 | Relax rms_norm tolerance for Pallas bf16 | 7433e64 | 2026-09-12 | bf16 rms_norm 1e-3 容差下 14/8.4M 失配、max 0.03125（1 ulp） | B | verified |
| 6 | https://arxiv.org/abs/1910.07467 ；https://github.com/bzhangGo/rmsnorm | Root Mean Square Layer Normalization（原论文+官方仓库） | NeurIPS 2019 / master | 2026-09-12 | 原论文公式无 eps；rmsnorm 语义出处 | A/B | verified（论文正文 partial，官方 README verified） |
| 7 | https://handwiki.org/wiki/Pairwise_summation | Pairwise summation（Higham 误差界转述） | Higham 1993 | 2026-09-12 | O(n·ε) vs O(log n·ε) vs Kahan O(ε) | C | verified |
| 8 | https://developer.nvidia.cn/blog/cuda-math-method-cn/ | CUDA C Programming Guide 附录 H 数学方法 | CUDA 11.x+ | 2026-09-12 | rsqrtf 2 ulp、sqrtf 0-1 ulp、rintf 单指令 | A | verified |
| 9 | https://github.com/openxla/xla/issues/40862 | XLA rsqrt f64 1 ULP off（Blackwell sweep） | 2026-04-14 开 | 2026-09-12 | rsqrt 与 1/sqrt 位级差异实证（26% 样本 1 ULP） | B | verified |
| 10 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | CANN ops-transformer：RMSNorm 算子数值精度分析 | 2026-05-26 | 2026-09-12 | fp16 平方先溢出后 cast 已晚；乘法前 cast FP32 | C | verified |
| 11 | https://blog.csdn.net/2401_82857325/article/details/155560947 | 昇腾 CANN 训练营：RMSNorm Ascend C 开发 | 2025-12-04 | 2026-09-12 | ReduceSum 必须 FP32 累加"黄金法则"；32K 需分段汇总 | C | verified |
| 12 | https://xie.infoq.cn/article/acee34538e3422096a0e9dbee | 国产芯片大模型精度排查（DeepLink，A2） | 2026-01-14 | 2026-09-12 | A2 上 rms_norm 精度问题案例；浮点求和顺序不确定性与确定性开关 | C | verified |
| 13 | https://www.mindspore.cn/mindformers/docs/zh-CN/r1.7.0/advanced_development/precision_optimization.html | MindSpore 大模型精度调优指南 | r1.7.0 | 2026-09-12 | layernorm_compute_type（Norm 计算精度独立配置）、rms_norm_eps 对齐检查项 | A | verified |
| 14 | https://github.com/anviit/triton-llm-kernels | triton-llm-kernels（RMSNorm ATOL 表） | main | 2026-09-12 | RMSNorm fp16 ATOL 5e-3 理由"fp16 rounding accumulates with D" | C | verified |
| 15 | https://build.nvidia.com/station/kernel-dev-ft/instructions | NVIDIA Station：RMSNorm Triton 内核课 | - | 2026-09-12 | bf16 RMSNorm 放宽容差测试（max diff 1.56e-02 PASSED） | C | verified |
| 16 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts/{AddRmsNormBias.py, verify_result.py, gen_data.py} | 官方模板 golden/判定/数据生成脚本 | problem_1742 | 2026-09-12 | golden 语义（FP32 全中间）、判定参数（1e-3/1e-3/0.1%，equal_nan） | A | verified |
| 17 | /tmp/agent06/exp.py, exp2.py, exp3.py | 本报告 CPU 参考实验脚本（numpy 2.0.2/ml_dtypes 0.5.4/torch 2.4.1） | 2026-09-12 | 2026-09-12 | 第 3 节全部数字 | 本地 | verified |

未找到/无法确认的事项：
- A2（dav-2201）上 `Sqrt`/`Rsqrt`/`Cast` 的官方 ulp 精度数值承诺——公开资料未见，列入真机复核。
- CANNJudge 平台判定脚本的 tol 比例与 isclose 语义是否与模板 verify_result.py 完全一致——平台未公开，只能按 0 失配目标设计。
- 判题 15 个测试点的具体 shape/dtype/数值范围清单——本地模板仅含 1 个 fp16 用例。

---

## 附：给实现环节的三条硬性结论（浓缩）

1. **平方与归约累加必须在 FP32**：fp16 累加器 D=32768 时 sum(y²) 相对误差 63.4%、bf16 累加器 ≥2.7%，均为必挂级；FP32 顺序累加最差 1.15e-5，分块两阶段更优（5.8e-7）。
2. **epsilon 加在 mean 之后、sqrt 之前**：`sqrt(mean)+eps` 与 `1/(sqrt(mean)+eps)` 在小方差行失配率 99.88%，必挂。
3. **fp32→bf16 输出必须 CAST_RINT**：TRUNC/FLOOR 失配率 ~40%（容忍的 400 倍），RNE 0% 且对 1e-6 级中间扰动免疫；fp16 输出各模式实测均可过但统一用 RNE。
