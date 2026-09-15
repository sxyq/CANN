# Agent 10 收官报告：证据审阅、矛盾裁决与方案合并（AddRmsNormBias）

- 角色：Agent 10（证据审阅、方案合并和反例验证）
- 日期：2026-09-12
- 输入：调研2 的 9 份子代理报告 + 本地源码/提交/文档/官方模板（全部只读）
- 补证：4 次定点网络补证（2 成功：C-WA-01 原文用法、WholeReduceSum 官方签名页；2 失败：hiascend 900 原页与 asc-devkit v9.0.0 raw 均不可达，缺口转真机闭环）
- 约束遵守：未调用 CANNJudge、未上传、未修改任何本地文件（本报告为唯一落盘产物）、未写算子实现代码
- 证据等级沿用：A=官方文档/官网；B=官方仓库/样例/培训材料；C=社区；D=仅摘要。本机无 NPU，全部结论止于静态/资料级。

---

## 一、来源质量审阅

### 1.1 重复来源清单（跨代理同 URL / 同仓库）

| # | 来源 | 重复引用方 | 处置 |
|---|------|-----------|------|
| D1 | `blog.csdn.net/gitblog_00410/article/details/143789539`（cann-learning-hub CANNJudge 提交 Skill 转载） | agent-01 S08（定 B）＋ agent-09 S12/C-WA-06/C-GE-01（定 C） | **同 URL 双重登记且分级不一致（B vs C）**。CSDN 转载链路未核对 GitCode 原文时按 AGENTS.md 应取 C；agent-01 的 B 定级偏高，本报告统一降为 C（转载），其"平台不开放测试用例 API"结论经 agent-01 的 A 级 contest API 独立佐证仍成立 |
| D2 | `hiascend.com/developer/blog/details/0289201670005562056`（bisheng `--npu-arch` 版本案例博文） | agent-02（辅证）＋ agent-05 S28（A 级主证据） | 同 URL，结论一致（--npu-arch 需 CANN≥8.3.RC1），无冲突，合并登记 |
| D3 | ReduceSum 官方页三个版本点 | agent-02（900 缓存＋gitcode 最新＋8.0.0.alpha001）＋ agent-07 S23（8.0.RC2 商用版） | 三版本口径一致（count 仅受 UB 限制、A2 方式二），互为交叉验证，不属冗余 |
| D4 | `atlasascendc_api_07_0215.html`（GET_TILING_DATA_WITH_STRUCT） | agent-02（900 缓存）＋ agent-09 S15（800alpha002 在线页） | 两版本均明文"暂不支持 kernel 直调工程"，结论加固，无冲突 |
| D5 | ops-nn 仓库结论 | agent-03 主研（源码级）；agent-07 §2.1/§3.5/§4 明示"引用前序 Agent 3"再引用（SINGLE_N InitBuffer 195584B、SPLIT_D 阈值） | 属合规的链式引用而非重复检索，无需处置 |
| D6 | `asc.gitcode.com` SIMD-API 文档族 | agent-02（API 签名）＋ agent-07（性能/架构页 S1-S7）＋本报告补证（WholeReduceSum） | 分工不同页面，无重复 |

### 1.2 过时版本清单（版本/commit 老于 CANN 9.0.0 或停更）

| # | 来源 | 版本 | 风险与处置 |
|---|------|------|-----------|
| O1 | agent-02 `agent02_vector_qualifier_850a002.html`（C++ 语言拓展） | 8.5.0.alpha002 | 已自标注为演进链一环；与 gitcode 最新（9.x 后期）口径存在 `__vector__` 语义差异（"不生效"→"执行空间限定符"），9.0.0 处于区间内（详见 M4 裁决） |
| O2 | agent-02 83RC1alpha002 在线快照 | 8.3.RC1.alpha002 | 同上，仅作中间点佐证 |
| O3 | agent-09 S15（GET_TILING 0215，800alpha002 URL） | 8.0.0.alpha002 | 结论已被 agent-02 的 900 缓存同文印证，可弃用旧 URL |
| O4 | agent-07 S10（SetDim，8.0.RC2）、S22-S24（msProf/ReduceSum/WholeReduce，8.0.RC 商用版） | 8.0.RC 系 | 用于 910B3"20 AIC/40 AIV"与归约语义交叉验证；msprof 参数以 9.0 真机 `--help` 复核 |
| O5 | agent-05 S22（毕昇指南 8.5.0 PDF）、S27（8.0.RC3 快速上手） | 8.5.0 / 8.0.RC3 | 编译选项历史口径；9.0.0 行为按版本序列推断（8.3 起支持 --npu-arch），已标注未验证 |
| O6 | agent-03 #5 `Ascend/samples` 旧仓 | 2023-11-22 停更 | 已确认归一化教学样例被清空/迁移，仅作历史注记，不得作为实现依据 |
| O7 | agent-08 大量 8.0.RC2/80RC3 安装与故障文档 | 8.0.RC 系 | 安装链路示例；判题对齐版本必须用 9.0.0 + HDK 配套表（agent-08 §7.1 A 级） |
| O8 | agent-09 C-CE-01（CANN 8.5.1 BLK 宏）、C-RE-02（8.1.RC1）、C-WA-04（A3 设备） | 8.x / A3 | 失效模式成立但触发阈值随版本漂移；V001 `pipe_` 平台实锤（判题环境真实发生）优先级最高 |

### 1.3 官方与社区混淆清单（C 级被当 A/B 级使用的地方）

| # | 位置 | 问题 | 处置 |
|---|------|------|------|
| C1 | agent-01 S08（见 D1） | 媒体转载定 B | 降 C；关键结论另有 A 级支撑，不受影响 |
| C2 | agent-07 S11/S12/S13/S16/S17（CSDN gitblog 转载 asc-devkit/cann-learning-hub/cannbot-skills 样例） | 转载定 B | 转载内容标注"原仓 B"，可接受，但 **S12 Add 样例 Case6 数字缺失即转载截断实证**——凡性能数字引用必须回 asc-devkit 原仓复核（真机阶段） |
| C3 | agent-05 S21（TileLang AscendNPU IR 官方技术文章） | 宣传材料 | 已自标注"性能声称未经独立复核"，处理正确 |
| C4 | agent-06 S10/S11/S12（CSDN 昇腾文章：平方先 cast、ReduceSum FP32 黄金法则） | C 级 | 定级正确；其结论与 agent-06 自身 CPU 实验互证后使用，链条干净 |
| C5 | agent-09 S19/S20（高校/媒体报道） | 结论性资料 | 已单独隔离在 4.2 节，不作独立证据，处理正确 |
| C6 | agent-08 S4/S5（CSDN zhangfeng1133 安装指南） | C 级 | 命令与官网下载页一致（A 级互证），可用 |

**总体判定：9 份报告没有发现把 C 级硬当 A 级支撑关键结论的实质错误**；唯一分级不一致是 D1（agent-01 S08），且其结论有 A 级旁路，不影响裁决。

### 1.4 相互矛盾的 API/事实说明清单（进入第二节裁决）

| # | 矛盾 | 双方 | 裁决编号 |
|---|------|------|---------|
| V1 | 单遍 y 驻留 UB vs 两遍扫描 GM 中转 | agent-07 §3.1（两遍 +67% 流量）vs agent-03 卡片 1.4（官方 SPLIT_D D>12288 两遍+GM 中转） | M1 |
| V2 | DataCopyPad UB→GM dummy 丢弃 vs 32B 写出溢出覆盖相邻行 | agent-02 定案 2（A 级）vs agent-09 C-WA-01（C 级） | M2（已补证闭环） |
| V3 | ReduceSum 一条龙 vs Add 折叠+WholeReduceSum 两级 | agent-02 #9（可用）vs agent-07 §3.3（最慢）vs agent-03 卡片 2.2（官方写法） | M3 |
| V4 | `__global__ __aicore__` vs `__global__ __vector__` | agent-02 §三（两者均合法，__aicore__ 证据链最强） | M4 |
| V5 | "每天 50 次提交额度" | agent-01 §7（无页面依据+统计反证） | M5 |
| V6 | flashinfer weight_bias 乘性 vs 本题加性 | agent-04 §2.4（已自我修正） | M6 |
| V7 | epsilon 位置 / CAST_RINT | agent-06（CPU 实测红线）vs agent-02/03（官方矩阵与官方实现） | M7 |
| V8 | **V002 输出舍入实际为 CAST_ROUND 而非 CAST_RINT**（本报告代码核对新发现） | 代码行 2805 vs 实验记录/文档/agent-09 R10（均称 CAST_RINT） | M8 |
| V9 | **V002 fp16 宽行路径 half 域加法与 half 域 epilogue 乘加**（新发现） | 代码行 2330/2720/2733-2737 vs AGENTS.md 合规边界与 agent-06 E3b/E6；官方 ops-nn half 分支亦为 half Add（agent-03 卡片 1.3） | M9 |
| V10 | Vector 整卡算力 22T vs 23.5T；GM 带宽 1.8 vs 1.6TB/s；950PR UB 256 vs 248KB | agent-07 §2.1/§2.2（自标 contradicted/存疑） | M10 |
| V11 | cann-learning-hub GitHub 存在性 | agent-03（"未发现同名活跃仓库"）vs agent-07 S19 / agent-08 S43/S44（verified 引用 hicann/cann-learning-hub） | M11 |
| V12 | 两遍重读的真实流量成本 | agent-07 §3.1 纯 GM 模型（10B/元素） vs L2 192MB 第二遍可命中（agent-07 S13/§2.3 自有数据） | M12 |

### 1.5 仅适用 GPU 或非 dav-2201/A2 型号的方案/数据清单

| # | 内容 | 出处 | 适用性判定 |
|---|------|------|-----------|
| E1 | rms_norm_quant_story 全部优化倍数（1.13x/1.55x/1.11x 等）、kv_rms_norm_rope_cache_story | agent-03 卡片 3/4（950PR/950DT，dav-3510 实测） | CMake 白名单含 dav-2201（编译可过），**数值不可直接引用，仅方法论可迁移**（agent-03 已自标） |
| E2 | VF MicroAPI / RegBase / MaskReg / `__simd_vf__` | agent-03 卡片 4 | 950 代际能力，220+CANN 9.0.0 可用性未确认；本题走 MemBase 标准 API（agent-03 §4.2-5） |
| E3 | Compact 搬运模式 | agent-07 §3.2 | **A 级明示仅 950PR/950DT，A2 不支持**——非对齐优化手段只剩对齐切分+多行合并 |
| E4 | 950PR UB 256KB 结构数字 | agent-07 §2.1 | 与本题无关（本题 192KB）；248/256 口径自身存疑 |
| E5 | agent-03 卡片 2.1 非 220 架构分支（对齐 DataCopy+标量修补） | ops-nn rms_norm_base | 310P 等旧架构路径；220 上官方自己用 DataCopyPad |
| E6 | ops-nn arch35 目录、ASCEND310P 排除条件 | agent-03 §4.2-6 | 950 服务，与本题无关 |
| E7 | agent-04 全部 7 个 GPU 源码机制 | PyTorch/vLLM/TE/flashinfer/Liger/unsloth/Triton | 方法论参考；TE cooperative launch、PDL、packed half2/bf16 数学、原地双写回、乘性 weight_bias 均不可迁移（agent-04 §5 清单） |
| E8 | agent-05 TVM/Triton/TorchInductor/MLIR/TileLang-Ascend | 全部 | 全部"不可提交，仅研究参考"（agent-05 §5 判定表，已逐条裁决）；判题只收手写 kernel.asc |
| E9 | agent-06 CUDA ulp 表（rsqrtf 2 ulp 等） | NVIDIA 文档 | 量级参照，A2 实际 ulp 未承诺（真机复核） |
| E10 | agent-08 cannsim 仿真 | zhangfeng1133 | **9.0 仿真器仅支持 950PR，A2/910B 无本地仿真捷径**（agent-08 §2 步骤 4） |
| E11 | agent-07 §2.2 A3 对比数据、S14 A2/A3 架构对比 | cann-outreach | A3=910C 非本题；A2 行可用 |

---

## 二、矛盾裁决表

### M1：单遍"y 驻留 UB" vs 两遍扫描重读 GM —— **可裁决：按 D 与 UB(192KB) 分档**

**证据**：UB=192KB/196608B（A 级，agent-07 §2.1；agent-09 C-RE-01 引 CANN 9.0.0 源码常量互证）；官方 tiling 单行因子 UB_FACTOR_B16=12288（fp16/bf16）、UB_FACTOR_B32=10240（fp32）（B/A 级，agent-03 卡片 1.2）；fp16 D=32768 时 x+y+residual 三块 64KB×3=192KB 恰好打满且无双缓冲空间（agent-07 §3.5 推断）；单遍 6B/元素 vs 两遍 10B/元素（fp16，agent-07 §3.1 模型）。

**裁决分界**：
1. **D ≤ 官方单行因子（fp16/bf16 ≤12288、fp32 ≤10240）→ 单遍 y 驻留 UB**。此时 y 的 FP32 副本（fp16 D=12288→48KB）+输入/输出/gamma 缓冲可控制在 192KB 内；6B/元素为理论下界。
2. **D > 单行因子 → 两遍扫描，且选"两遍重读 x/residual 重算 y（全 FP32）"，不选"y 写 GM 中转"**。理由：(a) 两者 GM 口径流量同为 10B/元素；(b) 重读方案少一次 MTE3 写（省 y 的写出）；(c) y 以原始 dtype 落 GM 会引入一次量化（fp16 下 y=x+r 可能溢出/失真，见 M9），重读重算保持全 FP32 链；(d) 官方 SPLIT_D 的 GM 中转（写回 xGm）服务于入图生态的额外输出 rstd/x（agent-03 卡片 1.4），本题无此需求。
3. **M12 修正（本报告新增分析）**：agent-07 的 +67% 是纯 GM 口径；A2 L2=192MB、带宽约 5TB/s（B 级，agent-07 S13），两遍重读的第二遍 x/residual 及 gamma/bias 高概率 L2 命中，**实际代价介于 6B 与 10B 之间，取决于测试点工作集与 L2 替换**。因此两遍方案在 L2 可容纳 (outer×D×2dtype 字节) 的测试点上被高估了劣势；真机用 msprof L2Cache.csv 实测（agent-07 §6.1）。
4. agent-04 C3 中"块内 y 不淘汰可继续二遍 epilogue"的表述不成立：RMS epilogue 需整行 sum 先行确定，不存在"部分块提前 epilogue"的中间态；真实选择只有"整行驻留"或"重读/重算"。

**结论**：分档执行——小中 D 单遍、大 D 两遍重读；阈值取官方 12288/10240（保守）或按实测 UB 预算微调（V002 的 fp16 cached-row 路径用 y 的 half 副本驻留，把上界外推到 D=32768，属激进变体，见 M9）。

### M2：DataCopyPad 尾块（dummy 丢弃）vs 手工尾块 —— **已闭环：A 级语义成立，C 级案例的踩坑条件已查明**

**证据链**：
- A 级（agent-02 定案 2）：CANN 9.0.X 缓存页与 gitcode 最新镜像双版本同文——UB→GM 方向 blockLen 非 32B 对齐时框架自动补 dummy、落 GM 时丢弃；结构性佐证：UB→GM 原型无 padParams 形参；官方样例 data_copy_pad_gm2ub_ub2gm 佐证。
- C 级（agent-09 C-WA-01）+ **本报告补证（原文全读）**：Segment Reduce 案例的溢出机制是——原文 3.1 节明确"burst 长度=32 字节（包含 16 字节数据 + 16 字节 padding）"，即**调用侧把 blockLen 主动取整到 32B**（其修复方案 A"feat_dim%8==0 保证 burst 对齐"、教训表"lenBurst 必须是 32 的倍数"均为同一用法的自我印证）。多搬出的 16B 是"有效载荷"而非框架 dummy，DMA 照写不误——**该案例与官方 dummy 丢弃语义不构成真冲突，而是踩坑写法的实录**。

**裁决**：
1. **安全条件**：三参 `DataCopyPad(dst_GM, src_UB, DataCopyExtParams{blockCount, blockLen=valid_count×sizeof(T), 0, 0, 0})`，blockLen 以字节为单位、传真实有效长度、不做任何向上取整——此时 dummy 由框架补齐并落 GM 丢弃，不写相邻内存（A 级）。V002 Store（行 2780-2782）与源码版 CopyOut 均为此写法 ✓。
2. **踩坑写法**（C-WA-01 实录）：(a) blockLen 主动取整到 32B（"burst 对齐"思维）；(b) 混用 DataCopyParams（四参）版本时 blockLen 单位口径（dataBlock vs 字节）搞错；(c) 用裸 DataCopy 搬非 32B 倍数长度（count 向下取整截断）；(d) C-WA-01 原文"溢出只影响不写出的空段"提示另一类风险：**若输出存在"本实现不写的空洞位置"，任何写出侧溢出都会永久残留**——本题 output 每个元素都会被写出，天然无空段，该残留模式不适用。
3. **保留真机第一验证项**（成本极低、收益高）：D=67/129/1000 + GM 相邻行哨兵值，逐字节核对（文档 7 节既定计划维持）。若证伪 → 切手工尾块（第二路线）。
4. A2 上非对齐尾块 GM→UB 端到端 -21.6%（B 级，agent-07 §2.3）——**性能损失而非正确性风险**；对齐主体走 DataCopy、仅尾块走 DataCopyPad 的双轨制（agent-09 C-TLE-03 erf 样板）是性能版优化项。

### M3：ReduceSum（一条龙）vs 手工 Add 折叠+WholeReduceSum 两级 —— **可裁决：按阶段分工**

**证据**：官方性能指南（A 级，agent-07 §3.3）：大数据量场景"二分累加方案 > ReduceRepeat 单指令 > ReduceSum 接口"，ReduceRepeat 延迟为 Add 的 2-5 倍，二分累加 172 cycle vs ReduceRepeat 242 cycle（30000 float）；ops-nn 官方实现即"Add 折叠到 64 槽（src1RepStride=0 广播）+ WholeReduceSum"（B 级，agent-03 卡片 2.2）；ReduceSum tensor 前 n 个接口在 A2 走方式二、count 仅受 UB 限制（A 级，agent-02 定案 3）。

**本报告补证（WholeReduceSum 官方页全文核读）**：A2 支持 ✓、类型 half/float、mask 连续模式 fp32 ∈[1,64]/迭代、repeatTime ∈[0,255]（即单次调用上限 64×255=16320 元素，与 agent-02 推导一致）、dst 4B 对齐（float）/src 32B 对齐、二叉树两两相加且 **half 溢出饱和 65504**（fp32 归约不受此限，本题全 FP32 归约无虞）；`isSetMask=false` 时 mask 传 MASK_PLACEHOLDER 并需外部 SetVectorMask——agent-03 摘录的 220/AIV 参数形态 `(MASK_PLACEHOLDER,1,0,1,0)` 依赖 `g_coreType==AIV` 判断，该形态来自 ops-nn 源码（B 级），官方文档未直接给出该组合，**性能版落地前需按 ops-nn 原文逐参对照**（已列缺口 G3）。

**裁决**：
1. **首版用 ReduceSum**（tensor 前 n 个接口）：签名经 A 级逐项核对（agent-02 #9/#10）、V002 已验证写法、A2 走方式二对 sharedTmpBuffer 尺寸不敏感——正确性锚点，性能劣势（官方指南排序中三者最慢）在首版可接受。
2. **性能版换"Add 折叠 + 末端一次 WholeReduceSum/ReduceRepeat"**（A 级指南 + B 级官方实现双重佐证），官方 ops-nn 写法可直接对抄；大 D（TP11-15）点预期归约收益 29% 量级（172 vs 242 cycle 口径）。
3. `GetReduceRepeatSumSpr` 不引入（文档 7 节既定裁决维持）。

### M4：`__global__ __aicore__` vs `__global__ __vector__` —— **可裁决：对齐模板用 `__vector__`，风险低**

**证据**：模板注释明示 `__global__ __vector__ void add_rms_norm_bias_custom(...)`（B 级，官方模板 kernel.asc）；题面 kernel_pattern=vector + "内部启动 __global__ __vector__ 核函数"（A 级，agent-01 §3）；agent-02 演进链：850alpha002 要求 __global__ 配 __aicore__ 且 __vector__ 在耦合架构"不生效"（被忽略非报错）→ gitcode 最新规范推荐纯 Vector 算子用 __vector__ 且官方示例即此写法；V002 现用 `__global__ __vector__`（代码行 2870 核对 ✓）、源码版用 `__global__ __aicore__`。

**裁决**：**提交版对齐模板逐字用 `__global__ __vector__`**（题面 A 级 + 模板 B 级 + 最新规范三重支持，且 V001 教训表明与判题环境对齐是第一原则）；`__aicore__` 为合法回退。9.0.0 当页原文不可达（本报告两次补证失败，hiascend cookie 墙 + v9.0.0 raw 404），**残余风险**：9.0.0 若沿用 850 措辞，__vector__ 被忽略 → 等效 __aicore__ 语义，不构成编译失败；真机安装目录头文件/文档可闭环（缺口 G1）。文档 8a 节"当前候选为 __aicore__"的记录已过时（V002 已改），应更新。

### M5："每天 50 次提交额度" —— **可裁决：不成立，不按此规划**

agent-01 §7：无任何页面依据（A 级题面/contest API 均无此条款），且 stats API 反证（单队 8 天 558 次、多队 >300 次，均值约 70 次/天 > 50/天）。**裁决：该限制按不存在的传闻处理，删除出规划依据**；提交节奏由"每次提交绑定一个可回滚假设"的质量原则约束（agent-09 §3.7），而非额度恐慌。真实限额（若存在）登录提交页确认（缺口 G6）。

### M6：flashinfer weight_bias 乘性 vs 本题加性 —— **已消解，无残留**

agent-04 §2.4 语义辨析框与 §5 清单已双处修正（"epilogue 必须改为 `x_vec[j]*rms_rcp*w + b`，gamma 与 bias 各自独立加载"）。**逐报告排查残留**：agent-01/02/03/05/06/07/08/09 均未引用 flashinfer 或乘性 bias 写法；本地文档 problem-add-rms-norm-bias.md 公式为加性 ✓；V002 代码 epilogue 为乘 gamma 后加 bias ✓。**无残留误引，结案**。

### M7：epsilon 位置与 CAST_RINT —— **可裁决：三方一致，无冲突**

- epsilon：agent-06 E4/E4b（CPU 实测：`sqrt(mean)+eps` 变体小方差行失配 99.88% 必挂）＝ agent-02/题面（`sqrt(mean(y²)+eps)`）＝ agent-03 官方实现（`Muls(avgFactor)→ReduceSum→Adds(eps)→Sqrt`）。唯一实现红线：`rms=sqrt(mean(y²)+eps)`。注意原论文无 eps（agent-06 §2.4），以题面为准。
- CAST_RINT：agent-06 E6（bf16：RINT 0% vs TRUNC/FLOOR ~40% 失配，决定性）＋ agent-02 A2 Cast 矩阵（fp32→bf16 支持 RINT/FLOOR/CEIL/ROUND/TRUNC；CAST_RINT=RNE 与 PyTorch/numpy 一致）＋ agent-03 官方 bf16 输出用 CAST_RINT。三方收敛。
- fp16 输出：agent-06 实测各模式均可过但 RINT 最稳；agent-03 记官方 fp16 用 CAST_NONE（有损时等同 RINT，agent-02 枚举注释）。**统一显式 CAST_RINT**。

### M8（新发现）：V002 输出舍入实际为 CAST_ROUND —— **裁决：改回 CAST_RINT**

代码核对：V002 `FromFloat`（行 2805）用 `RoundMode::CAST_ROUND`，注释称"matches the round-to-nearest conversion used by the reference implementation"。**与三处记录矛盾**：实验记录 E 节与文档均写"使用 CAST_RINT"；agent-09 R10 亦记录"V002 用 CAST_RINT"（未逐行核对代码）。事实：CAST_ROUND（round-half-away-from-zero）在 A2 支持矩阵内（agent-02 #12）、agent-06 E6 实测 bf16 失配 0.001%（<0.1% 容忍，能过）——**但 RINT 为 0% 且对中间扰动免疫（E6 第三组）**。agent-06 §6-8 结论"按 0 失配目标实现是唯一稳妥策略"（平台 tol 是否 0.1% 未证实）→ **裁决：一行修改 CAST_ROUND→CAST_RINT，对齐文档记录与 0 失配目标**；同时修正 agent-09 R10 的记录偏差。

### M9（新发现）：V002 fp16 宽行路径 half 域运算 —— **裁决：首版改 FP32 域，性能版真机验证后再议**

代码核对（三处）：
1. `ProcessWideFp16CachedRows` 行 2330：`Add(outputLocal[col], xLocal, residualLocal, valid)`——**half 域加法**，结果存整行 y 的 half 副本（outputBuf=rowWidth×2B，D=32768 时 64KB）；
2. `ProcessWideLowPrecision` 行 2719-2722：half 域 `Add` 后 ToFloat；
3. 行 2732-2737：epilogue `FromFloat(half)→Mul(half 域乘 gamma)→Add(half 域加 bias)`——归一化结果先量化到 half 再在 half 域仿射。

对照证据：agent-06 E3b（x=r=33000 时 half 加法 y=inf→全链 NaN；题面"输入数值不超出 dtype 原生表达范围"**不排除** 33000<65504）＋ E6/E8（低精度中间链 bf16 失配 18-19%；fp16 依赖值域侥幸）；AGENTS.md 合规边界"FP16/BF16 只在输入搬运和最终输出阶段保留目标类型"。张力：官方 ops-nn half 分支同样 half Add 后升 FP32（agent-03 卡片 1.3 CopyIn 段，B 级先例），但官方 bf16 分支反而全 FP32。**裁决**：(a) half 域加法——官方有先例但大数值必挂是 CPU 实测确凿风险，且 cast 提前到加法前只多一条 Cast 指令，首版全 dtype 统一"先 Cast FP32 再 Add"；(b) half 域 epilogue 乘加——属"中间量化+低精度仿射"，与 agent-06 环节 6"FP32 域完成全部仿射"的红线冲突，首版改 FP32 仿射后单次 CAST_RINT；(c) V002 的 bf16 路径（行 2739-2746，ToFloat gamma/bias→FP32 乘加）与 fp32 路径本身正确，保留。

### M10：数字双口径（22T/23.5T、1.8/1.6TB/s、256/248KB）—— **无法裁决，均不承重**

agent-07 已自标 contradicted/存疑。三组数字均不进入本题关键决策路径（计算非瓶颈、有效带宽 1.49TB/s 已按实测推导口径使用、950PR UB 与本题无关）。**处置：维持"存疑"标注，不引用 23.5T；反推测试点形态的误差按 agent-07 §5 声明的 ±15%~2 倍口径接受**。

### M11：cann-learning-hub GitHub 存在性 —— **可裁决：存在，agent-03 表述过弱**

agent-03 笼统称"GitHub 检索中未发现同名活跃仓库"；agent-07 S19 与 agent-08 S43/S44 以 verified 状态引用 `github.com/hicann/cann-learning-hub`（README、ipynb 全读）。**裁决：hicann org 即 CANN 官方 GitHub 镜像组织（与 ops-nn/cann-samples 同源，agent-03 自己对 ops-nn/cann-samples 即如此认定），cann-learning-hub 存在且为官方镜像；agent-03 的"未发现"应更正为"未检索到"**。对结论无实质影响（该仓贡献的主要结论——提交接口与泛化要求——已有 A 级旁路）。

### M12：两遍重读的流量模型 —— **修正 agent-07 模型（见 M1 第 3 条）**

已并入 M1 裁决：两遍重读第二遍可 L2 命中，真实成本 6B~10B 之间；SPLIT_D 的 y 中转同样可 L2 命中。**性能版决策必须以 msprof L2Cache.csv 实测为准，不预设两遍必慢 67%**。

---

## 三、14 条候选路线反例与失败条件

| # | 路线 | 反例 / 失败条件（引具体报告+来源） |
|---|------|-----------------------------------|
| 1 | **两遍扫描**（重读重算） | 反例：GM 口径 +67% 流量（6B→10B/元素，agent-07 §3.1 模型）；每行标量同步与块循环开销×2（agent-09 C-TLE-01 标量黑洞）。失败条件：D≤UB 单行因子时纯浪费带宽（大流量点 TP13-15 受损）；但 M12 修正后若 L2 命中率高，损失收窄。D>12288/10240 时**必选**（agent-03 卡片 1.4 官方 SPLIT_D 先例，B/A 级）。 |
| 2 | **单遍保存中间结果**（y 驻留 UB） | 反例：UB=192KB 硬上限（A 级，agent-07 §2.1）；fp16 D=32768 时 x+y+residual=192KB 打满、无双缓冲空间（agent-07 §3.5）；fp32 D=32768 时 y 单独 128KB。失败条件：D 超阈值仍单遍 → InitBuffer 超限 EB0000（agent-09 C-RE-05，A 级错误码：`data_ub exceed bound of memory: local.UB`）或挤掉双缓冲/gamma 预载反而变慢。D≤4096 时单遍峰值 80~112KB 安全（实验记录 D 节）。 |
| 3 | **FP32 全中间计算** | 反例（仅性能视角）：fp32 64 elem/cycle 为 half 128 的一半（A 级指令表，agent-07 §2.2）。失败条件：**无精度失败条件**——fp16 累加 D=32768 误差 63.4%、bf16 ≥2.7%（agent-06 E1）均为反面教材；带宽受限场景计算非瓶颈（agent-07 §2.2 推论），fp32 计算链代价被搬运掩盖。**保留**。 |
| 4 | **低精度中间计算**（仅对照） | 反例：fp16 域平方 \|y\|>255.96 溢出 25% 元素全行非有限（agent-06 E1b）；大数值加法 inf→NaN（E3b）；bf16 中间链失配 18-19.4%（E8）；torch 低精度 RMSNorm 失配 99.86%（E9）。失败条件：**任何大 D 或大数值/小数值测试点必挂**（fp16 累加 D=1024 即 2.4%>1e-3 容差）。仅作对照，禁止。 |
| 5 | **大 tile** | 反例：UB 超限（192KB/195584B 上限，ops-nn SINGLE_N 实测 InitBuffer 195584B，agent-07 §2.1 B 级）；tile 过大挤掉双缓冲与 gamma/bias 预载。失败条件：ΣInitBuffer>UB → 编译期/运行期 EB0000 拦截（好消息：不烧提交次数，agent-09 C-RE-05）。 |
| 6 | **小 tile** | 反例：中块 8KB/次比大块 +148% 耗时；小块 128B/次 MTE2 占 Task 98.9%（A2 官方样例实测，agent-07 §2.3，B 级）；块循环的标量开销与同步次数随 tile 数线性放大（agent-09 C-TLE-01）。失败条件：大 outer 小 D 逐行小 DMA 场景性能崩塌；TP9-15 受损。 |
| 7 | **行分配核** | 反例：outer<核数时核闲置（agent-07 §4）；SINGLE_N 每核 1 行时单核 MTE2 ≈41GB/s 成上限（推导，agent-07 §4/C 级）；每行一次 V_S/S_V 标量同步绝对时延无官方数字（agent-07 §3.4）。失败条件：小 outer 大 D 点（TP1-3，TBest 1.5-2.5μs 量级）单核带宽不足。**仍是默认主路径**（每行单核保证归约确定性与无核间同步，V002/源码版/官方均如此）。 |
| 8 | **tile 分配核**（一行拆多核） | 反例：直调单文件无跨 block 协同启动/全局 barrier 原语对应物（agent-04 §5 TE 机制不可迁移）；需 GM workspace+核间同步而直调工程无此基建（agent-02 #25/#26：GET_TILING 直调禁用；agent-05 §5）。失败条件：**直调形态下无合法实现路径**，强行实现即挂起（agent-09 C-TLE-04：直调不支持 MIX/协同类会 hang 而非报错，浪费提交次数）。D≤32768 单核 ReduceSum 可完成（agent-04 §5 自证），无必要。**不建议**。 |
| 9 | **DataCopyPad 尾块** | 反例：C-WA-01 写出溢出（C 级；M2 已裁决为"blockLen 主动取整"的用法问题，非官方语义缺陷）；A2 非对齐端到端 -21.6%（B 级，agent-07 §2.3）。失败条件：**若真机证伪 dummy 丢弃语义则 WA（静默数据损坏型）**——保留第一验证项；全行 Pad 搬运（对齐主体也走 Pad）损失 21.6% 性能。安全条件见 M2。 |
| 10 | **手工尾块**（掩码/Duplicate/标量修补） | 反例：非 220 架构的"重搬最后 block+标量 SetValue 修补"路径——SetValue 逐元素标量黑洞（agent-09 C-TLE-01：as_strided 案例 `aiv_scalar_ratio 31.5%`）；官方在 220 上明确不走此路（agent-03 卡片 2.1：220 分支直接 DataCopyPad）。失败条件：标量修补在大 outer 时 TLE；GPU masked store 思维直接翻译不成立（agent-04 表第 5 行：Ascend 无逐元素 mask load/store）。仅作 DataCopyPad 证伪后的备选，且应采用"对齐块 DataCopy+尾段≤31 元素 GlobalTensor::SetValue"而非全标量修补。 |
| 11 | **ReduceSum 方案** | 反例：官方性能指南排序"三者中最慢"（A 级，agent-07 §3.3）。失败条件：**无正确性失败条件**（签名 A 级核对、A2 方式二、count=0 NOP）；大 D 性能点冲榜时归约延迟劣势。首版安全牌。 |
| 12 | **手工向量归约**（Add 折叠+WholeReduceSum） | 反例：BinaryRepeatParams/repStride 参数复杂（agent-03 卡片 2.2）；220/AIV 参数形态 `(MASK_PLACEHOLDER,1,0,1,0)` 与非 220 不同，官方文档未直接给出该组合（M3 补证结论）；`g_coreType==AIV` 判断、SetVectorMask 外部设 mask 等前置。失败条件：参数用错（srcRepStride/dstRepStride 口径）→ 结果错或 CE；`require D<255×8` 类约束（ReduceSumMultiN 注释，agent-03）在 D 大时需分段。**收益**：172 vs 242 cycle（A 级）。 |
| 13 | **纯 Ascend C** | 无反例——判题唯一合法提交物（agent-05 §5 判定表：所有 DSL 均不可提交）；陷阱清单为宏冲突（V001 `pipe_`、BLK/LOWER，agent-09 C-CE-01/02）、命名空间（C-CE-03）、GET_TILING 直调禁用（A 级）。失败条件：仅工程性失败（标识符/上传完整性），可控。 |
| 14 | **CUDA/Triton/PyTorch 迁移实现** | 反例：不可提交（agent-05 §5：平台无 Python/torch-npu 运行时；Triton 无官方 Ascend 后端）；迁移陷阱：Welford 多余（RMSNorm 无均值，agent-04 表第 9 行）、PDL/协同启动/packed bf16 数学无对应（agent-04 §5）、`_CASTING_MODE_LLAMA`"转回原 dtype 乘 weight"与本题 FP32 要求相反（agent-04 §2.5/§5）。失败条件：**作为实现直接不成立**；方法论价值：行驻留+掩码+fp32 累加三件套、分块循环大 D 组织（Triton 教程 05）、int32 溢出前车之鉴（vLLM #43390，C 级）。仅研究参考。 |

---

## 四、统一方案评估矩阵

> 说明：编号 1-4 为计算组织、5-8 为分块与多核、9-12 为尾块与归约实现、13-14 为代码来源；各条**可组合**，逐条独立评估后附推荐组合。访存次数按 fp16 每元素 GM 口径（agent-07 §3.1），UB 占用按 192KB 上限（fp16/bf16 主 dtype，D 以 4096 tile 为例）。

| # | 方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
|---|------|------|---------|---------|----------|----------|----------|----------|--------------|----------|
| 1 | 两遍扫描（重读重算） | Pass1 归约+Pass2 epilogue，重读 x/r/g/b | 10B（L2 命中后 6-10B）（agent-07 §3.1，推断/C 级模型+M12 修正） | 低（tile 级缓冲 ~76KB）（agent-02 #18，A 级约束） | 低：全 FP32 链（agent-06 E1/E11，本地实验级） | 与 9/10 联动 | 中：大 D 点可行、小中 D 让位单遍 | 低（agent-03 官方 NORMAL 同构，B 级） | 低（纯标准 API） | **可作为第二路线**（大 D 档为必选路径） |
| 2 | 单遍 y 驻留 UB | y 的 FP32/half 副本驻留，一遍完成 | 6B（下界）（agent-07 §3.1） | 高：D≤4096 峰值 80-112KB；fp16 D=32768 half 副本 64KB+缓冲逼近 184KB（V002 实测布局核对） | 中：y 存 half 副本则引入量化（agent-06 E3b，本地实验级） | 与 9/10 联动 | 高：省 28.6-40% 流量（实验记录 D 节推算） | 中（UB 预算精细规划） | 低 | **首选**（D≤4096 档）；D>12288 禁用 |
| 3 | FP32 全中间计算 | Cast 前置+全 FP32 链+单次 CAST_RINT | —（与 1/2 正交） | 中：FP32 中间缓冲为 half tile 2 倍 | **无**（agent-06 E1/E10：全链 0 失配；E11 fp32 容差不敏感） | — | 计算非瓶颈（agent-07 §2.2，A 级表） | 低 | 无（A2 类型矩阵全支持，agent-02 #12-15，A 级） | **首选**（非可选项，红线） |
| 4 | 低精度中间计算 | half/bf16 域平方/累加/仿射 | — | 低 | **致命**：累加误差 63%/2.7%、溢出、失配 18-40%（agent-06 E1/E1b/E8，本地实验级） | — | — | 低 | bf16 计算指令 A2 不支持（agent-02 §4.1，A 级） | **不建议**（仅对照） |
| 5 | 大 tile（8192+） | 减少 DMA 次数 | 次数↓（agent-07 §2.3 大块最优，B 级） | 高：逼近/超 192KB（agent-07 §2.1，A 级） | 无（FP32 链） | 尾块占比小 | 中高（大流量点） | 中 | EB0000 超限风险（agent-09 C-RE-05，A 级错误码） | 可作为第二路线（性能版按 UB 余量取最大 tile） |
| 6 | 小 tile | 细分减 UB | 次数↑（+148%@8KB、98.9%@128B，agent-07 §2.3，B 级） | 低 | 无 | 尾块占比大 | **低**（小 DMA 崩塌） | 低 | 低 | 不建议（仅 D<tile 时的自然尾块） |
| 7 | 行分配核 | 每核一段连续行（差≤1） | GM 连续大块（agent-07 §4，B 级） | 与 1/2 联动 | 无（行内归约确定）（agent-04 表 9，B 级） | 多核写回边界（M2 裁决后可控） | 高（并行效率 93.4% 先例，agent-03 卡片 3，B 级@950PR） | 低 | 无 | **首选** |
| 8 | tile 分配核 | 一行拆多核+workspace | 流量增加（中间量落 GM） | 中 | 跨核累加顺序不定（agent-09 C-WA-05，C 级） | — | 理论高（超宽行） | **极高**（直调无协同原语，agent-04 §5/agent-05 §5） | **致命**：hang 而非报错（agent-09 C-TLE-04，C 级） | 不建议 |
| 9 | DataCopyPad 尾块 | 具名 Pad 搬入补 0+三参 Pad 搬出 | 非对齐 -21.6%（B 级，agent-07 §2.3） | 无额外 | 补 0 对归约无污染（agent-06 §2.1，与 Triton other=0 等价，agent-04 表 5） | **M2 已裁决**：blockLen=真实字节则安全（A 级 agent-02）；主动取整 32B 必溢出（C-WA-01 原文补证） | 中（对齐双轨制可回收 21.6%） | 低 | 真机验证项保留（若判题机行为与文档不符） | **首选** |
| 10 | 手工尾块 | 对齐块 DataCopy+尾段标量/GatherMask | 非对齐部分走小搬运 | 无额外 | 无 | 修补路径正确（C-WA-01 同构场景的根治法） | 低（标量黑洞，agent-09 C-TLE-01，C 级） | 中 | 中 | **可作为第二路线**（仅 9 被证伪时启用） |
| 11 | ReduceSum 方案 | tensor 前 n 个一条龙 | — | workLocal 小（128B 级即可，agent-02 定案 3） | 无（A2 方式二，A 级） | count=0 NOP（A 级） | 低-中（三者最慢，A 级指南） | **最低** | 无（A 级签名核对） | **首选**（首版） |
| 12 | 手工向量归约 | Add 折叠+WholeReduceSum/ReduceRepeat | — | 64 槽 work 区（agent-03 卡片 2.2） | 低（FP32） | mask 连续模式 ∈[1,64]（本报告补证，A 级） | **高**（172 vs 242 cycle，A 级指南 agent-07 §3.3） | 中高（220 参数形态需对抄 ops-nn） | 中（参数形态 B 级来源） | **可作为第二路线→性能版主选** |
| 13 | 纯 Ascend C | 手写 kernel.asc 直调 | — | — | — | — | 判题形态上限 | 中（2955 行 V002 为例） | 宏冲突/命名空间类 CE（agent-09 R1，V001 实锤） | **首选**（唯一合法通道，agent-05 §5） |
| 14 | CUDA/Triton/PyTorch 迁移 | 方法论映射 | — | — | 迁移偏差（乘性 bias、LLAMA casting，agent-04 §5） | GPU mask 思维不成立（agent-04 表 5） | 不可提交 | — | **不可提交**（平台无运行时） | 仅作研究参考 |

**推荐组合**：
- **首版（正确性）**＝ 2（D≤4096 单遍）+ 1（D>4096 两遍重读）+ 3（FP32 全链）+ 7（行分配）+ 9（DataCopyPad 尾块）+ 11（ReduceSum）+ 13（纯 Ascend C）。
- **第二候选（首版遇阻）**＝ 10（9 被真机证伪时替换尾块）或 12（11 行为异常时替换归约）；1 作为 D>12288 的兜底恒在组合内。
- **性能版（首版 15 点全过后）**＝ 12（二分累加）+ 5（按 UB 余量放大 tile）+ 多行合并 DMA（blockCount+dstStride 补齐，agent-03 卡片 3.2，B 级）+ 双缓冲（大点，agent-07 §3.5 A 级指南+agent-03 实测 1.55x@950PR）+ 512B 对齐主体/尾块分离双轨（agent-07 §2.3）+ L2 bypass 输入（-28.9%，agent-07 §2.3 B 级）+ 小点单缓冲极简路径（agent-07 §3.5 反例约束）。

---

## 五、三条推荐路线

### 5.1 首版（正确性优先，目标 15 点全过拿有效分）

- **算法骨架**：两档分发——`D ≤ 4096`（fp16/bf16；fp32 减半）走单遍 y 驻留（Pass1 算 y 存 FP32 副本并归约出 invRms，Pass2 直接消费 UB 里的 y 做 epilogue，不重读 GM 输入）；`D > 4096` 走两遍重读（Pass1 归约，Pass2 重读 x/residual 重算 y 再 epilogue）。全部 FP32 中间链：Cast(输入,CAST_NONE)→Add(FP32)→Mul 平方→归约→`mean=×1/D`→`+eps`→`sqrt`→`invRms=1/rms`→Muls→Mul(gamma_F32)→Add(bias_F32)→Cast(输出,CAST_RINT)。
- **tiling/多核**：tile=4096（fp16/bf16）/2048（fp32）（V002 现值）；`blockCount=min(availableCoreNum, UINT32_MAX)`，行均分+余数给前 blockCount%N 核（V002 现行算法，官方 ops-nn CalculateBlockParameters 同构，agent-03 卡片 1.1）；核内 `GetBlockNum()` 自适应（agent-07 §4：判题 SKU 20-48 AIV 未确认，运行时自适应消除该风险）。
- **尾块**：搬入 DataCopyPad（具名赋值：leftPadding=0、rightPadding=对齐补差、paddingValue=0）；搬出三参 DataCopyPad，blockLen=valid×sizeof(T) **不做任何取整**（M2 裁决）。
- **归约**：ReduceSum(fp32, workLocal=1024 float) + V_S/S_V 成对同步 + GetValue(0)；块间 rowSum 标量累加（源码版 ReduceRowSum 结构，agent-02 #9 A 级签名）。
- **精度策略**：全 FP32 中间（含 fp16 的加法与 epilogue——修正 V002 的 M9 两处）；CAST_RINT 输出（修正 V002 的 M8）；epsilon 严格取入参（agent-09 C-WA-06 教训）；不做任何 NaN/Inf 特判与"稳定化"补丁（agent-06 §5）。
- **UB 预算（192KB）**：单遍档（fp16 D=4096）：x 8KB+residual 8KB+y(F32) 16KB+gamma 8KB+bias 8KB+reduce work 16KB+输出 8KB ≈ 72KB，余量充足；两遍档（tile 4096）：源码版布局 ~92KB。均在 195584B 安全线内（agent-07 §2.1）。
- **依据**：agent-02（API A 级 27+3 项）、agent-03（官方 NORMAL/SPLIT_D 骨架与 tiling 阈值，A/B 级）、agent-06（CPU 精度链决定性实验）、agent-09 R1-R11 风险清单、agent-01（接口与判题规则 A 级）。
- **风险与回退**：宏冲突（提交前扫描 pipe_/BLK/LOWER/裸 T 等短名，agent-09 R1）；上传完整性（编辑器回读首末行/行数/run_kernel，agent-09 R2）；DataCopyPad 写出真机证伪→路线 5.2 尾块替换；ReduceSum 行为异常→路线 5.2 归约替换；平台出现成片"未定义"报错→补 `using namespace AscendC;`（agent-09 C-CE-03）。

### 5.2 第二候选（首版遇阻的备选）

- **触发条件 A（DataCopyPad 写出被真机证伪，M2 保留项命中）**：尾块写出改手工方案——对齐主体走 DataCopy（blockLen=32B 整数倍部分），尾段 ≤31 元素（不足 32B 的余数）用 GlobalTensor::SetValue 逐元素写出（每行常数次标量写，非逐元素循环，规避 C-TLE-01 黑洞）；或整行写出时在 UB 内把行尾补齐至 32B 后按对齐块写出、再由下一行所有者覆写被覆盖的头部——**仅当相邻行同核时安全**（跨核即 C-WA-01 踩踏模式），故首选前者。
- **触发条件 B（ReduceSum 行为异常或大 D 点延迟不可接受）**：换 Add 折叠+WholeReduceSum（agent-03 卡片 2.2 官方写法对抄；本报告已补证 WholeReduceSum A2 签名/对齐/16320 上限，参数形态 `(MASK_PLACEHOLDER,1,0,1,0)` 按 ops-nn 原文+`g_coreType==AIV` 判断落地，缺口 G3 真机复核）。
- **触发条件 C（单遍档 UB 或行为异常）**：退化为统一两遍重读（首版已内置该路径，天然回退）。
- **依据**：agent-02/03（A/B 级）+ 本报告 M2/M3 补证。

### 5.3 性能优化候选（首版验证后冲榜）

- **算法骨架**：保持首版两档分发，按测试点形态（agent-07 §5 TBest 反推，推断级）细分路径：
  - 小点（TP1-3，≲0.6MB）：单遍单缓冲、指令数最小化、禁双缓冲（agent-07 §3.5 官方反例：小数据强行双缓冲适得其反）；
  - 中大点（TP4-12）：gamma/bias 预载一次（1.13x@950PR，B 级方法论）+ 多行合并 DMA（blockCount=N 行+UB 侧 dstStride 行间补齐，agent-03 卡片 3.2 标准解法；MTE2 次数 -71% 同源）+ 双缓冲（BUF_NUM=2，大点 1.5x 量级，agent-07 §3.5）；
  - 巨型点（TP13-15，0.9-2.2GB）：512B 对齐主体+尾块分离双轨（-21.6% 回收，agent-07 §2.3）+ SetL2CacheHint(CACHE_MODE_DISABLE) 于只读一次的输入（-28.9%，agent-07 §2.3 B 级）+ L2 命中监控（M12：两遍档第二遍 L2 命中率决定是否值得换激进单遍/中转方案）。
- **归约**：二分累加替换 ReduceSum（A 级 172 vs 242 cycle；agent-03 官方写法）。
- **尾块**：对齐块 DataCopy 快路径+非对齐 DataCopyPad 兜底（erf 样板双轨制，agent-09 C-TLE-03，C 级正面样板 15/15）。
- **UB 预算**：按 UB 余量放大 tile（8192 档，V002 宽行路径已示范 184KB 布局）；bank group 布局检查（四 buffer 间距避免 12KB=192KB/16 整数倍，agent-07 §2.1，A 级）。
- **测量纪律**：msprof op `--warm-up=10 --launch-count=5` 对齐判题 iterations=5（口径未确认，agent-07 §6.3）；瓶颈判定流程与 `aiv_time` 核间均衡 <10% 达标（agent-07 §6.2）。
- **依据**：agent-07（A 级指南+A2 实测 B 级）、agent-03（官方 tiling 五模式与样例实测）、agent-09（erf/as_strided 判题节奏样本）。
- **风险与回退**：每个优化单独成版本提交并绑定可回滚假设（agent-09 §3.7 节奏）；rsqrt 替换 1/sqrt 仅在 15 点精度实测后启用（agent-07 §2.2/agent-06 §6-1）；性能改动与精度验证成对进行（agent-04：tiling 参数改变浮点累加顺序）。

### 5.4 V002 复用 vs 参考首版重构的建议

**建议：复用 V002 骨架，做定向修复与裁剪，不整体重写、也不原样提交。**

理由：(a) V002 已对齐直调接口全项（run_kernel 签名、`__global__ __vector__`、tpipe、uint64 寻址、尾块具名 Pad、ReduceSum+成对同步——与 5.1 首版要求重合度高，且 V001/V002 两次平台学费买在这些工程修复上）；(b) 但存在三处必须处理的问题——M8（CAST_ROUND→CAST_RINT，一行）、M9（fp16 half 域加法/epilogue→FP32 域，两处路径）、多路径复杂度（2955 行十余个 Process 变体使 15 点×三 dtype×路径数的验证矩阵爆炸，任一路径 bug 只在特定 shape 触发，agent-09 C-GE-01 泛化风险的放大器）；(c) 163 处 PipeBarrier<PIPE_V> 在直调默认自动同步下冗余（agent-02 §五-7，无害但性能版需评估）。

**落地形态**：以 V002 为基础保留"单遍 y 驻留（D≤4096）+两遍重读（宽行）+行分配+DataCopyPad 尾块+ReduceSum"主干与全部工程防御，砍掉宽行批处理/缓存行/流水等未验证性能路径（退回性能版阶段按 5.3 逐条加回并逐条验证）；同步完成 M8/M9 修复。此即"V003 规划"的实证内容。**若选择保守替代**：源码/ v1 的干净两遍结构直调化（换入口+tiling 直传+uint64+__vector__）也是可行首版，代价是丢弃 V002 已验证的直调工程细节，不推荐作为主路径。

---

## 六、证据缺口与无法确认结论清单

| # | 缺口 | 缺什么证据 | 去哪补 | 什么级别能闭环 |
|---|------|-----------|--------|---------------|
| G1 | CANN 9.0.0 核函数限定符原页（`__vector__`/`__aicore__` 当版措辞） | 9.0.0 版本原文（agent-02 仅有 850/83RC1/gitcode 最新三点演进链；本报告 2 次补证均不可达：hiascend cookie 墙、v9.0.0 raw 404） | 真机 `$ASCEND_TOOLKIT_HOME` 安装目录文档/头文件；或 gitcode asc-devkit v9.0.0 tag 逐路径核对 | A 级（官方版本原文）。**当前按 M4 裁决执行，残余风险已评估为低**（被忽略≠报错） |
| G2 | DataCopyPad UB→GM dummy 丢弃的真机行为 | A2+CANN 9.0.0 实测（A 级文档 vs C 级反例的结构性冲突已由 M2 补证消解为"用法问题"，但版本/SoC 行为漂移未排除） | 真机 D=67/129/1000+相邻行哨兵值逐字节核对；官方样例 `data_copy_pad_gm2ub_ub2gm` 逐行核读（agent-02 已给路径未逐行读） | B 级（官方样例源码）即可强化、真机运行记录（内部 A 级等价）闭环 |
| G3 | WholeReduceSum 220/AIV 参数形态 | ops-nn 调用形态 `(MASK_PLACEHOLDER,1,0,1,0)` 的官方文档直接依据（本报告补证已核签名/对齐/16320 上限/二叉树饱和语义，A 级；但该参数组合仅有 B 级源码先例） | ops-nn `reduce_common.h` 原文对抄+真机编译运行 | B 级源码+真机验证 |
| G4 | A2 Sqrt/Rsqrt/标量除 ulp 精度 | 官方无 ulp 承诺（agent-06 §6-1）；当前余量约 240 倍（容差 1e-3/注入 4.2e-6） | 真机已知 FP32 值→Sqrt/Rsqrt→与 CPU 对照 | 真机误差表（内部闭环）；rsqrt 启用的前置条件 |
| G5 | 判题 SKU（910B1-4，AIV 40/48） | 题面/API soc_version=null（agent-01 §6.3） | 真机 npu-smi / 平台提交日志 Block Dim 字段；**GetBlockNum() 运行时自适应已消除决策依赖**（agent-07 §4） | 任何级别；当前设计已免疫 |
| G6 | 判题端精度判定脚本（tol 比例/isclose/equal_nan） | 平台不公开（agent-01 §4.2） | 登录提交页文案 / 一次试探性提交的 precision_ratio 反推 | 平台行为级；**按 0 失配目标设计已免疫**（agent-06 §6-8） |
| G7 | 15 测试点 shape/dtype/epsilon 配置 | 平台不开放（agent-01 §4.1；cann-learning-hub Skill 明示必须泛化，C 级旁证） | 无法补；以覆盖矩阵防御（agent-06 §4 矩阵+版本实验记录 C 节用例表） | 不可闭环，防御性设计 |
| G8 | iterations=5 统计口径（均值/最优/中位） | 题面未说明（agent-01 §4.1） | 无法从公开信息补；本地按 msprof warm-up+launch-count 对齐 | 不可闭环；测量纪律对齐 |
| G9 | V002 上传异常根因 | 平台收到的内容与本地不一致（agent-09 L-02）；在线编辑器粘贴行为无法外部复现 | 下次完整上传（新提交编号）验证；上传后回读 | 平台行为级闭环 |
| G10 | `pipe_` 冲突宏的定义位置 | V001 日志只有报错文本（agent-09 §6-1） | 真机 `grep -rn "define pipe_" $ASCEND_TOOLKIT_HOME` | 真机源码级（A 等价）；防御（tpipe 改名）已落地 |
| G11 | 9.0.0 直调工程 find_package(ASC) 完整行为 | 本机无 CANN（agent-08 §8-2） | 真机 cmake 输出 | 真机闭环 |
| G12 | TileLang-Ascend 产物形态（能否导出 .asc） | 与提交无关的可选研究项（agent-05 §6-3） | 如需：运行 ascendc_pto 分支 | B 级；优先级最低 |

---

## 七、与本地代码差距对照

### 7.1 源码/（msopgen v1 两遍正确性版）距推荐首版的差距

| # | 差距 | 现状 | 首版要求 | 依据 |
|---|------|------|----------|------|
| S1 | **工程形态** | op_kernel/op_host 分离+`GET_TILING_DATA_WITH_STRUCT`+`REGISTER_TILING_DEFAULT`（行 278-279） | 直调单文件 run_kernel 入口+参数直传；该宏 9.0.X 明文不支持直调工程，搬入即 CE | A 级（agent-02 #25；agent-09 C-CE-04） |
| S2 | 入口限定符 | `__global__ __aicore__`（行 273） | 对齐模板 `__global__ __vector__` | M4 裁决 |
| S3 | 寻址宽度 | `uint32_t base = row * D`（行 126/175） | uint64_t 行基址（outer×D>2³¹ 溢出，vLLM #43390 前车之鉴） | agent-04 §3-6；V002 已修 |
| S4 | GetBlockIdx 隐式收窄 | `uint32_t coreIdx = GetBlockIdx()`（行 90，int64_t→uint32_t） | 显式 static_cast（实际安全，严格口径） | agent-02 #23 |
| S5 | tiling 来源 | op_host TilingFunc 生成（框架接入型） | host main 自算（cann-samples PlatformAscendCManager 模式） | agent-03 卡片 3.2 |
| S6 | gamma/bias 搬运 | Pass2 每块每行重搬（行 187-188） | 每核预载一次循环复用（官方 1.13x） | agent-03 卡片 1.3/3.3 |
| S7 | 单遍档 | 无（统一两遍重读） | D≤4096 单遍 y 驻留 | 本报告 5.1 |
| S8 | dtype 编码 | 1=fp16/2=bf16/3=fp32（自定） | 直调 TensorInfo 枚举 0=fp32/1=fp16/2=bf16（转换层必须写对） | agent-01 §3 |

**优点保留**：全 FP32 中间链（含 fp16 先 Cast 再 Add——与 M9 裁决一致）、TQue 正规流水、ReduceSum+成对事件、DataCopyPad 尾块写法全部正确——计算内核可作为直调化的语义参考基准。

### 7.2 提交/V002（2955 行直调候选）距推荐首版的差距

| # | 差距 | 现状（代码行核对） | 首版要求 | 依据 |
|---|------|-------------------|----------|------|
| P1 | **输出舍入模式** | `FromFloat` 用 CAST_ROUND（行 2805） | CAST_RINT（0 失配目标） | M8 裁决；agent-06 E6 |
| P2 | **fp16 half 域加法** | 行 2330/2720（`Add(outputLocal[col], xLocal, residualLocal)` half 域） | 先 Cast FP32 再 Add（大数值 inf 风险） | M9 裁决；agent-06 E3b |
| P3 | **fp16 half 域 epilogue** | 行 2733-2737（FromFloat→Mul→Add 全 half 域） | FP32 域仿射后单次 CAST_RINT | M9 裁决；agent-06 E6/E8 |
| P4 | 多路径复杂度 | 10+ 个 Process 变体（SmallFp32Batched/ContiguousBatched、Fp16FullTile/FullRow、WideFp32 Batched/CachedRows、WideLowPrecision、CachedRows 等），分支条件含 kCacheElems=8192、kWideTileElems=8192、kWideFp32TileElems=12288 等 13 个常量 | 首版裁剪至"单遍档+两遍档"两条主干；性能路径退回 5.3 逐条加回 | 本报告 5.4；agent-09 C-GE-01 |
| P5 | PipeBarrier 冗余 | 手动 PipeBarrier<PIPE_V> 遍布（agent-02 统计 163 处） | 直调默认自动同步下非必需（无害；性能版评估） | agent-02 #22/§五-7（A 级注记） |
| P6 | Load 对齐快路径缺失 | 全部 Load 走 DataCopyPad（行 2758-2774，含对齐块） | 性能版：对齐块 DataCopy+尾块 Pad 双轨 | agent-07 §2.3；agent-09 C-TLE-03 |
| P7 | 平台验证状态 | 一次上传异常（L-02），2955 行版本从未上平台 | 完整上传+回读 checklist | agent-09 R2 |

**已达标项（无需动）**：run_kernel 签名与 `<<<blockCount, nullptr, stream>>>` ABI（行 2880-2951）、`__global__ __vector__`（行 2870）、tpipe 标识符、uint64 寻址、DataCopyPadExtParams 逐字段赋值（行 2768-2773）、Store blockLen=真实字节不取整（行 2780-2782）、ReduceSum+V_S/S_V 成对同步、bf16 路径全 FP32（行 2739-2746）、fp32→fp32 用 Adds 零拷贝（行 2788-2791，规避 Cast 矩阵无 fp32→fp32 行）。UB 布局均在 192KB 内（最紧的 fp16 cached-row 宽行 ~184KB）。

### 7.3 本地文档与代码的一致性修正项

| # | 文档 | 问题 | 处置 |
|---|------|------|------|
| D1' | 实验记录 E 节 / agent-09 R10 | 均记录"V002 用 CAST_RINT"，与代码 CAST_ROUND 不符 | 更正记录；代码按 M8 修复 |
| D2' | 文档/problem 8a 节 | "当前候选为 `__aicore__`"已过时（V002 已是 __vector__） | 更新 |
| D3' | 文档/competition-rules.md | "每天最多 50 次提交"（M5 已裁决不成立） | 删除或标注反证 |
| D4' | 文档/problem 7 节 | "多核 32B Cache Line 撕裂"防御（k×D×sizeof(T)≡0 mod 32 行块对齐）基于 C-WA-01 推演——M2 裁决后该踩踏模式仅在"blockLen 取整写出"时发生；三参 DataCopyPad 真实字节写出下无需行块对齐约束 | 降级为"备选防御，随 G2 真机验证定夺"，避免无谓的调度复杂度 |

---

## 八、结语：审阅置信度声明

- 本报告全部结论止于静态核对与资料级证据（本机 macOS 无 CANN/NPU）；**不构成任何 NPU 编译、精度或性能通过的声明**。
- 9 份子代理报告的证据质量总体良好：关键结论均有 A/B 级支撑、C 级均作了隔离标注；发现 1 处分级不一致（D1）、2 处记录与代码不符（M8/R10、M9 相关文档缺失）、1 处覆盖表述过弱（M11），均已裁决或更正。
- 最高价值的三项闭环：M2（DataCopyPad 踩坑条件随 C-WA-01 原文补证落地，A 级语义+安全条件+踩坑写法三分）、M8/M9（V002 两处精度偏差在提交前修复，避免烧一次平台验证）、M12（两遍方案 L2 修正，性能版不误判基线）。
- 最大证据缺口仍是**真机**：G1-G4、G9-G11 全部指向"获得 CANN 9.0.0 + dav-2201 环境"这一唯一解锁路径；在真机到位前，一切性能结论以"推断"标注，不进入提交决策。
