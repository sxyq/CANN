# AddRmsNormBias 技术方案调研报告（调研2 · 最终版）

> 生成：2026-09-12｜主 Agent 汇总，基于 10 份子代理报告（`调研/调研2/agents/`）与本地代码/模板只读核对。
> 环境声明：本机为 macOS，无 CANN 编译器与昇腾 NPU。本报告全部结论止于资料核对、源码审阅与 CPU 参考实验，**不构成任何 NPU 编译、精度或性能通过的声明**。
> 证据分级：A=官方文档/官网题面/官方仓库；B=官方样例/源码/测试/平台自身源码；C=社区文章/论坛/个人仓库；D=仅搜索摘要未核验。来源清单见 `sources.md`（230 条）。

---

## 1. 调研范围和平台覆盖情况

**十路子代理并行覆盖**（详细职责卡见 `00-平台与研究覆盖计划.md`，各路报告见 `agents/`）：

| # | 方向 | 平台覆盖 | 产出 | 证据主力 |
| --- | --- | --- | --- | --- |
| 1 | 题面/规则/提交接口 | CANNJudge 页面+公开只读 API+前端 JS 逆向+GitCode 赛事页 | agent-01 | A 级为主 |
| 2 | 官方 Ascend C API | hiascend 9.0.X 缓存 30 页+gitcode 官方镜像 | agent-02（27+3 项 API） | A 级 |
| 3 | 官方开源仓库 | hicann/ops-nn、cann-samples、op-plugin、Ascend/samples | agent-03（3 仓源码级+2 仓目录级） | A/B 级 |
| 4 | GPU/CUDA/Triton/PyTorch | PyTorch aten、vLLM、TE、flashinfer、Liger、unsloth、Triton 教程 | agent-04（7 个源码级卡片+12 行迁移表） | B 级 |
| 5 | 编译器/IR/算子生成 | TVM、Triton、TorchInductor、MLIR、TileLang-Ascend、msopgen/bisheng | agent-05（8 条目+适用性判定表） | A/B/C |
| 6 | 数值精度与验证 | PyTorch Issue/PR、CUDA 数学附录、MindSpore、CPU 实验 | agent-06（12 组实验+测试矩阵） | 实验+A/B/C |
| 7 | 性能/UB/硬件 | asc-devkit 官方文档/样例实测、cann-learning-hub | agent-07（硬件数字表+成本模型+分形态 tiling） | A/B 级 |
| 8 | Linux 环境与真机 | hiascend、昇腾论坛、V2EX、CSDN、GitHub | agent-08（12 步链路+13 报错案例） | A/B/C |
| 9 | 竞赛失败案例 | 本地 V001/V002+GitHub Issue+昇腾论坛+CSDN 竞赛作品 | agent-09（本地 2+外部 22 案例） | 混合 |
| 10 | 证据审阅与方案合并 | 全部 9 份报告+本地代码 | agent-10（矛盾裁决 12 项+方案矩阵+三路线） | 收官审阅 |

**来源统计**（`sources.md`）：在线 197 条（A 81、B 44、C 65、D 1）+本地 33 条；URL 去重合并 6 组；等级/状态冲突 4 条（双标注保留）。

**覆盖不足如实登记**：Linux DO 未命中昇腾技术帖（5 轮检索，如实记录）；hiascend.com 900 版原页被 cookie 墙拦截（以本地缓存+gitcode 镜像替代）；OpenXLA/IREE 仅间接覆盖；判题 15 测试点参数、判题 SKU、判定脚本均不公开。

## 2. 题目约束

A 级（题面 desc API 原文 + contest/problem API，agent-01）：

- **语义**：`y=x+residual`；`rms=sqrt(mean(y²,dim=-1)+eps)`；`output=y/rms*gamma+bias`（epsilon 在 mean 之后、开方之前；bias 在归一化之后）。与 PyTorch `F.rms_norm(x+residual, D, weight=gamma, eps)+bias` 完全对齐。
- **接口**：4 输入+1 属性+1 输出；fp16/bf16/fp32；ND；2D/3D/4D；batch∈[1,8192]、seq∈[1,32768]、D∈[64,32768]；D 可非 32 倍数；输入数值不超出 dtype 原生范围；NaN 输入→输出 NaN 不崩溃；结果确定。
- **判题**：15 测试点全过才计分（fp32<1e-4、fp16/bf16<1e-3）；单点得分 `100/(1+log₁.₅(t/T))`，总分=均值，同分早提交优先；iterations=5；性能唯一计分。
- **提交形态**：直调单文件 `kernel.asc`（`extern "C" void run_kernel(..., int64_t availableCoreNum, aclrtStream stream, float epsilon)`，内部 `<<<blocks, nullptr, stream>>>` 启动 `__global__ __vector__` 核函数）；在线编辑器 files 数组上传（.asc/.h，judge.asc/data_utils.h/main.asc 受保护）；dtype 枚举 0=fp32/1=fp16/2=bf16。
- **环境**：CANN 9.0.0、vector、SoC 官方未明示（模板默认 `dav-2201` 为唯一官方物料证据，推断级）。
- **已裁决不成立**：「每天 50 次提交额度」无页面依据且被统计反证（单队 8 天 558 次）——不按此规划（agent-01/agent-10 M5）。

## 3. Ascend C 官方 API（agent-02，27+3 项全部 A 级核对）

关键结论（与本地代码用法逐项对照后）：

1. **三项历史定案全部维持**：① `DataCopyPadExtParams<T>` 字段顺序 `{isPad, leftPadding, rightPadding, paddingValue}`（表 6+官方示例双证）；② UB→GM 搬出 dummy 假数据落 GM 时被框架丢弃、不写相邻内存（9.0.X 与最新镜像双版本同文；agent-10 M2 补证：C 级"溢出"案例实为调用侧把 blockLen 主动取整 32B 的用法问题）；③ ReduceSum 无单一 count 硬上限，唯一约束 UB 容量（255=通用 repeatTime 口径、16320=硬件归约指令推导口径、4096 无官方出处）。
2. **A2（dav-2201）类型红线**：Add/Mul/Muls/Sqrt/Rsqrt/ReduceSum 均不支持 bfloat16_t → bf16 输入必须 Cast FP32 计算；fp32→bf16 Cast 支持 CAST_RINT/FLOOR/CEIL/ROUND/TRUNC。
3. **入口限定符**：`__global__ __vector__`（模板+题面+最新规范三重支持，V002 现用）与 `__global__ __aicore__`（850 文档基准）均合法；对齐模板用 `__vector__`，残余风险低（agent-10 M4）。
4. **直调工程约束**：`GET_TILING_DATA_WITH_STRUCT` 明文"暂不支持 kernel 直调工程"（A 级）——tiling 必须参数直传。
5. 其他：TQue VECIN buffer≤8；TPipe Buffer 总数≤64；`PipeBarrier<PIPE_S>` 硬件错误禁用；直调工程默认自动同步（手动 PIPE_V 屏障冗余）；GetBlockIdx/Num 返回 int64_t；9.0.0 原页一处不可达（cookie 墙），以缓存+镜像三点演进链替代。

## 4. 开源实现对比（agent-03，源码级）

- **hicann/ops-nn `norm/add_rms_norm`**（A 级）：与本题语义最接近（只差 `+bias` 一条 Add）。五种 tiling 模式：NORMAL（一次一行，ubFactor 12288/10240）、SPLIT_D（D>12288 两遍+GM 中转）、MERGE_N（D≤2000 多行合并+双缓冲）、SINGLE_N（每核 1 行手工事件流水）、MULTI_N（fp16 对齐 D 整块多行）。多核=行级均分+尾核余数（`CalculateBlockParameters`）。全 FP32 中间；bf16 输出 CAST_RINT。220 架构尾块统一 `DataCopyPad`；归约=Add 折叠到 64 槽+WholeReduceSum 两级。
- **hicann/cann-samples `rms_norm_quant_story`**（A 级）：直调 `.asc` 同构样例（CMake 白名单含 dav-2201）；950PR 实测优化链 7693μs→49μs：gamma 预载 1.13x → 多核 59.8x → 双缓冲 1.55x → UB 多行 1.11x（MTE2 -71%）→ 二分累加。数字不可直接引用（950PR），方法论可迁移。
- **Ascend/op-plugin**（A 级）：`npu_add_rms_norm`→aclnnInplaceAddRmsNorm→ops-nn 同一 kernel/tiling，仅桥接层，无增量参考。
- **差异表**（vs 本题直调）：入图工程的 tiling 分发框架（TILING_KEY/GET_TILING）不可搬，只搬 kernel 内计算/搬运结构；tiling 在 host main 自算（cann-samples 的 `PlatformAscendCManager` 模式）。

## 5. GPU/NPU 迁移分析（agent-04，7 个源码级参考）

- **标杆结构**：vLLM `fused_add_rms_norm`（y=x+residual 语义一致）与 flashinfer `FusedAddRMSNorm`（y 的 FP32 副本驻留 smem 免二次读 GM）。事实修正：TE 前向核不带 residual；flashinfer 的 weight_bias 是乘性（Gemma 1+γ），本题 bias 是加性，不能照搬。
- **核心迁移结论**：① RMSNorm 无需 Welford（PyTorch/TE 源码中该分支均退化为 FP32 单遍平方和+rsqrt+二遍 epilogue）；② FP32 纪律与 A2 bf16 指令缺失互相印证（统一 Cast 后运算）；③ "y 驻留片上+分块两遍"是 UB 预算内主方案（D=32768 时 y 副本 128KB 需分块）；④ 尾块三段式：搬入 DataCopyPad 补零（零对 sum(y²) 无污染）、搬出必须按真实长度；⑤ 行跨步循环+int64 索引（vLLM #43390 溢出前车之鉴）。
- **不可迁移清单**：TE 跨 CTA 协同启动/自旋 barrier（直调无对应物）、PDL、packed half2/bf16 数学、原地双写回、Liger/unsloth 的"转回原 dtype 乘 weight"（与本题 FP32 要求相反）、Welford 三元组。
- **编译器侧**（agent-05）：所有 DSL/自动生成方案（TileLang-Ascend、TVM、Triton、MLIR）一律不可提交——判题只收手写 kernel.asc；TileLang-Ascend（同硬件 A2、有 normalization/reduce 示例、UB 显式规划+自动复用 pass）是"手写前看别人怎么分块"的最佳参照；msopgen 生成多文件工程非提交物；bisheng `--npu-arch=dav-2201` 需 CANN≥8.3.RC1（9.0.0 版本序列推断支持）。

## 6. 数值精度方案（agent-06，CPU 参考实验）

三条硬性红线（CPU 实验决定性证据，需真机复核 A2 指令 ulp）：

1. **平方与归约累加必须 FP32**：fp16 累加器 D=32768 时 sum(y²) 相对误差 63.4%（D=1024 即 2.4%>容差）、bf16 ≥2.7%——必挂级；FP32 顺序累加最差 1.15e-5，分块两阶段 5.8e-7 更优。大数值下 fp16 域平方直接全行溢出 inf（|y|>255.96）。
2. **epsilon 必须在 sqrt(mean+eps) 位置**：`sqrt(mean)+eps` 与 `1/(sqrt(mean)+eps)` 变体在小方差行失配率 99.88%，必挂。
3. **fp32→bf16 输出必须 CAST_RINT（RNE）**：TRUNC/FLOOR 失配 ~40%（容忍的 400 倍）；RNE 0% 且对 1e-6 级中间扰动免疫；fp16 输出各模式实测均可过但统一 RINT。

补充：fp16 域加法在模板分布 U(-2,2) 可侥幸过、大数值（x=r=33000）必挂 → 加法前统一 Cast FP32；rsqrt 与 1/sqrt 差异仅 ulp 级（±8 ulp 注入失配 0%），容差余量约两个数量级；NaN/Inf 全行传播不做特判（IEEE 自然传播，勿加"稳定化"补丁）；分块两阶段归约数值安全（大 D 必用）；2D/3D/4D 展平后数值等价（E12 逐位一致）。

**新发现（agent-10 代码核对，V002 两处必须修复）**：M8——V002 `FromFloat` 实际用 CAST_ROUND（行 2805）而非文档所记 CAST_RINT（ROUND=四舍五入远离零，实测 bf16 失配 0.001% 能过但非 0 失配目标）；M9——V002 fp16 宽行路径为 half 域加法（行 2330/2720）与 half 域 epilogue 乘加（行 2733-2737），违反 FP32 中间红线，大数值必挂。

## 7. UB 分块和性能方案（agent-07）

**硬件关键数字**：UB=192KB/196608B（16 bank group×3 bank，A 级；官方模板 InitBuffer 上限 195584B）；AIV 910B1/B2=48、910B3/B4=40（判题 SKU 未确认→GetBlockNum 自适应）；单 AIV 向量吞吐 half 128/float 64 elem/cycle、Sqrt 32、Rsqrt 128/64（A 级）；GM 有效带宽大块 ≈1.49TB/s；非对齐搬运 A2 端到端 -21.6%（B 级）；L2=192MB。

**成本模型**：单遍 6B/元素 vs 两遍 10B/元素（fp16，GM 口径）——但 M12 修正：两遍第二遍可 L2 命中，真实成本 6~10B 之间（msprof L2Cache.csv 实测定夺）；归约官方排序"二分累加 > ReduceRepeat > ReduceSum 接口"（A 级，172 vs 242 cycle）；双缓冲大点收益 1.5x 量级、小点适得其反（A 级反例）；多行合并 DMA 回收小 DMA 崩塌（+148%@8KB、98.9%@128B）。

**分档 tiling**（结合 TBest 反推 15 点形态，推断级）：
- 小点（TP1-3，≲0.6MB）：单遍单缓冲、指令数最小化、禁双缓冲；
- 中大点（TP4-12）：gamma/bias 预载+多行合并 DMA+双缓冲；
- 巨型点（TP13-15，0.9-2.2GB）：512B 对齐双轨（主体 DataCopy+尾块 Pad）+ L2 bypass 只读输入（-28.9%）+ 负载均衡；
- D 档位：D≤4096 单遍 y 驻留（fp16/bf16；fp32 减半）；D>4096（官方阈值 12288/10240 保守口径）两遍重读重算。

## 8. 编译和真机环境方案（agent-08）

- **12 步链路**：锁内核 → HwHiAiUser 属组 → HDK 26.0.RC1/25.5.2/25.5.1（A2 系包名 `ascend910b-*`，lspci d802=910B）→ npu-smi 验证 → toolkit 9.0.0+910b-ops 9.0.0（先 toolkit 后 ops）→ `source set_env.sh`（推荐 `latest/bin/setenv.bash`）→ 模板 `./run.sh`（`--npu-arch=dav-2201` 与判题一致）。
- **无本地仿真捷径**：CANN 9.0 仿真器仅支持 950PR，A2/910B 必须真机。
- **关键坑**：① ccec/bisheng 内建宏冲突（LOWER 实锤案例，与 V001 pipe_ 同根因）；② SetFlag/WaitFlag 不配对→核异常且污染设备状态（判题 15 点连跑放大）；③ GM 越界（EZ9999/EI9999）；④ 3 秒超时语义（死等=判失败+可能连坐）。
- **性能测量**：msprof op `--warm-up=10 --launch-count=5`（对齐 iterations=5；短任务降频污染数据必须预热）；8 个 CSV 中 PipeUtilization/ResourceConflictRatio 定瓶颈与 bank 冲突。

## 9. 竞赛失败案例（agent-09，本地 2+外部 22）

- **本地**：V001 `pipe_` 宏冲突 15/15 CE（平台实锤）；V002 上传内容异常（平台收到首行 `return false;`，上传通道内容损坏，非代码缺陷）。
- **外部高价值案例**：BLK/LOWER 宏冲突随 CANN 小版本漂移（CPU 通过不拦截）；CANNJudge 命名空间差异（补 `using namespace AscendC;`）；DataCopyPad blockLen 主动取整 32B 的写出溢出（M2 已裁决为用法问题）；复旦赛"数学等价改写"与"绕队列快路径"致 WA；UB 按 248KB 假设崩溃（本题 SoC 即 192KB）；标量黑洞 TLE；测试数据缺陷致正确实现被判 WA。
- **提交策略**：每版本绑定一个可回滚假设；双闸门（提交前本地静态核对+提交后平台回读）。

## 10. 方案矩阵（agent-10 统一评估）

| # | 方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
|---|------|------|---------|---------|----------|----------|----------|----------|--------------|----------|
| 1 | 两遍扫描（重读重算） | Pass1 归约+Pass2 epilogue | 10B（L2 命中后 6-10B） | 低（~76KB tile 级） | 低（全 FP32） | 与 9/10 联动 | 中（大 D 必选） | 低 | 低 | **可作为第二路线**（大 D 档必选） |
| 2 | 单遍 y 驻留 UB | y 副本驻留一遍完成 | 6B（下界） | 高（D≤4096 峰值 80-112KB） | 中（half 副本有量化风险→用 FP32 副本） | 与 9/10 联动 | 高（省 28.6-40% 流量） | 中 | 低 | **首选**（D≤4096；D>12288 禁用） |
| 3 | FP32 全中间计算 | Cast 前置+全 FP32+单次 CAST_RINT | —（正交） | 中 | **无**（E10 全链 0 失配） | — | 计算非瓶颈 | 低 | 无 | **首选**（红线，非可选） |
| 4 | 低精度中间计算 | half/bf16 域运算 | — | 低 | **致命**（63%/2.7% 误差、失配 18-40%） | — | — | 低 | bf16 指令 A2 不支持 | **不建议**（仅对照） |
| 5 | 大 tile | 减少 DMA 次数 | 次数↓ | 高（逼近 192KB） | 无 | 尾块占比小 | 中高（大流量点） | 中 | EB0000 超限 | 可作为第二路线（性能版按余量取最大） |
| 6 | 小 tile | 细分减 UB | 次数↑（+148%@8KB） | 低 | 无 | 尾块占比大 | **低**（小 DMA 崩塌） | 低 | 低 | 不建议 |
| 7 | 行分配核 | 每核一段连续行 | GM 连续大块 | 联动 | 无（行内归约确定） | 多核写回边界（M2 后可控） | 高（93.4% 并行效率先例） | 低 | 无 | **首选** |
| 8 | tile 分配核 | 一行拆多核+workspace | 流量增加 | 中 | 跨核累加顺序不定 | — | 理论高 | **极高**（直调无协同原语） | **致命**（hang 而非报错） | 不建议 |
| 9 | DataCopyPad 尾块 | 具名 Pad 搬入+三参 Pad 搬出 | 非对齐 -21.6% | 无额外 | 补 0 无污染 | **blockLen=真实字节则安全（A 级）**；主动取整必溢出 | 中（对齐双轨可回收） | 低 | 真机验证项保留 | **首选** |
| 10 | 手工尾块 | 对齐 DataCopy+尾段 SetValue | 非对齐走小搬运 | 无额外 | 无 | 修补正确 | 低（标量黑洞） | 中 | 中 | 可作为第二路线（9 被证伪时） |
| 11 | ReduceSum 方案 | tensor 前 n 个一条龙 | — | workLocal 小 | 无（A2 方式二） | count=0 NOP | 低-中（三者最慢） | **最低** | 无 | **首选**（首版） |
| 12 | 手工向量归约 | Add 折叠+WholeReduceSum | — | 64 槽 work 区 | 低（FP32） | mask∈[1,64]/迭代 | **高**（172 vs 242 cycle） | 中高（220 参数形态对抄 ops-nn） | 中 | 可作为第二路线→**性能版主选** |
| 13 | 纯 Ascend C | 手写 kernel.asc 直调 | — | — | — | — | 判题形态上限 | 中 | 宏冲突类 CE | **首选**（唯一合法通道） |
| 14 | CUDA/Triton/PyTorch 迁移 | 方法论映射 | — | — | 迁移偏差（乘性 bias 等） | GPU mask 思维不成立 | 不可提交 | — | **不可提交**（无运行时） | 仅作研究参考 |

（每格证据出处见 agent-10 第四节原表；访存按 fp16 每元素 GM 口径，UB 按 192KB。）

**推荐组合**：首版 = 2（D≤4096 单遍）+1（D>4096 两遍重读）+3（FP32 全链）+7（行分配）+9（DataCopyPad 尾块）+11（ReduceSum）+13（纯 Ascend C）；第二候选 = 10 或 12 按触发条件替换；性能版 = 12+5+多行合并 DMA+双缓冲（大点）+512B 对齐双轨+L2 bypass+小点极简路径。

## 11. 推荐实现路线（首版：正确性优先，目标 15 点全过拿有效分）

- **算法骨架**：两档分发——D≤4096（fp16/bf16；fp32 减半）单遍 y 驻留（FP32 副本）；D>4096 两遍重读重算（全 FP32 链，不选 y 写 GM 中转）。全链：Cast(输入,CAST_NONE)→Add(FP32)→Mul 平方→ReduceSum→×1/D→+eps→Sqrt→invRms→Muls→Mul(gamma_F32)→Add(bias_F32)→Cast(输出,CAST_RINT)。
- **tiling/多核**：tile=4096/2048；行均分+余数给前 blockCount%N 核（官方 `CalculateBlockParameters` 同构）；核内 `GetBlockNum()` 自适应（免疫 SKU 未确认）。
- **尾块**：搬入 DataCopyPad（rightPadding=对齐补差、paddingValue=0）；搬出三参 DataCopyPad，**blockLen=valid×sizeof(T) 不做任何取整**（M2 安全条件）。
- **精度策略**：全 FP32 中间（含 fp16 的加法与 epilogue）；CAST_RINT 输出；epsilon 严格取入参；不做 NaN/Inf 特判。
- **UB 预算**：单遍档 ~72KB、两遍档 ~92KB，均在 195584B 安全线内。
- **V002 复用建议**：复用 V002 骨架+定向修复+裁剪，不整体重写、不原样提交。必须处理：M8（CAST_ROUND→CAST_RINT，一行）、M9（fp16 half 域加法/epilogue→FP32 域，两处）、裁剪宽行批处理/缓存行/流水等未验证性能路径（退回性能版逐条加回）；163 处手动 PIPE_V 屏障无害但性能版需评估。
- **提交双闸门**：宏扫描（pipe_/BLK/LOWER/裸 T）+上传后平台回读（首末行/行数/run_kernel）。

## 12. 第二候选路线（首版遇阻的备选）

- **触发 A（DataCopyPad 写出被真机证伪）**：尾块改手工——对齐主体 DataCopy+尾段 ≤31 元素 GlobalTensor::SetValue 逐元素写出（常数次标量写，规避标量黑洞）；不用"行尾补齐 32B 写出+下行覆写"（跨核即踩踏）。
- **触发 B（ReduceSum 异常或大 D 延迟不可接受）**：换 Add 折叠+WholeReduceSum（对抄 ops-nn `reduce_common.h`；WholeReduceSum A2 签名/16320 上限已 A 级补证，220/AIV 参数形态 `(MASK_PLACEHOLDER,1,0,1,0)` 按 ops-nn 原文落地）。
- **触发 C（单遍档 UB/行为异常）**：退化为统一两遍重读（首版已内置）。
- 保守替代：`源码/` v1 干净两遍结构直调化（换入口+tiling 直传+uint64+__vector__），代价是丢弃 V002 已验证的直调工程细节，不推荐作主路径。

## 13. 风险清单（提交前必查）

| # | 风险 | 等级 | 预防 |
| --- | --- | --- | --- |
| R1 | 标识符撞平台宏（pipe_ 已烧 V001；BLK/LOWER 随版本漂移；裸 T 模板名高危） | 致命 | 提交前 grep 扫描；tpipe 已改；避免短名/下划线尾 |
| R2 | 上传内容异常（V002 实锤一次） | 致命 | 上传后回读首末行/行数/入口；严禁快照混用 |
| R3 | 尾块写出越界（D%32≠0+多核写回） | 致命（若 M2 证伪） | blockLen=真实字节；真机哨兵值用例 P0-1 |
| R4 | bf16 输出舍入模式 | 致命 | CAST_RINT（M8 修复）；真机与 numpy RNE 对照 |
| R5 | fp16 half 域运算大数值溢出 | 高 | M9 修复：加法/epilogue 全 FP32 |
| R6 | UB 超 192KB（勿引 248KB 数字） | 高 | InitBuffer≤195584B；编译期即拦（不烧提交） |
| R7 | epsilon 假设固定值/公式变体 | 高 | 严格入参+sqrt(mean+eps) |
| R8 | 索引 int32 溢出（outer×D>2³¹） | 高 | uint64 行基址（V002 已做） |
| R9 | 事件不配对/核异常连坐 | 高 | V_S/S_V 成对+eventID 唯一 |
| R10 | 多路径泛化爆炸（V002 10+ 变体） | 中 | 首版裁剪两主干 |
| R11 | GET_TILING_DATA_WITH_STRUCT 误入直调 | 中 | 参数直传（V002 已规避） |
| R12 | iterations=5 口径/降频 | 中 | msprof warm-up 对齐；小点预热敏感 |

## 14. 未确认事项（缺口清单，agent-10 G1-G12）

1. CANN 9.0.0 核函数限定符当版原页（cookie 墙+raw 404；按 M4 裁决执行，残余风险低）。
2. DataCopyPad UB→GM dummy 丢弃的真机行为（A 级文档 vs 版本/SoC 漂移；哨兵值用例闭环）。
3. WholeReduceSum 220/AIV 参数形态的官方文档直接依据（现有 B 级源码先例）。
4. A2 Sqrt/Rsqrt/标量除 ulp（无官方承诺；余量约 240 倍，真机误差表闭环）。
5. 判题 SKU（AIV 40/48）——GetBlockNum 自适应已免疫。
6. 判题端精度判定脚本（tol 比例/equal_nan）——按 0 失配目标已免疫。
7. 15 测试点 shape/dtype/epsilon 配置——不公开，覆盖矩阵防御。
8. iterations=5 统计口径——不可闭环，测量纪律对齐。
9. V002 上传异常根因——下次完整上传+回读闭环。
10. `pipe_` 冲突宏定义位置——真机 grep（防御已落地）。
11. find_package(ASC) 9.0.0 完整行为、TileLang-Ascend 产物形态（优先级最低）。

## 15. 下一阶段实验计划

**阶段 0（本地，无真机）**：
1. V003 制作：复用 V002 骨架，修复 M8（CAST_RINT）+M9（fp16 全 FP32 域），裁剪至"单遍档+两遍档"两主干；保留全部工程防御（tpipe/uint64/具名 Pad/成对同步）。
2. 静态核对：宏扫描清单、行数/入口核对、UB 预算表复核（≤195584B）、`文档/` 四处一致性修正项（D1'-D4'，含 CAST_ROUND 记录更正、__aicore__ 记录过时、50 次/天删除、行块对齐防御降级）。
3. CPU 参考实验扩展：按 agent-06 §4 测试矩阵生成全 dtype×全尾块 golden 对照数据（真机到位即用）。

**阶段 1（真机正确性轮）**：环境按 agent-08 12 步搭建 → 模板 run.sh 本地 case0 → P0 清单（尾块哨兵值、CAST_RINT 对照、宏扫描、上传回读、全矩阵 0 失配）→ P1 清单（NaN/Inf、确定性三连跑、大 D 分段、UB 编译期）→ 首次平台提交（保守首版，目标 15 点全过拿有效分）。

**阶段 2（性能轮）**：msprof 瓶颈判定（warm-up=10/launch-count=5）→ 按分形态 tiling 逐条引入性能版优化（二分累加归约→多行 DMA→双缓冲大点→512B 双轨→L2 bypass），每条单独成版本+精度验证成对；L2Cache.csv 实测两遍档命中后再决定激进单遍变体。

**阶段 3（冲榜）**：rsqrt 替换（15 点精度实测前置）、bank 布局优化、小点极简路径微调；提交节奏绑定可回滚假设。

---

## 结语：本轮完成与未完成事项声明

- **本轮完成了**：10 路子代理全平台技术方案调研（题面/规则/API/官方仓库/GPU 迁移/编译器/精度/性能硬件/Linux 环境/失败案例/证据审阅），14 条候选路线的统一评估矩阵与反例分析，三条推荐路线（首版/第二候选/性能候选），以及 V002 的三处新发现偏差（M8/M9/多路径复杂度）。
- **有源码或官方 API 证据的方案**：官方 ops-nn add_rms_norm 五模式（A/B 级源码）、cann-samples 直调样例（A 级）、27+3 项 Ascend C API（A 级文档）、vLLM/flashinfer 等 7 个 GPU 源码参考（B 级）——方案矩阵中标注"首选"的各条均有 A/B 级支撑。
- **仅为迁移参考的方案**：全部 GPU 机制（warp shuffle/协同启动/PDL/packed 数学）、全部 DSL/自动生成路线（TileLang-Ascend/TVM/Triton/MLIR）、950PR 实测数字、低精度中间链（对照用，不建议）。
- **仍需真实 CANN/NPU 验证的结论**：G1-G11 全部缺口（DataCopyPad 真机行为、A2 ulp、CAST_RINT 严格性、判题 SKU/判定脚本/测试点配置、上传通道行为）；一切性能数字（TBest 反推为推断级）。
- **本轮没有执行 CANNJudge 上传或提交**；没有消耗任何提交次数；没有写入任何凭据；没有修改 `源码/`、`提交/`、`文档/` 下任何文件（本轮全部产出位于 `调研/调研2/`）。
