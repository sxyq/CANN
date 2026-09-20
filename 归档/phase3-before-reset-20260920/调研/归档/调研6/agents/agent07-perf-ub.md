# Agent 7 报告：AddRmsNormBias 性能 / UB / 硬件架构（量化推算）

> 角色：10 个并行子代理之一，负责「怎么分块、怎么分核、带宽账怎么算」。
> 范围：硬件参数复核、带宽账、tiling/分核、32B 对齐、标量同步、性能测量方法。
> **本机 macOS，无 CANN、无 NPU。本报告所有带「推算」字样的数字均为基于公开参数与公式推算，未在任何真机实测；带「实测」字样的内容均引自第三方来源并标注。**
> 平台字符串说明：判题形态给出的默认 `SOC_ARCH="dav-2201"`；本报告的硬件数字（UB=192KB、24 AIC / 48 AIV）对应昇腾 910B 系列（架构代号 `dav-c220`）。两者映射关系以 Agent 1/2 后续核实为准，本报告不擅自认定 `dav-2201 == 910B`，仅按 910B 系列硬件参数做推算。

---

## 1. 执行摘要（关键数字）

1. **UB 容量**：昇腾 910B1/910B2 每 AI Core 的 Unified Buffer = **192 KB**（= 196608 B）；架构代号 `dav-c220`（来源 S181/S182，等级 B，非官方但源自芯片规格表）。
2. **核数**：910B2 = **24 个 AI Core / 48 个向量核（AIV）**（来源 S182，等级 B）。判题 `availableCoreNum` 来自 Host，实测物理向量核即 48。
3. **向量单元吞吐粒度**：每次 repeat 读 **256 B = 8 × 32 B datablock**；fp16 单次迭代 mask 上限 **128 元素**，fp32 上限 **64 元素**（来源 S185/S186，官方文档，等级 A）。
4. **GM↔UB 带宽量级**：910B HBM 标称 **1.6 TB/s**（来源 S183，华为开发者联盟，等级 B）；社区/第三方数据在 **1.2–2.0 TB/s** 间浮动（来源 S184/S197），本报告统一用 **1.6 TB/s** 做带宽账推算。
5. **带宽账结论**：两遍扫描 S1 总访存 = **5·N·s**；单遍 S2 = **3·N·s**；S2 比 S1 **省 40% GM 流量**。RMSNorm 类是**典型 memory-bound**（算术强度 ~0.8–1.3 FLOPs/Byte，远低于 roofline 拐点 ~200 FLOPs/Byte）。
6. **S2 单遍可行 D 上限（UB=192KB，扣 16KB 开销后可用 ~180KB）**：fp16/bf16 **D ≤ 22528**；fp32 **D ≤ 11264**（严格模型，需同时容纳 y+output+gamma+bias 四份）；仅存 y 单缓冲的宽松上限为 fp16/bf16 98304、fp32 49152。
7. **必须回退 S1 的区间**：fp16/bf16 当 **D ≥ 24576**（尤其 32768）；fp32 当 **D ≥ 12288**（含 32768）。这些 D 下整行放不进 UB，单遍 S2 退化。
8. **32B 对齐风险真实存在**：DataCopy 搬运粒度是 32 B，行尾未对齐会把邻居 32 B 缓存行一起搬入/写脏（来源 S194，华为开发者联盟，等级 B）。按 `k·D·s ≡ 0 (mod 32)` 分 k 行/核可消除跨核踩踏，正确；更简做法是 output 每行 stride 补齐到 32 B 整数倍。
9. **ReduceSum 含标量同步等待**：WholeReduceSum 内部有 scalar wait 会阻塞流水（来源 S192，华为专家确认，等级 B/C）；改用 `BlockReduceSum + WholeReduceSum` 组合（256 个 fp32 累加 8.44 µs vs 两次 WholeReduceSum 13 µs，来源 S193，官方最佳实践，等级 A）。
10. **测量手段**：真机用 `msprof op ./kernel_binary` → `OpBasicInfo.csv` 的 `Task Duration(us)` 为 kernel 总时长，`PipeUtilization.csv` 看 `aiv_time / mte2 / mte3` 占比（来源 S188/S189/S190）。

---

## 2. 硬件参数表

> 等级含义：A=官方文档（hiascend.com / 华为开发者联盟技术文）；B=源自芯片规格表或华为专家确认的社区内容；C=社区教程/博客（含具体数值但非官方口径）；D=无法核实。
> 「是否官方」：标注该数字是否直接来自华为官方发布。

| 参数 | 值 | SoC / 平台 | 来源 | 等级 | 是否官方 |
|---|---|---|---|---|---|
| UB 容量（每 AI Core） | 192 KB（196608 B） | Ascend 910B1 / 910B2 (dav-c220) | S181, S182 | B | 否（源自 NPUTargetSpec.td 规格表，社区转引） |
| UB 容量（每 AI Core） | 256 KB | Ascend 310B 系列 (dav-m300) | S181 | B | 否 |
| AI Core 数 | 24 | Ascend 910B2 | S182 | B | 否（arxiv 论文） |
| 向量核（AIV）数 | 48（= 2 × AI Core） | Ascend 910B2 | S182 | B | 否 |
| Cube 核（AIC）数 | 24 | Ascend 910B2 | S182 | B | 否 |
| 向量单元 repeat 宽度 | 256 B = 8 × 32 B datablock | DaVinci (AIV) | S185, S186 | A | 是（官方 API 文档） |
| fp16 单次迭代 mask 上限 | 128 元素 | AIV | S185 | A | 是 |
| fp32 单次迭代 mask 上限 | 64 元素 | AIV | S185 | A | 是 |
| GM/HBM 带宽（标称） | 1.6 TB/s | Ascend 910B | S183 | B | 否（华为开发者联盟对比表） |
| GM/HBM 带宽（第三方） | 1.2 TB/s | Ascend 910B（910B1/早期） | S184 | C | 否（经销商页） |
| GM/HBM 带宽（社区） | 1.6–1.8 TB/s（950PR 亦同量级） | A2/A3/950 | S197 | C | 否 |
| L2 Cache | 192 MB（A2/A3） | 片上 | S197 | C | 否 |
| 峰值 FP16 算力 | 320 TFLOPS | Ascend 910B | S183 | B | 否 |
| 向量 add/reduce 相对成本 | WholeReduceSum 含 scalar wait，阻塞流水 | AIV | S192 | B/C | 否（华为专家口述） |
| DataCopy 对齐粒度 | 32 B（datablock） | MTE2/MTE3 | S194 | B | 否（华为开发者联盟） |
| 峰值 MTE 效率建议对齐 | 512 B（cacheline） | MTE2/MTE3 | S194 | B | 否 |
| Double Buffer 加速比 | ~1.6× | AIV 流水 | S194 | B | 否 |
| msprof 单算子命令 | `msprof op ./binary` | CANN | S188, S190 | A | 是（官方 Profiling 文档） |

---

## 3. 带宽账（S1 vs S2）

> 符号：N = 元素总数（2D 时 N = outer×D；3D/4D 时 outer = 除最后一维外各维之积）；s = `sizeof(T)`（fp16/bf16 = 2，fp32 = 4）。

### 3.1 两遍扫描 S1（公式 + 代入）

- Pass 1：读 `x`（N·s）+ 读 `residual`（N·s），算平方和（累加在寄存器/UB，不写回 GM）。
  - 访存 = 2·N·s
- Pass 2：再读 `x`（N·s）+ 再读 `residual`（N·s）重算 `y = x + residual`，做归一化，写 `output`（N·s）。
  - 访存 = 2·N·s（读）+ 1·N·s（写）= 3·N·s
- **S1 总 GM 访存 = 5·N·s** ✅（与题面一致）

### 3.2 单遍保存中间结果 S2（公式 + 代入）

- 一次读 `x`（N·s）+ 读 `residual`（N·s），在 UB 内保留 `y`，算得 `rms` 后直接从 UB 取 `y` 归一化，写 `output`（N·s）。
- **S2 总 GM 访存 = 3·N·s** ✅

### 3.3 带宽节省比例

```
节省比例 = (S1 - S2) / S1 = (5·N·s - 3·N·s) / (5·N·s) = 2/5 = 40%
```

**结论：S2 比 S1 省 40% 的 GM 流量。** 注意：节省的是「访存字节数」，不是时间；是否转化为时间收益取决于瓶颈是否真在带宽（见 3.5）。

### 3.4 S2 可行的 D 上限（UB=192KB）

S2 要在 UB 内保留整行 `y`，且还需容纳 `output / gamma / bias` 等。设 UB 裸容量 `U = 196608 B`，预留开销 `OV = 16 KB = 16384 B`（reduce 临时、双缓冲控制、partial sum），可用 `U' = 180224 B`。

**严格模型**（同时容纳 y + output + gamma + bias 四份，即 `4·D·s + OV ≤ U`）：

| dtype | s | 上限 D（严格, 4·D·s≤180224） | 上限 D（裸容量, 仅 y 单缓冲 D·s≤196608） |
|---|---|---|---|
| fp16 | 2 | 22528 | 98304 |
| bf16 | 2 | 22528 | 98304 |
| fp32 | 4 | 11264 | 49152 |

**说明**：题面提示「D·sizeof(FP32) 或 D·sizeof(T) 能塞进 UB」是「仅存 y 单缓冲」的宽松判据（右列）；但真实 S2 还需 output/gamma/bias 各一份，故**按左列（严格模型）做可行性判断**。

**给定 D 区间 [64, 32768] 的 S1/S2 可行性矩阵**（基于严格模型，扣 16KB 开销）：

| dtype | D=64 | D=576 | D=1024 | D=11264 | D=12288 | D=22528 | D=24576 | D=32768 |
|---|---|---|---|---|---|---|---|---|
| fp16/bf16 | S2 ✅ | S2 ✅ | S2 ✅ | S2 ✅ | S2 ✅ | S2 ✅（紧） | S2 ⚠️（4×=192KB，零开销才可行） | **S1**（4×=256KB > 192KB） |
| fp32 | S2 ✅ | S2 ✅ | S2 ✅ | S2 ✅（紧） | **S1**（4×=192KB 紧） | **S1** | **S1** | **S1** |

> ⚠️ 紧边界：恰好等于 UB 容量，需零额外开销，实际不建议，按 S1 处理更稳。

### 3.5 是否 memory-bound（roofline 推算）

```
算术强度 AI = FLOPs / 字节
  RMSNorm 每元素约 6–8 FLOPs（add + square + mean/sqrt + mul + add）。
  S1: AI = 8 / (5·s)；fp16(s=2) → AI ≈ 0.8 FLOPs/Byte
  S2: AI = 8 / (3·s)；fp16(s=2) → AI ≈ 1.3 FLOPs/Byte
roofline 拐点 AI* = 算力峰 / 带宽峰 = 320e12 / 1.6e12 ≈ 200 FLOPs/Byte
```

**结论**：AI（0.8–1.3）≪ AI*（≈200），**AddRmsNormBias 是典型 memory-bound 算子**。因此：
- 减少 GM 流量（S2 省 40%）方向正确；
- 但单核内 MTE 搬运效率（对齐、双缓冲、合并 DMA）往往比「少算一遍」更决定实际收益；
- 当 outer 极小（形态 a）时总数据量太小，瓶颈转为 kernel 启动 + 标量同步开销，带宽账收益可忽略。

### 3.6 推算带宽下界时间（非实测，仅供量级参考）

取 `BW = 1.6 TB/s`（带宽下界，忽略 MTE 效率损失与同步）：

| 形态 | N | dtype | S2 流量 | 带宽下界 t_S2 | S1 流量 | 带宽下界 t_S1 |
|---|---|---|---|---|---|---|
| (b) outer=8192,D=1024 | 8.39e6 | fp16 | 50.3 MB | 0.031 ms | 83.9 MB | 0.052 ms |
| (c) outer=48,D=1024 | 4.92e4 | fp16 | 295 KB | 0.18 µs | 492 KB | 0.31 µs |
| (a) outer=8,D=32768 | 2.62e5 | fp16 | 1.57 MB | 0.98 µs | 2.62 MB | 1.64 µs |
| (d) outer=48,D=32768 | 1.57e6 | fp32 | 18.9 MB | 11.8 µs | 31.5 MB | 19.7 µs |

> 上述为纯带宽推算下界；真实时间 = 带宽 + 启动 + 标量同步 + MTE 非满效率，通常数倍于下界。

---

## 4. 四种形态的 tiling 与分核建议

> 通用约定：`cores = 48`（910B2 向量核）。UB=192KB，可用 ~180KB（扣 16KB 开销）。gamma/bias 若 D 可整行缓存则每核缓存一份、跨行广播；否则按 tile 随用随读。

### 4.1 (a) 小 outer、大 D（outer=1~8，D=32768）

**矛盾**：outer ≪ cores，按行分核填不满 48 核。必须把 D 维切开分给多核（inter-row D-tiling）。

**fp16 示例**（row = 32768×2 = 64 KB）：
- D-tile 大小 `t`：要求 `4·t·2 ≤ 180KB → t ≤ 22528`，取 **t = 16384（32KB/tile）**。
- tiles/行 = 32768 / 16384 = **2**；总 D-tile 数 = outer×2 = **2 ~ 16**。
- 仅 2–16 个 tile，远少于 48 核 → **最多 16 核忙，32 核空**。这是 outer 极小时的结构性欠利用，无法靠 tiling 消除。
- UB 预算（每 tile，单缓冲、gamma/bias 按 tile 随读）：y_tile 32KB + output_tile 32KB + gamma_tile 32KB + bias_tile 32KB + reduce ~4KB ≈ **132KB ≤ 192KB** ✅。
- **双缓冲**：可开 ping-pong 的 x/residual 输入缓冲（2×32KB）进一步隐藏 MTE，但受限于总 UB，建议仅对输入做 2× 双缓冲，gamma/bias 单份复用。
- **瓶颈**：总数据量 tiny（outer=8 → N=262144，fp16 仅 0.5MB），**带宽不是瓶颈，kernel 启动 + 每 tile ReduceSum 标量同步才是主耗**。
- **建议**：outer≤8 时不要硬上 S2 跨核两阶段归约（跨核合并 partial sum 的同步代价 > 带宽收益）。直接 **S1 两遍 + 行/D 混合分核** 更简单稳妥；若要单遍，仅在 single-core 内顺序处理整行（见 4.4 d1）而非跨核切 D。

### 4.2 (b) 大 outer、小 D（outer=8192，D=64~576）

**矛盾**：outer ≫ cores，纯行并行最自然。

- 每行一 tile（D 小，整行轻松进 UB）：fp16 行 128B(D=64) ~ 1152B(D=576)；fp32 256B ~ 2304B。
- 每核行数 = 8192 / 48 = **170.67** → 分配：**32 核担 171 行，16 核担 170 行**（余数 32 行均摊给前 32 核，尾核多担 1 行）。
- UB 预算（D=576, fp16）：y 1152B + output 1152B + gamma 1152B + bias 1152B + reduce ~1KB ≈ **~5.8 KB ≪ 192KB**，富余；可放大 single-buffer 同时持多行做流水。
- **关键优化（合并 DMA）**：每行太小，逐行 DataCopy 无法打满 MTE。按 **K 行合并成一次搬运**，使单次搬运 ≥ 512 B（cacheline）：
  - D=64 fp16：128B/行 → **K=4 行/次 DMA = 512B**；
  - D=576 fp16：1152B/行 → K=1 已 ≥512B，无需合并。
  - gamma/bias：**每核缓存整份 D 向量一次**，跨 170 行广播（避免每行重读）。
- **双缓冲**：开 ping-pong（两份输入 buffer + 两份 y/output），CopyIn 与 Compute 重叠，预期 ~1.6× 加速（来源 S194）。
- **瓶颈**：bandwidth（memory-bound），但受 tiny-DMA 效率拖累；合并 DMA + 双缓冲是主优化点。形状巨大（N=8.39e6）使带宽收益显著（见 3.6：S2 省 ~0.02ms 量级）。

### 4.3 (c) outer≈核数（outer=48，D=1024）

**理想形态**：48 行 = 48 核，**恰好 1 行/核**，无负载不均、无跨核依赖。

- fp16 行 = 1024×2 = 2 KB；fp32 = 4 KB。UB 占用：y 2KB + output 2KB + gamma 2KB + bias 2KB + reduce ~1KB ≈ **9 KB ≪ 192KB**。
- 单核流程：读 x(2KB)+residual(2KB) → y；`BlockReduceSum + WholeReduceSum` 得 rms（1 次标量读回）；读 gamma+bias；`output = y/rms*gamma + bias`；写 output(2KB)。
- **S2 单遍天然可行**（整行留 UB）。**双缓冲**可选（数据太小，收益有限，但开 ping-pong 无害）。
- **瓶颈**：标量同步（每行 1 次 GetValue）+ 计算；近 memory-bound 但数据量极小（N=49k），实际耗时主要由 48 核同步与启动决定。
- **建议**：直接用 S2；reduce 必须用 `BlockReduceSum+WholeReduceSum`（来源 S193），勿用纯 `WholeReduceSum`。

### 4.4 (d) D 超过单 tile 容量（D=32768 且 fp32，一行 128KB）

一行 fp32 = 32768×4 = **131072 B = 128 KB**。128KB < 192KB（单行 y 能进 UB），但 y+output+gamma+bias 四份 = 512KB > 192KB，故**不能「整行四份同留」**，必须 intra-row 切 tile + 复用。

**d1：行并行（outer ≥ cores，如 outer=48）— S2 仍可做：**
- 核内分两阶段，UB 预算（y 整行保留 + 子 tile 工作）：
  - Phase 1（算 rms）：D 按 **t1 = 8192（32KB）** 切 tile，循环读 x_tile/residual_tile → 算 y_tile 存入 **y_full 区（128KB）**，并累加平方 partial sum；循环 4 次得整行 rms。
  - Phase 2（算 output）：D 按 **t2 = 4096（16KB）** 切子 tile，循环读 gamma_sub/bias_sub，从 y_full 取 y_sub 算 `output_sub = y_sub/rms*gamma_sub + bias_sub`，写回。
  - UB 占用 ≈ y_full 128KB + 子 tile 工作（y/x/res/gamma/bias/output 各 16KB = 96KB）+ reduce ~KB ≈ **~225KB** → 略超 192KB，**需进一步缩小 t2 到 2048（8KB）使工作集 = 4×8=32KB，总 ≈ 162KB ✅**。或只保留 y_full + 单份 working tile（不双缓冲）≈ 128+32 = 160KB ✅。
  - **GM 流量 = 读 x,residual 各 1 次 + 读 gamma,bias 各 1 次 + 写 output 1 次 = 3·N·s = S2** ✅（单遍成功）。
- **瓶颈**：UB 极紧张、流水受限；但带宽账达成 S2（省 40%）。

**d2：行被切给多核（outer < cores，如形态 a 的 fp32 版）— 跨核合并 rms：**
- 一行 4 个 D-tile 落在 4 个不同核 → 必须跨核合并 partial sum（GM workspace 或 EnQue/DeQue）。
- 跨核合并需要「先收齐 4 个 partial → 算 rms → 再分发」，**等价于再做一遍读取**，流量退化为 S1。
- **建议**：outer < cores 且 D 超 UB 时，**直接 S1 两遍**（实现简单、无跨核归约同步），不要追求 S2。

---

## 5. 分核与负载均衡（含 32B 对齐风险评估）

### 5.1 按行分核 vs 按 tile 分核（跨行）

- **按行分核（推荐，outer 大时）**：每行独立、无跨核数据依赖；归约在核内完成，rms 天然 per-row。适合形态 (b)(c)，以及 (d1) outer≥cores。
- **按 tile 分核（跨行切 D）**：仅当 outer ≪ cores（形态 a）被迫使用；引入跨核归约依赖（同行的多个 D-tile 需合并 partial sum）。**代价高、收益低**，仅作兜底。
- **余数行分配**：`rows_per_core = floor(outer / cores)`，余数 `r = outer mod cores` 行均摊给前 `r` 个核（尾核多担 1 行）。例：outer=8192 → 170 余 32 → 前 32 核 171 行、后 16 核 170 行。余数≤cores，负载差 ≤1 行，均衡可接受。

### 5.2 32B 对齐风险（多核并发写回）

**风险是否真实：是（有据）。**
- DataCopy（MTE3 写回）搬运粒度为 **32 B datablock**（来源 S194，华为开发者联盟，等级 B）。若某行 `D·s` 不是 32 B 整数倍，写回尾块会按整 32B 搬运，**把相邻行的有效数据所在 32B 缓存行一起覆盖/读改写**（来源 S194、S198）。
- 本算子 GM 排布：行与行在 output 张量里连续（`[outer, D]` 行主序）。若 `D·s` 非 32 对齐，第 i 行尾块与第 i+1 行头块落在**同一 32B 物理缓存行**；当核 A 写第 i 行、核 B 写第 i+1 行**并发 DMA** 时，二者触碰同一缓存行 → **总线事务冲突 / 邻居有效字节被写脏**（正确性风险，非仅性能）。
- 注：读方向（x/residual/gamma/bias）越界只读邻居、不破坏数据，仅浪费带宽；**写回方向（仅 output）才是危险面**，故对齐处理只需针对 output 张量。

**对策评估：**
- 题面建议「按 `k·D·s ≡ 0 (mod 32)` 的最小 k 行作为分配粒度」：**正确**。把 k 行整体分给同一核，使该核写回区总字节 = `k·D·s` 为 32 的整数倍，核间边界落在 32B 对齐处；区内各行虽可能行内非对齐，但由**同一核顺序写**，无并发冲突，且区尾恰好整 32B。**有效消除跨核踩踏**。✅
- **更简等价方案（推荐首选）**：在 Host 侧把 output 张量的**每行 stride 补齐到 `ceil(D·s / 32)·32`**（即每行 GM  footprint 32B 对齐），并 `aclrtMalloc` 时多留 32B 安全余量（来源 S198）。这样每行起点天然 32B 对齐，跨核写回零冲突，且 MTE 能打满 burst（512B cacheline 对齐更佳）。
- 尾块处理：非对齐 D 用 `DataCopyPad`（配合 `DataCopyExtParams.blockLen` 可非对齐字节数 + pad 值），让硬件处理尾部而不踩邻居（来源 S194）。前提是 output GM 已按 stride 补齐。

**结论**：风险真实且属正确性问题；以「output 每行 32B stride 补齐」为主对策，`k·D·s≡0(mod 32)` 行组分配为等价的次选。

### 5.3 标量同步代价与批量化归约（见第 6 节）

---

## 6. 标量同步与批量化归约

### 6.1 ReduceSum 的标量同步代价（真实）

- 现象：`ReduceSum`（尤指 `WholeReduceSum`）API 内部存在 **scalar 的同步等待**，会阻塞流水线、使 Vector 计算完一段后 Scalar 发不出下一条指令，导致流水断流（来源 S192：华为专家确认 ReduceSum 含 scalar 同步等待、会阻塞流水，等级 B/C）。
- `GetValue(0)` 读回归约标量 = 一次 **V→S 的 `PipeBarrier`**，强制全流水同步（来源 S199）。
- **每行一次**的高频场景：形态 (b) outer=8192 时每核约 170 次 / 形态 (c) 48 次。若串行等待，量级参考：256 个 fp32 单次 `BlockReduceSum+WholeReduceSum` 约 **8.44 µs / 100 次循环 = ~84 ns/次**（来源 S193，官方最佳实践，等级 A；此为推算量级，非本算子实测）。

### 6.2 缓解手段

1. **替换指令组合**：用 `BlockReduceSum`（块内归约，快、无 scalar 等待）+ 单次 `WholeReduceSum`，替代多次 `WholeReduceSum`。256 fp32：8.44 µs vs 13 µs（来源 S193），**约 +35%–40%**。
2. **双缓冲掩盖**：在 scalar 等待期间让 MTE2/MTE3 搬下一批数据（CopyIn/Compute/CopyOut 流水），使同步等待与搬运重叠，不空转（来源 S194/S199）。
3. **减少 PipeBarrier**：仅在与 Cube/跨流水真正有依赖处插 `PipeBarrier`，归约内尽量用细粒度 Queue 同步替代 `PIPE_ALL`（来源 S199）。
4. **批量化归约的边界**：rms 是 **per-row 独立标量**，无法把多行 rms 合并成一个（数学上不能共享）。因此「多行一起归约再统一读回」**只能节省读回次数到「每行仍 1 次」**，收益在于：
   - 把多行 partial sum 留在 UB，流水线处理，使 `GetValue` 等待与邻行 MTE 重叠（隐藏延迟，非消除次数）；
   - 增大每核行数（形态 b）让核不空，但每行标量同步次数不变。
   - **真正能省同步的场景**：若相邻行共享同一 rms（如 batch 内同 shape 且语义允许），才可一次读回广播——但本题 `rms = sqrt(mean(y²)+eps)` 每行独立，**不可广播**。故批量化在此算子只能「重叠隐藏」，不能「减少次数」。

### 6.3 推荐 reduce 实现（per row）

```
BlockReduceSum(y_local, tmp, ...)   // 块内(32元素)归约，无 scalar wait
PipeBarrier()                         // 仅此处必要同步
WholeReduceSum(tmp, z_local, ...)     // 跨块收尾
PipeBarrier()
rms = Sqrt(z_local.GetValue(0) / D + eps)
```
配合双缓冲，使上述两次 `PipeBarrier` 与下一批 MTE 重叠。

---

## 7. 性能测量方法（真机执行清单）

> 本机无法执行，以下为**有 CANN 真机时的操作清单**；全部为方法论，非实测结果。

### 7.1 工具与产物

| 工具 | 命令/用法 | 读什么 | 来源 |
|---|---|---|---|
| msprof（单算子） | `msprof op ./ascendc_kernels_binary` | `OpBasicInfo.csv` → `Task Duration(us)`（kernel 总时长）；`PipeUtilization.csv` → `aiv_time / mte2 / mte3` 占比；`Block Dim`（并行核数） | S188, S190 |
| msprof（应用） | `msprof --application=./app --aicore=on --l2-cache=on` | `op_summary_*.csv` / `task_time_*.csv` | S190 |
| 打点 | `AscendC::AscendCTimeStamp(n)` + `ascendebug ... --dump-mode time_stamp` | 核内各阶段耗时 | S188 |
| 可视化 | MindStudio Insight 打开 `profiling_output` | Timeline / 算子分析视图 | S188, S190 |

### 7.2 测什么指标

- **端到端 vs kernel 时间**：评分只认 kernel 计算时间。用 `msprof op` 隔离单算子，`Task Duration(us)` 即 grid 总执行时间；`aiv_time` 均值 × 波次 ≈ 总时间（来源 S189：4096 逻辑核 / 40 物理核 → 总时间 = aiv_time × ceil(逻辑核/物理核)）。
- **负载均衡**：比对各 `block_id` 的 `aiv_time`，确认尾核未多担导致长尾（对应第 5.1 余数分配）。
- **瓶颈定位**：`mte2` 占比高 → memory-bound（本算子预期）；`aiv` 高 → 计算/同步瓶颈（关注 reduce 标量等待）。

### 7.3 iterations=5 意味着什么

- 判题每点 `iterations=5`：同参数点跑 5 次取聚合（具体聚合方式由 Agent 1 定义；通常取中位数或均值用于 `100/(1+log_1.5(t/T))`）。
- **本地 benchmark 应复现**：同参数循环 5 次，记录每次 kernel 时间；上报与判题一致的聚合（建议中位数，抗偶发抖动）。

### 7.4 避免冷启动计入

- **冷启动来源**：首次执行含 CANN context 初始化、workspace 分配、可能的 JIT/编译、GM 分配。
- **对策**：
  1. 在计时循环前跑 **≥1 次 warmup**（不计入）；
  2. 计时循环内用 `aclrtSynchronizeStream` 或 `aclrtEventElapsedTime` 卡**设备端**时间，避免只测到 host 发射；
  3. 不在循环内做 `aclrtMalloc`/`Free`、printf；
  4. `msprof op` 自身会多次执行二进制，注意其是否内含 warmup（以官方文档为准）。

### 7.5 不依赖判题端的本地 benchmark（构造法）

```
1) Host 程序：aclrtSetDevice → aclrtMalloc 输入/输出 GM（output 多留 32B 补齐）。
2) 填入随机/定值 x, residual, gamma, bias（与判题同 dtype/shape）。
3) 装载 kernel.asc 导出的 __global__ __vector__ run_kernel，用 availableCoreNum=48 启动 <<<blockNum, nullptr, stream>>>。
4) warmup 1 次（aclrtSynchronizeStream）。
5) 计时循环 5 次：{ t0=event; launch; aclrtSynchronizeStream; t1=event; 记录 t1-t0; }
6) 导出 5 次时间；用 msprof op 同跑取 PipeUtilization 做瓶颈分析。
```
- 该 benchmark 与判题「Direct Invocation 直调单文件 kernel.asc」形态一致，可脱离判题端独立回归；120s 超时仅对单点，本地 5 次循环总时长应远小于此。

---

## 8. 「推算值 vs 官方值」区分表

| 编号 | 数字 / 结论 | 类别 | 依据 | 备注 |
|---|---|---|---|---|
| 1 | UB = 192 KB | 官方转引（B） | S181/S182（NPUTargetSpec.td / arxiv） | 非本机实测，源自规格表 |
| 2 | 910B2 = 24 AIC / 48 AIV | 官方转引（B） | S182 | 同上 |
| 3 | 向量 repeat = 256 B（fp16 128 / fp32 64 元素） | **官方（A）** | S185/S186 | hiascend 官方 API 文档 |
| 4 | GM 带宽 = 1.6 TB/s | 官方转引（B） | S183 | 华为开发者联盟对比表；另有 1.2–2.0 TB/s 说法 |
| 5 | S1 = 5·N·s，S2 = 3·N·s，省 40% | **推算** | 题面语义 + 公式 | 公式确定，属算术结论 |
| 6 | S2 可行 D 上限（fp16 22528 / fp32 11264） | **推算** | UB=192KB 模型 + 4·D·s 假设 | 依赖「四份缓冲」假设，非官方 |
| 7 | roofline 拐点 ≈ 200 FLOPs/Byte | **推算** | 320TFLOPS / 1.6TB/s | 带宽取 1.6TB/s 假设 |
| 8 | 形态 (a)~(d) 的 tile/分核数字 | **推算** | UB 预算公式 | 供方案设计，非实测 |
| 9 | 32B 对齐风险真实 + k·D·s≡0(mod32) 对策正确 | 官方转引（B）+ **推算** | S194（风险）+ 公式推导（对策） | 风险有据；对策为正确工程推断 |
| 10 | ReduceSum 含 scalar wait | 官方转引（B/C） | S192（华为专家） | 非本机实测 |
| 11 | BlockReduceSum+WholeReduceSum 快 ~40% | 官方（A） | S193 | 256 fp32 实测 8.44 vs 13 µs（第三方算子，非本算子） |
| 12 | Double Buffer ~1.6× | 官方转引（B） | S194 | 社区优化经验值 |
| 13 | 带宽下界时间（3.6 表） | **推算** | BW=1.6TB/s 纯带宽 | 非实测，仅供量级 |
| 14 | msprof `Task Duration` = kernel 总时长 | 官方（A） | S188/S190 | 工具行为，非性能值 |

**明示**：本报告**没有任何在本机或任何 NPU 上的实测性能数字**。所有性能数值要么引自第三方实测（标注来源与等级），要么为基于公开参数的推算（标注「推算」）。

---

## 9. 来源清单（S181–S210）

```
- [S181] CANN/cannbot-skills NPU硬件架构总览（含 NPUTargetSpec.td 规格表：910B1/910B2 UB=192KB、24 AIC/48 AIV、dav-c220；310B UB=256KB）| https://blog.csdn.net/gitblog_01164/article/details/153903579 | CSDN | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：复核 UB/核数 | 可支持的结论：910B2 UB=192KB、48 AIV
- [S182] ENEC: Lossless AI Model Compression on Ascend NPUs（arxiv，确认 910B2 = 24 AI Cores / 24 AIC / 48 AIV，UB ~192KB）| https://arxiv.org/html/2604.03298 | arxiv | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：复核核数与 UB | 可支持的结论：48 向量核、UB 192KB
- [S183] 昇腾 vs 英伟达/AMD 主流 AI 芯片关键参数对比表（910B HBM 带宽 1.6 TB/s、FP16 320 TFLOPS）| https://developer.huawei.com/consumer/cn/blog/topic/03202360837318320 | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：HBM 带宽/算力 | 可支持的结论：BW≈1.6TB/s、FP16≈320TFLOPS
- [S184] Huawei Ascend 910B AI Accelerator Card（经销商页：64GB HBM2e、1.2 TB/s）| https://omnixonglobal.com/products/huawei-ascend-910b-ai-accelerator-card | Omnixon | 访问日期 2026-09-12 | 等级 C | 状态 partial（与 S183 带宽不一致，疑为 910B1/早期） | 用途：交叉核对带宽 | 可支持的结论：带宽有 1.2TB/s 说法，取 1.6TB/s 为标称需谨慎
- [S185] Ascend C Exp API 官方文档（向量单元每次读 256B；fp16 mask≤128、fp32 mask≤64）| https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0025.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：向量 repeat 宽度 | 可支持的结论：256B/repeat、128/64 元素上限
- [S186] Ascend C Duplicate API 官方文档（256B = 8 block × 32B）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha001/API/ascendcopapi/atlasascendc_api_07_0088.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：datablock=32B | 可支持的结论：搬运粒度 32B
- [S187] 2024CANN训练营：Ascend C API 接口（repeat=8 block×32B）| https://bbs.huaweicloud.com/blogs/436918 | 华为云社区 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：repeat 机制 | 可支持的结论：256B/repeat
- [S188] 性能探针：Ascend C 算子性能分析与 Profiling 工具链实战（msprof op 命令、OpBasicInfo/PipeUtilization、AscendCTimeStamp）| https://ai6s.net/6942197fbf6b0e4b285c1821.html | 社区教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：测量方法 | 可支持的结论：msprof op 取 Task Duration、aiv/mte2/mte3 占比
- [S189] 使用 msprof 分析 Ascend C kernel 执行耗时（OpBasicInfo.csv Task Duration；4096 逻辑核/40 物理核 → 总时间=aiv_time×波次）| https://www.toutiao.com/article/7644776436096385563 | 头条博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：核数映射 | 可支持的结论：逻辑核>物理核时排队，Block Dim 为逻辑核数
- [S190] CANN msprof 环境准备官方文档（msprof --output、op_summary/task_time 产物）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha003/devaids/auxiliarydevtool/atlasprofiling_16_0004.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：测量工具 | 可支持的结论：msprof 产出文件结构
- [S191] Ascend C 算子性能优化实践指南（BlockReduceSum+WholeReduceSum 比两次 WholeReduceSum 快 39%；100 循环 52us vs 85us）| https://ai6s.net/6a4dd516662f9a54cb8adcb4.html | 社区教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：reduce 优化 | 可支持的结论：reduce 组合有显著收益
- [S192] 昇腾 Ascend C 算子开发-ReduceSum 导致的性能问题（华为专家确认 ReduceSum 含 scalar 同步等待、阻塞流水）| https://ascendai.csdn.net/69d7935172111d255bf8761f.html | 昇腾开源生态 | 访问日期 2026-09-12 | 等级 B/C | 状态 verified | 用途：标量同步代价 | 可支持的结论：WholeReduceSum 阻塞流水，改用 BlockReduceSum+WholeReduceSum
- [S193] 针对不同场景合理使用归约指令（官方最佳实践：shape=256 fp32，BlockReduceSum+WholeReduceSum=8.44us vs 两次 WholeReduceSum=13us）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/82RC1/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0034.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：reduce 指令组合 | 可支持的结论：Block+Whole 优于两次 Whole
- [S194] Ascend C 算子性能优化实战：从跑通到打满带宽的四步走（32B 对齐硬要求、DataCopyPad 处理尾部、512B cacheline 对齐、Double Buffer ~1.6×）| https://developer.huawei.com/home/forum/ascend/thread-02200221215465808268-1-1.html | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：对齐/双缓冲 | 可支持的结论：32B 对齐风险真实、DataCopyPad 对策、双缓冲加速比
- [S195] 内存金字塔：Ascend C 多级存储与高效访存（UB 由 32–64 个 Bank 组成，每 Bank 256bit=32B；bank conflict）| https://ascendai.csdn.net/69d4d5d472111d255bf7e316.html | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：UB bank | 可支持的结论：UB 32B bank 粒度
- [S196] 昇腾 CANN 训练营：UB Bank Conflict 深度解析（dstStride padding 打破 bank 冲突）| https://blog.csdn.net/2401_82857325/article/details/155499501 | CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：bank conflict 规避 | 可支持的结论：padding stride 可缓解冲突
- [S197] [AI][昇腾950]数据搬运（GM ~1.6–1.8TB/s，mte2 ratio 常 >95% 显 memory-bound；UB 192KB for A2/A3）| https://hwcomputing.csdn.net/6a5ec2d9662f9a54cb927882.html | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：带宽/瓶颈 | 可支持的结论：本类算子 memory-bound、UB=192KB
- [S198] 昇腾 CANN 训练营：32-Byte 内存对齐与 Burst 性能哲学（aclrtMalloc size+32 安全余量；fp16 tileLength 须为 16 倍数）| https://blog.csdn.net/2401_82857325/article/details/156026480 | CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：对齐对策 | 可支持的结论：Host 侧多留 32B、fp16 行须 16 对齐
- [S199] Ascend C 算子性能优化实用技巧（减少 PipeBarrier 使用、仅在跨流水依赖处同步）| https://www.hiascend.cn/developer/techArticles/20241107-1?envFlag=1 | 昇腾社区 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：同步优化 | 可支持的结论：细粒度同步替代 PIPE_ALL
- [S200] Ascend 950 架构总览（DaVinci AIC/AIV 解耦、HBM/UB 层级；社区整理，含未发布规格）| (无 URL，社区长文) | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 partial（含前瞻规格，谨慎引用） | 用途：架构背景 | 可支持的结论：AIC/AIV 分工、memory-bound 普遍性
- [S201] 华为昇腾计算官网（产品页，无具体带宽数字）| https://e.huawei.com/cn/products/computing/ascend | 华为官网 | 访问日期 2026-09-12 | 等级 A | 状态 partial（无本任务所需具体数） | 用途：平台背景 | 可支持的结论：无数值结论
- [S202] Ascend Hardware Background（arxiv 2505.15112：DaVinci 架构、AIV 支持 reduce/gather）| https://arxiv.org/pdf/2505.15112v1 | arxiv | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：架构背景 | 可支持的结论：AIV 支持 reduction
- [S203] 华为开发者联盟 昇腾 vs N卡 对比（复用 S183 链接，FP16 320TFLOPS、HBM 1.6TB/s）| https://developer.huawei.com/consumer/cn/blog/topic/03202360837318320 | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：算力/带宽交叉验证 | 可支持的结论：与 S183 一致
```

---

> 范围声明：本报告仅覆盖性能 / UB / 硬件架构（Agent 7 主题），未检索题面与提交规则（Agent 1）、Ascend C API 用法细节（Agent 2）、官方仓库源码（Agent 3）、GPU/Triton 实现（Agent 4）、编译器代码生成（Agent 5）、数值精度（Agent 6）、Linux 环境（Agent 8）、失败案例（Agent 9）；未修改 `源码/`、`提交/` 下任何文件，仅产出本报告。所有性能数字均标注来源等级或「推算」，无任何本机/真机实测声明。
