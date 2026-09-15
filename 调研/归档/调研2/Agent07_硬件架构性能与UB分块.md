# Agent 7 专题报告：硬件架构分析、UB 内存规划与极限性能调优

> 负责代理：Agent 7  
> 目标硬件：华为 Atlas A2 系列（Ascend 910B / `dav-2201`）  
> 考察方向：片上存储容量、带宽瓶颈分析、Tiling 切分与双缓冲流水线设计。

---

## 1. 目标硬件（Atlas A2 / dav-2201）微架构关键参数

依据华为昇腾架构官方白皮书与 CANN Profiling 手册：

| 架构组件 | 物理参数与能力 | 对算子设计的影响 |
| :--- | :--- | :--- |
| **Vector 单元数量** | 单芯片通常包含 24 ~ 32 个 AI Vector Core | 算子核数 `availableCoreNum` 约为 24 或 30+，必须充分利用多核并行度 |
| **Unified Buffer (UB)** | 每个 AI Core 独立拥有 **192 KB**（部分型号标称 256 KB，安全预算按 192 KB） | 所有局部张量、工作队列、中间累加区的总和严禁超出 192 KB，否则 AllocTensor 致命崩溃 |
| **MTE 数据搬运引擎** | MTE2（GM $\to$ UB 读）、MTE3（UB $\to$ GM 写） | 负责 DMA 异步搬运，可与 Vector 向量计算引擎完全并发重叠（Overlap） |
| **向量计算位宽** | 256 字节（一次指令处理 128 个 half 或 64 个 float） | 块长度为 64（float）或 128（half）的整数倍时可发挥硬件最高吞吐 |
| **算子瓶颈属性** | 算术强度（FLOP/Byte）较低，典型 **访存受限型（Memory-Bound）** 算子 | **减少全局内存（HBM）的读写次数是获取最高得分的唯一决定性路径** |

---

## 2. 片上 UB 内存预算规划表（Peak Memory Budget）

为了保障在最恶劣场景下绝对不超出 192 KB 的物理红线，且满足高吞吐要求，对单 Core 内部的 UB 资源进行精细化规划：

### 2.1 方案 A：单缓冲安全基准（V001 / V002 采用）
- **分块配置**：
  - 半精度（FP16 / BF16）：$\text{tile\_len} = 4096$ 元素（$4096 \times 2\text{B} = 8\text{ KB}$）
  - 单精度（FP32）：$\text{tile\_len} = 2048$ 元素（$2048 \times 4\text{B} = 8\text{ KB}$）

| 队列 / 缓冲区名称 | 数量与类型 | 缓冲阶数 | 单块容量 | 占用 UB 内存 |
| :--- | :--- | :--- | :--- | :--- |
| `x_queue_` | `TQue<VECIN>` | 1 | $\text{tile\_len} \times \text{sizeof}(T)$ | 8 KB |
| `residual_queue_` | `TQue<VECIN>` | 1 | $\text{tile\_len} \times \text{sizeof}(T)$ | 8 KB |
| `gamma_queue_` | `TQue<VECIN>` | 1 | $\text{tile\_len} \times \text{sizeof}(T)$ | 8 KB |
| `bias_queue_` | `TQue<VECIN>` | 1 | $\text{tile\_len} \times \text{sizeof}(T)$ | 8 KB |
| `output_queue_` | `TQue<VECOUT>` | 1 | $\text{tile\_len} \times \text{sizeof}(T)$ | 8 KB |
| `x_float_` | `TBuf<VECCALC>` | 1 | $\text{tile\_len} \times 4\text{B}$ | 8 ~ 16 KB |
| `residual_float_` | `TBuf<VECCALC>` | 1 | $\text{tile\_len} \times 4\text{B}$ | 8 ~ 16 KB |
| `value_float_` | `TBuf<VECCALC>` | 1 | $\text{tile\_len} \times 4\text{B}$ | 8 ~ 16 KB |
| `work_` (归约临时区) | `TBuf<VECCALC>` | 1 | $1024 \times 4\text{B}$ | 4 KB |
| `sum_` (归约目标区) | `TBuf<VECCALC>` | 1 | $\text{tile\_len} \times 4\text{B}$ | 8 ~ 16 KB |
| **总计 UB 占用峰值** | — | — | — | **约 88 KB ~ 108 KB** |

**安全评估**：$108\text{ KB} \ll 192\text{ KB}$，裕量超过 40%，绝对安全，无任何 UB 溢出风险。

### 2.2 方案 B：双缓冲流水线（Ping-Pong Double Buffering）
- 将 `BUFFER_NUM` 提升为 `2`，使 MTE2 搬入下一块的同时，Vector 计算当前块。
- **总 UB 占用**：$8\text{ KB} \times 5 \times 2 + 16\text{ KB} \times 4 + 4\text{ KB} \approx 148\text{ KB}$。
- **安全评估**：$148\text{ KB} \le 192\text{ KB}$，完全处于硬件允许范围内，可在性能优化版中放心开启。

---

## 3. 核心算法路线访存与吞吐深度对比

RMSNorm 算子的性能关键在于对全局内存（HBM）的访存次数（Traffic）。设一行长度为 $D$，元素字节为 $S$：

```text
[路线 1: 两遍扫描 (Two-Pass Scanning)]
Pass 1 (计算 RMS):
  GM 读: x (D*S), residual (D*S)                   --> 2*D*S
Pass 2 (归一化与仿射输出):
  GM 读: x (D*S), residual (D*S), gamma (D*S), bias (D*S) --> 4*D*S
  GM 写: output (D*S)                              --> 1*D*S
总访存流量 = 7 * D * S (字节/行)

[路线 2: 单遍暂存 (Single-Pass Cached in UB, 适用于 D <= 2048)]
Pass 1 & 2 融合:
  GM 读: x (D*S), residual (D*S), gamma (D*S), bias (D*S) --> 4*D*S
  片上: y = x + residual 暂存在 UB
  片上: 直接算均方和与 rms，直接从 UB 归一化并完成仿射
  GM 写: output (D*S)                              --> 1*D*S
总访存流量 = 5 * D * S (字节/行)
--> 访存流量直接减少 28.6%！
```

### 推荐的自适应 Tiling 策略（动态分水岭）：
- **若 $D \le 2048$**：执行「单遍暂存路线」，充分发挥片上 UB 缓存优势，消除对 `x` 和 `residual` 的二次 GM 读，获得极高竞赛得分。
- **若 $D > 2048$**：自动无缝切换到「两遍扫描流水线」，以分块循环保障任何超大维度（直至 $D=32768$）均不爆显存、不溢 UB。

---

## 4. 多核并行切分与标量同步瓶颈消除

### 4.1 多核按行切分
- 由于行与行之间无任何依赖，而同一行内强制共享同一个标量 RMS，**按行切分是唯一最优的多核划分方式**。
- 利用余数均分算法：
  $$\text{count} = \frac{\text{outer}}{\text{blocks}} + (\text{block} < \text{extra} ? 1 : 0)$$
  消除多核之间的长尾效应。

### 4.2 标量同步性能优化
- 传统通过 `LocalTensor::GetValue(0)` 读取归约值，会导致 CPU/Scalar 单元与 Vector 单元强行同步（PipeBarrier），在大 `outer` 时产生严重流水线气泡。
- **优化路径**：
  1. 升级为 `GetReduceRepeatSumSpr<float>()`，直接从 SPR 寄存器读取；
  2. 每一行内部由多个 Tile 组成的均方和，先在片上由向量加法累加，整行全部计算完毕后仅执行 **一次** 标量读取与 `sqrtf`，彻底摊薄同步开销。
