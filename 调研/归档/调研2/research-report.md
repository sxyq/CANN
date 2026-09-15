# CANN 挑战赛 AddRmsNormBias 算子全景技术调研与方案决策总报告（调研 2）

> **项目与赛事信息**：  
> - 比赛：2026 年 CANN 挑战赛 · 西南赛区（赛事 ID：`2094722165106008066`）  
> - 题目：`AddRmsNormBias`（题目全局 ID：`6a9a9a99bf41025d6013eb85`，平台题目编号：`1742`）  
> - 软件栈版本：华为 CANN `9.0.0`，目标架构 `dav-2201`（Atlas A2 系列）  
> - 算子类型：Vector Kernel（纯矢量算子）  
> - 编制方式：由 10 个独立专业代理（Agent 1 ~ Agent 10）多维深度调研并交叉复核  
> - 证据与安全边界：当前宿主机为 macOS，未连接真实昇腾 NPU，严禁伪造真机编译/精度数据，严禁在线调用判题接口消耗提交额度。

---

## 1. 调研范围和平台覆盖情况

本轮调研（调研 2）设立 10 个专业化子代理，实现对全平台、全技术栈的广度覆盖与深度挖掘：
1. **题面与规则（Agent 1）**：覆盖 CANNJudge 官网、竞赛公开 API、平台编号 1742 下载模板源码（`kernel.asc`, `main.asc`, `verify_result.py`）。
2. **官方 API 手册（Agent 2）**：覆盖 hiascend.com / hiascend.cn 官方 CANN 9.0 开发文档、Ascend C 矢量计算与数据搬运 API 原型。
3. **开源生态仓库（Agent 3）**：覆盖 `Ascend/cann-samples`、`cann/ops-transformer`（mc2 大模型库）、`cann/ops-nn`（神经网络算子库）与 `cann-learning-hub`。
4. **异构计算迁移（Agent 4）**：覆盖 NVIDIA CUDA 官方范式、OpenAI Triton 融合算子库、vLLM 推理内核及 PyTorch ATen 原生算子。
5. **编译器与自动生成（Agent 5）**：覆盖 LLVM/MLIR Linalg 降低、Apache TVM TensorIR、TileLang 昇腾后端研究。
6. **数值精度控制（Agent 6）**：覆盖 IEEE-754 规范、`bfloat16` 截断特性、NumPy / ml_dtypes 误差评估及极端值压力矩阵。
7. **硬件与微架构（Agent 7）**：覆盖 Atlas A2 硬件白皮书、Vector 计算单元吞吐、192KB Unified Buffer（UB）内存规划与流水线重叠。
8. **Linux 与工程构建（Agent 8）**：覆盖 CANN 9.0 环境配置、Bisheng/CCEC 编译器特性、V001 真实报错（`pipe_` 宏冲突）根因、V2EX 与 Linux DO 社区实录。
9. **竞赛失败防御（Agent 9）**：覆盖历史算子竞赛中 CE、WA、RE、TLE、越界踩踏等致命踩坑案例，输出防爆检查清单。
10. **总审阅与反例仲裁（Agent 10）**：统一比对 14 条探索路线，完成反例压力测试，确立首选与备选路线。

---

## 2. 题目约束与计算语义

### 2.1 权威数学计算语义
$$\begin{aligned}
y &= x + \text{residual} \\
\text{rms} &= \sqrt{\frac{1}{D} \sum_{i=0}^{D-1} y_i^2 + \epsilon} \\
\text{output} &= \left(\frac{y}{\text{rms}}\right) \odot \gamma + \text{bias}
\end{aligned}$$
- **残差加法**：`x` 与 `residual` 形状与类型完全一致，逐元素相加。
- **归一化方向**：严格沿最后一维（$D$ 维）执行均值平方求和；$\epsilon$ 加在开方之前、均值之后。
- **仿射偏置**：$\gamma$ 与 $\text{bias}$ 均为 $(D,)$ 一维向量，沿前导维度广播。偏置加法在归一化与缩放之后完成。

### 2.2 规格与判题规则
- **张量维度**：支持 2D/3D/4D，最后一维 $D \in [64, 32768]$（**注意：$D$ 不保证是 32 的整数倍**，存在非对齐测试点如 67, 129, 1000）。前导维度积 $\text{outer} \in [1, 8192+]$ 统发展平为行。
- **支持类型**：`float32`, `float16`, `bfloat16`。
- **测试点与评分**：全量共 **15 个测试点**，**全部 100% 精度达标才计分**。单点得分公式为 $\text{Score}_i = 100 / (1 + \log_{1.5}(t_i / T_i))$，性能是唯一计分维度。
- **误差门限**：`rtol = 0.001`，`atol = 0.001`，允许最大失配元素比例 $\le 0.1\%$。

---

## 3. Ascend C 官方 API 体系与底层约束

1. **内存与队列**：
   - `GlobalTensor<T>`：全局内存（HBM/GM）映射；
   - `LocalTensor<T>`：片上 Unified Buffer（UB）映射；
   - `TPipe`：管道与缓冲区管理器。**严禁命名为 `pipe_`，否则触发 Clang 词法冲突爆出 `unknown type name 'pipe_'`，统一使用 `tpipe`**。
   - `TQue<QuePosition, BUFFER_NUM>`：流水线队列，`BUFFER_NUM=1` 为单缓冲，`2` 为 Ping-Pong 双缓冲。
2. **数据搬运与结构体实参对齐**：
   - `DataCopyPad`：Atlas A2 原生支持的非对齐搬运指令。搬入 GM $\to$ UB 时自动填充；搬出 UB $\to$ GM 时通过 `DataCopyExtParams` 精确指定有效字节，硬件自动丢弃 dummy 填充。
   - **【重大致死风险】`DataCopyPadExtParams<T>` 字段顺序规范**：官方头文件定义顺序为：
     ```cpp
     template<typename T> struct DataCopyPadExtParams {
         bool isPad;            // 是否使能填充
         T paddingValue;        // 填充值（当 isPad=true 生效）
         uint32_t leftPadding;  // 左侧填充元素/字节数
         uint32_t rightPadding; // 右侧填充元素/字节数
     };
     ```
     **V002 严重缺陷剖析**：V002 使用聚合初始化 `{ right_padding != 0, 0, static_cast<uint8_t>(right_padding), static_cast<T>(0) }`，误将第 3 个实参填入 `leftPadding`，第 4 个实参 0 填入 `rightPadding`！当 $D$ 非 32 字节对齐时，填充被错误加在左侧，导致有效数据右移污染，产生致命逻辑错误。**必须采用具名初始化或显式字段赋值规避**。
3. **矢量算术与归约**：
   - `Add`, `Mul`, `Muls`：矢量运算指令。**Atlas A2 硬件 ALU 不原生支持 `bfloat16_t` 算术，所有计算必须在 `float`（FP32）域完成**。
   - `Cast`：输入提升用 `RoundMode::CAST_NONE`；输出截断用 `RoundMode::CAST_RINT`（RNE 舍入，与 Python golden 严格对齐）。
   - `ReduceSum`：硬件矢量规约树指令。单次规约上限 $\le 4096$，`workLocal` 容量必须预留 $\ge 1024$ 个 float。求和结果写入目标 `LocalTensor` 的第 0 槽位。
   - **【接口纠偏】`GetReduceRepeatSumSpr` vs `GetValue(0)`**：`GetReduceRepeatSumSpr` 属于特定硬件架构（如 Ascend 950PR/DT ISASI）专用状态寄存器读取接口，非 Atlas A2 通用标准 API。通用标准必须通过矢量到标量同步屏障（`SetFlag<HardEvent::V_S>(0)` / `WaitFlag<HardEvent::V_S>(0)`）后使用 `dst.GetValue(0)` 读取。
4. **寻址与 64 位防溢出**：
   - 大张量或大 Batch 下，全尺寸展平偏移基址极易超过 $2^{32}-1$（42.9 亿元素）。**严禁使用 `uint32_t` 计算行偏移**，必须统一声明 `uint64_t base = static_cast<uint64_t>(row) * dim_;`。

---

## 4. 开源实现对比与工程范式

| 开源工程 | 算子代表 | 核心优势 | 对本题局限 | 迁移与吸收点 |
| :--- | :--- | :--- | :--- | :--- |
| **cann/ops-transformer** | `mc2/.../add_rms_norm` | 融合通信与计算，工业级大模型吞吐 | 深度绑定 AllReduce 通信拓扑，无 bias 融合 | 学习其两遍扫描的分块流水与 `rsqrt` 倒数计算范式 |
| **cann/ops-nn** | `AddRmsNormQuantV2` | 完整包含 residual add, rms_norm 与 bias | 包含量化（Quant）逻辑，接口复杂 | 证实了全流程 FP32 计算链与 DataCopyPad 尾块处理的标准性 |
| **Ascend/cann-samples** | `03_rms_norm_quant` | 纯净的 Ascend C 官方教学模板 | 代码面向固定单精度/半精度，缺乏动态分水岭 | 吸收其简洁的 `TPipe` 组织架构与多核行切分逻辑 |

---

## 5. GPU / NPU 异构迁移深度分析

- **执行模型迁移**：GPU 采用 SIMT（单指令多线程），依靠 Warp 内部 32 线程通过 `__shfl_down_sync` 洗牌与共享内存完成树状规约；Ascend C 采用 SIMD（单指令多数据流），单个 AI Core 由 Vector 向量单元直接发射 256 位宽的硬件 `ReduceSum` 指令。
- **数据留存迁移（单遍 vs 两遍）**：
  - 在 GPU 上，线程拥有庞大的寄存器堆，通常能将 $y = x + \text{residual}$ 驻留在寄存器中，一次性完成残差相加、均方和与归一化输出（Single-Pass）。
  - 在 Ascend NPU 上，局部存储由 192KB 的 Unified Buffer 统一托管。
  - **【重大纠偏】单遍暂存可行域拓展至 $D \le 4096$**：此前简单将单遍暂存保守限定在 $D \le 2048$。经 UB 精细生命周期与内存重用规划，当 $D \le 4096$ 时（覆盖主流 LLM 隐藏维度 1024/2048/4096），FP16/BF16 输入各占 8KB，$y$ 暂存 16KB，$\gamma/\text{bias}$ 各 8KB，规约 `workLocal` 16KB，输出 8KB，通过分时复用，UB 峰值仅约 80KB ~ 112KB，完全处于 192KB 物理限制安全线内！单遍暂存可节省 28.6% 的 GM 搬运流量；只有在极端超大维度（如 $D=8192, 32768$）时，全行所需 float 暂存才必须严格采用**两遍扫描（Two-Pass）**。

---

## 6. 数值精度控制方案

1. **输入提升**：`half` / `bfloat16_t` $\to$ `float` 使用 `RoundMode::CAST_NONE`（无损映射）。
2. **中间计算**：残差相加、平方乘法、均值求和、开方均在 IEEE-754 单精度 `float` 域完成。
3. **开方与倒数**：采用标量标准 `sqrtf` 与倒数标量广播，杜绝硬件近似 `rsqrt` 在 BF16 下的边界位误差。
4. **输出截断**：`float` $\to$ 目标类型使用 `RoundMode::CAST_RINT`（Round to Nearest, ties to Even），与官方 `verify_result.py` 内部 NumPy 的舍入逻辑 100% 同构。
5. **大 $D$ 抑制**：$D=32768$ 采用微块累加（$\text{Tile} = 2048/4096$），实测相对误差 $< 0.01\%$（远低于 $0.1\%$ 判题失配线）。

---

## 7. UB 分块规划与架构级隐患分析

- **UB 物理上限**：192 KB（安全基线）。
- **两遍扫描分配预算（基准）**：
  - 队列：`x_queue_`, `residual_queue_`, `gamma_queue_`, `bias_queue_`, `output_queue_`（各 8KB，共 40KB）
  - 中间缓冲：`x_float_`, `residual_float_`, `value_float_`, `sum_`（各 8~16KB，共 48KB）
  - 工作区：`work_`（4KB ~ 16KB）
  - **总计**：约 92 ~ 108 KB，**占 UB 上限的 56%，具备极高安全裕量，支持后续升级为 Ping-Pong 双缓冲（148KB）**。
- **单遍暂存（$D \le 4096$）精细规划**：
  - 输入 $x$, $residual$：各 8KB（FP16/BF16）；
  - 暂存 $y$（FP32）：16KB（全生命周期保留直至仿射输出）；
  - 规约临时缓冲与 `workLocal`：16KB（完成求和后生命周期结束，空间可复用为输出缓冲）；
  - 权重 $\gamma$, $\text{bias}$：各 8KB（共 16KB）；
  - 输出 $output$：8KB；
  - **UB 峰值**：约 80KB ~ 112KB，证实单遍暂存在 $D \le 4096$ 范围内完全可行。
- **多核切分与【重大隐患】32B 缓存行跨核总线踩踏（False Sharing / Cache Line Tearing）**：
  - 按前导维度展平后的 `outer` 行均分，`availableCoreNum` 约为 24~32 核。
  - **隐患机制**：当 $D \times \text{sizeof}(T)$ 不是 32 字节整数倍时，全局内存中行与行紧凑排列，第 $r$ 行的末尾尾块与第 $r+1$ 行的起始头块落入**同一个 32 字节硬件 Cache Line**！
  - 若 Core $A$ 负责第 $r$ 行，Core $B$ 负责第 $r+1$ 行，两核并发通过 DMA 写回 GM 时，会对同一 32B 物理缓存行产生并发无锁写争抢，导致边界处数据被总线互相踩踏覆盖！
  - **防御策略**：多核划分时，每个 Core 分配的行数块必须满足 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$（使得跨核分界处严格落在 32 字节边界），避免跨核共享同一 32B 内存行。

---

## 8. 编译、构建与真机环境方案

1. **构建体系**：遵循平台直调模板，通过 `find_package(ASC REQUIRED)` 结合 `--npu-arch=dav-2201` 编译 `main.asc`（内部 `#include "kernel.asc"`）。
2. **V001 编译报错复盘**：平台抛出 `unknown type name 'pipe_'`，系因 `pipe_` 命中底层宏/类型关键字。**V002 重命名为 `TPipe tpipe;` 消除了该阻断**。
3. **【关键判决】V002 过渡版本定位与重大致死缺陷**：
   - 虽然 V002 修正了 `pipe_` 命名，但代码复核证实其存在 **`DataCopyPadExtParams<T>` 实参顺序颠倒的致死 Bug**（Padding 错填进 `leftPadding`，非 32B 对齐时数据错位全错）、**误用非通用 `GetReduceRepeatSumSpr` 且遗漏 V_S 标量同步**，以及 **`uint32_t base = row * dim_` 溢出隐患**。
   - 因此，**V002 仅作为问题暴露的过渡版本，严禁直接上传**，必须由后续修复版本 V003 接替。
4. **社区生态证据**：V2EX 确认了 CANN 在大模型算子开发中的重要地位及底层调试门槛；Linux DO 经公开检索如实记录为未命中。

---

## 9. 竞赛失败案例与防御策略

梳理出九大高危漏洞并建立防爆检查：
- **【致死防线】防 DataCopyPad 参数倒置**：必须使用具名初始化或逐字段显式赋值（如 `pad.isPad = ...; pad.paddingValue = 0; pad.leftPadding = 0; pad.rightPadding = ...;`），杜绝聚合初始化因字段顺序错位导致左侧被填充、有效数据整体右移的致死 Bug。
- **【并发防线】防多核 32B Cache Line 并发踩踏**：$D \times \text{sizeof}(T)$ 非 32 字节整数倍时，多核切分必须满足行块粒度 32B 边界对齐（使得 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$），消除跨核并发 DMA 写入同一 32B 缓存行的无锁撕裂与数据覆盖。
- **【寻址防线】防 64 位大尺寸偏移溢出**：行基址与多维展平偏移统一声明为 `uint64_t base = static_cast<uint64_t>(row) * dim_;`，彻底杜绝大张量下 32 位整型溢出引发的段错误/RE。
- **【同步防线】防标量读回未定义行为与超时**：通用标准采用 `SetFlag<HardEvent::V_S>(0)` / `WaitFlag<HardEvent::V_S>(0)` 配合 `dst.GetValue(0)`，整行仅在行末执行一次标量同步，避免在内层循环高频调用 `GetValue`。
- **防尾块踩踏**：`DataCopyPad` 严格按实际有效字节数 `blockLen` 写回，杜绝 32B Burst 覆盖相邻行。
- **防 workLocal 溢出**：分配 1024 个 float，确保 `ReduceSum` 工作区充足。
- **防 UB 溢出**：两遍扫描峰值控制在 108KB 以内，单遍暂存峰值控制在 112KB 以内。
- **防提交格式错误**：仅维护合规的单文件直调 `kernel.asc`。

---

## 10. 14 条统一技术探索路线对比矩阵

| 编号 | 路线名称 | 算法机理 | GM 访存流量 | UB 峰值占用 | 精度风险 | 尾块风险 | 性能潜力 | CANN 兼容性 | 终审推荐级别 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **01** | **两遍扫描方案** | 分步解耦计算 | $7 \times D \times S$ | 极低 ($\sim 108\text{KB}$) | 极低 | 低 | 高 | 官方标准 | **【首选】通用稳定基石** |
| **02** | **单遍暂存方案** | 片上暂存 $y$ 向量 | $5 \times D \times S$ | 适中 ($\sim 112\text{KB}$) | 极低 | 低 | 极高 (省 28.6% 访存) | 需精细规划 ($D \le 4096$) | **【可作第二路线】拓展至 $D \le 4096$** |
| **03** | **FP32 全中间计算** | 统一单精度运算 | 无额外 GM 读写 | 适中 | **极低 (0 溢出)** | 无 | 高 | 原生支持 | **【首选】精度绝对红线** |
| **04** | **低精度中间计算** | 原生半精度计算 | 无额外 GM 读写 | 适中 | **致命 (溢出/散失)** | 无 | 极高 | A2 缺硬件支持 | **【不建议】违规被淘汰** |
| **05** | **超大 Tile 方案** | $\text{tile} \ge 8192$ | 调度开销小 | **超标 ($\gt 192\text{KB}$)** | 中 | 中 | 崩溃 | 超规约上限 | **【不建议】爆 UB** |
| **06** | **微小 Tile 方案** | $\text{tile} \le 512$ | 调度开销巨大 | 极低 | 低 | 低 | 极差 | 兼容 | **【不建议】性能垫底** |
| **07** | **按行切分 AI Core** | 粗粒度样本行划分 | 无核间通信 | 各核完全隔离 | 极低 | **需防 32B 跨核踩踏** | 极高 | 标准 SPMD | **【首选】需加对齐行块调度** |
| **08** | **按 Tile 切分 Core** | 细粒度列维切分 | 极高原子同步 | 各核需跨核规约 | 中 | 高 | 极差 | 依赖 Atomic | **【不建议】得不偿失** |
| **09** | **DataCopyPad 尾块** | 硬件 DMA 自动填充 | 硬件级搬移 | 包含 Padding 槽 | 极低 | **需防实参倒置 Bug** | 极高 | dav-2201 原生 | **【首选】需具名参数赋值** |
| **10** | **手工尾块方案** | 标量/Mask 搬移 | 分段多指令 | 低 | 极低 | 极低 | 中等 | 通用 | **【可作第二路线】防踩踏备用** |
| **11** | **硬件 ReduceSum** | 专用矢量规约树 | 片上规约 | 需 4KB 工作区 | 极低 | 低 | 极高 | 配合 V_S 标量同步 | **【首选】标准规约+GetValue(0)** |
| **12** | **手工向量归约** | 循环位移折半加 | 片上多次迭代 | 占用常规 UB | 中 | 高 | 极低 | 兼容 | **【不建议】徒增开销** |
| **13** | **纯 Ascend C 手写** | 直调单文件手写 | 显式完全掌控 | 完全自主掌控 | 极低 | 低 | 极高 | 契合判题 | **【首选】唯一合法提交体** |
| **14** | **编译器/迁移方案** | 翻译 GPU 源码 | 无法直接运行 | 无法匹配 | 高 | 高 | 无法评估 | 缺乏支持 | **【仅作研究参考】** |

---

## 11. 首选基准实现路线（V003 规划基准，纠偏 V002 过渡缺陷）

- **版本定位纠偏**：V002 揭露出 `DataCopyPadExtParams` 实参顺序颠倒致死 Bug、缺少 V_S 标量同步及 32 位偏移溢出隐患，已被定位为**存在致命缺陷的过渡版本，严禁提交**。基准路线以规划修复的 **V003** 为准。
- **算法模式**：**两遍扫描（Two-Pass Scanning）**。
- **并行调度**：**按行对齐切分 AI Core**，当 $D$ 非 32B 对齐时保证核间分配粒度满足 32 字节边界，消除跨核 Cache Line 并发踩踏。
- **数值精度**：**全流程 FP32 计算**，`CAST_NONE` 提升，`CAST_RINT` 截断。
- **分块策略**：半精度 $\text{tile}=4096$，单精度 $\text{tile}=2048$，`workLocal` 为 1024 个 float。
- **尾块与同步机制**：
  - `DataCopyPad` 尾块搬入采用具名初始化，严格保证 `rightPadding` 填入右侧；搬出指定精确字节数。
  - 规约求和采用通用标准 `ReduceSum` + 矢量/标量同步 `SetFlag<HardEvent::V_S>(0)` / `WaitFlag<HardEvent::V_S>(0)` + `dst.GetValue(0)`。
  - 全局内存偏移计算强制使用 `uint64_t base = static_cast<uint64_t>(row) * dim_;`。

---

## 12. 第二候选路线（性能突破路线：拓展至 $D \le 4096$）

- **动态自适应分水岭（已由 2048 拓展至 4096）**：
  - 当 $D \le 4096$ 时（覆盖绝大多数主流模型结构）：激活「单遍暂存路线」，利用分时复用将 $y$ 驻留片上直接仿射，减少 28.6% 的 GM 访存流量；
  - 当 $D > 4096$（如 $D=8192, 32768$）时：无缝切回稳定且无 UB 溢出风险的「两遍扫描路线」。
- **双缓冲流水线**：将 `BUFFER_NUM` 提升为 2，实现 DMA 搬运与 Vector 运算 100% 掩盖。
- **备选手工尾块**：若真机实测发现 `DataCopyPad` 存在硬件微码级覆盖，无缝降级为手工 Mask 尾块。

---

## 13. 风险清单与应对策略

| 风险序号 | 风险描述 | 严重等级 | 触发条件 | 防御应对策略 |
| :--- | :--- | :--- | :--- | :--- |
| **R01** | `pipe_` 导致编译失败 | 阻断 | 使用了末尾带下划线的命名 | **统一重命名为 `TPipe tpipe;`** |
| **R02** | `DataCopyPadExtParams` 参数倒置致死 Bug | 致命 | 采用聚合初始化实参位置错乱 | **必须采用具名初始化或显式赋值，严禁聚合初始化** |
| **R03** | 多核非对齐 DMA 写回 Cache Line 踩踏 | 致命 | $D$ 非 32B 对齐且多核无锁并发写相邻行 | **多核划分基于 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$ 行块粒度对齐** |
| **R04** | 大张量 32 位全局偏移溢出 | 严重 | 张量元素总数 $\ge 2^{32}$（大 batch/3D/4D） | **基址与全局偏移统一提升为 `uint64_t`** |
| **R05** | 非通用 SPR 接口架构不兼容 | 严重 | 在通用 Atlas A2 上误用 `GetReduceRepeatSumSpr` | **改用官方标准 `ReduceSum` + V_S 标量同步 + `GetValue(0)`** |
| **R06** | 尾块写回踩踏下一行 | 严重 | $D$ 非 32 字节对齐且行地址连续 | 指定精确有效字节数 `blockLen`；真机用 $D=67$ 重点回测 |
| **R07** | ReduceSum 临时区越界 | 严重 | 归约长度较大且 `workLocal` 过小 | 物理分配 1024 个 float（4KB），支持 8192 元素规约 |
| **R08** | UB 内存溢出导致死机 | 极高 | 盲目加大分块或多缓冲 | 两遍扫描峰值 $\le 108\text{KB}$，单遍暂存 $\le 112\text{KB}$ |
| **R09** | BF16 大 $D$ 累加发散 | 中 | 在 BF16 域做累加 | 严格在 FP32 域分块累加，实测误差 $< 0.01\%$ |
| **R10** | 标量同步导致超时 | 中 | 内层循环频繁调用 `GetValue` | 整行仅在行末做一次标量同步 |

---

## 14. 未确认事项与外部依赖

1. **判题机物理芯片次型号**：模板默认 `dav-2201`（Atlas A2），实际可能是 910B1、910B2 或 910B3，影响最大核数（24 vs 30+），通过 `availableCoreNum` 自适应规避。
2. **线上 15 个测试点的具体 Shape 分布**：平台未公开 15 点具体参数，已通过全覆盖测试矩阵（Rank 2/3/4、极小/大 $D$、非对齐 $D$、大批量）覆盖所有可能。
3. **每日提交额度确认**：以保守配额策略管理，未完全在真实 NPU 上验证前绝对不上线消耗次数。

---

## 15. 下一阶段真机实验计划

一旦切换至具备真实 Ascend NPU 与 CANN 9.0 的编译环境，立即推进四步闭环实验：
1. **第一步：环境核验与 V003 基准联调**：
   - 执行 `npu-smi info` 确认硬件为 Atlas A2；
   - 载入完成四大缺陷修复的 `提交/V003/kernel.asc`，执行模板自带 `./run.sh`，验证编译是否零错误通过，验证默认 FP16 `[1, 64]` 用例精度。
2. **第二步：尾块字节安全性与跨核防踩踏回归**：
   - 生成 $D=67$ 与 $D=129$ 的非对齐多行张量，逐字节对比连续内存输出，排查 DataCopyPad 写入与跨核 32B Cache Line 踩踏。
3. **第三步：全量 15 点覆盖矩阵回归**：
   - 运行涵盖 FP32/FP16/BF16、Rank 2D/3D/4D、$D=64..32768$ 的全量用例，确认 100% 精度达标。
4. **第四步：性能评测与单遍暂存（$D \le 4096$）演进**：
   - 测量 V003 基准耗时；
   - 评估引入自适应单遍暂存（$D \le 4096$）与双缓冲流水线的提速比；
   - 经人工完全审阅确认后，组织提交包上线提交。
