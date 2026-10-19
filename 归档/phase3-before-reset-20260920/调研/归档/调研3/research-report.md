# AddRmsNormBias 技术方案调研报告（调研2）

> 日期：2026-09-12
> 范围：方案空间调研，不写最终实现
> 本机：macOS，无 CANN、无 Ascend C 编译器、无昇腾 NPU
> 边界：本轮未执行 CANNJudge 上传；未声称 NPU 编译/精度/性能通过

子报告目录：`调研/调研2/agents/agent01` … `agent10`。

---

## 1. 调研范围与平台覆盖

| Agent | 主题 | 平台命中 | 输出 |
| --- | --- | --- | --- |
| 1 | 题面/规则/提交 | CANNJudge 公开 JSON API（题面、比赛、模板包、排行、stats、editor.js） | `agent01-competition.md` |
| 2 | 官方 Ascend C API | hiascend.com CANN 9.0.0 文档（21 条 A 级 URL） | `agent02-api.md` |
| 3 | 官方开源仓库 | Ascend/samples、op-plugin、mojo_opset、cann_op_contrib、canncamp、MindIE-Turbo | `agent03-official-samples.md` |
| 4 | GPU/CUDA/Triton | Apex、PyTorch CUDA、Triton 教程、Liger、flash-attention | `agent04-gpu-migration.md` |
| 5 | 编译器/IR | TVM、MLIR、IREE、OpenXLA、Inductor、TileIR、PyPTO、msopgen | `agent05-compiler-ir.md` |
| 6 | 数值精度 | 官方 golden、PyTorch、NVIDIA 混合精度、本机 CPU 定量实验 | `agent06-precision.md` |
| 7 | 性能/UB | 架构文档、Profiling、访存模型 | `agent07-perf-ub.md` |
| 8 | Linux/环境 | Linux DO、V2EX、SO、官方安装链路 | `agent08-linux-env.md` |
| 9 | 失败案例 | tilelang-ascend、triton-ascend、vllm-ascend、Liger、InfiniCore 等 Issue/PR | `agent09-failure-cases.md` |
| 10 | 证据审阅与合并 | 前 9 份 + 本地代码 | `agent10-synthesis.md` |

未找到 / 不可用：官方开源 AddRmsNormBias 的 Ascend C kernel；cann-samples 独立仓（实际为 `Ascend/samples`）；ops-transformer / ops-nn；GitCode 赛事正文（SPA）；hiascend 部分正文页（JS 壳，API 2 另法抓取成功）。

---

## 2. 题目约束（A 级，CANNJudge API + 模板）

```text
y = x + residual
rms = sqrt(mean(y^2, dim=-1) + epsilon)
output = y / rms * gamma + bias
```

| 项 | 值 |
| --- | --- |
| 赛事 | 2026 CANN 挑战赛·西南初赛，contest `6a9a9295bf41025d601255a3` |
| 题目 | AddRmsNormBias / 平台 1742 / problem `6a9a9a99bf41025d6013eb85` |
| 时间 | 2026-09-05 00:00 ~ 2026-10-17 18:00 CST |
| IO | x/residual 同 `(...,D)`；gamma/bias=`(D,)`；out 同 x；ND |
| dtype | fp16 / bf16 / fp32，输出同型；枚举 0=fp32 1=fp16 2=bf16 |
| rank | 2D / 3D / 4D |
| 范围 | batch[1,8192]，seq[1,32768]，D[64,32768]；D 可非 32 倍数 |
| epsilon | float，默认 1e-5 |
| 测试点 | **15**，全过才计分；每点 iterations=5 |
| 计分 | 单点 `100/(1+log₁.₅(t/T))`，总分均值；T=该点全局最优 |
| 精度 | fp32 相对/绝对 <1e-4；fp16/bf16 <1e-3 |
| 上传 | 仅 `kernel.asc`；`code_template=npu_kernel_dev`；`POST /api/submissions/submit` |
| 入口 | `extern "C" void run_kernel(..., availableCoreNum, stream, epsilon)` + `__global__ __vector__` |
| 环境 | CANN 9.0.0；模板默认 SoC `dav-2201` |
| 特殊值 | NaN/Inf 不崩溃；结果确定性 |
| golden | 全 FP32 计算，末尾一次 cast 回原 dtype |

未确认：每日约 50 次提交；15 点 shape/dtype/eps；判题端精度算法是否与本地 `verify_result.py` 一致；判题 SoC 最终型号；西南决赛细则。

榜上 15 点 TBest（μs）：`1.47 … 3750.12, 8656.25`——后两点是性能主战场。

---

## 3. Ascend C 官方 API 要点（A 级）

| 主题 | 结论 |
| --- | --- |
| DataCopyPad | GM↔UB 非对齐；Global 端无对齐约束；Local 端 32B；A2 支持 bf16 搬运 |
| DataCopyPadExtParams | **证据冲突**：Agent02 官方表6+示例支持 `isPad, leftPadding, rightPadding, paddingValue`；旧项目文档写 `isPad, paddingValue, leftPadding, rightPadding`。真机第一验证项；代码强制逐字段赋值或 Designated Initializers |
| ReduceSum | 9.0.0 参数名 `sharedTmpBuffer`；dtype half/float（无 bf16）；src 与 work 32B 对齐；count 受 UB 限制 |
| 结果读取 | `dst.GetValue(0)` + V_S/S_V；或官方 `GetReduceRepeatSumSpr<T>()`。主线用前者 |
| Cast | 上行 half/bf16→fp32 用 CAST_NONE；下行 CAST_RINT（RNE） |
| Add/Mul/Muls/Sqrt/Rsqrt | A2/A3 **不支持 bfloat16_t**——BF16 必须 FP32 域计算 |
| Tiling 宏 | `REGISTER_TILING_DEFAULT` / `GET_TILING_DATA_WITH_STRUCT` **「暂不支持 Kernel 直调工程」** |
| 入口 | 直调：`__global__ __vector__`；成员：`__aicore__ inline` |
| 同步 | TPipe 场景 eventID 必须 `FetchEventID`；`PipeBarrier<PIPE_S>()` 硬件错误 |
| TPipe | 一 Kernel 一 TPipe；A2 同 TPosition TQue 上限约 8 |

---

## 4. 开源实现对比

| 来源 | residual | bias | 形态 | 定位 |
| --- | --- | --- | --- | --- |
| op-plugin AddRmsNorm | 有 | **无** | ACL 封装，闭源 kernel | 官方语义锚点；rstd=FP32 |
| mojo_opset fused_add_rms_norm | 有 | **无** | Triton/TTX，两遍+FP32 | **正确性对照，非代码迁移源** |
| Ascend/samples Add_tile | — | — | Ascend C 教学样例 | 工程骨架可参考，API 需升 9.0 |
| GPU Apex/Liger/Triton/flash-attn | 视实现 | 语义不一 | CUDA/Triton | 算法结构参考；指令不可直迁 |
| TVM/MLIR/IREE/PyPTO/msopgen | — | — | 自动生成 | **均不能产出判题 `kernel.asc`**；tiling 思想可借鉴 |

官方开源无 AddRmsNormBias 的 Ascend C 实现，本题必须自写。

---

## 5. GPU/NPU 迁移分析（摘要）

| GPU 机制 | Ascend C 对应 | 可迁移 | 不可直迁 |
| --- | --- | --- | --- |
| shared memory | UB (TQue/TBuf) | 分块、复用 | 指令与布局 |
| warp shuffle reduction | ReduceSum / 分块+标量 | 分块 FP32 合并 | shuffle 原语 |
| blockIdx | GetBlockIdx | 按行分核 | CUDA 启动配置 |
| float4 / 非对齐 load | DataCopyPad | 补 0 尾块、mask=0 | CUDA 地址空间 |
| acc_type=float | Cast→FP32 | **强制** | — |
| residual 融合进 kernel | 同 | 共识 | — |
| Liger weight-offset | 语义不同 | — | 本题是 post-bias |
| 按列切分 feature | — | — | **反对**跨核切 D |

GPU 证据共识：residual 融合、FP32 归约、按行分核、补零尾块；反对低精度中间与按列切核。

---

## 6. 数值精度方案（含本机 CPU 实验）

**强制 FP32 中间计算。** 低精度中间路线定量否决：

| 路线 | 失败起点 | 最严重失配率 |
| --- | --- | --- |
| FP16 顺序累加 | D≥129 | D=32768 时 99.9% |
| BF16 顺序累加 | D=32 | D=32768 时 99.99% |
| BF16 全程 add/mul/div | 几乎所有 D | 31%~65% |
| FP16 向量平方和 | D=32768 溢出 | 99.9% |

FP32 链 vs float64：D≤32768 绝对误差 <5e-7，距 1e-4 约 3 个数量级余量。

分块 ≤4096 + FP32 块间合并：D=32768 失配率 0。

独立 WA：分母必须是原始 D，不是 pad 长度（D=1000 误用时 max_rel≈37）。

eps 位置：`sqrt(mean)+eps` 错；正确为 `sqrt(mean+eps)`。

rsqrt：相对误差须 ≤2^-20；优先向量 Sqrt。

测试矩阵：dtype×{fp16,bf16,fp32} × rank×{2,3,4} × D×{1,31,32,33,64,127,128,129,1024,4096,32768} × outer 小/中/大；另加 NaN/Inf。详见 `agent06-precision.md`。

---

## 7. UB 分块与性能方案

本算子 memory-bound。

| 场景 | 策略 |
| --- | --- |
| 基线两遍 | tile fp16/bf16=4096、fp32=2048；BUFFER_NUM=1；约 92KB UB（fp16） |
| gamma/bias 常驻 | GM 读 9D→7D（约 −22%），最低风险最高性价比 |
| D≤4096 单遍 | GM 5D（再 −28.6%）；UB 峰值约 50–60KB |
| 块间向量累加 | 行末一次 GetValue，同步次数降一个数量级 |
| 双缓冲 | tile 减半；仅 D/tile≥2 时开 |
| 小 outer 大 D | 接受少核；不跨核切 D |
| 大 outer 小 D | 按行均分已接近理想 |

性能优先级：P0 正确性闭环 → P1 gamma/bias 常驻 → P2 块间向量累加 → P3 单遍 → P4 双缓冲。

---

## 8. 编译与真机环境

前提链：`npu-smi info` → CANN Toolkit 9.0.0 → `source set_env.sh`（`ASCEND_HOME_PATH`）→ `find_package(ASC)` + `--npu-arch=dav-2201` → `./run.sh`。

npu-arch：910B/C=`dav-2201`（模板默认）、950=`dav-3510`、推理 Core=`dav-2002`。

Linux DO / V2EX 真实命中（社区现场，不能替代官方安装说明）见 `agent08-linux-env.md` §6。

本机未执行任何安装或编译。

---

## 9. 竞赛失败模式（Agent09 摘要）

| 模式 | 本题检查项 |
| --- | --- |
| CE：保留标识符 `pipe_`/`block_idx` | grep 全文扫描（V001 已踩 pipe_） |
| CE：BF16 标量 cast | 确认 ToFloat 路径（tilelang-ascend #1762） |
| CE：UB 溢出 | 静态加总 <192KB |
| WA：DataCopyPad 非对齐损坏 | 尾块真机逐字节核对（#1682 / PR#1777） |
| WA：分母用 pad 长度 | 锁死原始 D |
| WA：32B Cache Line 跨核撕裂 | `k×D×s ≡ 0 (mod 32)` 分配 |
| RE：尾块越界 | `min(tile, valid)` 截断 |
| WA：32 位偏移溢出 | 全链 uint64_t |
| WA：eps 位置 / 低精度累加 | 与 golden 同链 + FP32 强制 |

---

## 10. 14 条方案矩阵（详表见 `agent10-synthesis.md` §2）

| # | 方案 | 推荐级别 |
| --- | --- | --- |
| 1 | 两遍扫描 | **首选（正确性基线）** |
| 2 | 单遍暂存 y | 可作为第二路线（D≤4096） |
| 3 | FP32 全中间计算 | **首选（强制）** |
| 4 | 低精度中间计算 | **不建议**（定量否决） |
| 5 | 大 tile | 可作为第二路线（UB 紧时慎用） |
| 6 | 小 tile（≤4096 分块） | **首选（大 D 必须）** |
| 7 | 按行分配 AI Core | **首选** |
| 8 | 按 tile 分配 AI Core | **不建议** |
| 9 | DataCopyPad 尾块 | **首选（非 32 倍数 D）** |
| 10 | 手工尾块 | 可作为第二路线（Pad 写方向异常时） |
| 11 | ReduceSum | **首选** |
| 12 | 手工向量归约 | 可作为第二路线 |
| 13 | 纯 Ascend C 手写 kernel.asc | **首选（唯一提交路径）** |
| 14 | CUDA/Triton 迁移参考 | 仅作研究参考 |

---

## 11. 推荐实现路线：V003 基线

组合：**1+3+6+7+9+11+13**

1. 单文件 `提交/V003/kernel.asc`，入口与模板逐字对齐。
2. `run_kernel` 解析 TensorGroupInfo → outer/dim/dtype；校验失败 return。
3. 内核 `__global__ __vector__`；dtype 0/1/2 分派 half/bf16/float 模板。
4. 多核按行均分；所有 GM 偏移 `uint64_t`。
5. Pass1：DataCopyPad 搬入（逐字段赋值补右 0）→ Cast FP32 → Add → Mul 平方 → ReduceSum → V_S → GetValue → 块间累加。分母锁死原始 D。
6. Pass2：重读 x/residual/gamma/bias → FP32：`y*(1/rms)*gamma+bias` → CAST_RINT → DataCopyPad 写有效字节。
7. 保留标识符扫描；无 tiling 宏；无 bf16 算术指令；无 clamp。

步骤级代码结构与 12 项静态核对表见 `agent10-synthesis.md` §3。

---

## 12. 第二候选路线（性能）

在 V003 真机精度闭环后：

1. **P1** gamma/bias 常驻（D 不太大时）→ 9D→7D。
2. **P2** 块间向量累加，行末一次 GetValue。
3. **P3** D≤4096 单遍暂存 y_fp32 → 5D。
4. **P4** 双缓冲（仅多 tile 行）。

D>4096 回退两遍 + P1/P2。不做方案 4/8，不做跨核 D 切分（除非真机确认 outer=1 且 D=32768 TLE）。

---

## 13. 风险清单（Top）

| 风险 | 级别 | 处置 |
| --- | --- | --- |
| DataCopyPadExtParams 字段序证据冲突 | 高 | 逐字段赋值；真机读头文件 + D=67/129/1000 |
| 搬出覆盖相邻行（A vs C 冲突） | 高 | 真机第一验证项；后备手工尾块 |
| 分母误用 pad 长度 | 致命 | 锁死 `sum / (float)dim` |
| 低精度累加 | 致命 | 强制 FP32；CPU 已否决 |
| 保留标识符 CE | 高 | grep `pipe_`/`block_idx`/`ubuf`/`tid` |
| 32 位偏移溢出 | 严重 | 全链 uint64_t |
| 多核 32B 撕裂 | 高 | 行块对齐分配 |
| ReduceSum count 上限争议 | 中高 | 统一分块 ≤4096 |
| BF16 标量 cast / A2 无 bf16 加乘 | 中高 | FP32 链；ToFloat 后备 |
| 标量 sqrtf ulp（bf16 大 D） | 中 | 真机测；向量 Sqrt 后备 |
| 判题 SoC 与 dav-2201 不一致 | 中 | `npu-smi` + `-DNPU_ARCH` |
| ValidateInputs 过严拒合法输入 | 中 | 对照模板行为 |
| 每日提交额度未公开 | 低中 | 保守记账 |

---

## 14. 未确认事项

1. DataCopyPadExtParams 在目标机头文件中的成员顺序。
2. DataCopyPad 搬出是否覆盖相邻行。
3. 判题 SoC、15 点 shape/dtype/eps、判题精度算法。
4. ReduceSum 实际 count 上限、UB 实际可用量。
5. sqrtf/向量 Sqrt 在目标编译链的 ulp。
6. 每日提交次数上限、决赛细则。
7. `__vector__` 与 `__aicore__` 的完整语义差异（以模板为准）。

---

## 15. 下一阶段实验计划（真机）

第一上机必做（未过不进性能）：

1. 读 `kernel_struct_data_copy.h` 确认字段序。
2. D=67/129/1000 搬入/搬出逐字节核对。
3. 多核非对齐并发写回 100 次无 WA。
4. `./run.sh` dav-2201 编译无 CE（无保留标识符）。
5. 模板 case0 + bf16/fp32 小例 verify 通过。

然后：UB 实测 → ReduceSum 上限 → sqrt ulp → CAST_RINT → BF16 标量 cast → NaN/Inf → L0–L2 精度矩阵 → 性能基线 → P1/P2/P3。

完整 18 项见 `agent10-synthesis.md` §6。

---

## 16. 本轮完成与边界

**已完成**：本地项目与模板只读核对；10 个子代理并行调研；方案矩阵与 V003/V00x 路线；证据冲突登记；旧文档关键过时表述识别。

**有源码或官方 API 证据**：题面与提交接口；CANN 9.0.0 关键 API；golden 语义；BF16 硬件限制；直调入口与禁用 tiling 宏；低精度否决（CPU 实验）；官方开源无 Bias 融合 kernel。

**仅作迁移参考**：GPU/CUDA/Triton 实现；自动生成工具；mojo_opset Triton。

**仍需真实 CANN/NPU 验证**：编译、精度矩阵、尾块写方向、字段序、性能、SoC。

**本轮未执行 CANNJudge 上传或提交。**
