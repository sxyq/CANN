# Agent 10：证据审阅、方案合并与反例验证

> 日期：2026-09-12
> 输入：agent01–agent09 九份报告 + 本地源码/提交/模板/题面文档（只读）
> 性质：交叉审阅 + 方案合并 + 反例清单。不改主线源码/提交/文档；不上传；不声称 NPU 验证。
> 本机状态：macOS，无 CANN / 无 Ascend C 编译器 / 无昇腾 NPU。下列结论全部为文档与源码静态交叉核对。

---

## 0. 执行摘要

九份报告在以下六条主线上高度一致，可直接作为 V003 基线：

1. residual add、平方、最后一维归约、epsilon、平方根、归一化、gamma、bias 必须全部在 Kernel 内完成；FP32 中间计算是强制路径，不是偏好。
2. 多核按行均分（outer 维 parallel），D 维不得跨核切分。
3. D 非 32 倍数的尾块用 DataCopyPad 搬入补 0；搬出写回长度 = 有效字节数。
4. 所有 GM 行基址用 `uint64_t`。
5. 提交物只有 `kernel.asc`，入口与模板逐字对齐 `run_kernel` + `__global__ __vector__`。
6. 性能是唯一计分维度；正确性闭环后优先攻 gamma/bias 常驻、块间向量累加、D≤4096 单遍。

存在两处证据冲突（DataCopyPadExtParams 字段顺序、GetReduceRepeatSumSpr 可移植性）和一处常见误解（mojo_opset Triton 的定位），必须按本报告 §1 的写法处置，不得单方面宣布对错。

---

## 1. 证据冲突与缺口清单

### 1.1 DataCopyPadExtParams 字段顺序（真机第一验证项）

| 来源 | 声称的字段顺序 | 证据 |
| --- | --- | --- |
| 项目旧文档 `文档/problem-add-rms-norm-bias.md` §7/§9 | `isPad, paddingValue, leftPadding, rightPadding` | 旧笔记整理，无官网原文引用 |
| Agent 02（CANN 9.0.0 官方页 0265 表6 + 官方示例） | `isPad, leftPadding, rightPadding, paddingValue` | 官方示例 `DataCopyPadExtParams<half> padParams{true, 0, 2, 0};` 释义为 isPad=true, left=0, right=2, value=0；A 级 |

两种顺序下的同一聚合初始化 `{isPad, 0, right, 0}` 含义完全相反：

- 若官方序为 `isPad, left, right, value` → `{isPad, 0, right, 0}` 恰好正确。
- 若官方序为 `isPad, value, left, right` → `{isPad, 0, right, 0}` 把 right 填进 left，尾块语义静默出错。

**本地代码实际情况（只读核对，与两份报告的描述略有出入）**：

| 版本 | 代码写法 | 位置 | 对字段序的敏感性 |
| --- | --- | --- | --- |
| 源码 V001 | 聚合初始化 `pad{true, 0, 0, static_cast<T>(0)}` | `add_rms_norm_bias.cpp` L115 | 敏感。按 Agent02 序解读为 isPad=true, left=0, right=0, value=0（右 padding 未补）；按旧文档序解读为 isPad=true, value=0, left=0, right=0（同样未补）。两种解读都未补右 padding，尾块仅靠 `blockLen=len*sizeof(T)` 非对齐搬入 |
| 提交 V002 | **逐字段赋值** `pad.isPad=…; pad.paddingValue=0; pad.leftPadding=0; pad.rightPadding=…` | `提交/V002/kernel.asc` L67–71 | **不敏感**。字段名绑定，与声明顺序无关。Agent 02 报告把 V002 描述成聚合初始化 `{isPad, 0, right, 0}`，与当前文件内容不符；当前 V002 已是安全写法 |

**合并结论（不得单方面宣布 V002 对或错）**：

1. 文档表序 + 官方 brace-init 示例强烈支持 `isPad, leftPadding, rightPadding, paddingValue`，但本机无 `kernel_struct_data_copy.h`，无法做字节级复核。证据冲突保留。
2. 当前 V002 用逐字段赋值，不依赖字段顺序，两种顺序下都正确。旧文档把 V002 判为「聚合初始化致死」与当前文件内容不符，应视为已过时描述。
3. 真机第一验证项：用 `#include` 的头文件或运行时打印确认 `offsetof(DataCopyPadExtParams<T>, paddingValue/leftPadding/rightPadding)`；同时用 D=67/129/1000 做搬入/搬出逐字节核对。
4. **V003 强制安全写法（两种顺序下都正确）**：

```cpp
// 写法 A：Designated Initializers（C++20 / 毕昇若支持）
DataCopyPadExtParams<T> pad{.isPad = true, .leftPadding = 0,
                            .rightPadding = right, .paddingValue = T(0)};

// 写法 B：逐字段赋值（当前 V002 写法，兼容性最好）
DataCopyPadExtParams<T> pad;
pad.isPad = (right != 0);
pad.leftPadding = 0;
pad.rightPadding = static_cast<uint8_t>(right);
pad.paddingValue = static_cast<T>(0);

// 禁止：任何形式的裸聚合初始化 DataCopyPadExtParams<T> pad{a,b,c,d};
```

### 1.2 GetReduceRepeatSumSpr 可移植性

| 来源 | 结论 | 证据 |
| --- | --- | --- |
| 旧文档 §9 风险表 | 「架构不兼容」，统一改 `ReduceSum + GetValue(0)` | 旧笔记，未给具体架构差异 |
| Agent 02（CANN 9.0.0 专页 0225） | 官方提供 `template <typename T> T GetReduceRepeatSumSpr()`，T 支持 half/float，官方示例直接跟在 ReduceSum 后 | A 级 |

**合并结论**：A 级官方确认 9.0.0 存在此接口，旧文档「不存在/不兼容」的说法不成立；但该接口与 ReduceSum 的具体实现绑定（读取的是「上次 ReduceSum 的 repeat 累计和」），在跨版本/跨架构移植时语义不透明。**V003 优先 `sum.GetValue(0)` + 显式 `HardEvent::V_S`/`S_V` 同步**（当前 V002 已是此写法），GetReduceRepeatSumSpr 仅作备选，不作为主线依赖。

### 1.3 官方开源无 AddRmsNormBias 完整 kernel vs mojo_opset Triton

| 来源 | 结论 |
| --- | --- |
| Agent 03 | Ascend 官方 GitHub/GitCode 全仓检索：无 AddRmsNorm / RMSNorm 的 Ascend C kernel 源码；op-plugin 只是 ACL 封装；mojo_opset `fused_add_rms_norm.py` 是 Triton（TTX 后端）实现，两遍扫描 + FP32 归约，无 bias；README 写明 Ascend C 后端是 Future Work |
| Agent 04/05 | GPU/Triton 生态无「RMSNorm + residual + post-bias」一字不差的实现；最接近的是 Liger 融合骨架 + flash-attn beta 位置的拼接 |

**合并结论**：mojo_opset Triton 版是**算法正确性对照物**（两遍、FP32 归约、列分块、按行分核结构同构），**不是代码迁移源**。不要把 Triton 语法、`tl.sum`、mask 语义直接翻译成 Ascend C。官方开源没有可抄的 Ascend C AddRmsNormBias；本题必须自写。

### 1.4 其余交叉确认但需真机落实的冲突/缺口

| # | 事项 | 状态 | 处置 |
| --- | --- | --- | --- |
| 1 | ReduceSum count 上限：文档「受 UB 限制」/ 官方博客 ≈4096 / 源码注释 ≈16320 | 三方不一致 | 统一分块 ≤4096；真机再测 |
| 2 | DataCopyPad 搬出（UB→GM）是否覆盖相邻行：官方称自动丢弃 dummy；社区 C 级实测称 32B burst 可能覆盖 | A vs C 冲突 | 真机第一验证项（与 §1.1 并列）；D=67/129/1000 逐字节核对 |
| 3 | UB 容量 192KB：教程长期引用，9.0.0 官方正文本轮未独立抓到 | B/C | 真机 `GetLibApiWorkSpaceSize()` 扣除后实测 |
| 4 | 判题 SoC：模板默认 dav-2201；题面未明文 | A 模板 / 5 | 真机 `npu-smi info` + 平台确认 |
| 5 | 判题端精度算法是否与本地 `verify_result.py`（rtol=0.001, atol=0.001, tol=0.001, equal_nan）一致 | 5 | 无法从外部确认 |
| 6 | 15 个测试点的 shape/dtype/eps | 5 | ranking 只暴露 id/tbest |
| 7 | 标量 `sqrtf` 在 bf16 大 D 的 ulp | 未测 | Agent06 实验显示 rsqrt 误差 ≥2^-10 时中大 D 击穿阈值；真机改向量 Sqrt 或确认误差界 |
| 8 | BF16 标量 cast（tilelang-ascend #1762：BiSheng dav-2201 报 `not support bf16 type cast`） | B | 真机确认；必要时走 `AscendC::ToFloat` |
| 9 | 多核 32B Cache Line 撕裂：`k × D × sizeof(T) ≡ 0 (mod 32)` 粒度 | 本地文档红线 + Agent07/09 | 真机多核并发写回验证 |
| 10 | 每日约 50 次提交限制 | 2 | 题面/前端无明文；保守记账 |
| 11 | `__vector__` vs `__aicore__` 完整语义差异 | 模板 A 级要求 `__vector__`；API 页未展开 | 以模板为准 |
| 12 | CANN 9.0.0 头文件 `kernel_struct_data_copy.h` 本体 | 未取得 | 真机第一件事读头文件确认字段序 |

---

## 2. 14 条方案评估表

字段：方案 | 算法 | 访存次数（相对一行 D 元素的 GM 流量倍数） | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 | 来源

访存次数按「x/residual 各算 1 次读 + output 1 次写」为基准 3D 计；gamma/bias 共享按是否常驻折算。

| # | 方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 | 来源 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 两遍扫描 | Pass1 归约平方和；Pass2 归一化+gamma+bias 写回 | 7D–9D（gamma/bias 常驻后 7D；每 tile 重读 9D） | tile 线性，T=4096 约 92KB（fp16） | 低：分块 FP32 已 CPU 验证 | 低：尾块一次 Pad | 中：正确性基线；带宽冗余 | 低（V001/V002 已实现） | 低 | **首选（正确性基线）** | A6/A4/A5/A7 |
| 2 | 单遍暂存中间 y | UB 内保留整行 y_fp32，一次归约后 epilogue | 5D（省 28.6%） | D≤4096 约 50–60KB；D=32768 必须回退 | 低：整行一次 ReduceSum 精度更好 | 中：整行缓冲管理 | 高：D≤4096 首选性能路径 | 中 | 低 | **可作为第二路线** | A4/A7/A5 |
| 3 | FP32 全中间计算 | Cast→FP32→算→Cast 回 | 不变 | 不变 | **强制**：低精度归约在 D≥129 系统性 FAIL | 无 | 无额外成本 | 低 | 低 | **首选（强制）** | A6/A4/A2 |
| 4 | 低精度中间计算 | 全程 fp16/bf16 累加 | 略省 | 略省 | **致命**：bf16 尾数 8 位，D=32 起失配；fp16 D=1024 失配率 >80% | 无 | 理论上省带宽 | 低 | 低 | **不建议** | A6（定量否决） |
| 5 | 大 tile | tile 尽量大（≤UB） | 不变 | T=8192 约 176KB，双缓冲会超 | 低（块内 FP32） | 低 | 高：DMA 效率好 | 低 | 低 | **可作为第二路线**（UB 紧时慎用） | A7/A5 |
| 6 | 小 tile | 分块 ≤4096，块间 FP32 合并 | 略增（块间合并） | 小 | 低：CPU 实测 chunk=4096 失配率 0 | 中：块边界 | 中：大 D 必须 | 低 | 低 | **首选（大 D 必须）** | A6/A7/A5 |
| 7 | 按行分配 AI Core | outer 维 parallel，按行均分 | 不变 | 不变 | 无：行间独立 | 无 | 高：outer≥核数时理想 | 低（V001/V002 已实现） | 低 | **首选** | A5/A4/A7 |
| 8 | 按 tile 分配 AI Core | 把同一行的 D 切给多核 | 不变 | 需 workspace | **高**：跨核归约需二次合并 | 高 | 低：仅 outer=1 且 D 极大时有意义 | **高** | 中 | **不建议** | A5（reduction 维不能跨核） |
| 9 | DataCopyPad 尾块 | 搬入补 0；搬出写有效长度 | 不变 | 不变 | 低（补 0 不影响平方和）；分母必须是原始 D | **高**：搬出是否覆盖相邻行有 A/C 冲突 | 中 | 低 | 中（字段序 + 写方向） | **首选（非 32 倍数 D）** | A2/A6/A9 |
| 10 | 手工尾块 | 对齐块 DataCopy + 尾部标量/窄 mask | 不变 | 不变 | 低 | 中：需自行 mask | 中 | 中 | 低 | **可作为第二路线**（DataCopyPad 写方向异常时的后备） | A4/A5/A7 |
| 11 | ReduceSum | 官方归约原语 | 不变 | work 公式化 | 低（FP32） | 无 | 高 | 低 | 中（count 上限争议 → 分块 ≤4096） | **首选** | A2/A6/A7 |
| 12 | 手工向量归约 | Mul→分段累加→标量合并 | 不变 | 小 | 低 | 无 | 中 | 中 | 低 | **可作为第二路线**（ReduceSum 异常时） | A4/A5 |
| 13 | 纯 Ascend C | 手写 `kernel.asc`，不依赖自动工具 | — | — | — | — | — | — | **唯一合法提交路径** | **首选（唯一提交路径）** | A5（自动生成路线均不产 `kernel.asc`） |
| 14 | CUDA/Triton 迁移参考 | 借数据流与精度策略，不抄指令 | — | — | — | — | — | — | — | **仅作研究参考** | A4/A3/A5 |

**表内关键判据（合并后）**：

- 访存次数：Agent07 §2.1 推导。当前 V002 基线在 Pass2 对 gamma/bias 也 CopyIn，为 9D；gamma/bias 常驻后 7D；单遍 5D。
- 精度：Agent06 定量实验。低精度中间在题面 D 范围内没有安全区，五条独立否决理由（归约误差、吸收效应、bf16 尾数、fp16 溢出窗口、A2 硬件不支持 bf16 加乘）。
- 尾块：Agent09 收集的 tilelang-ascend #1682/PR#1777 实证（非对齐 DataCopyPad 静默损坏，max_diff=896）。
- 兼容：Agent02 确认 REGISTER_TILING / GET_TILING 在直调工程「暂不支持」；Agent05 确认 TVM/MLIR/IREE/Triton/PyPTO/msopgen 均不能产出符合 CANNJudge 直调格式的 `kernel.asc`。

---

## 3. 首选实现路线（V003 基线）步骤级描述

V003 定位：正确性绝对优先的两遍扫描，修复 V001/V002 已知红线，作为后续所有性能优化的回归基线。建议目录 `提交/V003/kernel.asc`。

### 3.1 结构与入口

1. 单文件 `kernel.asc`，`#include "kernel_operator.h"`，不加 main / pragma once / include guards（模板注释明确）。
2. 入口逐字对齐模板：

```cpp
extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& info_x,
    GM_ADDR residual, const TensorGroupInfo& info_residual,
    GM_ADDR gamma, const TensorGroupInfo& info_gamma,
    GM_ADDR bias, const TensorGroupInfo& info_bias,
    GM_ADDR output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
```

3. `run_kernel` 内解析 `TensorGroupInfo` → `outer / dim / dtype`，校验后 `add_rms_norm_bias_custom<<<blocks, nullptr, stream>>>(...)`。
4. 内核入口 `extern "C" __global__ __vector__ void add_rms_norm_bias_custom(...)`。dtype 按模板编码 0=fp32 / 1=fp16 / 2=bf16 分派三个模板实例。
5. **禁止**：`REGISTER_TILING_DEFAULT`、`GET_TILING_DATA_WITH_STRUCT`（直调工程不支持）；禁止 Host 代算；禁止写死测试输入/输出。

### 3.2 输入校验（run_kernel 侧）

1. 每个 TensorGroupInfo：`numTensors==1`，shape 非空，numDims∈[2,4]，各维 >0。
2. `x/residual/output` 同 shape；`gamma/bias` 为 1 维且 `shape[0]==x.shape[last]`；五者 dtype 一致且 ∈{0,1,2}。
3. 元素总数用 `uint64_t` 累乘并确认无溢出（`MAX_ELEMENTS=0xffffffffULL` 上限按题面 batch/seq/D 范围取）。
4. `outer = element_count / dim`；`availableCoreNum<=0` 时取 1；`blocks = min(availableCoreNum, outer)`。
5. 校验失败直接 return（不启动 kernel），避免非法访存 RE。

### 3.3 Kernel 类骨架

1. 一个 `TPipe`；`TQue<VECIN,1>` × 4（x/residual/gamma/bias）+ `TQue<VECOUT,1>`；`TBuf<VECCALC>` × 4（xF/rF/yF/work）+ sum 小缓冲。
2. tile：fp16/bf16 = 4096 元素；fp32 = 2048 元素。`WORK_LEN=1024` 覆盖 count≤4096 的 sharedTmpBuffer 需求。
3. **保留标识符全文扫描**（Agent09）：禁止用户变量名 `pipe_`、`block_idx`、`block_num`、`ubuf`、`tid`。成员名用 `x_queue_`/`tpipe` 风格，避开 `pipe_` 前缀（V001 已因此 15/15 CE）。
4. 所有 GM 偏移：`uint64_t base = static_cast<uint64_t>(row) * dim_;`；tile 内偏移也用 `uint64_t`。

### 3.4 多核按行均分（方案 7）

```cpp
uint32_t block = GetBlockIdx();
uint32_t blocks = GetBlockNum();
uint32_t each = outer / blocks;
uint32_t extra = outer % blocks;
uint32_t first = block * each + (block < extra ? block : extra);
uint32_t count = each + (block < extra ? 1 : 0);
if (count == 0) return;
```

与 V002 一致，每核行数差 ≤1。不做跨核 D 切分（方案 8 否决）。

### 3.5 Pass1：归约（方案 1 + 6 + 11）

每行循环 tile：

1. **CopyIn**：`DataCopyPad` GM→UB。`DataCopyExtParams{1, len*sizeof(T), 0, 0, 0}`；`DataCopyPadExtParams` 用 §1.1 写法 B（逐字段赋值），`isPad=(aligned_len-len)!=0`，`rightPadding=aligned_len-len`，`paddingValue=T(0)`。对齐块可走 `DataCopy`（V001 分支）或统一 DataCopyPad（V002 写法，更简单）。
2. **FP32 链**：fp16/bf16 先 `Cast(..., CAST_NONE, len)` 到 FP32；`Add(yF, xF, rF, len)`；`Mul(yF, yF, yF, len)` 平方。**计算长度用 `calc_len = AlignedLen(valid_len)`**（V002 写法），保证向量指令对齐；补 0 部分平方后仍为 0。
3. **归约**：`ReduceSum(sum, yF, work, calc_len)`；`PipeBarrier<PIPE_V>`；`FetchEventID(HardEvent::V_S)` → Set/Wait；`total += sum.GetValue(0)`；`FetchEventID(HardEvent::S_V)` → Set/Wait；`PipeBarrier<PIPE_V>`。不用 `GetReduceRepeatSumSpr`（§1.2）。
4. 块间用标量 `float total` 累加（V002 现状）。V003 可先保留；性能阶段再改「块间向量累加，行末一次 GetValue」（Agent07 P2）。
5. **分母锁死**：`rms = sqrtf(total / static_cast<float>(dim) + epsilon)`。分母永远是原始 `dim`，不是 `aligned_len`，不是 `calc_len`（Agent06 实验：D=1000 误用 pad 长度时 max_rel=37）。

### 3.6 Pass2：归一化 + 仿射 + 写回

每行循环 tile：

1. CopyIn x/residual/gamma/bias（同 Pass1 尾块策略）。
2. `y = x + residual`（FP32）；`Muls(y, y, scale, calc_len)`，`scale = 1.0f / rms`。
3. 仿射：fp32 输入直接 `Mul(y, y, g)` + `Add(y, y, b)`；fp16/bf16 先 Cast gamma/bias 到 FP32 再 Mul/Add（A2 无 bf16 加乘，Agent02 §2.2）。
4. **输出 cast**：`Cast(out_local, yF, RoundMode::CAST_RINT, len)`（RNE，与 golden `astype` 一致）；fp32 输入可直接 DataCopy 从 yF 搬出。
5. **CopyOut**：`DataCopyExtParams{1, len*sizeof(T), 0, 0, 0}`；`DataCopyPad(dst[base+off], src, params)`。写回长度 = 有效字节数，不用 aligned_len。
6. 运算次序严格按 golden：`y / rms * gamma + bias`，即 `Muls(scale)` 后 `Mul(gamma)` 后 `Add(bias)`（Agent06 实验：`(y*g)/rms+b` 有 1 ulp 级差异，虽过阈值但不逐位对齐）。

### 3.7 精度链锁定（与 golden 同构）

```
CopyIn:  DataCopyPad GM→UB，尾块补 0
Cast:    fp16/bf16 → FP32（CAST_NONE）
Add:     y = x + residual          （FP32）
Square:  y2 = y * y                （FP32）
Reduce:  sum = Σ y2，分块 ≤4096，块间 FP32 合并
Mean:    mean = sum / D            （分母 = 原始 D）
Rms:     rms = sqrt(mean + eps)    （标量 sqrtf 起步；真机后评估向量 Sqrt）
Norm:    t = y * (1/rms)           （Muls）
Affine:  t = t * gamma + bias      （FP32）
Cast:    FP32 → 目标 dtype（CAST_RINT）
CopyOut: DataCopyPad UB→GM，长度 = 有效字节数
```

### 3.8 V003 提交前静态核对表（来自 Agent08/09）

| # | 核对项 | 动作 |
| --- | --- | --- |
| 1 | 保留标识符 | `grep -nE 'block_idx|block_num|pipe_|ubuf|\btid\b' kernel.asc`，用户代码零命中 |
| 2 | DataCopyPad 字段 | 仅逐字段赋值或 Designated Initializers；无裸聚合初始化 |
| 3 | 64 位偏移 | 所有 `base`/`offset` 为 `uint64_t` |
| 4 | 分母 | `static_cast<float>(dim)`，无 aligned_len |
| 5 | dtype 编码 | 直调侧 0/1/2；与源码工程的 1/2/3 不混用 |
| 6 | 入口 | `__global__ __vector__`；`run_kernel` 签名逐字对齐模板 |
| 7 | 无 tiling 宏 | 不出现 REGISTER_TILING / GET_TILING |
| 8 | Cast 舍入 | 上行 CAST_NONE，下行 CAST_RINT |
| 9 | BF16 路径 | 无 `Add<bfloat16_t>` / `Mul<bfloat16_t>` / `ReduceSum<bfloat16_t>` |
| 10 | NaN/Inf | 无 clamp、无有限化分支 |
| 11 | TQue 数量 | 同 TPosition 队列数 ≤8（A2 eventID 限制）；当前 4+1 安全 |
| 12 | UB 预算 | 静态加总 < 192KB（按 tile=4096 fp16 约 92KB，有余量） |

---

## 4. 第二候选路线（性能：gamma/bias 常驻 + D≤4096 单遍）

定位：V003 真机精度闭环后的性能版本，对应 Agent07 的 P1+P3 组合，即场景 S1→S3。

### 4.1 触发条件

- `D ≤ 4096` 且输入为 fp16/bf16（fp32 时阈值降为 2048，因元素字节翻倍）。
- `D > 4096` 时回退 V003 两遍扫描 + 下面的常驻/累加优化（不单遍）。
- 运行时分支：`if (dim <= 4096) SinglePass(); else TwoPass();`。

### 4.2 gamma/bias 常驻（P1，两条路线通用）

1. Init 阶段把整行 gamma/bias 一次搬入 UB 常驻缓冲（fp16、D=4096 时约 16KB）。
2. 所有行、所有 tile 直接引用，不再 CopyIn。
3. 两遍路径 GM 读从 9D→7D（约 22%）；单遍路径从 6D→5D。
4. D=32768、fp16 时 gamma/bias 常驻需 128KB，与工作区冲突——此时只在 Pass2 按 tile 搬入，或整行搬入后分块引用（真机 UB 预算决定）。

### 4.3 单遍暂存（P3）

1. 每核每行：CopyIn x/residual 一次 → Cast FP32 → Add 得 y_fp32 整行暂存于 UB → Mul 平方 → **一次** ReduceSum（D≤4096 时 count 在限内）→ 一次 GetValue → 标量算 rms/scale → 用暂存的 y_fp32 做 Muls + Mul(gamma) + Add(bias) → Cast → CopyOut。
2. GM 流量 5D（x 1 次 + residual 1 次 + output 1 次；gamma/bias 常驻）。
3. 标量同步次数 = outer（每行 1 次），相对 V003 基线的 `outer × tiles` 降一个数量级。
4. UB 预算（fp16, D=4096, 推断）：y_fp32 16KB + gamma/bias 16KB + 输出 8KB + x/residual 输入缓冲可复用 + 工作区 8–16KB ≈ 50–60KB，远低于 192KB。

### 4.4 配套优化（按 Agent07 优先级）

| 优先级 | 动作 | 预期 | 风险 |
| --- | --- | --- | --- |
| P2 | 块间向量累加，行末一次 GetValue | 大 D 多 tile 行同步次数降一个数量级 | 低 |
| P4 | 双缓冲 BUFFER_NUM=2（tile 相应降为 2048 fp16） | 隐藏 DMA 延迟 | UB 超限，需真机预算 |
| P5 | 短行多行打包（D≤256 且 outer 极大） | 减少 DMA 启动固定开销 | 尾块与对齐复杂化 |
| P7 | 向量 Sqrt / 全向量 scale 替代标量 sqrtf | 精度更好 + 微小性能 | API 需真机确认 |

### 4.5 明确不做

- 不做按 tile 分配 AI Core（方案 8）：reduction 维跨核需 workspace + 二次归约，复杂度远超收益，仅 outer=1 且 D 极大时才值得，而此时单核大 tile + 双缓冲通常已够。
- 不做低精度中间计算（方案 4）：Agent06 定量否决。
- 不做跨核 D 切分的「部分平方和写 GM 再合并」（Agent07 S6-B）：除非确认判题点含 outer=1 且 D=32768 且单核两遍已 TLE，否则不碰。

---

## 5. 每个推荐方案的失败条件 / 反例

### 5.1 首选组合（1+3+6+7+9+11+13）——V003 基线

| 风险 | 触发条件 | 反例 | 后果 | 检测/后备 |
| --- | --- | --- | --- | --- |
| DataCopyPadExtParams 字段序错填 | 若改回裸聚合初始化且顺序假设错误 | D=67，right_padding 填进 leftPadding | 整行数据右移，全错（WA） | 真机读头文件；强制逐字段赋值 |
| 搬出覆盖相邻行 | D×sizeof(T) 非 32B 倍数 + 行间紧凑布局 | D=67/129/1000，fp16 | 下一行开头被踩（WA/RE） | 真机逐字节核对；后备方案 10 手工尾块 |
| 分母误用 pad 长度 | `rms = sum/aligned_len` 而非 `sum/D` | D=1000（pad 到 1024） | max_rel≈37，失配率 92% | 代码审查锁死 `static_cast<float>(dim)` |
| 低精度累加 | 归约或 Add/Mul 留在 fp16/bf16 | bf16，D=64 | 失配率 >50%（Agent06 实验 1） | 方案 3 强制；代码无 bf16 算术指令 |
| eps 位置错 | `sqrt(mean)+eps` | 小激活 scale=1e-3 | max_rel≈130，失配 100% | 与 golden 同链 |
| 保留标识符 | 用户变量名 `pipe_`/`block_idx` | V001 历史 CE 15/15 | 编译失败 | grep 核对表 §3.8 |
| 32 位偏移溢出 | outer×D > 2^32 | 4D shape 极大 | 地址错乱 RE | 全链 uint64_t |
| 多核 Cache Line 撕裂 | D×s 非 32B 倍数 + 多核并发写相邻行 | D=50，fp32（200B） | 间歇 WA | 按 `k×D×s≡0 (mod 32)` 分配；真机验证 |
| ReduceSum count 超限 | tile 计算长度 > 4096 且实现上限更低 | fp32 tile=2048 安全；若改 4096 需验证 | 结果错或 CE | 统一分块 ≤4096 |
| 标量 sqrtf 误差 | bf16 大 D，sqrtf ulp 差 | D=4096，注入 2^-10 误差 | 击穿 1e-3 阈值 | 真机测 ulp；后备改向量 Sqrt |
| BF16 标量 cast 编译失败 | BiSheng dav-2201 标量 bf16→fp32 | tilelang-ascend #1762 | CE | `AscendC::ToFloat` 后备 |
| NaN/Inf 崩溃 | 输入含 NaN/Inf | 行含 1 个 NaN | 题面要求整行 NaN，不能崩溃 | 真机 L4 用例；不加 clamp |
| 校验过严拒掉合法输入 | ValidateInputs 条件与判题端不一致 | 若判题允许 rank=1 | kernel 不启动，输出未写 → WA | 真机对照模板行为；校验失败时考虑是否应照常计算 |

### 5.2 第二路线（2+常驻+双缓冲）——性能版

| 风险 | 触发条件 | 反例 | 后果 | 检测/后备 |
| --- | --- | --- | --- | --- |
| 单遍 UB 溢出 | D≤4096 但 dtype/double buffer 叠加超 192KB | fp32 + 双缓冲 + tile 整行 | InitBuffer 失败或 CE | 静态预算；真机 `GetLibApiWorkSpaceSize` |
| 阈值误判 | D 边界 4096/4097 附近的 off-by-one | D=4097 走单遍但 UB 不够 | 同上 | 保守阈值；运行时分支加余量 |
| 双缓冲 tile 减半后 DMA 效率反降 | 短行（D≤tile）本就单 tile | D=64，双缓冲无意义还占 UB | 性能不升反降 | 仅 D/tile≥2 时开双缓冲 |
| gamma/bias 常驻与工作区冲突 | D 大且 dtype 宽 | D=32768 fp16，常驻 128KB | UB 不足 | D 大时不常驻整行，Pass2 按需搬 |
| 精度回归 | 单遍整行 ReduceSum 的结合律与分块不同 | D=4096 与 D=32768 分块版结果不完全逐位相同 | 可能仍过阈值但与基线有 diff | 与 V003 基线 diff；阈值内可接受 |
| 小 outer 大 D 并行度不足 | outer < 核数/2 且 D>4096 | outer=1, D=32768 | 多数核空转，性能差 | 接受少核（方案 6）；跨核切分仅研究 |

### 5.3 仅作研究参考 / 不建议

| 方案 | 失败条件（为何不能进提交） |
| --- | --- |
| 4 低精度中间 | 五条独立定量否决；A2 硬件甚至不支持 bf16 加乘 |
| 8 按 tile 分配 AI Core | reduction 维跨核需通信与二次归约；GPU 无对应模式；复杂度远超收益 |
| 14 CUDA/Triton 迁移 | 指令集、同步模型、内存层次均不可直接翻译；只能借算法结构 |

---

## 6. 仍需真实 CANN/NPU 验证的事项清单

按验证顺序排列。前 5 项为「第一上机必做」，未通过不进入性能阶段。

| # | 事项 | 方法 | 通过标准 | 来源 |
| --- | --- | --- | --- | --- |
| 1 | DataCopyPadExtParams 字段序 | 读 `${ASCEND_HOME_PATH}/include/ascendc/basic_api/interface/kernel_struct_data_copy.h` 的成员声明顺序；或写探针 kernel 分别填 left/right 验证 | offsetof 与写法 B 一致；D=67 搬入后 UB 前 67 元素正确、其后为 0 | §1.1 / A2 |
| 2 | DataCopyPad 搬出（UB→GM）是否覆盖相邻行 | D=67/129/1000，fp16/bf16/fp32；连续两行，逐字节比对第二行开头 | 第二行开头未被踩 | §1.4-2 / A7/A9 |
| 3 | 多核 32B Cache Line 撕裂 | D×s 非 32B 倍数（如 D=50 fp32），outer 远大于核数，多核并发写回 | 无间歇 WA；可重复 100 次 | 文档 §7 红线 / A7 |
| 4 | 保留标识符与编译 | `./run.sh` 在 dav-2201 上编译 | 无 CE；特别确认无 `pipe_`/`block_idx` | A8/A9 |
| 5 | 入口与 dtype 分派 | 模板 case0（FP16 [1,64]）+ 自备 bf16/fp32 小例 | verify_result 通过 | A1/A8 |
| 6 | UB 实际可用容量 | 真机打印 / 逐步放大 tile 观察 InitBuffer | 确认 192KB 口径；记录 `GetLibApiWorkSpaceSize` | A7 |
| 7 | ReduceSum count 上限 | 分别用 count=2048/4096/8192 归约已知和 | 确认实际安全上限 | A6/A7 |
| 8 | 标量 sqrtf / 向量 Sqrt ulp | 同一 mean+eps 分别走 sqrtf 与向量 Sqrt，比对 | 相对误差 ≤2^-20（Agent06 预算） | A6 |
| 9 | CAST_RINT 是否严格 RNE | 构造恰好中点的 FP32 值转 fp16/bf16 | 与 numpy `astype` 一致 | A6 |
| 10 | BF16 标量 cast | 确认 `(float)bf16_scalar` 或 `AscendC::ToFloat` 可编译 | 无 CE（对照 tilelang-ascend #1762） | A9 |
| 11 | NaN/Inf 稳定性 | L4 用例：行含 NaN、±Inf | 不崩溃；输出与 golden 一致（整行 NaN / Inf 位置 NaN 其余 0） | A6 |
| 12 | 精度矩阵 L0–L2 | Agent06 §3 矩阵（dtype×rank×D×outer×分布） | fp32 相对/绝对 <1e-4；fp16/bf16 <1e-3；失配率 ≤0.1% | A6 / 题面 |
| 13 | 性能基线 | 模板计时 + 15 点覆盖矩阵自建输入 | 记录每点耗时；与榜上 tbest 对比 | A1/A7 |
| 14 | gamma/bias 常驻收益 | 对比 9D vs 7D 路径耗时 | 大 outer 时有可测收益 | A7 |
| 15 | 单遍 vs 两遍 | D∈{64,1024,4096,4097,32768} 分别计时 | 确认 D≤4096 单遍更快；D>4096 回退正确 | A7 |
| 16 | 双缓冲收益 | BUFFER_NUM=2 + tile 减半 vs BUFFER_NUM=1 | 大 D 多 tile 行有加速；小 D 无回退 | A7 |
| 17 | SoC 确认 | `npu-smi info` | 记录型号；若非 dav-2201 用 `-DNPU_ARCH=` 重编 | A1/A8 |
| 18 | 判题端 15 点 shape/dtype/eps | 无法外部获取 | 用本地矩阵逼近；提交后按失分点反推 | A1 |

**边界重申**：本机 macOS 无 CANN/NPU。上述 18 项全部未执行。本报告任何表述不得被解读为「已编译 / 已通过精度 / 性能已达标」。CPU 参考计算（Agent06）只能排除语义错误、分母错误、低精度路线三类问题，不能替代真机。

---

## 7. 对主 Agent 的交接要点

1. **V003 基线** = 方案 1+3+6+7+9+11+13 组合，按 §3 步骤实现；提交目录 `提交/V003/kernel.asc`。
2. **两处证据冲突的处置写法**已固定在 §1.1/§1.2：DataCopyPadExtParams 写成「证据冲突 + 真机第一验证项 + 两种顺序下都安全的写法」；GetReduceRepeatSumSpr 写成「A 级官方有此接口 vs 可移植性风险，主线用 GetValue(0)+V_S」。
3. **mojo_opset Triton 定位**：正确性对照，不是迁移源（§1.3）。
4. **性能第二路线**（§4）在 V003 真机精度闭环后再做，按 Agent07 的 P1→P2→P3→P4 顺序落地。
5. **旧文档需修订的过时描述**（不在本轮修改范围，供主 Agent 后续处理）：
   - `文档/problem-add-rms-norm-bias.md` §7/§9 关于 DataCopyPadExtParams 字段序与「V002 错误写法」的段落，与当前 V002 代码（逐字段赋值）不符，且与 CANN 9.0.0 官方文档冲突。
   - 同文档 §9 关于 `GetReduceRepeatSumSpr`「架构不兼容」的表述，与 9.0.0 官方专页冲突。
   - 源码注释中的「workLocal」在 9.0.0 文档中已更名为 `sharedTmpBuffer`（功能无影响，检索时注意）。
6. **本轮未修改** `源码/`、`提交/`、`文档/`、模板任何文件；未上传 CANNJudge；未执行任何安装。

---

## 8. 来源索引（合并，按证据等级）

### A 级

| # | 来源 | 用途 |
| --- | --- | --- |
| A1 | CANNJudge 题面/比赛/排行/模板 API（agent01 S1–S7） | 计分、15 点、提交接口、dtype、精度阈值 |
| A2 | CANN 9.0.0 官方 API 页（agent02 §5 全表） | DataCopyPad/ReduceSum/Cast/SetFlag/TPipe 签名与约束 |
| A3 | 官方模板 8 文件（`~/Downloads/addrmsnormbias_problem_1742_template/`） | run_kernel 签名、`__vector__`、dtype 编码、golden、verify |
| A4 | 本地 `源码/op_kernel/add_rms_norm_bias.cpp` 与 `提交/V002/kernel.asc` | 现行实现基线 |
| A5 | TVM/MLIR/IREE/StableHLO/Triton/TileIR/PyPTO 官方文档（agent05 S1–S9） | 自动化路线不可用；tiling 思想 |
| A6 | Agent06 全部 CPU 实验 + golden/verify/PyTorch/NVIDIA 混合精度指南 | 低精度否决、分母、eps、rsqrt 预算、测试矩阵 |
| A7 | Agent07 访存模型与 UB 预算（部分标推断） | 7D/5D 流量、tile 选择、同步开销 |
| A8 | cann-samples / asc-devkit / 模板 run.sh / CMakeLists（agent08） | 环境、NPU_ARCH、编译链路 |

### B 级

| # | 来源 | 用途 |
| --- | --- | --- |
| B1 | Ascend/op-plugin AddRmsNorm 文档与实现（agent03） | 官方语义锚点：无 bias、rstd=FP32 |
| B2 | Ascend/mojo_opset fused_add_rms_norm.py（agent03） | 两遍 FP32 归约算法对照（非迁移源） |
| B3 | NVIDIA apex / PyTorch layer_norm / Liger / flash-attn / Triton 教程（agent04） | GPU 共性骨架、post-bias 位置、单双遍分界 |
| B4 | tilelang-ascend #1682/#1717/#1762/PR#1777、InfiniCore #1519、flash-linear-attention PR#324 等（agent09） | 失败模式实证 |

### C 级

| # | 来源 | 用途 |
| --- | --- | --- |
| C1 | Linux DO / V2EX 昇腾帖（agent08 §6） | 驱动版本、环境现场；无逐步安装手册 |
| C2 | 社区 UB=192KB 说法 | 与 B 级教程一致但未独立核验 9.0.0 正文 |

### 本轮新增本地核对

| # | 对象 | 结论 |
| --- | --- | --- |
| L1 | `提交/V002/kernel.asc` L67–71 | 实为逐字段赋值，非聚合初始化；Agent02 对 V002 的描述与文件不符 |
| L2 | `源码/op_kernel/add_rms_norm_bias.cpp` L115 | 聚合初始化 `{true,0,0,T(0)}`；两种字段序解读下均未补右 padding |
| L3 | `文档/problem-add-rms-norm-bias.md` §7/§9 | 字段序与 V002 判定已过时，与 A2 冲突 |
| L4 | 模板 `kernel.asc` / `AddRmsNormBias.py` | 入口与 golden 与 agent01/06 描述一致 |
