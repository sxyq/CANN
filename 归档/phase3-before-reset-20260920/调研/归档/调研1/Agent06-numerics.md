# Agent06 数值精度与验证方法调研报告

> 主题：AddRmsNormBias（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，Ascend C vector Kernel）的数值精度分析与 CPU 验证实验。
> 日期：2026-09-11。作者：Agent 6（数值精度与验证方法调研）。
> 性质：CPU 实验仅作参考，**本轮所有结论均未在真实 NPU 验证**（本机无 CANN/NPU）。
> 实验脚本：`/tmp/agent06_numerics/`（临时产物，不入仓库，seed 固定可复现）。

---

## 0. 结论速览（TL;DR）

1. **判题本质是"Kernel 量化输出 vs golden 量化输出"逐元素 `np.isclose(rtol, atol, equal_nan=True)` 比较，允许 0.1% 元素不匹配（tol=0.001）**，不是"Kernel 相对数学真值误差 < 1e-3/1e-4"。因此决定通过性的关键不是绝对精度，而是 **Kernel 与 golden 的计算链一致程度**。
2. **FP32 内部计算 + 与 golden 同链**（Cast→fp32 加 residual→平方→归约→+eps→sqrt→除→乘 gamma→加 bias→末尾一次 Cast 回原 dtype）时，CPU 实验显示 **fp16/bf16/fp32 三种 dtype、D 到 32768、所有归约方式下 mismatch 全部为 0**（逐位一致）。
3. **bf16 是唯一有真实风险的 dtype**：输出量化相对精度仅 2^-8≈3.9e-3 > 1e-3 容差，|output|≥0.25 处差 1 ulp 即判不过。它只能靠"与 golden 逐位一致"通过，且对两个实现细节敏感：
   - **归约必须分块/树形**：整行顺序累加在 D=32768 时 mismatch 0.154% > 0.1% 失败；分块（块大小 ≤4096）后 <0.01%，余量 ~10 倍。
   - **sqrt/rsqrt 精度必须接近正确舍入**：rsqrt 相对误差 2^-14 时 bf16 mismatch 0.4%~1.2% 全挂；误差 ≤2^-21（约一次牛顿迭代）后 <0.013% 通过。向量 `Sqrt` 文档保证 0 ulp，**优先用 Sqrt，勿用低精度 rsqrt**。
4. **fp16 最安全**：1 ulp 相对误差恒 2^-10≈9.77e-4 < 1e-3 容差，且 atol 兜底；实验里即使 rsqrt 误差 2^-14、顺序累加、误用 half-away 舍入，mismatch 恒 0（最大比值 mr≤0.65）。
5. **fp32 中间态无溢出风险**（fp16 输入 y 最大 131008，y² 约 1.7e10 远小于 fp32 max 3.4e38）；**fp16 内累加 y² 必溢出**（|y|≥256 单步溢出、|y|=10 且 D=1000 也溢出），必须 FP32 归约。
6. **NaN/Inf 会沿 rms 标量污染整行**（行内任一 NaN/Inf → mean(y²) 异常 → 整行 NaN 或 inf 位 NaN、有限位输出 = 0·γ+b）。Kernel 不要做 clamp/分支，直接按公式传播，与 golden 自动一致。

---

## 1. 判题语义解读（一切分析的前提）

模板 `scripts/verify_result.py` 判定逻辑（已读源码）：

```python
cmp_output = output.astype(np.float32) if dtype == bfloat16 else output
cmp_golden  = golden.astype(np.float32) if dtype == bfloat16 else golden
isclose = np.isclose(cmp_output, cmp_golden, rtol=rtol, atol=atol, equal_nan=True)
errors   = np.sum(~isclose) + missing
error_rate = errors / total_size
pass if error_rate <= tol        # 模板 case0 中 tol=0.001
```

- 比较对象：**两侧都是最终 dtype 的二进制结果**（判题端 golden 由 `scripts/AddRmsNormBias.py` 的 `impl()` 生成：x/residual/gamma/bias 各自先量化到目标 dtype、再转 fp32 计算、最后 cast 回原 dtype）。
- 语义：`|a-b| ≤ atol + rtol·|b|`；NaN==NaN 算匹配；允许 tol=0.001（0.1%）元素不匹配。
- 由此得到三个推论：
  - **Kernel 与 golden 同链 → 量化输出逐位一致 → 必过**（无论真实精度多"差"）；这是 bf16 能以 2^-8 表示精度过 1e-3 容差的唯一原因——判题基准本身也是 bf16 量化后计算。
  - 误差源（归约方式、sqrt 精度、舍入模式、计算顺序）只有"让 Kernel 与 golden 的量化结果不同"时才起作用，影响大小 = 该差异造成的 1-ulp 翻转元素占比。
  - golden 的 `np.mean` 是 **pairwise 求和**（非顺序累加），`np.sqrt` 是正确舍入 fp32，cast 是 round-to-nearest-even——这三个细节是"同链"的对齐基准。

---

## 2. 精度主题逐项分析

### 2.1 FP32 累加 vs fp16 累加（主题 1）

**结论：必须 FP32 累加，fp16 累加必溢出。**

- fp16 max=65504。`y²` 在 |y|≥256 时单步即 ≥65536 溢出为 inf；即使 |y|=10，D=1000 时顺序累加和=1e5 也溢出（CPU 实验 D 节实证：三个样例全部得 inf）。
- 溢出 → rms=inf → 整行输出 NaN/0，直接判挂。
- 上溢边界（FP32 内部计算）：fp16 输入最大 |x|≤65504，|y|=|x+r|≤131008，y²≤1.7e10，D=32768 时总和 ≤5.6e14，远小于 fp32 max 3.4e38，无溢出。bf16 的指数域同 fp32，也无溢出风险。
- 下溢：fp32 subnormal 下限 ~1.4e-45；判题数据量级（x~[-2,2]）不会触发；即使触发（y~1e-20 级），golden 也在 fp32 中同样下溢，同链一致。
- 误差上界（FP32 累加 y²）：每步乘法舍入 ≤0.5 ulp，累加误差期望 O(√D·2^-24)（见 2.6）。

证据：CPU 实验 D 节（fp16 累加溢出）+ 社区资料（ops-transformer RMSNorm 分析：FP16 直接累加 65504² 第二步溢出；昇腾硬件 FP32 累加器是精度保障关键）C 级。

### 2.2 residual add 是否提升 FP32（主题 2）

**结论：是。先各自 Cast 到 FP32 再加，与 golden 逐位一致。**

- golden：`y = x.astype(f32) + residual.astype(f32)`——加法发生在 FP32 域。
- 若 Kernel 在 fp16 域做加法（先 fp16 舍入再加），y 与 golden 差 ~1 ulp fp16（相对 9.77e-4）。fp16 判题仍能过（1 ulp < 容差），但白白消耗容差预算；bf16 情形若实现误用了 fp16 加法精度（Ascend half 加法），误差可达 2^-10 级，叠加到 bf16 输出上会造成 ~25% 元素 1-ulp 翻转（按 2.5 的边界窗口估算），**必挂**。所以必须 fp32 加。
- 附带要求：x、residual、gamma、bias 四者都必须先量化到目标 dtype 再转 fp32（不能直接搬 fp32 输入——输入本身就是目标 dtype，天然满足；gamma/bias 同理）。

证据：模板 golden 源码（A 级：判题参考实现本身）+ CPU 实验（同链 mismatch=0）。

### 2.3 epsilon 位置（主题 3）

**结论：必须 `sqrt(mean(y²)+eps)`（题面已定），不能 `sqrt(mean(y²))+eps`。**

- 两者差异量级：mean(y²)=a≫eps 时，`sqrt(a+eps)-sqrt(a) ≈ eps/(2√a)` ≈ 5e-6（a≈1, eps=1e-5），对正常数据无害；但 **a 接近 0 时（整行或近零行）差异巨大**：`sqrt(1e-5)=3.16e-3` vs `sqrt(0)+1e-5=1e-5`，rms 相差 300 倍。
- 对输出的影响：y/rms 在 a≈0 时相差可达 ~2 个量级（虽然此时 y 也小，但 bf16 输出 1-ulp 翻转不可避免），且 golden 用前者，Kernel 用后者必然大面积 mismatch。
- 实现注意：eps 是 float 属性（默认 1e-5），需以 FP32 参与计算链，`mean_fp32 + (float)eps`。

证据：题面原文公式（A 级）+ 模板 golden（A 级）。

### 2.4 sqrt vs rsqrt（主题 4）

**结论：优先向量 `Sqrt`（正确舍入，0 ulp）；`Rsqrt` 必须确认其精度 ≤ ~2^-20（约一次牛顿迭代后），否则 fp16/bf16 有挂风险。**

| 实现 | 误差 | fp16 判题 | bf16 判题 | fp32 判题 |
| --- | --- | --- | --- | --- |
| `Sqrt`（正确舍入，≤0.5 ulp） | ~6e-8 相对 | ✓ 逐位一致 | ✓ 逐位一致 | ✓ |
| 高精度 `Rsqrt`（误差 2^-21） | 4.8e-7 相对 | ✓ mm=0 | ✓ mm≤0.013% | ✓ |
| 低精度 `Rsqrt`（误差 2^-14，查表无 refine） | 6.1e-5 相对 | ✓ mm=0（mr 0.65） | ✗ mm=0.39~1.17% 全挂 | △ mr=0.49，余量仅 2 倍 |

- 实测依据（CPU 实验 1 的 r14/r21 列）：rsqrt 误差注入 2^-14 时，bf16 在全部 D 上 mismatch 0.39%~1.17%（>0.1% tol 失败）；误差 2^-21 时 ≤0.013% 通过。
- 真实案例：tilelang-ascend issue #1225 报告 `T.tile.rsqrt` 在昇腾 910 上被直接透传到底层向量 rsqrt 近似指令（无 Newton 迭代），`rtol=atol=1e-3` 下与 `1/torch.sqrt` 无法对齐——**证实"裸 rsqrt 指令误差可能达不到 1e-3 容差"并非杞人忧天**（C 级，Ascend 910 + CANN 9.1 beta）。
- 官方参考：TBE `Rsqrt` 算子实现 = `vsqrt` + `vdiv`（先开方再取倒数），不是裸 rsqrt（B 级，InplaceRsqrt 算子设计文档）。CANN 训练营 RMSNorm 样例用 rsqrt（B 级），其精度需真机确认。
- **建议：Kernel 用 `Sqrt` + 除（或 `Sqrt` 后乘 1/rms）**；若性能要求必须用 `Rsqrt`，先真机核对 rsqrt 指令相对误差，且误差须 ≤2^-20 才安全。
- 附注：`y/rms`（一次除，正确舍入）与 `y·(1/rms)`（乘倒数，两次舍入）之间差 ~1 ulp fp32（6e-8），对 fp16/bf16 输出造成的 1-ulp 翻转占比 ~0.012%/0.003%，可接受；为最大化一致性，优先 `y/rms` 与 golden 同序。

证据：asc-devkit Sqrt API 文档（B 级，950 系 0 ulp 说明；A2 系需真机核对传统 API）；tilelang issue（C 级）；InplaceRsqrt 设计文档（B 级）；CPU 实验。

### 2.5 输出舍入：CAST_RINT 与 bf16 表示精度（主题 5）

**结论：**
1. **CAST_RINT 就是 round-to-nearest-even（RNE），与 numpy 的 fp16/bf16 cast 完全一致**。官方文档原文：CAST_RINT 模式下"待舍入部分第一位为 0 不进位；第一位为 1 且后续不全为 0 进位；第一位为 1 且后续全为 0 时，M 最后一位为 0 不进位、为 1 进位"（A 级，CANN Kit 精度转换指令文档）。A2 系支持矩阵：fp32→half 支持 NONE（有损时等价 RINT）、bf16 支持 RINT 等（B 级，AscendOpGenAgent 实测知识库）。**因此 Kernel 用 CAST_RINT/NONE 即与 golden 同舍入语义。**
2. 误用 CAST_ROUND（half-away，第一位为 1 即进位）的影响：只差 tie 位，占比 ~2^-13（CPU 实验 B 节实测 fp16 0.024~0.049%、bf16 0~0.0023%），isclose 不过占比 fp16=0、bf16≤0.0008% < 0.1%——**即使误用也不挂，但推荐 CAST_RINT 保持一致**。
3. bf16 只有 8 位尾数，表示精度 2^-8≈3.9e-3，看起来"必然超 1e-3 相对误差"，但判题比较两侧都是 bf16 量化值：同链计算 → 逐位一致 → 通过。**通过机制是"误差相关"而非"绝对精度达标"**。isclose 分档（CPU 实验 A 节）：

| dtype | \|b\| 区间 | 1 ulp 绝对差 | 阈值 atol+rtol·\|b\| | 1 ulp 是否过 |
| --- | --- | --- | --- | --- |
| fp16 | [0.125, 8) 全区间 | \|b\|·2^-10 | \|b\|·1e-3 + 1e-3 | ✓ 恒过（比值恒 0.977） |
| bf16 | [0.125, 0.25) | 9.8e-4 | ≥1.125e-3 | ✓ 过 |
| bf16 | [0.25, 0.5) 起 | \|b\|·2^-8 | \|b\|·1e-3 + 1e-3 | ✗ 恒不过（比值恒 3.9） |

   → **bf16 的"逐位一致"必须覆盖 ≥99.9% 元素**（0.1% tol 内），即 Kernel 必须与 golden 计算链高度一致，不能有任何系统性偏差。

证据：CANN Kit Cast 文档（A）；AscendOpGenAgent 知识库（B）；CPU 实验 A/B 节。

### 2.6 大 D 归约误差（主题 6）

**结论：误差量级：顺序累加 ~O(√D·2^-24)，pairwise/树形 ~O(log D·2^-24)，Kahan ~O(2^-24)。分块归约（块内顺序+块间顺序）介于两者之间，D=32768 时实测 ~1.6e-7。**

CPU 实验 E 节（mean(y²) 相对 fp64 真值，4 行随机数据，最大相对误差）：

| D | 顺序累加 | pairwise(np.sum) | Kahan | 分块1024 |
| --- | --- | --- | --- | --- |
| 64 | 1.96e-7 | 4.98e-8 | ~0 | 1.96e-7 |
| 1024 | 8.30e-7 | 3.31e-8 | ~0 | 8.30e-7 |
| 4096 | 2.03e-6 | 6.58e-8 | ~0 | 3.84e-7 |
| 32768 | 3.66e-6 | 5.69e-8 | 1.7e-16 | 1.61e-7 |

- 理论界：顺序累加期望 O(√D·2^-24)（D=32768 → √32768·5.96e-8 ≈ 1.08e-5 上界，实测 3.7e-6 在界内）；最坏 O(D·2^-24)=1.95e-3；pairwise O(log₂D·2^-24)≈2.8e-7 上界。
- 对判题的影响：mean(y²) 误差 δ → rms 相对误差 ~δ/2 → y/rms 相对误差同量级 → 量化边界 1-ulp 翻转占比 ≈ δ/(2·ulp_rel)。bf16 下 δ=3.7e-6（整行顺序，D=32768）→ 占比 ~0.095%，实测 0.159%（超 0.1% 失败）；δ=1.6e-7（分块）→ ~0.004%，实测 0.007%（通过，余量 ~15 倍）。
- **因此 Kernel 的归约不能是"整行顺序累加"**；硬件 ReduceSum（树形）或"分块求和 + 块间归约"均安全（chunk 32~4096 实测全部 <0.01%）。Kahan 在 CPU 上最准但昇腾向量化实现成本高、非必需。

证据：CPU 实验 E 节 + 3 节 + 标准误差分析（高德纳浮点误差理论，教科书级，标 C）。

### 2.7 NaN/Inf 传播（主题 7）

**结论：NaN/Inf 沿 rms 标量污染整行；Kernel 不做特殊处理、按公式直算即与 golden 自动一致；equal_nan=True 保证 NaN 位匹配；禁止 clamp/截断/分支。**

CPU 实验 C 节（fp16，x 行内注入异常值，golden 输出）：

| 场景 | golden 输出 |
| --- | --- |
| x 含 1 个 NaN | 整行 8 个 NaN（mean(y²) 含 NaN → rms=NaN → 全行 NaN） |
| x 含 1 个 +Inf | 仅 inf 位 NaN；其余有限位 = y/rms = 0，输出 = 0·γ+b = b |
| x=+Inf, r=-Inf | y=NaN → 整行 NaN |
| Inf 经 rms（重置 r=0） | 仅 inf 位 NaN（同"含 1 个 +Inf"） |

- 机理：rms 是行级标量，行内任一元素异常会污染整行（rms=NaN/Inf）。
- Kernel 同链 fp32 计算自动一致；若用 rsqrt 实现，rms=inf 时 rsqrt(inf)=0，inf 位 y·0=NaN、有限位 y·0=0，与 golden 的 y/rms 行为一致（y/rms：inf/inf=NaN、有限/inf=0）。**唯一风险是 Kernel 对异常值加了 clamp/分支/特殊路径**——禁止。
- 判题 side：输出含 Inf 时，golden 与 kernel 的 Inf 位置需一致（isclose(inf,inf)=True）；本算子公式下输出几乎不产生 Inf（y/rms 最大为有限/inf 或 inf/inf），NaN 由 equal_nan=True 兜底。

证据：CPU 实验 C 节 + 模板 golden 行为（A）。

### 2.8 绝对 vs 相对误差判定（主题 8）

**结论：`np.isclose` 的 atol 在 |output|≲1 时主导、rtol 在 |output|≫1 时主导；fp16 全程 1 ulp 内即过；bf16 在 |output|≥0.25 处必须逐位一致，任何量级下 1 ulp 都不过（比值恒 3.9 倍）。**

- 阈值 `atol+rtol·|b|` 的交叉点在 |b|=atol/rtol=1。|b|≪1：阈值≈1e-3（atol 兜底，宽松）；|b|≫1：阈值≈1e-3·|b|（rtol 主导）。
- fp16：1 ulp 相对 2^-10=9.77e-4 < 1e-3，比值恒 0.977，任何量级 1 ulp 都过；|b|≪1 时 ulp 绝对更小，更稳。
- bf16：1 ulp 相对 2^-8=3.9e-3 > 1e-3，比值恒 3.9；|b|<0.25 时 atol 兜底（1 ulp 绝对 < 1e-3）才过；|b|≥0.25 全不过。**即 bf16 的通过不依赖量级、只依赖逐位一致率 ≥99.9%**。
- CPU 实验 3 B 节验证：gamma/bias 缩放 1e-4~1e4 倍（|out| 从 0.029 到 2.9e4），与 golden 同链下 mismatch 恒 0——量级不影响同链一致性。
- 对判题的实际含义：fp32 的 1e-4 容差（rtol=atol=1e-4）比 bf16 宽松得多（同链逐位一致，即使 rsqrt 误差 2^-14 也只到 mr=0.49）；fp16 的 1e-3 容差下 1-ulp 翻转全部可容忍。

证据：CPU 实验 A 节、3 B 节 + numpy isclose 语义（A）。

---

## 3. CPU 实验方法与结果

### 3.1 方法

- 环境：macOS，Python 3 + numpy 2.0.2 + ml_dtypes 0.5.4（`pip install --user ml_dtypes`，未污染系统）。无 CANN/NPU。
- 复刻判题链路：输入 x/residual/gamma/bias 按目标 dtype 量化 → 转 fp32（golden 语义）→ 计算 → 输出 cast 回目标 dtype（RNE）→ 与模板 `impl()` 的 golden 输出做 `np.isclose(rtol, atol, equal_nan=True)` 统计 mismatch 占比（判题 tol=0.001）。
- 数据：x, residual ~ U(-2,2)；gamma ~ U(0.8,1.2)；bias ~ U(-0.3,0.3)（模板 gen_data 同分布）；outer=8 行；seed=20260911 固定。
- 对照变量：归约方式（pairwise / 顺序 / Kahan / 分块1024）、rsqrt 误差注入（2^-14、2^-21 行级同向偏差）、cast 舍入模式（RNE vs half-away）、输出量级（γ/b 缩放）、rank（2D/3D/4D）。
- fp64 参考：x64+r64 → mean64(y²) → sqrt(+eps) → y/rms·g+b（全 fp64）。

### 3.2 主结果表（mismatch 占比 %，判题 tol=0.1%；mr= max|diff|/(atol+rtol|gold|)，>1 即该元素不过）

| dtype | D | golden vs 真值 maxrel | 同链 pairwise | 顺序累加 | Kahan | 分块1024 | rsqrt 2^-14 | rsqrt 2^-21 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| fp16 | 64 | 4.8e-4 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 (0.59) | 0.0000 |
| fp16 | 1024 | 4.8e-4 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 (0.65) | 0.0000 |
| fp16 | 32768 | 4.1e-3¹ | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 (0.65) | 0.0000 |
| bf16 | 64 | 3.7e-3 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | **1.17** | 0.0000 |
| bf16 | 1000 | 3.8e-3 | 0.0000 | 0.0125 | 0.0000 | 0.0125 | **0.96** | 0.0000 |
| bf16 | 4096 | 4.7e-3 | 0.0000 | 0.0183 | 0.0000 | 0.0153 | **0.83** | 0.0122 |
| bf16 | 32768 | 5.0e-3 | 0.0000 | **0.1587 ✗** | 0.0000 | 0.0069 | **0.88** | 0.0061 |
| fp32 | 64 | 4.1e-6 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 (0.45) | 0.0000 |
| fp32 | 32768 | 2.4e-3¹ | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 (0.49) | 0.0000 |

¹ maxrel 偏大的位置是 |真值| 极小处（相对误差被量化步长放大），不影响判题（判题对量化值比较）。
注：fp16/bf16 阈值 rtol=atol=1e-3，fp32 为 1e-4。rsqrt 注入为行级同向偏差（最坏情形）。

判读：
- **同链 pairwise：三种 dtype、全部 D，mismatch 全 0 → 逐位一致。**
- **bf16 是唯一会挂的 dtype**：整行顺序累加 D=32768 挂（0.16%）；rsqrt 2^-14 全 D 挂（0.4~1.2%）。其余组合全部通过。
- **fp16 对归约与 rsqrt 误差完全不敏感**（mm 恒 0，最大 mr 0.65，即最坏元素也只是容差的 65%）。
- **fp32 对 rsqrt 2^-14 有 2 倍余量**（mr 0.49），rsqrt 误差再大一倍（2^-13）即可能挂。

### 3.3 补充实验表

| 实验 | 结果 | 结论 |
| --- | --- | --- |
| rank 等价性（fp16 D=129 / bf16 D=4096 / fp32 D=1000 × 2D/3D/4D） | 全 0.0000% | rank 不影响数值（沿最后一维归约），精度测试可只跑 2D |
| chunk 大小敏感性（bf16 D=32768） | chunk 32/128/512/1024/4096：0.002~0.010%；chunk=65536（整行顺序）：0.154% ✗ | 真机任何分块（块 ≤4096）对 bf16 均安全 |
| cast RNE vs half-away | fp16 tie 位 0.024~0.049%（isclose 不过 0%）；bf16 tie 位 0~0.002%（isclose 不过 ≤0.0008%） | 误用 CAST_ROUND 也不挂，但推荐 CAST_RINT 对齐 |
| 输出量级（|out| 0.029~2.9e4） | 同链 mismatch 全 0 | 量级不影响同链一致性（isclose 语义正确） |
| fp16 内累加 y² 溢出 | \|y\|=256 单步 inf；\|y\|=10,D=1000 亦 inf | 必须 FP32 累加 |
| NaN/Inf 传播 | 见 2.7 | 禁止 clamp/分支 |

---

## 4. 可复现测试矩阵设计（建议本地/真机统一采用）

判定标准（每格统一）：**输出与 golden（模板 `impl()` 生成、同 dtype 二进制）逐元素 `np.isclose(rtol, atol, equal_nan=True)`，mismatch 占比 ≤0.1%**；阈值 fp32=1e-4、fp16/bf16=1e-3。真机每格另记录最大绝对/相对误差余量（mr 比值）。

### 4.1 主矩阵：dtype × rank × D

| dtype | rank | D ∈ {1, 31, 32, 33, 64, 127, 128, 129, 1024, 4096, 32768} 每格验证内容 |
| --- | --- | --- |
| FP16 | 2D (outer,D) | 同链逐位一致（期望 mm=0）；D=1 验证 rms=sqrt(y²+eps) 无除零；D=31/33 验证非 32 倍数尾块搬运/归约；D 递增验证归约误差不敏感（期望 mm=0，mr≤0.65） |
| FP16 | 3D (b,s,D) | 与 2D 数值等价性（期望 mm=0）；非 32 倍数 D 尾块 |
| FP16 | 4D (b,s,h,D) | 与 2D 数值等价性（期望 mm=0）；非 32 倍数 D 尾块 |
| BF16 | 2D/3D/4D × 同上 D | **最严 dtype**：期望同链 mm=0；必须验证归约实现为分块/树形（chunk ≤4096，D=32768 时 mm<0.01%，余量 ~10 倍）；rsqrt 若使用，验证误差 ≤2^-21（D=32768 时 mm<0.013%）；尾块 D=31/33 重点看 DataCopyPad 补 0 是否污染归约 |
| FP32 | 2D/3D/4D × 同上 D | 同链 mm=0，相对真值 ~1e-6~1e-5（maxabs ~5e-7），阈值 1e-4 余量 ~100 倍；若用 rsqrt 验证 mr<0.3（误差 <2^-15） |

每格期望误差量级与判定（统一说明）：
- 同链实现：mismatch=0，判 PASS，余量 = 判题 tol(0.1%) 与实测 mm 之比（≥10 倍）。
- 有意引入差异的对照格（开发期自查）：顺序累加 + bf16 + D=32768 应 FAIL（验证测试能抓问题）；rsqrt 2^-14 + bf16 应 FAIL。
- fp16 期望 golden-vs-真值 maxrel ~4.8e-4（量化上限 9.77e-4）；bf16 ~5e-3（量化上限 3.9e-3）；fp32 ~1e-6~2.4e-3（后者在近零元素，无碍）。

### 4.2 outer 维度（单独一维）

| outer 档 | 取值 | 验证内容 |
| --- | --- | --- |
| 小规模 | 1~8 行 | 行间 rms 独立、多核/单核路径一致 |
| 多行 | 64~512 行 | 分块循环边界、gamma/bias 逐行广播正确性 |
| 大规模 | 8192 行（batch 上限） | 多核按行切分后每行 rms 仍独立（防跨行污染）；大 outer 下逐行标量同步无精度影响 |

### 4.3 补充边界格

- 数值边界：全 0 行（rms=sqrt(eps)）；单元素非零行；gamma=0（输出=bias）；bias=0；y 极大（|x|、|r| 近 fp16 max，验证 fp32 中间态不溢出）。
- 异常格：x 行内含 1 个 NaN / +Inf / -Inf（期望与 golden NaN 位置一致，PASS）；inf+(-inf)（整行 NaN，PASS）。
- 数据分布：U(-2,2) 主分布 + U(-1e3,1e3) 大动态范围抽查（覆盖 bf16 大值 isclose 分档）。

---

## 5. 对 1e-3 / 1e-4 容差下各 dtype 通过性结论

| dtype | 判题容差 | 通过性判断（有依据） | 关键前提 |
| --- | --- | --- | --- |
| FP16 | 1e-3（相对+绝对，0.1% 元素容忍） | **必然可通过，余量 ~2 倍**（1 ulp 相对 9.77e-4 < 1e-3） | 中间计算 FP32；与 golden 同链。即使归约方式、rsqrt 误差（≤2^-14）、舍入模式有偏差也不挂（实测 mm 恒 0） |
| BF16 | 1e-3 | **可通过，但必须满足两条硬约束**：① 归约分块/树形（禁整行顺序累加，D=32768 时顺序累加实测 0.16%>0.1% 挂）；② rsqrt 误差 ≤~2^-20（2^-14 时实测 0.4~1.2% 全挂；2^-21 时 ≤0.013%）。满足后实测 mm<0.01%，余量 ~10 倍 | 与 golden 计算链一致（Cast→FP32 加→FP32 归约→sqrt→除→乘→加→末尾 CAST_RINT）；不能有任何系统性偏差 |
| FP32 | 1e-4 | **可通过，余量 ~100 倍（同链）~2 倍（rsqrt 2^-14）** | 同链逐位一致；若用低精度 rsqrt（误差 2^-13 级）会挂，须用 Sqrt 或高精度 rsqrt |

判题基准（golden）本身也是"量化输入 + FP32 内部 + 量化输出"，故 bf16 能以 2^-8 表示精度通过 1e-3 容差——**通过机制是 Kernel 与 golden 的误差相关（同链逐位一致），而非绝对精度达到 1e-3**。这是本报告最核心的结论。

---

## 6. 建议的 Kernel 内计算顺序（精度视角；性能取舍见 Agent 7）

```
1. 搬运：x、residual、gamma、bias 搬入 UB（DataCopyPad 处理非对齐/尾块，补 0 不影响平方和）
2. Cast：四者全部 Cast 到 FP32（fp16 用 Cast<NONE/RINT>；bf16 用 Cast<RINT>）——与 golden 的量化后转 fp32 一致
3. y = x_f + r_f                                # FP32 残差加法
4. sq = y * y                                   # FP32 平方
5. sum = ReduceSum(sq, 沿最后一维) / D           # FP32 归约；必须分块/树形（块 ≤4096，块间顺序或树形）；
                                                #   禁整行顺序累加（bf16 D=32768 挂）；可先各块部分和再归约部分和
6. m = sum + (float)eps                         # eps 以 FP32 参与，顺序必须 mean+eps 再 sqrt
7. rms = Sqrt(m)                                # 首选向量 Sqrt（正确舍入，0 ulp 级）
                                                #   若必须用 Rsqrt：先真机核对误差 ≤2^-20，否则 bf16 挂
8. out = y / rms * g_f + b_f                    # 与 golden 同序 (y/rms)*g+b；除法正确舍入
                                                #   （乘 1/rms 亦可，1-ulp 差对判题可忽略，但同序更稳）
9. Cast 回原 dtype：CAST_RINT（RNE，与 numpy 一致）  # 末尾一次量化；不做任何 clamp/分支
```

禁止项（会导致与 golden 不一致而挂）：
- fp16/bf16 域内做加/乘/累加（必须先 Cast FP32）；
- 整行顺序累加平方和（bf16 大 D 挂）；
- 低精度 rsqrt（<2^-20）或自写查表近似；
- 对 NaN/Inf 做 clamp、截断、特殊分支；
- 在归约前先乘 1/D 再用 D 次累加之外的结构性改写（如先算均值再平方等任何改变计算链的变换需 CPU 同链验证）。

---

## 7. 本轮新增来源清单

> 证据等级：A=官方文档/官方仓；B=官方样例/官方派生实现；C=社区/论文。本轮全部来源**未在真实 NPU 验证**。

| # | 名称 | URL / 出处 | 等级 | 用途 | 是否真实 NPU 验证 |
| --- | --- | --- | --- | --- | --- |
| 1 | CANN Kit 精度转换指令（Cast/CAST_RINT/CAST_ROUND 舍入规则） | https://device.harmonyos.com/cn/docs/apiref/harmonyos-guides/cannkit-precision-conversion-instruction （2026-08 版） | A | 确认 CAST_RINT=RNE、CAST_ROUND=half-away、各模式定义 | 未验证 |
| 2 | vec_conv 精度转换（hiascend 商用 8.0.RC2） | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlastikapi_07_0083.html | A | f322f16 舍入模式与饱和语义交叉确认 | 未验证 |
| 3 | asc-devkit Sqrt API（0 ulp 精度、产品支持矩阵） | https://gitcode.com/cann/asc-devkit/docs/api/reg/Sqrt（CSDN 转述 2026-05） | B | 向量 Sqrt 精度 0 ulp；A2 系传统 API 精度待真机确认 | 未验证 |
| 4 | CANN InplaceRsqrt 算子设计文档（TBE Rsqrt=sqrt+div 实现） | https://gitcode.com/cann/cann-ops-competitions（CSDN 转述 2026-06） | B | 官方 rsqrt 语义 = vsqrt+vdiv，非裸近似；Cast 饱和语义 | 未验证 |
| 5 | tilelang-ascend issue #1225（rsqrt 精度不足，rtol=1e-3 失配） | https://github.com/tile-ai/tilelang-ascend/issues/1225 | C | 实证：裸硬件 rsqrt（无 refine）在 1e-3 容差下与 1/sqrt 不可对齐 | 未验证（Ascend 910/CANN 9.1beta 环境报告） |
| 6 | AscendOpGenAgent 知识库（A2/CANN 8.5 fp32→fp16/bf16 Cast 支持矩阵） | https://github.com/Just-it/AscendOpGenAgent/pull/146 | C | fp32→half 支持 NONE（有损=RINT）、bf16 支持 RINT 等 | 未验证（Atlas A2/CANN 8.5 实测记录） |
| 7 | CANN ops-transformer：RMSNorm 数值精度分析 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | C | FP16 平方和溢出、FP32 累加器为硬件要求、Kahan 讨论 | 未验证（原文称在昇腾 NPU 验证，本机未复核） |
| 8 | 昇腾 AI Core rsqrtf() 硬件指令说明（训练营/实战文章） | https://ascendai.csdn.net/693ad54a0800f3458b818b10.html | C | rsqrtf 硬件指令延迟 1/3、FP32 中间累加是通用做法 | 未验证 |
| 9 | numpy 文档：mean/sum pairwise、cumsum 顺序累加语义 | https://numpy.org/doc/stable/（np.cumsum 文档明确"不使用 pairwise 求和"） | A | 归约误差模拟的依据（np.sum=pairwise，cumsum=顺序） | 不适用（CPU 语义） |
| 10 | 模板判题与 golden 实现 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts/{AddRmsNormBias,verify_result}.py | A（判题参考实现） | 判题语义（isclose/tol/equal_nan）与 golden 计算链 | 未验证（无 NPU） |
| 11 | 浮点误差累积理论（顺序/树形/Kahan 误差界 O(√D·ulp)/O(logD·ulp)） | 高德纳《计算机程序设计艺术》Vol.2 / IEEE 754 标准文献 | C（教科书） | 大 D 归约误差界推导依据 | 不适用 |

---

## 附：实验脚本位置与复现

- `/tmp/agent06_numerics/exp1_main.py`：主实验（dtype×D×归约×rsqrt×rank），`python3 exp1_main.py`。
- `/tmp/agent06_numerics/exp2_fix.py`：isclose 分档 / cast 舍入 / NaN-Inf / fp16 溢出 / 大 D 误差。
- `/tmp/agent06_numerics/exp3_chunk.py`：chunk 大小敏感性 / 输出量级。
- 依赖：`numpy>=2.0`、`ml_dtypes`（`pip install --user ml_dtypes`）、模板 `AddRmsNormBias.py`（sys.path 注入）。全部 seed 固定，可复现。
- 所有脚本为临时产物，不进入仓库（按 AGENTS.md 约定）。
