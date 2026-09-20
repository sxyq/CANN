# Agent 06 报告：AddRmsNormBias 数值精度与验证方法

- 主题：数值精度理论、误差分析、可复现 CPU 参考测试矩阵设计
- 研究对象：`AddRmsNormBias`（CANN 9.0.0，vector kernel，昇腾 NPU）
- 参考实现：`scripts/AddRmsNormBias.py`（golden，numpy，内部全 float32）
- 判题脚本：`scripts/verify_result.py`（同 dtype 比对 + `isclose` + 失配比例 `tol`）
- 候选实现：`提交/V002/kernel.asc`（两遍扫描，fp16/bf16→fp32→归约→sqrtf→RNE 回写）
- 实验环境：macOS，**无 CANN/NPU**；实验运行于 `/usr/bin/python3`（numpy 2.0.2 + ml_dtypes 0.5.4）
- 口径声明：本报告所有数字均为**本机 CPU / numpy 口径，非 NPU 实测**。NPU 实际行为（尤其 sqrt/CAST/rsqrt 指令精度）需真机确认。

---

## 1. 结论摘要（≤12 条）

1. **BF16 能否满足 rel<1e-3 取决于判定口径**：与「同 dtype BF16 golden」比对（当前 verify 设计）+ `tol=0.001` 容忍时**可通过**；与「FP64 高精度真值」比对时**结构性不可能**（BF16 自身 ulp≈3.9e-3 起，最差单元素相对误差可达 ~16%）。
2. 实测 BF16 vs 同 dtype golden：D=1024 失配 1/8192（0.012%）、D=32768 失配 1517/16777216（0.009%），均 < `tol=0.1%` → **通过**。失配元素均为 1 个 BF16 ulp 在舍入边界的翻转。
3. `isclose(rtol=1e-3, atol=1e-3)` 对 BF16 实质上要求「逐元素精确相等」：BF16 在值≈1 处相邻可表示数相对间距 = 2^-7 ≈ 0.78% > 0.1% rtol，故单 ulp 偏差即判不过。**BF16 通过靠的是 `tol` 容忍，非严格相等**。若判题 `tol=0` 则可能失守边界元素。
4. **D=32768 归约（顺序 vs 分块）误差极小且可忽略**：FP32 累加下，不同分块（512/1024/2048/4096/8192/不分块）平方和相对误差均 < 1e-5（区间 2.8e-7 ~ 7.9e-6），与 `tol`/精度预算相比可忽略。候选 kernel 选 tile=4096/2048 → 误差 7.2e-7/4.4e-7，优异。
5. **`rsqrt` 近似误差预算充足**：即便原始 rsqrt 仅 2^-20（≈9.5e-7）精度，因 `output = y*scale*gamma`、`scale` 误差乘性传递，输出相对误差 ≤ 2^-20 << 1e-3。实测 FP32 `1/sqrt` 路径最大相对误差 6.4e-8；1 次 Newton 迭代后 5.4e-8。
6. **标量 `sqrtf` 风险**：IEEE-754 要求 sqrt 正确舍入，FP32 实测误差 ~6e-8。风险仅在编译器用 fast-math/低精度倒数近似替换 `sqrtf` 或 `1/rms` 除法时显现——属需真机确认项。
7. **FP16 平方溢出边界 `|y| = 255.94`**（y²>65504→Inf）。候选实现先 `CAST_NONE`→FP32 再 Mul，**完全消除**该风险（FP32 上限 3.4e38，FP16 可表示输入平方最大 ~4.3e9，远不溢出）。
8. **`epsilon` 位置写错**：普通输入（|y|~1）影响仅 ppm 级；但当该行 `mean(y²) << eps`（近零残差/小输入）时偏差剧增——量化示例 `mean=1e-3` 时 rms 偏差 -0.46%，`mean=1e-8` 时 -96.5%（输出差约 28 倍）。属**必须避免**的实现错误。
9. **NaN/Inf 传播**：含 NaN 输入 → 输出 NaN（verify 用 `equal_nan=True` 判过）；含 Inf 输入 → `Inf/Inf=NaN` → 整行 NaN（与 golden 一致）。风险点：若实现用 `rsqrt` 且 `rsqrt(Inf)=0`（部分硬件/近似），则 `y*0=0` ≠ NaN，整行判不过 → 需真机验证 `rsqrt(Inf)`。
10. 候选实现数值链与 golden **高度一致**（同 fp32 内部、同 `sqrt(mean+eps)`、同 RNE 回写）；主要风险为 RNE 舍入确认、sqrtf 正确性、`rsqrt(Inf)` 行为、BF16 边界翻转对严格 `tol` 的脆弱性。
11. **FP16/FP32 在两种口径下均稳过**：FP32 vs FP64 真值 maxRel 9.6e-5 < 1e-4；FP16 vs 同 dtype golden 零失配。
12. 绝对误差与相对误差口径：判题用 `|a-b| <= atol + rtol*|b|`（b 为 golden 参考）；小量级输出由 `atol=1e-3` 兜底，接近 0 时 `rtol` 失效、改看 `atol`。

---

## 2. 七个必答问题的逐条回答（含实测数字）

### Q1. BF16 输出能否满足「相对误差 < 1e-3」？

**结论：取决于判定口径。**

- **口径 A（与同 dtype BF16 golden 比对，当前 verify 设计）**：可达标。
  实测（本机 CPU/numpy，NPU 模拟实现 vs golden，BF16，`isclose(1e-3,1e-3)`）：
  - D=1024：`exact=99.98%`，失配 `fail=1/8192`（0.012%），maxAbsDiff=7.812e-3（恰为 1 个 BF16 ulp）。
  - D=32768：`exact=99.98%`，失配 `fail=1517/16777216`（0.0090%），**< `tol=0.001`（0.1%）→ 判过**。
  - 失配元素全部是 BF16 舍入边界上的 1-ulp 翻转（归约求和顺序不同所致）。

- **口径 B（与 FP64 高精度真值比对）**：结构性不可能。
  实测（NPU 模拟 BF16 输出 vs FP64 真值相对误差）：
  - D=64：maxRelErr=3.68e-3，meanRelErr=1.43e-3；
  - D=32768：maxRelErr=1.64e-1，meanRelErr=1.40e-3（最差单元素因输出近 0 而相对误差大）。
  原因：BF16 尾数 7 位（含隐含位 8 位），在值≈1 处的**相对间距 = 2^-7 = 0.78%**，ulp≈3.9e-3。无论实现多精确，BF16 输出与「真值」的距离由格式本身决定，必 ≥ 半个 ulp，且最差相对误差可远超 1e-3。

- **关键辨析**：`isclose(rtol=1e-3, atol=1e-3)` 对 BF16 近似「逐元素精确相等」判据——因为相邻 BF16 值相对差 0.78% > 0.1% rtol，单 ulp 偏差即判不过（除非落入 `atol=1e-3` 兜底的小量级区）。因此 BF16 通过**依靠 `tol=0.001` 的 0.1% 失配容忍**，而非严格相等。若判题 `tol=0`（严格），归约顺序差异造成的边界翻转可能使 BF16 失守。

> 来源：BF16 格式定义（Intel BF16 HW Numerics WP，A；Nick Higham BF16 算术，A；CSDN/ Multigrid 科普，C）；`isclose` 语义（numpy 官方文档，A）；量化数字=本机实测。

### Q2. D=32768 时，顺序归约 vs 分块归约的误差差多少？

实测（本机，FP32 累加，平方和相对误差 vs FP64 真值，rows=64）：

| 分块大小 | 最大相对误差 | 平均相对误差 | 最大绝对误差 |
| --- | --- | --- | --- |
| 512   | 4.14e-7 | 1.17e-7 | 9.03e-3 |
| 1024  | 2.81e-7 | 9.28e-8 | 6.18e-3 |
| 2048  | 4.36e-7 | 1.71e-7 | 9.59e-3 |
| 4096  | 7.23e-7 | 2.55e-7 | 1.60e-2 |
| 8192  | 1.72e-6 | 7.60e-7 | 3.72e-2 |
| 32768（单遍顺序）| 7.90e-6 | 3.03e-6 | 1.72e-1 |
| numpy float32.sum（pairwise）| 1.06e-7 | 3.62e-8 | — |

**结论**：
- 分块与顺序归约的相对误差差异约 **28×**（最差 7.9e-6 vs 最优 2.8e-7），但**绝对值均 < 1e-5 相对**。
- 经 `rms=sqrt(sum/D+eps)` 与 `output=y/rms*...` 后，输出相对误差 ≈ 归约相对误差的一半量级（仍 < 1e-5），远低于 FP32 预算 1e-4。
- 候选 kernel 选 tile=4096（fp16/bf16）/2048（fp32）→ 实测误差 7.2e-7 / 4.4e-7，**精度无忧**。
- 理论支撑：naive 顺序求和误差界 O(εn)，pairwise/分块为 O(ε log n)（Higham 1993，A）；本实验正数求和条件数=1，误差更小。

### Q3. `rsqrt` 近似误差（典型 ≤2^-20）在本题预算下是否足够？

**结论：足够，且余量极大。**

- `output = y * (1/rms) * gamma + bias`，其中 `scale = 1/rms`。`scale` 的相对误差 δ 对输出是**乘性**传递：输出相对误差 ≈ δ（gamma 引入的误差独立且小）。
- 若 `rsqrt` 原始精度 = 2^-20 ≈ 9.54e-7，则输出相对误差 ≤ 9.5e-7 << 1e-3（FP16/BF16 预算），更 << 1e-4（FP32 预算）。
- 实测：FP32 `1/sqrt` 路径最大相对误差 6.4e-8；对 `1/sqrt` 施加 2^-20 偏置（模拟原始 rsqrt）后最大相对误差 1.0e-6；1 次 Newton 迭代（`y' = y*(1.5 - 0.5*s*y*y)`）后降到 5.4e-8。
- 硬件事实：x86 `rsqrtps` ~12 bit、AVX-512 `vrsqrt14` ~14 bit，1 次 Newton 即达 ~24 bit（≈fp32 全精度）（docs.rs/innr，C；arXiv astro-ph/0511062，C）。故即便用 rsqrt 也只需 ≤1 次 Newton 即完全达标。

### Q4. 标量 `sqrtf`（可能被编译为低精度近似）对 bf16/fp16 判定的风险

- IEEE-754（2008）要求 `sqrt` 正确舍入（round-to-nearest），FP32 `sqrtf` 相对误差 ≤ 0.5 ulp ≈ 6e-8。实测 FP32 `1/sqrt` 路径 6.4e-8，与理论一致。
- **风险场景**：编译开启 `-ffast-math`、用倒数近似（`1/(rms)` 经 `rsqrt` + 不足 Newton 次数）或标量 sqrt 被替换为更低精度实现。此时误差可能放大到 >> 2^-20，若达 ~1e-3 量级则直接威胁 BF16/FP16 的 `isclose(1e-3)` 判据。
- 候选 kernel 用 `sqrtf(square_sum/dim + epsilon)`（C 标准库标量 sqrt），通常正确舍入；但**真机上该调用最终映射到哪条指令、是否经 fast-math 改写，无法在 CPU 推断**，列为需真机确认项。
- 缓解：判题对 FP16/BF16 用 1e-3，即使 sqrt 退化到 1e-3 也仅压线；FP32 用 1e-4，需 sqrt 误差 < 1e-4（正确舍入轻松满足）。

### Q5. FP16 平方和的溢出边界：|y| 多大时 y² 超过 65504？转 FP32 累加是否完全消除风险？

- FP16 最大有限值 = 65504；平方溢出阈值 `|y| = sqrt(65504) = 255.9375`。
- 实测：`|y|=256/300/1000/1e4` 在 FP16 中平方均得 `inf`，在 FP32 中分别为 65536/90000/1e6/1e8（无溢出）。
- **候选实现安全**：`MakeValue` 先 `Cast(x, fp32, CAST_NONE)`（精确，FP32 是 FP16 超集），再 `Add` 再 `Mul(y,y)`——平方在 **FP32** 中完成。FP16 可表示的输入最大 65504，其平方 4.3e9 << FP32 上限 3.4e38，**完全消除溢出风险**。
- 仅在「错误地在 FP16 中做 Mul」时，`|y|≥256` 即 Inf 并污染整行。候选实现无此写法 → 风险为「已知安全」。

### Q6. `epsilon` 位置实现错误会导致多大误差？（量化）

- 正确：`rms = sqrt(mean(y²) + eps)`；错误：`rms = sqrt(mean(y²)) + eps`。
- 实测（本机，随机 |y|~1 输入）：错误位置 maxRelErr=3.29e-2（个别近零输出元素驱动），meanRelErr=6.50e-6。
- 量化单值示例（固定其他，仅看 rms 偏差）：
  - `mean(y²)=0.5, eps=1e-5`：正确 rms=0.70711385，错误 rms=0.70711678，**rms 相对差 +4.1e-6**。
  - `mean(y²)=1e-3, eps=1e-5`：正确 0.03178050，错误 0.03163278，**rms 相对差 -4.65e-3（输出 ~ -0.46%）**。
  - `mean(y²)=1e-8, eps=1e-5`：正确 0.00316386，错误 0.00011000，**rms 相对差 -96.5%（输出偏差约 28 倍）**。
- 极端（近零残差/极小输入，`mean << eps`）：maxRelErr=27.8（=2784%），即输出差近 28 倍。
- **结论**：普通输入下影响微小（ppm~0.5%），但近零行会灾难性错误。**必须与 golden 一致把 eps 放在 sqrt 内**（PyTorch RMSNorm 官方文档亦为 `sqrt(eps + mean(x²))`，B）。

### Q7. NaN / Inf 输入的传播行为

- 按公式：含 NaN 的 `y` → `y²=NaN` → `sum=NaN` → `rms=NaN` → `output=NaN`。含 Inf 的 `y` → `y²=Inf` → `sum=Inf` → `rms=Inf` → `y/rms = Inf/Inf = NaN` → 整行 NaN。
- 实测（golden，FP32 内部）：
  - 1 个 NaN 输入 → 输出含 NaN（`equal_nan=True` 故判过）；
  - 1 个 Inf 输入 → 输出全行 NaN（与 golden 的 NaN 一致）；
  - 全 0 输入（`eps=1e-5` 保驾）→ 输出 = bias，均=0.0122，等于 bias 均值，正确。
- **实现风险**：若用 `rsqrt` 且 `rsqrt(Inf)=0`（部分硬件/近似实现如此），则 `y*0 = 0`（而非 NaN），与 golden 的 NaN **不一致** → 该元素判不过（`equal_nan` 要求 NaN）。候选 kernel 用 `sqrtf` 而非 rsqrt，理论上 `sqrtf(Inf)=Inf`、`1/Inf=0`、`Inf*0=NaN` 仍得 NaN；但**rsqrt 路径与标量除法在 Inf 处的具体行为需真机确认**。
- 另：若 `eps=0` 且整行全 0，则 `rms=0`、`0/0=NaN`（golden 也 NaN，一致）；默认 `eps=1e-5` 下无此问题。

---

## 3. 数值链分析（对照 `提交/V002/kernel.asc`）

| 步骤 | golden（`AddRmsNormBias.py`） | 候选 kernel（`V002/kernel.asc`） | 一致性 |
| --- | --- | --- | --- |
| 输入提升 | `x.astype(float32)` | `Cast(x, fp32, CAST_NONE)`（FP16/BF16） | ✅ 精确（FP32 为子集超集） |
| 残差加 | `y = x_f + r_f` | `Add(value, x_f, r_f)` | ✅ |
| 平方 | `y*y`（FP32） | `Mul(value, value, value)`（FP32，先提升再乘） | ✅ 无 FP16 溢出 |
| 归约 | `np.mean(y*y)`（numpy pairwise，FP32 内部） | `ReduceSum` 分块 → 标量 `total += sum`（FP32，顺序累加块和） | ⚠️ 顺序差异，仅 BF16 边界 1-ulp 翻转 |
| 归一化分母 | `rms = sqrt(mean + eps)` | `rms = sqrtf(square_sum/float(dim) + eps)` | ✅ `mean = sum/D`，等价 |
| 缩放 | `y / rms` | `Muls(value, 1.0f/rms)`（每行 1 个标量 scale） | ✅ 整行一致 |
| 仿射 | `out = (y/rms)*gamma + bias` | `((y*scale)*gamma) + bias`（FP32） | ✅ 运算顺序一致 |
| 回写 | `out.astype(orig_dtype)`（RNE） | `Cast(dst, src, CAST_RINT)`（RNE） | ✅ 假定 CAST_RINT=RNE（需真机确认） |

**与 golden 的差异与风险（仅 4 项）**：
1. **归约求和顺序**：golden 用 numpy 的 pairwise 求和；kernel 用「块内顺序 + 块间顺序累加标量」。二者 FP32 结果差 ≤ 1e-5，仅在 BF16 末级舍入边界造成 1-ulp 翻转（实测失配 0.009%–0.012%）。
2. **末级舍入模式**：`CAST_RINT` 须为 RNE（round-to-nearest-even）才与 numpy `astype` 默认一致。若 NPU 实际为截断/就近奇数/随机，BF16 失配率会上升（仍多 ≤ 1 ulp）。
3. **`sqrtf` 实际精度**（Q4）：映射到正确舍入则误差 6e-8；若被 fast-math 改写则存疑。
4. **`rsqrt(Inf)` / 标量除法在 Inf 处行为**（Q7）：影响 NaN/Inf 用例。

> 其余（FP32 内部计算、eps 在 sqrt 内、整行单一 scale）均与 golden 一致，无额外数值风险。

---

## 4. 实验设计与实测结果

> 运行命令（本机）：
> `/usr/bin/python3 /Users/sunyiyang/Desktop/Project/cann/调研/调研2/数值参考脚本/numerics_experiments.py`
> 环境：`/usr/bin/python3`，numpy 2.0.2，ml_dtypes 0.5.4。所有数字为 CPU/numpy 口径，非 NPU 实测。

### 实验 A：golden 复现 + 测试矩阵跑通
- 目的：确认 golden 实现可在矩阵上运行，并量化「golden(BF16/FP16) vs FP64 真值」的固有格式误差。
- 命令：见脚本「实验 A」。
- 结果（maxRelErr vs FP64 真值）：FP32 D=64 → 4.6e-6；FP16 D=64 → 4.7e-4；BF16 D=64 → 3.7e-3；FP32 D=32768 → 1.8e-2（个别近零元素）；BF16 D=32768 → 1.6e-1。
- 结论：BF16/FP16 自身格式误差远大于其判题预算（1e-3）与 FP64 真值比对时必失；但判题比对的是同 dtype golden，故不冲突。

### 实验 B：两种判定口径
- 目的：分别用「vs FP64 真值」与「vs 同 dtype golden」评估 NPU 模拟实现。
- 命令：见脚本「实验 B」，NPU 模拟 = fp32 内部 + 分块归约 + RNE 回写。
- 结果：
  - FP32 D=1024：vsFP64 maxRel 9.6e-5（<1e-4 ✅）；vsGolden exact 13.27%、fail=0。
  - FP16 D=1024：vsFP64 maxRel 4.8e-4（<1e-3 ✅）；vsGolden exact 99.96%、fail=0。
  - BF16 D=1024：vsFP64 maxRel 5.1e-3（>1e-3，格式固有）；vsGolden exact 99.98%、fail=1（maxAbsDiff 7.8e-3=1 ulp）。
  - BF16 D=32768：vsFP64 maxRel 1.74；vsGolden exact 99.98%、fail=1517/16.7M（0.009% < tol 0.1%）→ **通过**。
  - FP16 D=32768：vsGolden fail=0 → **通过**。
- 结论：FP16/FP32 双口径稳过；BF16 靠 `tol` 容忍边界翻转通过（详见 Q1）。

### 实验 C：D=32768 分块归约误差
- 目的：量化分块大小对平方和的影响（见 Q2 表）。
- 命令：见脚本「实验 C」。
- 结果：block∈{512..32768} 相对误差 2.8e-7~7.9e-6；numpy pairwise 1.1e-7。
- 结论：分块选择对精度无实质影响，候选 kernel tile=4096/2048 误差 7.2e-7/4.4e-7，优异。

### 实验 D：sqrt vs rsqrt
- 目的：评估用 rsqrt 替代 `1/sqrt` 的误差（见 Q3）。
- 命令：见脚本「实验 D」。
- 结果：FP32 `1/sqrt` 路径 maxRel 6.4e-8；对 `1/sqrt` 施加 2^-20 偏置 → 1.0e-6；1 次 Newton → 5.4e-8。
- 结论：rsqrt（即便 2^-20 原始精度）对输出误差 ≤ 9.5e-7 << 1e-3，预算充足。

### 实验 E：epsilon 位置
- 目的：量化 eps 放在 sqrt 内 vs 外的偏差（见 Q6）。
- 命令：见脚本「实验 E」。
- 结果：普通输入 maxRel 3.3e-2（近零输出驱动），mean 6.5e-6；极小输入 mean<<eps 时 maxRel 27.8；单值示例 mean=1e-3 → rms 差 -0.46%，mean=1e-8 → -96.5%。
- 结论：普通输入影响微小；近零行灾难性；必须与 golden 一致（eps 在 sqrt 内）。

### 实验 F：FP16 溢出边界
- 目的：确定平方溢出阈值并验证 FP32 提升消除风险（见 Q5）。
- 命令：见脚本「实验 F」。
- 结果：`|y|>=255.94` 时 FP16 平方=Inf；FP32 路径对所有测试值（256/300/1000/1e4）均无溢出。
- 结论：候选实现先提升 FP32 再平方，完全消除 FP16 溢出风险。

### 实验 G：NaN/Inf 传播
- 目的：确认特殊值传播与判题兼容性（见 Q7）。
- 命令：见脚本「实验 G」。
- 结果：NaN→NaN（equal_nan 判过）；Inf→整行 NaN；全 0→输出=bias（正确）。
- 结论：公式行为一致；风险仅在 rsqrt(Inf)=0 的错误实现路径，需真机确认。

---

## 5. 精度风险清单

| # | 风险 | 触发条件 | 量化影响 | 缓解措施 | 需真机验证 |
| --- | --- | --- | --- | --- | --- |
| R1 | BF16 末级舍入边界翻转导致 `isclose(1e-3)` 失配 | 归约顺序差异使 BF16 舍入跨边界 | 单 ulp（~0.78% 相对）偏差；实测失配 0.009%–0.012%（<tol 0.1% 通过） | 保证 FP32 内部与 golden 一致；依赖 `tol` 容忍 | 否（CPU 已量化，NPU 行为应一致） |
| R2 | 判题 `tol=0`（严格）时 BF16 失守 | 严格逐元素相等 + 边界翻转 | 部分元素判不过 | 推动判题保留 `tol>=1e-3`；或归约改用 pairwise 对齐 numpy | 建议 |
| R3 | `sqrtf` 被 fast-math/低精度近似替换 | 编译器优化、标量 sqrt 映射异常 | 若误差达 ~1e-3 直接威胁 FP16/BF16 判据 | 关闭 fast-math；确认 sqrt 正确舍入 | **是** |
| R4 | `rsqrt(Inf)=0` 导致 NaN 用例输出 0 | 用 rsqrt 且 Inf 输入 | 整行 0 ≠ NaN，判不过 | 用 `sqrtf` 路径（候选已用）；或显式处理 Inf | **是** |
| R5 | `CAST_RINT` 非 RNE（截断/其他） | NPU 舍入模式与 numpy 默认不同 | BF16 失配率上升（仍 ≤1 ulp） | 确认 CAST_RINT=RNE | **是** |
| R6 | `epsilon` 位置写错 | 实现把 eps 放在 sqrt 外 | 近零行输出偏差达 28 倍 | 严格按 golden：`sqrt(mean+eps)` | 否（实现审查可定） |
| R7 | FP16 在 FP16 中平方溢出 | 错误地在 FP16 做 Mul 且 |y|≥256 | 整行 Inf 污染 | 先提升 FP32 再平方（候选已做） | 否（审查可定） |
| R8 | 极小/近零残差行数值不稳定 | `mean(y²)<<eps` | eps 位置错误时灾难性；正确位置则安全 | 正确位置 + 默认 eps=1e-5 | 否 |

---

## 6. 未确认事项

1. **真机 sqrt/rsqrt 指令精度**：`sqrtf`、`1/rms` 除法在 NPU 上是否映射到正确舍入指令，是否受编译优化影响（R3/R4）。
2. **`CAST_RINT` 舍入模式**：AscendC `CAST_RINT` 是否为 RNE（R5）。需对照 CANN 文档/真机打印。
3. **判题最终 `case_output_specs`**：模板默认 fp16/rtol=1e-3/atol=1e-3/tol=1e-3；BF16/FP32 各用例的实际 rtol/atol/tol 以赛题发布为准（本报告按「fp16/bf16<1e-3、fp32<1e-4」推演）。
4. **多核/多 tile 归约一致性**：kernel 按 `outer` 行分块到多 block，行内归约顺序固定，跨行独立，无跨行一致性风险；但多 block 间若有行间归约则不在此题（每行独立）。未实测 NPU 多核路径。
5. **BF16 子规范（subnormal）**：NPU 是否 flush-to-zero；影响极小量级输出，未实测。

---

## 7. 来源表

| 编号 | 来源 | URL | 访问日期 | 证据等级 |
| --- | --- | --- | --- | --- |
| S1 | numpy `isclose` 官方文档（语义 `|a-b|<=atol+rtol*|b|`，`equal_nan`） | https://numpy.org/doc/stable/reference/generated/numpy.isclose.html | 2026-09-11 | A |
| S2 | Higham, "The accuracy of floating point summation"（naive O(εn) vs pairwise O(ε log n)） | https://en.wikipedia.org/wiki/Pairwise_summation | 2026-09-11 | A（文献） |
| S3 | BF16 硬件数值定义白皮书（Intel，8 指数/7 尾数，RNE 默认，无 subnormal） | https://www.intel.com/content/dam/develop/external/us/en/documents/bf16-hardware-numerics-definition-white-paper.pdf | 2026-09-11 | A |
| S4 | Nick Higham, "What Is Bfloat16 Arithmetic?"（unit roundoff 2^-7≈7.8e-3） | https://nhigham.com/category/what-is/page/9/ | 2026-09-11 | A |
| S5 | BF16/FP16 格式科普（BF16 相对误差≈0.4%、近 1 间隔 1/128≈0.0078） | https://blog.csdn.net/ZZZZZLJ/article/details/152617970 | 2026-09-11 | C |
| S6 | 浮点格式精度（fp16 2^-11、bf16 2^-8、fp32 2^-24） | https://multigrid.ai/learn/floating-point-formats | 2026-09-11 | C |
| S7 | rsqrt 硬件精度（x86 ~12bit、AVX-512 ~14bit，1 次 Newton→~24bit） | https://docs.rs/innr/0.2.0/innr/fast_math/index.html | 2026-09-11 | C |
| S8 | rsqrt 近似与 Newton 迭代（SSE RSQRTSS ~12bit，1 次 Newton→24bit） | https://arxiv.org/pdf/astro-ph/0511062v1 | 2026-09-11 | C |
| S9 | Fast inverse square root（Newton 公式 `y'=y*(1.5-0.5*x*y*y)`，迭代收敛） | https://en.algorithmica.org/hpc/arithmetic/rsqrt/ | 2026-09-11 | C |
| S10 | PyTorch `nn.RMSNorm` 官方文档（规范 `RMS(x)=sqrt(eps + mean(x²))`，eps 在 sqrt 内） | https://docs.pytorch.org/docs/2.6/generated/torch.nn.RMSNorm.html | 2026-09-11 | B |
| S11 | IEEE 754 要求 `sqrt` 正确舍入（标准） | https://en.wikipedia.org/wiki/IEEE_754 | 2026-09-11 | A（标准） |
| S12 | 本机实测数字（CPU/numpy，非 NPU） | `/Users/sunyiyang/Desktop/Project/cann/调研/调研2/数值参考脚本/numerics_experiments.py` | 2026-09-11 | 本机实测 |

> 说明：A=官方文档/官方仓库/标准；B=官方教程/官方测试；C=社区文章/论坛；D=仅搜索摘要。本报告中所有具体数字（实验 A–G）均标注为「本机 CPU/numpy 口径，非 NPU 实测」，与引用结论明确区分。
