# Agent 07：性能、UB 与硬件形态下的 tiling 思路

> 日期：2026-09-11
> 题目：AddRmsNormBias（CANN 9.0.0 / vector kernel / 直调模板）
> 目标 SoC：文档与模板倾向 `dav-2201`（Ascend 910B / A2 系），未最终确认
> 定位：方案空间调研与 tiling 推荐，不改主线代码，不声称真机性能达标

---

## 0. 任务范围与基线

本报告覆盖：

1. UB 容量与预算（A2 / vector 常见说法）
2. GM→UB 搬运、DataCopyPad 性能特征
3. 向量计算吞吐、ReduceSum 成本
4. 多核负载：小 outer 大 D vs 大 outer 小 D
5. 两遍扫描 vs 单遍暂存
6. 大 tile vs 小 tile
7. 双缓冲、gamma/bias 常驻
8. 标量同步（GetValue / V_S）开销
9. Profiling 方法

**本地基线（当前 `源码/op_kernel/add_rms_norm_bias.cpp`）**：

| 项 | 当前值 | 备注 |
| --- | --- | --- |
| BUFFER_NUM | 1 | 同步流水，正确性优先 |
| TILE_HALF | 4096 | fp16/bf16 每块元素数 |
| TILE_FLOAT | 2048 | fp32 每块元素数 |
| WORK_LEN | 1024 | ReduceSum workLocal（float 元素） |
| SUM_LEN | 16 | ReduceSum 目的缓冲 |
| 扫描策略 | 两遍 | Pass1 平方和归约，Pass2 归一化输出 |
| 中间精度 | FP32 | 归约与累加均 FP32 |
| 多核策略 | 按行均分 | 每核行数差 ≤1 |
| 标量同步 | 每 tile 一次 GetValue + V_S/S_V | Pass1 每块都同步 |

**计分公式（A 级，题目 API 已确认）**：单点 `score = 100 / (1 + log₁.₅(t/T))`，T 为该点全局最优时间，15 点均值。性能是唯一计分维度。

---

## 1. 硬件与 UB 事实

### 1.1 SoC 与产品形态

| 事实 | 数值/结论 | 证据等级 | 来源 |
| --- | --- | --- | --- |
| 模板默认编译架构 | `dav-2201` | A | `addrmsnormbias_problem_1742_template/CMakeLists.txt` L12 |
| 核数获取方式 | `aclrtGetDeviceInfo(..., ACL_DEV_ATTR_VECTOR_CORE_NUM, ...)` | A | 模板 `main.asc` L42 |
| dav-2201 对应产品 | Ascend 910B / Atlas A2 训练系列、Atlas 800I A2 推理 | B | 官方样例产品矩阵（gitee.com/ascend/samples，operator/ascendc README，2026-09-11 抓取） |
| DataCopyPad 支持 | Atlas A2 训练系列 / Atlas 800I A2 支持（无 mode 参数版本）；Atlas 200/500 A2 不支持 | B | 本地调研1 已核对的官方 API 文档结论 |
| A2 Add/Mul 不支持 bfloat16_t | 必须在 FP32 域计算 | B | 本地调研1 官方 API 结论 |
| 判题 SoC 最终型号 | 未确认 | — | 题面未明文；需登录提交页或真机 `npu-smi info` 核对 |

### 1.2 UB 容量

| 项 | 常见说法 | 证据等级 | 说明 |
| --- | --- | --- | --- |
| 达芬奇架构 UB（通用教学值） | **192 KB / AI Core** | B | Ascend C 官方教程与训练营材料长期一致引用；本会话未能直接抓取到 hiascend 文档正文（站点 JS 壳），故不标 A |
| A2（dav-2201）UB | 社区与训练材料沿用 **192 KB** | C | 多篇社区文章与培训 PPT 一致；未在本次会话中从官方文档正文独立核验 |
| 部分新形态芯片 | 可能不同（256 KB 等说法偶见） | D | 无可靠来源，不采信 |

**设计口径（推断，非实测）**：按 UB = 192 KB 做预算。注意 `TPipe::InitBuffer` 的队列与 TBuf 共享同一 UB 空间；编译期/运行期若超出会报错或截断。实际可用量应以真机 `GetLibApiWorkSpaceSize()` 扣除后的剩余为准，192 KB 是物理上限的近似说法。

### 1.3 向量核数量

| 产品 | Vector Core 数 | 证据等级 | 来源 |
| --- | --- | --- | --- |
| 运行时查询 | `ACL_DEV_ATTR_VECTOR_CORE_NUM` | A | 模板 main.asc |
| Ascend 910B3（Atlas 800T A2） | 社区常见 20 | C | 社区文章/论坛；未独立核验官方规格表 |
| 判题机实际值 | 未知 | — | 必须真机查询 |

**推荐**：Kernel 内用 `GetBlockNum()` / Host 侧用 `availableCoreNum`，不写死核数。

### 1.4 HBM 带宽

本会话未能从官方 A2 产品页抓取到 dav-2201 的 HBM 带宽数字（页面 JS 渲染）。以下为分析用的保守假设，**均标为推断**：

| 项 | 假设值 | 等级 | 用途 |
| --- | --- | --- | --- |
| 单卡 HBM 有效带宽 | 400–800 GB/s 量级 | 推断 | 访存模型数量级估算；A2 训练卡社区口径常写 ~1.6 TB/s 峰值，有效带宽按 30–50% 折算 |
| GM↔UB 单核 DMA 有效带宽 | 远低于 HBM 峰值，受 MTE2/MTE3 管线与 UB 端口限制 | 推断 | 说明为何本算子是访存瓶颈 |

> **结论**：AddRmsNormBias 是典型 memory-bound 算子（FLOPs/Byte ≈ 5–10），优化目标是减少 GM 访存次数与隐藏 DMA 延迟，不是提高算力利用率。

---

## 2. 访存模型

### 2.1 单行流量（元素计）

设一行长度 D，dtype 字节宽 s（fp16/bf16=2，fp32=4）。

| 策略 | GM 读 | GM 写 | 合计 | 相对两遍 |
| --- | --- | --- | --- | --- |
| 两遍扫描（x/residual 各读 2 次；gamma/bias 只读 1 次，常驻或 Pass2 一次搬入） | 4D + 2D = 6D | D | **7D** | 100% |
| 两遍扫描（当前基线：Pass2 重读 gamma/bias） | 4D + 4D = 8D | D | **9D** | 129% |
| 单遍暂存 y（D ≤ UB 容限） | 2D + 2D = 4D | D | **5D** | **71%（省 28.6%）** |

当前基线在 Pass2 对 gamma/bias 也做 CopyIn，比「gamma/bias 常驻 + 两遍」多 2D 读。仅做 gamma/bias 常驻即可把 9D 降到 7D，收益约 22%，实现成本极低。

### 2.2 UB 峰值预算（推断，按 192 KB）

以 fp16 输入、FP32 中间为例，估算单遍与两遍在 tileLen = T 时的 UB 占用（不含队列双缓冲）：

**两遍扫描（BUFFER_NUM=1）**：

| 缓冲 | 用途 | 字节 |
| --- | --- | --- |
| inX, inR | 输入双份 T×s | 2×T×2 |
| inG, inB | gamma/bias T×s（可常驻整行则改为 D×s） | 2×T×2 |
| out | 输出 T×s | T×2 |
| xF, rF, yF | FP32 中间 3×T×4 | 12T |
| work + sum | ReduceSum 辅助 | ≈4 KB |
| **合计（T=4096, s=2）** | | **约 88 KB + 4 KB** |

T=4096 时约 92 KB，留有余量；T=8192 则约 176 KB，逼近 192 KB 上限，双缓冲后会超。

**单遍暂存（D ≤ 4096 整行进 UB）**：

| 缓冲 | 字节 |
| --- | --- |
| y_fp32 整行 D×4 | 16 KB（D=4096） |
| x/residual 输入或已融合后的 in-place | 可复用 |
| gamma/bias 常驻 D×s×2 | 16 KB（fp16） |
| 输出 | D×s = 8 KB |
| 工作区 | 8–16 KB |
| **合计** | **约 50–60 KB**（D=4096, fp16） |

单遍在 D≤4096 时 UB 压力小于两遍大 tile，且省 28.6% GM 流量。**这是性能路线的首选突破点。**

### 2.3 DataCopy vs DataCopyPad

| 特性 | DataCopy | DataCopyPad |
| --- | --- | --- |
| 对齐要求 | 32B 长度倍数（元素对齐） | 任意字节长度 |
| 搬入补值 | 不支持 | `isPad=true` 自动补 0 |
| 搬出 | 必须对齐长度 | 支持非对齐，只写有效字节（官方称） |
| 性能 | 对齐时走 burst DMA，吞吐最优 | 有 padding 逻辑，略慢；尾块占比小时可忽略 |
| 写方向风险 | 无 | 社区 C 级实测称不足 32B 时 padding 可能覆盖相邻行（与官方 A 级描述矛盾，真机第一验证项） |

**Tiling 含义**：tileLen 选 32B 倍数的元素数（fp16: 16 的倍数；fp32: 8 的倍数），对齐块走 DataCopy，仅最后一块走 DataCopyPad，把 Pad 开销压到每行一次。

---

## 3. 向量计算与 ReduceSum 成本

### 3.1 计算链（每元素）

Pass1：Cast×2（若 half）→ Add → Mul（平方）→ ReduceSum
Pass2：Cast×2 → Add → Muls(scale) → Cast×2(gamma/bias) → Mul → Add → Cast(输出)

A2 上 Vector 单元对 FP32 的 Add/Mul 吞吐远高于 GM 带宽供给速度。粗算（推断）：FP32 向量每 cycle 处理 64 元素量级，D=4096 一行计算约数十 cycle，而一次 GM↔UB 搬运延迟在数百到数千 cycle。**计算不是瓶颈，DMA 与同步才是。**

### 3.2 ReduceSum

| 项 | 事实/争议 | 等级 |
| --- | --- | --- |
| 签名 | `ReduceSum(dstLocal, srcLocal, workLocal, count)` | A（API 文档） |
| workLocal 需求 | fp32 公式 `RoundUp(count/64, 8) * 8` 元素；当前 WORK_LEN=1024 覆盖 count≤4096 | B |
| count 上限争议 | 官方文档「受 UB 限制」；官方博客实测 ≈4096；官方源码注释 <255 repeat ≈16320 fp32 | B（本地调研1 已记录三方说法） |
| 统一处置 | **分块 ≤4096**，块间标量累加或向量累加 | 项目约定 |
| 9.x 参数名 | `workLocal` 可能改名 `sharedTmpBuffer` | B |

**成本结构**：ReduceSum 本身是向量归约，吞吐可接受；真正的开销在归约后的 **标量读回**（见 §4）。

---

## 4. 标量同步（GetValue / V_S）开销

### 4.1 当前基线的问题

当前 `ReduceRowSum` 对每个 tile 做：

```text
ReduceSum → PipeBarrier<V> → SetFlag/WaitFlag<V_S> → sum.GetValue(0)
→ SetFlag/WaitFlag<S_V> → PipeBarrier<V>
```

`GetValue` 强制 Vector 管线排空到 Scalar，是一次完整的流水线停顿。D=32768、tile=4096 时一行要同步 8 次；大 outer 时总同步次数 = outer × ceil(D/tile)，可能达到数万次。

### 4.2 优化方向（按收益排序）

1. **块间向量累加，行末一次 GetValue**
   - 用一个小的 FP32 累加缓冲（如 16–64 元素）保存部分和：每 tile 的 ReduceSum 结果用 `Add` 累进该缓冲，整行结束后只做一次 V_S + GetValue。
   - 同步次数：`outer` 次 → 从 `outer × tiles` 降一个数量级。
   - 实现成本：低。推荐作为 v2 必做项。

2. **单遍 + 整行一次归约**
   - D≤4096 时整行 y 在 UB，一次 ReduceSum + 一次 GetValue。同步次数 = outer。

3. **避免不必要的 S_V 回程屏障**
   - 当前 GetValue 后立刻 `SetFlag/WaitFlag<S_V>` 再 `PipeBarrier<V>`。若后续只是标量浮点运算（sqrtf、除法）而不立刻写回 Vector 寄存器依赖，可精简屏障链；需真机确认是否触碰未定义行为。

4. **sqrt 用向量 Sqrt 替代标量 sqrtf**
   - 整行 y 已在 UB 时，可 `Duplicate` scale 向量后用向量除法/Sqrt，避免标量路径。A2 Sqrt 为 0 ulp（本地调研1 结论），精度更好。
   - 代价：多一次 Duplicate + 向量指令，但省标量往返。

---

## 5. 多核负载

### 5.1 两种形态

| 形态 | 特征 | 按行切分 | 风险 |
| --- | --- | --- | --- |
| **大 outer 小 D** | 如 outer=8192, D=64–256 | 理想：行数远大于核数，负载均匀 | 行太短时 DMA 启动开销占比高；应合并多行一个 tile 或跨行打包 |
| **小 outer 大 D** | 如 outer=1–4, D=4096–32768 | 按行切分后多数核空转 | 需按 D 维切分并做跨核归约（复杂度高）；或接受少核执行 |

### 5.2 推荐策略

**默认（覆盖绝大多数测试点）**：按行均分，与当前一致。outer ≥ 核数时足够均衡。

**小 outer 大 D 的过渡方案**（outer < 核数/2 且 D > 4096）：

- 方案 A：仍按行切分，空核直接 return。实现最简单，损失并行度。
- 方案 B：按 tile 切分同一行的不同区段到多核，各核输出部分平方和到 GM workspace，再由 0 号核二次归约。需要额外 workspace 与同步，**复杂度高，仅在确认测试点含 outer=1 且 D 极大时再做**。
- 方案 C：单核内用更大 tile + 双缓冲掩盖延迟，放弃跨核并行。

**32B Cache Line 对齐约束（本地调研1 已确认的红线）**：D×s 非 32B 倍数时，行边界可能与相邻行落入同一 cache line。多核按行均分并发写回可能踩踏。切分粒度必须满足 `k × D × s ≡ 0 (mod 32)`。

**64 位寻址**：行基址强制 `uint64_t base = (uint64_t)row * D`，防止大张量偏移溢出。

---

## 6. 两遍扫描 vs 单遍暂存

| 维度 | 两遍扫描 | 单遍暂存（D≤4096） |
| --- | --- | --- |
| GM 读 x/residual | 2 次 | 1 次 |
| GM 流量（含 gamma/bias/output） | 7D–9D | 5D |
| UB 峰值 | 随 tile 线性，易控 | 需放下整行 y_fp32 + 工作区 |
| D 上限 | 无（分块） | 受 UB 限制，推荐 ≤4096 |
| 标量同步 | 每 tile 或每行 | 每行 1 次 |
| 实现复杂度 | 低（当前已实现） | 中（需整行缓冲管理） |
| 精度 | 分块 FP32 累加，已验证路径 | 整行一次 ReduceSum，精度更好 |
| 推荐 | D>4096 或正确性基线 | **D≤4096 时首选** |

**组合推荐**：运行时分支——`D ≤ 4096` 走单遍，`D > 4096` 走两遍 + gamma/bias 常驻 + 块间向量累加。

---

## 7. 大 tile vs 小 tile

| tileLen（fp16 元素） | UB 占用（两遍, BUFFER_NUM=1） | DMA 效率 | 尾块浪费 | ReduceSum 同步 | 建议 |
| --- | --- | --- | --- | --- | --- |
| 1024 | ~24 KB | 启动开销占比高 | 小 | 次数多 | 不推荐 |
| 2048 | ~48 KB | 较好 | 小 | 中 | fp32 路径可选 |
| **4096** | **~92 KB** | **好** | **小** | **可控** | **fp16/bf16 推荐** |
| 8192 | ~176 KB | 最好 | 小 | 次数少 | 无双缓冲时勉强；有双缓冲会超 UB |

**结论**：

- fp16/bf16：tile=4096 是 UB 与 DMA 效率的平衡点，与当前一致。
- fp32：tile=2048（元素）与 fp16 tile=4096 字节等价，保持当前。
- 双缓冲（BUFFER_NUM=2）时 tile 需减半（fp16 用 2048），用 tile 减半换 DMA/计算重叠，通常值得。

---

## 8. 双缓冲与 gamma/bias 常驻

### 8.1 gamma/bias 常驻（优先级最高、实现最简单）

- gamma、bias 形状均为 (D,)，所有行共享。
- 当前基线每行每 tile 都重新 CopyIn gamma/bias，纯浪费。
- **改法**：Init 时（或首行前）把 gamma/bias 一次搬入 UB 常驻缓冲，后续所有行/所有 tile 直接引用。
- 约束：D×s×2 ≤ 可用 UB。fp16、D≤8192 时约 32 KB，可接受；D=32768 时 128 KB，需与工作区权衡——此时可只常驻、两遍扫描时 Pass1 不需要 gamma/bias，Pass2 再按需搬。
- **收益**：两遍路径 GM 读从 9D→7D（约 22%）；大 outer 时更明显。

### 8.2 双缓冲（BUFFER_NUM=2）

- TQue 的 Ping-Pong：一块在计算时另一块在搬入/搬出，隐藏 DMA 延迟。
- UB 代价：队列缓冲 ×2。tile=4096 fp16 时，inX/inR/inG/inB/out 各翻倍，需重新做 UB 预算。
- 推荐配置（推断预算，真机需核对）：
  - fp16 双缓冲：tile 降为 2048，inX/inR 双缓冲 + out 双缓冲 + FP32 工作区，约 100–120 KB。
  - 或保持 tile=4096 但只对 inX/inR 双缓冲（最热的搬运），gamma/bias 常驻不分块，out 单缓冲。
- **适用**：大 D 多 tile 行（D/tile ≥ 2）时收益明显；D ≤ tile 单 tile 行时双缓冲无意义。

### 8.3 CopyIn/Compute/CopyOut 流水

完整流水需要三层重叠。当前同步写法（Alloc→Copy→EnQue→DeQue→计算→Free）是全停顿的。双缓冲是第一步；再进一步是异步事件（`EnQue`/`DeQue` 配合 `HardEvent::MTE2_V` 等）精细控制，实现复杂度显著上升，建议真机 profile 后再做。

---

## 9. 分场景 tiling 推荐表

| 场景 | dtype | D 范围 | outer | 扫描 | tile | BUFFER_NUM | gamma/bias | 标量同步 | 预估 GM 流量 | 推荐级别 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S0 当前基线 | 全部 | 全范围 | 全范围 | 两遍 | half 4096 / float 2048 | 1 | 每块重读 | 每 tile | 9D | 正确性基线 |
| S1 常驻+块间累加 | fp16/bf16 | >4096 | ≥核数 | 两遍 | 4096 | 1 | Init 常驻 | 行末 1 次 | 7D | **第二步，低风险** |
| S2 双缓冲 | fp16/bf16 | >4096 | ≥核数 | 两遍 | 2048 | 2 | 常驻 | 行末 1 次 | 7D | 第三步 |
| S3 单遍突破 | fp16/bf16 | ≤4096 | ≥核数 | 单遍 | 整行 | 1 | 常驻 | 行末 1 次 | 5D | **首选性能路线** |
| S4 单遍+双缓冲 | fp16/bf16 | ≤4096 | ≥核数 | 单遍 | 整行/半行 | 1–2 | 常驻 | 行末 1 次 | 5D | S3 之后 |
| S5 fp32 路径 | fp32 | ≤2048 | ≥核数 | 单遍 | 整行 | 1 | 常驻 | 行末 1 次 | 5D | 同 S3 逻辑 |
| S6 小 outer 大 D | 全部 | >4096 | <核数/2 | 两遍 | 4096 | 1–2 | 常驻 | 行末 1 次 | 7D | 接受少核；跨核归约仅研究 |
| S7 短行打包 | fp16 | ≤256 | 极大 | 单遍 | 多行打包 | 1 | 常驻 | 行组末 | 5D | 减少 DMA 启动次数 |

---

## 10. 性能路线优先级

按「收益 / 实现成本 / 风险」排序，供真机阶段按序落地：

| 优先级 | 路线 | 预期收益 | 成本 | 风险 |
| --- | --- | --- | --- | --- |
| P0 | 正确性闭环（四大红线：DataCopyPad 参数、V_S 同步、64 位基址、32B 对齐） | 否则无性能可谈 | — | — |
| P1 | **gamma/bias 常驻** | GM 读 −22%（两遍路径） | 极低 | 低 |
| P2 | **块间向量累加，行末一次 GetValue** | 大幅降低流水线停顿 | 低 | 低 |
| P3 | **D≤4096 单遍暂存** | GM 流量 −28.6%，同步减半 | 中 | UB 预算需真机核对 |
| P4 | **双缓冲 BUFFER_NUM=2**（tile 相应调整） | 隐藏 DMA 延迟，大 D 收益明显 | 中 | UB 超限 |
| P5 | 短行多行打包 | 减少 DMA/同步固定开销 | 中 | 尾块与对齐复杂化 |
| P6 | 小 outer 大 D 跨核 tile 切分 | 提升极端点并行度 | 高 | 需 workspace 与二次归约 |
| P7 | 向量 Sqrt / 全向量 scale | 精度与微小性能 | 中 | 需真机验证 API |

每步真机验证顺序：`npu-smi info` 确认 SoC → `./run.sh` 精度 → `msopst` 或手写计时 → 记录 15 点覆盖矩阵耗时 → 决定是否进入下一步。

---

## 11. Profiling 方法

| 工具 | 用途 | 适用阶段 | 证据等级 |
| --- | --- | --- | --- |
| `npu-smi info` | 查看 SoC 型号、核数、HBM、驱动版本 | 环境确认第一步 | A（工具存在，模板与文档均提及） |
| `npu-smi` 监控 | 利用率、显存、功耗 | 粗粒度观察 | B |
| `msopst` | 算子级正确性与性能测试 | 精度 + 单算子计时 | B（官方算子测试工具） |
| CANN Profiling（msprof） | 指令级 timeline、流水线气泡、DMA/Vector 占比 | 定位同步与搬运瓶颈 | B（官方 profiling 文档） |
| `ascend-dmi` | 设备管理与诊断 | 环境问题排查 | B |
| 手写计时（模板 main.asc 的 stream sync + host timer） | 直调工程端到端耗时 | 与判题口径最接近 | A（模板自带 `aclrtSynchronizeStreamWithTimeout`） |

**Profiling 关注点（真机）**：

1. MTE2（搬入）与 V（计算）的时间占比 → 判断是否 memory-bound。
2. 每行/每 tile 的 V_S 停顿次数 → 验证 P2 收益。
3. 尾块 DataCopyPad 耗时占比 → 决定是否需要手工尾块路线。
4. 多核负载直方图 → 判断按行切分是否均衡。

**注意**：本机无 CANN/NPU，以上均为真机阶段操作指引，本报告不声称已执行。

---

## 12. 已确认 / 未找到 / 无法确认

### 已确认（有证据支持）

- 模板默认 SoC 为 `dav-2201`（CMakeLists.txt，A 级）。
- 核数通过 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 运行时获取（main.asc，A 级）。
- 计分公式 `100/(1+log1.5(t/T))`，15 点均值（题目 API，A 级）。
- 当前基线参数：TILE_HALF=4096、TILE_FLOAT=2048、BUFFER_NUM=1、两遍扫描（源码，A 级）。
- A2 训练/800I A2 支持 DataCopyPad；200/500 A2 不支持（本地调研1，B 级）。
- A2 Add/Mul 不支持 bfloat16_t，必须 FP32 域计算（本地调研1，B 级）。
- ReduceSum count 上限存在 4096 / 16320 争议，项目统一按 ≤4096 分块（本地调研1，B 级）。
- 单遍暂存相对两遍可省 28.6% GM 流量（访存模型推导 + 本地问题文档，分析结论）。
- 官方样例产品矩阵确认 A2 训练/800I A2 为 Ascend C 主要目标平台（gitee ascend/samples，B 级）。

### 未找到（本次会话未能获取）

- hiascend.com 文档正文（站点 JS 渲染，webfetch 仅返回导航壳）——UB 192 KB、DataCopyPad 性能数字、ReduceSum 精确上限均未能从官方文档正文直接引用。
- dav-2201 的官方 HBM 带宽与 Vector TFLOPS 规格表。
- 判题机 SoC 的最终确认。
- CANN 9.0.0 profiling 文档的具体命令行与指标名。

### 无法确认（需真机）

- UB 在 dav-2201 上的实际可用字节数（`GetLibApiWorkSpaceSize` 扣除后）。
- DataCopy 与 DataCopyPad 的实测吞吐差。
- GetValue / V_S 单次开销的绝对时间。
- 双缓冲的实际加速比。
- 判题机 vector core 数。
- 减少标量同步后精度是否仍满足 1e-3/1e-4 阈值。

---

## 13. 来源

| # | 来源 | URL / 路径 | 标题 | 访问日期 | 等级 | 用途 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 平台直调模板 | `~/Downloads/addrmsnormbias_problem_1742_template/CMakeLists.txt` | AddRmsNormBias 模板 CMake | 2026-09-11 | A | dav-2201 默认架构 |
| 2 | 平台直调模板 | `~/Downloads/addrmsnormbias_problem_1742_template/main.asc` | 模板 main（核数查询） | 2026-09-11 | A | ACL_DEV_ATTR_VECTOR_CORE_NUM |
| 3 | 本地源码基线 | `源码/op_kernel/add_rms_norm_bias.cpp` | 当前 kernel 实现 | 2026-09-11 | A | tile/buffer/两遍参数 |
| 4 | 本地题面分析 | `文档/problem-add-rms-norm-bias.md` | 题目分析与性能路线 | 2026-09-11 | A/B | 语义、风险、v1–v4 路线 |
| 5 | 本地编译说明 | `文档/source-build.md` | 源码编译说明 | 2026-09-11 | A/B | 版本敏感点、API 争议 |
| 6 | 调研覆盖计划 | `调研/调研2/coverage-plan.md` | 十代理职责与模板核对 | 2026-09-11 | A | 模板内容、证据分级 |
| 7 | 官方样例仓 | https://gitee.com/ascend/samples/tree/master/operator/ascendc | Ascend C 算子调用样例 README | 2026-09-11 | B | A2 产品支持矩阵、样例目录 |
| 8 | 官方样例仓（GitHub 镜像） | https://github.com/Ascend/samples | Ascend samples | 2026-09-11 | B | 样例仓存在与结构 |
| 9 | 官方文档入口（未取到正文） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/ | CANN 9.0 开发文档入口 | 2026-09-11 | D（仅入口） | 计划核对 API/UB/profiling；本会话未取到正文 |
| 10 | 官方文档入口（未取到正文） | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/... | CANN 8.0.RC3 商用文档 | 2026-09-11 | D（仅入口） | 同上 |
| 11 | 昇腾硬件产品页 | https://www.hiascend.com/hardware/ai-server | Atlas 服务器产品页 | 2026-09-11 | D | 页面 JS 壳，未取到 A2 规格表 |
| 12 | 推断/分析 | 本报告 §2、§6–§8 | 访存模型与 UB 预算 | 2026-09-11 | 推断 | 标注为推断，非实测 |

> 说明：本会话尝试多次抓取 hiascend.com 具体文档页，均返回导航壳或 403/404，未能获得官方正文。UB=192KB、HBM 带宽等数字在报告中已明确标注证据等级与「推断」字样。真机阶段应以目标机 `kernel_operator.h`、CANN 9.0.0 手册与 `npu-smi` 输出为准重新核对。
