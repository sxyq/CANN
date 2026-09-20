# Agent 10 报告：证据审阅、方案合并与反例验证（AddRmsNormBias）

> 角色：证据审阅 / 方案合并 / 反例验证代理（Agent 10）
> 日期：2026-09-11
> 输入：Agent01~Agent09 九份方案调研报告 + `提交/V001/kernel.asc`（当前候选）+ `research-report.md`（旧版，待更新）+ `sources.md`（来源总表）
> 平台前提：macOS，无 CANN/NPU。本报告所有结论均为文档/源码/实验静态核验，**一律未在真实 NPU 编译/精度/性能验证**；凡涉及"可编译/可跑/达标"的表述都只是预判，闭环条件见第 9 节证据缺口清单。
> 证据等级沿用：A=官方题面/官方API/官方仓库原文；B=官方样例/源码/测试/官方权威培训；C=社区；D=仅摘要未核验。

---

## 1. 来源去重与版本核对结果

### 1.1 跨报告重复登记（同一 URL/仓库被登记 ≥2 次，需主 Agent 合并）

| 来源 | 出现位置 | 处理建议 |
| --- | --- | --- |
| CSDN 训练营 RMSNorm 教程 `2401_82857325/155447666` | sources.md #13、Agent09 N21（同 URL） | 保留一条，附另一编号 |
| CSDN 训练营 RMSNorm 第二十期 `155560947` | sources.md #11（#13 为同作者同主题另一篇） | 两篇是不同文章，各自保留 |
| ops-transformer gitcode 仓 `e7019c299c...` | sources.md #16、Agent03 来源#2（同一 commit） | 合并 |
| cann-samples 仓 `23c981c0918...` | sources.md #23、Agent03 来源#1（同一 commit） | 合并 |
| CSDN as_strided 实战 `gitblog_01418/150380401` | sources.md #93、Agent01 #68、Agent07 A7-7、Agent09 N1（**4 处**） | 合并为一条（等级 C） |
| ops-transformer RMSNorm 精度分析 `hwcomputing/6a156b8510` | sources.md #15、Agent06 #7、Agent09 佐证（3 处） | 合并为一条（等级 C） |
| asc-devkit ReduceSum 转述 `gitblog_00089/151635178` | Agent02 #8、Agent07 A7-5 | 合并（B，转述） |
| asc-devkit DataCopy 最佳实践 `gitblog_00924/157925888` | Agent02 #28、Agent07 A7-2 | 合并（B，转述） |
| CANNBot 直调开发指南 `gitblog_00909/151507556` | Agent02 #29、Agent09 N12 | 合并（C） |
| cannbot-skills npu-arch 转述 `gitblog_01165/151219465` | sources.md #89、Agent01 #64 | 合并（B，转述） |
| ReduceSum API 手册（商用 8.0 `atlasascendc_api_07_0078`） | sources.md #7、Agent02 #6、Agent09 佐证 | 合并（A） |
| DataCopyPad 8.0 手册 `07_0253` | sources.md #4、Agent02 #3 | 合并（A） |
| 无 DataCopyPad 的处理方式 `atlas_ascendc_10_0037` | sources.md #6、Agent02 #19 | 合并（A） |
| 910B UB 1.5MB 说法（HIVM 博客） | Agent02 #34、Agent07 A7-18 | 合并（C，矛盾数据，仅记录） |

### 1.2 编号体系错乱（sources.md 主清单未同步）

- Agent01 声称"已同步追加编号 58-71"，主清单实际落在 **#83-96**；
- Agent04 声称"追加编号 54-61"，主清单实际落在 **#97-104**；
- **Agent02（34 条）、Agent03（5 条）、Agent06（11 条）、Agent07（20 条）、Agent08（28 条）、Agent09（21 条）的新增来源均未并入 sources.md**（主清单止于 #104）。合计约 119 条待统一重编号补登——这是下一轮主 Agent 的必做项，否则无法追溯"每条结论的来源证据链"。

### 1.3 版本核对（关键结论是否过时）

| 核对项 | 结论 | 证据 |
| --- | --- | --- |
| 判题 CANN 版本 | 题面标注 + API `cann_version` 字段 = **9.0.0**，无歧义 | A（Agent01 §6） |
| 9.0.0 本体 API 页 | `CANNCommunityEdition/900/API/ascendcopapi/...` 下 DataCopyPad/ReduceSum/Cast 页面 URL 抓取失败（404/空），Agent02 用 9.x master（2026-09 构建）+ 8.x 手册推断 9.0 差异 → **"推断兼容，未验证"** | Agent02 未确认 #2；Agent07 未确认 #6 |
| DataCopyPad 支持矩阵 | 8.0 手册：A2 训练/800I A2/200-500A2；9.x master 新增 A3、950（mode 重载、Compact、负 srcStride）；老训练系列/推理系列仍不支持；9.0 手册将 DataCopyPad 标注为 **(ISASI)**（跨硬件不保证兼容的指令体系接口）——新增风险信号 | A（Agent02 §1.1/§3） |
| ReduceSum 参数名 | 8.x 为 `workLocal`，9.x 改名 `sharedTmpBuffer`，含义相同；blockLen 上界 8.0 写 [1,2097151]，9.x master 写 [0,]，**以上界以安装版头文件为准** | A（Agent02 §1.1/§1.2） |
| 9.0.1/9.1.0 公告 | 只涉及 aclnn 层算子接口（aclnnMatmulAllReduceAddRmsNorm 废弃、拆出 aclnnAddRmsNorm），与 Kernel API 无关 | A（Agent02 §3） |
| SoC 型号 | 模板默认 `dav-2201` ↔ 910B1~B4/910B2C/910_93（cannbot-skills 映射，B 级）；`dav-1001`=910/910A 老训练系、`dav-2002`=310P 推理系、`dav-3002`=200I/500 A2、`dav-3510`=950 系。**模板默认值 ≠ 判题机型号确认** | B（Agent01/08） |
| 编译链 | 模板（ASC 语言 + find_package(ASC) + `--npu-arch`）是 **CANN ≥8.3 新式直调工程**，8.2 及以下不支持 `--npu-arch` | B/A（Agent08 §5） |

---

## 2. 官方 vs 社区推断区分

### 2.1 A/B 级官方证据（可直接作为实现依据）

- 题面 + 官方 API desc：15 测试点、得分公式 `100/(1+log₁.₅(t/T))`、T=全局最优、均值排名（A，Agent01 实测验证 78.79↔78.78）。
- 模板接口：`run_kernel(GM_ADDR×5, TensorGroupInfo×5, availableCoreNum, stream, epsilon)` + `<<<blocks,nullptr,stream>>>`；dtype 枚举 0=fp32/1=fp16/2=bf16；Kernel 类型 vector（A/B，Agent01）。
- DataCopyPad 字段语义（blockCount/blockLen/srcStride/dstStride/rightPadding 单位与上限、GM 侧 1B 对齐、padding ≤32B）、UB→GM 搬出"自动丢弃 dummy 精确写回"（A，Agent02，8.x 手册）。
- ReduceSum 签名、workLocal 公式 `RoundUp(count/64,8)*8`（fp32）、src 32B 对齐、**A2 count 版=方式二（同 repeat 二叉树、不同 repeat 顺序累加）**（A，Agent02）。
- Cast RoundMode 枚举与 A2 dtype 组合（half/bf16→fp32 CAST_NONE；fp32→bf16 无 CAST_NONE，用 RINT；Add/Mul 在 A2 不支持 bfloat16_t）（A，Agent02）。
- ops-nn `norm/add_rms_norm*`（含 **AddRmsNormQuantV2 公式与本题完全一致**：`y_i = x_i·γ_i/Rms(x)+bias`）、ops-transformer AddRmsNorm 五模式 Tiling（SPLIT_D/NORMAL/SINGLE_N/MERGE_N/MULTI_N）、cann-samples rms_norm_quant_story 优化链（A/B，Agent03，已读源码）。
- 归约性能指引：归约类指令延迟 ≈ Add 的 2~5 倍，二分累加（Add+ReduceRepeat）172 cycles < ReduceRepeat 242 cycles，ReduceSum 接口为软仿通常最慢（A，Agent02/07）。
- `GetValue/SetValue 仅调试用，性能极低`（官方文档原文，A，Agent02）；"标量吞吐极低、性能黑洞"（C 级实战，Agent07 2.4）。
- 向量 Sqrt 正确舍入 0 ulp（B 级文档，Agent06 2.4）；CAST_RINT=RNE 的官方舍入规则原文（A，Agent06 2.5）。
- 模板 `verify_result.py` 判题语义：`np.isclose(rtol,atol,equal_nan=True)` 逐元素 + 0.1% 失配容忍；golden 计算链 = 量化输入→fp32 计算→cast 回原 dtype（A/B，Agent06，读源码）。
- 910B 老训练系列不支持 DataCopyPad、ReduceSum 仅 half、Cast 组合极少（A，Agent02）——**这是"降级路线"的存在依据**。

### 2.2 C 级社区推断（只能作为预判/风险提示）

- 判题机具体 SoC 型号（910B1~B4 / 800I A2 / 910B2C 等）——推断为 A2 系，**未证实**（Agent01 ⑤）。
- 910B 系每核 UB=192KB（B 级 asc-devkit 样例口径 + C 级实战 + arXiv 反推一致；另有 1.5MB/2MB/256KB 矛盾说法被两代理排除）。
- 核数 20/24/25/32/40/48 互相矛盾（Agent07 1.1）。
- GetValue 单次开销绝对值——无公开数字（Agent07 未确认 #5）。
- DataCopyPad UB→GM 写方向 padding 覆盖相邻行（Agent09 案例 6.1，与官方 A 级语义冲突，见第 3 节矛盾 1）。
- ReduceSum(count) 实际上限 4096（Agent09 案例 7.1，B 级官方博客经验值，与 API 文档冲突，见矛盾 2）。
- 双缓冲收益 -60%（10.2ms→4.1ms）是训练营 C 级数据，非本算子实测（Agent07 未确认 #7）。
- 50 次/日提交额度：无任何公开页面依据（Agent01 §8，C/D）。

### 2.3 "推断但未证实"（必须真机闭环）

1. 判题机 = A2 系产品形态（DataCopyPad 可用）——由模板默认 dav-2201 + cannbot 映射 + CANN 9.1 下载页"910b-ops 对应 Atlas A2"推断。
2. 判题端精度判定实现 = 本地模板 verify_result.py（isclose + 0.1%）——推断一致，未证实（Agent01 ⑤）。
3. CAST_RINT 与 numpy RNE 在 A2 硬件上的逐位等价——官方规则文档支持，实际行为未验证（Agent06 2.5、Agent04 §5）。
4. 15 个测试点各自的 shape/dtype/epsilon 配置——平台不开放（Agent09 10.1）。
5. 标量 `sqrtf` 在 AI Core 标量单元的精度（是否正确舍入）——无文档，未验证（Agent06 2.4 只保证向量 Sqrt）。

---

## 3. 矛盾清单（≥3 条，全部有出处）

### 矛盾 1：DataCopyPad UB→GM 写方向是否"精确长度"（最危险矛盾）
- **A 级官方文档**（Agent02 §1.1）："blockLen 非 32B 对齐时，框架自动补 dummy 对齐，**搬到 GM 时自动丢弃 dummy，实现精确长度的非对齐写回**"。
- **C 级社区实测**（Agent09 案例 6.1，Segment Reduce 算子）：DataCopyPad 写 GM 方向 burst 最小单位 32B，有效 16B 写出时 **padding 真实覆盖了相邻段前 4 字节**，导致结果错误。
- 影响：直接决定"行间连续布局 + 尾块搬出"方案（首版 CopyOut 正是 `DataCopyPad(dst[offset], src, {1, len*sizeof(T), 0,0,0})`）在 D 非 32 倍数（题面明确允许 D=192/576 等）时是否踩相邻行。**真机第一验证项，无折中空间。**

### 矛盾 2：ReduceSum(count) 能力上限——三处说法不一致
- **A 级 API 手册**：count 上限"受 UB 大小限制"（Agent02 §1.2）——即没有 4096 特限。
- **B 级官方博客经验**（XJTUACM S7，Agent09 案例 7.1）：文档宣称任意长度，实际内部至多调用两次 WholeReduceSum，**count 版上限 ≈4096 元素**，作者被卡后改分段求和。
- **B 级官方源码**（ops-nn add_rms_norm_base.h）：`ReduceSumFP32ToBlock` 注释 count 需 <255 repeat ≈ **16320 个 FP32**（Agent03 2.1）。
- 统一处置：**分块 ≤4096 规避争议**（首版 fp32 tile=2048 安全；half tile=4096 恰好贴在上限，建议降 2048 或真机确认），与 Agent06"任何分块 ≤4096 对 bf16 D=32768 均安全"（0.002~0.010% mismatch）一致。

### 矛盾 3："910B 不支持 DataCopyPad" vs "A2 系支持"——产品命名混淆
- Agent02：DataCopyPad 支持按**产品系列**划分——"Atlas A2 训练/推理系列 √；**Atlas 训练系列产品（老 910A/910B 训练系列）×**；Atlas 推理系列产品（310P 等）×"，并注明"910B 不支持 DataCopyPad 的说法只对老训练系列成立，A2 架构（910B 的 A2 系列产品形态）支持"。
- Agent01/07：把模板默认 dav-2201 直接映射到"910B 系列"并假设 DataCopyPad 可用、UB=192KB。
- Agent03（官方 ops-nn 源码）：尾块用 **DataCopyPad（arch 220/3003/3113）或 DataCopyCustom 的 GetValue/SetValue 重排（910 系列）**——即官方代码对 910 系自带"免 DataCopyPad"的降级实现。
- 实质：官方文档"产品系列"命名（Atlas A2 训练 vs Atlas 训练系列 vs 800I A2 推理 vs 推理系列 310P）与芯片口语（910B）错位，**判题机形态未定前，DataCopyPad 可用性 = 悬案**；风险应对 = 保留 DataCopyCustom 手工尾块作为第二路线（矩阵路线 10）。

### 矛盾 4：入口限定符 `__aicore__` vs `__global__ __vector__`
- **A 级官方文档**（Agent02 §2）：`__aicore__`（不区分核型，耦合模式）与 `__vector__`（仅 Vector 计算）均为合法执行空间限定符，**纯向量算子两者均可用**。
- **C 级官方指南转述**（CANNBot 直调指南，Agent09 P1/N12）："矩阵类用 __aicore__，**纯向量类必须 __vector__**"。
- 处置：判题模板 main.asc 用 `__global__ __vector__`；首版 kernel.asc 用 `extern "C" __global__ __aicore__`。**最稳妥 = 提交前与模板逐字对齐（__global__ __vector__）**，改动成本极低。

### 矛盾 5：社区硬件数据互相矛盾（已统一主口径，待真机定案）
- UB 容量：192KB（主口径）vs 1.5MB/2MB/256KB（Agent02 #31/#34、Agent07 A7-18）。
- AI Core 核数：20/24/25/32/40/48（Agent07 1.1）。
- HBM 带宽：1.6TB/s（A 级官方彩页）vs ~1.5TB/s vs ~0.4TB/s（疑笔误，未采信）。

---

## 4. GPU / 错误型号陷阱（只适用于 GPU 或错误昇腾型号的方案）

### 4.1 GPU 机制（Agent04 §4）——在昇腾上不可行或需条件成立

| # | GPU 技巧 | 昇腾对应物缺失原因 | 处置 |
| --- | --- | --- | --- |
| 1 | warp shuffle / cub::BlockReduce / block_reduce（寄存器间归约） | 昇腾无线程/CTA 概念，归约全走 SIMD 指令；GPU 树形归约与昇腾二叉树求和顺序不同，**位级结果不可移植** | 换向量 ReduceSum + workLocal（矩阵路线 11/12） |
| 2 | atomicAdd / 自旋锁块间归约 | 昇腾 atomic 支持面窄、性能受限，照搬退化为串行写热点 | 行切分天然避免跨核归约 |
| 3 | cooperative groups / grid sync（flash-attention CTAS_PER_ROW>1） | 昇腾无 grid 级同步，同一行跨核归约需要 GM 原子/二次 kernel | 直接否掉（矩阵路线 8 评级依据） |
| 4 | 动态 shared memory（运行时按 N 伸缩） | UB 必须 tiling 期静态规划，kernel 内不能动态分配 | 预算静态化（Agent07 §2.1） |
| 5 | GPU mask 谓词"零成本、不越界"假设 | 昇腾 DataCopyPad 补 0 真实写 UB、占搬入带宽；**搬出无谓词 store，必须限长** | 尾块搬出必须 DataCopyPad/手工方案 |
| 6 | float4 / _f16Vec<8> 16B 对齐快速路径 | 昇腾对齐粒度 32B（DataCopy）/256B（块），非对齐行首只能 DataCopyPad | 按 32B/256B 粒度设计 |
| 7 | Welford 在线统计 | RMS 分支不需要均值，三状态更新是纯负担 | 直接用 Cast→Mul→ReduceSum |
| 8 | Triton autotune（num_warps 等） | 昇腾调参对象是 repeat/双缓冲份数/tile 大小 | 真机网格扫描（Agent07 §5.6） |

### 4.2 错误昇腾型号/工具（Agent05 §3.2、Agent03 不可迁移清单）

| 陷阱 | 说明 |
| --- | --- |
| Triton-Ascend / AKG / FlagTree | 输出编译后二进制 kernel，产物形态与直调模板 `kernel.asc` 不匹配，不可作为提交路径 |
| TileLang-Ascend | 生成依赖 Catlass 模板与 CANN 工具链的完整算子，需人工提取后按直调签名重写；且其 issue #890（N=513 尾块错）与 #1225（裸 rsqrt 精度不足）是本题的直接反例 |
| PyPTO / XLA-NPU / TorchInductor | 指令级/图级产物，均非单文件 kernel.asc |
| IREE / TVM 主线 / MLIR 主线 | 无 Ascend 后端 |
| cann-samples `rms_norm_quant_story` 的 dav-3510（Ascend 950）构建 | 950 系专属（VF RegAPI、mode 重载 DataCopyPad、dav-3510），照搬到 910B 会编译/运行失败 |
| StoreUnAlign（asc-devkit） | **A3/950 系专属，A2 不支持**，不能作为本题尾块方案 |
| SIMT `asc_atomic_add` | 仅 950 支持（A2/A3 ×）；降级路线用的是 MTE 级 SetAtomicAdd + DataCopy，两者须区分 |
| 老式 ccec / `-c ai_core-<soc>` / 8.x kernel_operator.h 教程 | 模板是 ASC 语言（bisheng + `--npu-arch`），8.2 及以下不支持；判题环境版本差是 Compile Error 头号来源（Agent09 案例 1.1） |

---

## 5. 各方案反例与失败条件（结合 Agent06/07/09 证据）

- **路线 1 两遍扫描**：反例 = memory-bound 下 x/residual 读两遍，访存量翻倍（fp32 20B/元素，Agent07 §2.3），若 T 由单遍实现者创下则得分折损；失败条件 = 判题机非 A2 系（DataCopyPad 不可用）→ 需换路线 10 或整块+mask。无精度反例（FP32+分块安全）。
- **路线 2 单遍存 y**：反例 = D 大时 y(fp32) 超 UB 必须退化两遍；**存 GM 对 fp16/bf16 净亏**（写读 y 8D 字节 > 少读的 4D 字节，Agent07 §2.7 账目）；失败条件 = D > 4096(fp32)/8192(half) 且未分块。vLLM 的 in-place 写回 GM 输入在直调模板下不可用（输入只读，Agent04 §3.1）。
- **路线 3 FP32 全中间**：反例 = 无（黄金法则）；唯一失败条件 = 与 golden 中间 Cast 时机不同步（Agent09 案例 2.2：全程 fp32 反而对不上 golden 的 S8 实测案例）→ 必须同链验证。
- **路线 4 低精度中间**：反例 = fp16 累加必溢出（|y|≥256 单步 inf，|y|=10 且 D=1000 也 inf，Agent06 §2.1）；bf16 域加法误差 ~25% 元素 1-ulp 翻转（Agent06 §2.2）；fp16 大数吃小数（Agent09 案例 2.3）。直接否定。
- **路线 5 大 tile**：反例 = count>4096 撞 ReduceSum 上限争议（矛盾 2）；fp32 D=8192 一行一块 + 双缓冲 + gamma/bias 驻留 ≈224KB 超 192KB（Agent07 §2.1 账目）；非 32 倍数 D 时单块搬大数据、尾块对拍风险集中。失败条件 = 未在真机确认 4096 上限前直接用 count=8192。
- **路线 6 小 tile**：反例 = 块数多 → 每块一次 GetValue 则打断流水次数多（性能黑洞放大，Agent07 §2.4）；单次搬运 <4KB 效率骤降（128B→8192B 搬运耗时差 +148%，Agent07 §2.2）。失败条件 = 每块标量同步 + D 大 outer 大。
- **路线 7 行分配**：反例 = outer=1 时只有 1 核干活（Agent07 §2.5）；行数 < 核数时负载不均；失败条件 = 分配边界算错漏行/重算（Agent09 P10）；跨行污染（tail 越界）。
- **路线 8 tile 分配跨核**：反例 = 跨核归约需 GM 原子/自旋锁/二次 kernel，昇腾无 grid sync（Agent04 #2/#3/#7，Agent05 §2.8）；"块间原子累加"照搬退化为串行写热点。直接否定（除非 outer 极小且真机证明收益，仅作研究参考）。
- **路线 9 DataCopyPad 尾块**：反例 = 写方向 padding 覆盖相邻行（矛盾 1，Agent09 案例 6.1）；非对齐代价 -21.6%（GM→UB）/-47.5%（GM→L1）（Agent07 §2.8）；失败条件 = 判题机非 A2 系。搬入补 0 方向无风险（padding ≤32B 满足文档约束，0² 不污染归约，Agent02/04）。
- **路线 10 手工尾块**：反例 = GetValue/SetValue 逐元素是性能黑洞（Agent07 §2.4）；官方 DataCopyCustom 只在尾部（≤32B 量级）用标量重排，量级可控（Agent03 §2.1）；失败条件 = 尾元素多（>32B 级）导致标量操作爆炸。GatherMask/UnPad 为 PIPE_V（A 级流水表）但签名未逐页核验（Agent02 未确认 #6）。
- **路线 11 ReduceSum**：反例 = count 上限三方争议（矛盾 2）；A2 count 版"方式二"累加（repeat 间顺序）与 golden 的 pairwise 求和逐位不一致未验证（Agent02 未确认 #5）；workLocal 空间不足会错（公式 `RoundUp(count/64,8)*8`）；失败条件 = count>4096 单次调用、workLocal 拍脑袋分配。
- **路线 12 手工向量归约**：反例 = 二分累加（Add+ReduceRepeat 172 cycles）快于 ReduceSum（242 cycles）但代码复杂度高（Agent07 §2.4）；Add 折叠就地修改 src 有别名风险；`ReduceSumHalfInterval` 就地折叠（Agent03 §2.2）；Reg API 版有 `UpdateMask 破坏 count 标量` 坑（Agent03 §2.5）；失败条件 = 折叠点/循环边界算错（死循环，Agent09 案例 4.1）。
- **路线 13 纯 Ascend C**：反例 = 无（唯一可行路径，Agent05 核心结论）；失败条件 = 命名空间（Agent09 案例 1.1）、`.asc` 后缀（案例 1.4）、入口限定符（矛盾 4）、栈上大数组（案例 3.6）、DeQue 未配 FreeTensor（案例 3.3）、WaitFlag 分支遗漏（案例 3.8）。
- **路线 14 GPU 迁移**：反例 = 执行机制层（warp 归约/atomic/grid sync/动态 smem/16B 向量化）全部不可照搬（Agent04 §4）；位级结果不可移植；失败条件 = 直接翻译任一 GPU kernel 到 Ascend C。

---

## 6. 14 条方案矩阵

> 列：方案/算法 / 访存次数 / UB 占用 / 精度风险 / 尾块风险 / 性能潜力 / 实现难度 / CANN 兼容风险 / 推荐级别。访存以"每元素字节数"计（ts=输入类型字节，fp32=4、half/bf16=2）。推荐级别四档：首选 / 可作为第二路线 / 仅作研究参考 / 不建议。所有评分为静态分析 + 上述反例证据，未真机验证。

| # | 方案/算法 | 访存次数（读/写/元素） | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 两遍扫描（Pass1 平方和归约 → Pass2 归一化+γ+b 输出） | 读 2×(x+res)=4·ts，写 out=ts，共 5·ts（fp32=20B） | 一行/tile 常驻，任意 D 可跑；tile 8KB 级 ~68KB | 低：FP32 归约 + 分块 ≤4096 安全（A6 实测 mm<0.01%） | 中：搬出尾块依赖 DataCopyPad 写方向语义（矛盾 1） | 中：memory-bound，访存翻倍是上限 | 低 | 中：依赖 DataCopyPad（A2 系 OK） | **首选**（正确性基线，Agent03 官方 NORMAL/SPLIT_D 同构） |
| 2 | 单遍保存中间结果（y 驻留 UB，Pass2 直接用） | 读 x+res=2·ts，写 out=ts，共 3·ts | y(fp32)=D·4B；D≤4096(fp32)/8192(half) 才现实 | 低：同链 FP32 | 同路线 1 | 高（小 D 形态省一次 GM 读，接近单遍上限） | 中 | 同路线 1；D 大需退化两遍 | **可作为第二路线**（仅小 D 形态，Agent07 §2.7；Agent04 方案 A） |
| 3 | FP32 全中间计算（输入 cast fp32→归约→输出 cast 回） | 与载体路线相同 | 需额外 fp32 tile（首版 3×8KB） | 无（黄金法则）；唯一风险=与 golden 中间 Cast 时机不同步（A9 案例 2.2） | 无 | 中（多两次 Cast 的指令开销可忽略） | 低 | 低（A2 Cast 组合已确认，A2） | **首选**（强制前提，A2/A3/A4/A6 一致） |
| 4 | 低精度中间计算（对照方案） | 同载体 | 省 fp32 tile | **高：fp16 累加必溢出；bf16 域加法 ~25% 元素翻转**（A6 §2.1/§2.2；A9 2.3） | 无 | 高（省 Cast） | 低 | 中 | **不建议** |
| 5 | 大 tile 方案（D≤8192 一行一块，官方 SINGLE_N/NORMAL） | 同路线 1（两遍）或 2（单遍） | 8192 half：双缓冲 x/res 64KB+y32KB+γ/b 16KB ≈112KB；fp32 同 D ≈224KB **超限** | 低（分块前提）；ReduceSum count=8192 撞上限争议（矛盾 2） | 中：大块尾块对拍集中 | 高：大搬运 8KB~128KB 高效区（A7 §2.2） | 中 | 中：需先解 count 上限 | **可作为第二路线**（性能向，约束：half/bf16 且 count 分两段或真机确认） |
| 6 | 小 tile 方案（1024 元素块，多块循环） | 同路线 1 | 最低（tile 4KB 级） | 最低（chunk≤1024 实测 0.002~0.010%，A6） | 低：尾块逻辑简单 | 中：4KB 在高效区边缘；块数多→GetValue 次数多（若每块同步） | 低 | 低 | **可作为第二路线**（精度最稳、尾块最简单） |
| 7 | 以行分配 AI Core（outer 均分，blockFactor 逻辑） | 无附加 | 无附加 | 无 | 无 | 高（多核并行，A7 §2.5） | 低 | 低 | **首选**（ops-nn blockFactor 同构，Agent03；首版已实现） |
| 8 | 以 tile 分配 AI Core（大 D 跨核分块+跨核归约） | 无附加 | 无附加 | 中（跨核部分和合并顺序） | 无 | 理论高（outer=1 时唯一并行手段），实际被跨核通信吃掉 | 高 | 高（GM 原子/自旋/二次 kernel，无 grid sync，A4 #2/#3/#7） | **不建议**（Agent07 §2.5：outer=1 接受单核） |
| 9 | DataCopyPad 尾块方案（搬入补 0 + 搬出真实字节） | 无附加 | 补 0 占用 ≤32B/tile | 低（0² 不污染归约） | **高：写方向 padding 覆盖相邻行（矛盾 1，A9 6.1）**；非对齐代价 -21.6%/-47.5%（A7 2.8） | 中（尾块非对齐有实代价） | 低 | 中：判题机须 A2 系（A2 文档） | **首选（条件性）**：真机验证写方向语义通过后；否则降级路线 10 |
| 10 | 手工尾块方案（GetValue/SetValue 重排 / Duplicate / GatherMask） | 无附加 | 无附加 | 低 | 低（显式控制尾部） | 低-中（标量操作性能黑洞，A7 2.4；官方仅尾部 ≤32B 用，A3 2.1） | 中-高 | 低（910 系列官方代码在用，A3 DataCopyCustom） | **可作为第二路线**（DataCopyPad 不可用时的官方降级；GatherMask/UnPad 签名待核验） |
| 11 | ReduceSum 方案（count 版 + workLocal；mask 连续版） | 无附加 | workLocal 按公式 `RoundUp(count/64,8)*8` | 低-中：count 上限争议（矛盾 2）；A2 方式二累加 vs golden pairwise 逐位差异未验证（A2 未确认 #5） | 无（归约侧） | 中：软仿最慢（A2/A7），块间标量合并可接受 | 低 | 低-中：分块 ≤4096 规避上限争议 | **首选**（分块 ≤4096 + 块间标量合并；首版已实现） |
| 12 | 手工向量归约方案（Add 折叠 + WholeReduceSum / 二分累加） | 无附加 | 复用输入 tensor 或独立 work（可省 4KB，A3 0_naive） | 低（全二叉树，与 golden pairwise 更接近，A6） | 无 | 高：二分累加 172 vs ReduceSum 242 cycles（A7 2.4） | 中-高（折叠点/别名/UpdateMask 坑） | 低（WholeReduceSum/BlockReduceSum 为官方 API，A3） | **可作为第二路线**（性能优化候选，真机 profiling 确认后启用） |
| 13 | 纯 Ascend C 实现（直调模板内手工 kernel，不依赖自动生成） | — | — | — | — | — | — | — | **首选**（强制：唯一可提交形态，A5 核心结论） |
| 14 | 从 CUDA/Triton/PyTorch 方案迁移的实现 | 算法结构可迁移，执行机制不可照搬 | — | 位级不可移植（归约顺序不同） | GPU mask 假设不成立 | 参考价值高 | 高（需按昇腾模型重写） | 高（直接翻译必挂，A4 总判断） | **仅作研究参考**（作为算法参考，不作为实现） |

---

## 7. 当前候选评估（`提交/V001/kernel.asc`）

### 7.1 在矩阵中的位置

首版 = **路线 1+7+9+11+13 的组合**（两遍扫描 + 按行均分 AI Core + DataCopyPad 搬入补 0/搬出 + ReduceSum count 分块 + 纯直调手工 kernel），并满足路线 3（FP32 全中间）+ CAST_RINT 输出。Agent03 对照结论：与官方 NORMAL/SPLIT_D 结构一致，方向正确，属于"正确性基线"形态（Agent07 v1 档）。

### 7.2 已知风险点逐条：哪些已有对策 / 哪些无对策

| # | 风险点 | 现状与对策 |
| --- | --- | --- |
| R1 | 入口 `extern "C" __global__ __aicore__` vs 模板 `__global__ __vector__`（矛盾 4） | **无对策（当前代码未改）**。A 级文档称两者对纯向量均可用，C 级指南称纯向量必须 __vector__；提交前按模板逐字对齐（低风险改动）。 |
| R2 | BUFFER_NUM=1 无双缓冲 | **无对策**（正确性阶段可接受）。性能优化 v4 目标（收益 -50~60% 级，C 级训练营数据，Agent07 未确认 #7）；开双缓冲需重算 UB 账目（~108KB < 192KB，Agent02 §4）。 |
| R3 | 每 tile 一次 GetValue + V_S/S_V 同步（D=32768 fp32 每行 16 次；循环内反复 FetchEventID） | **无对策**。性能风险（Agent07 §2.4、Agent09 P5）；FetchEventID 是否耗用事件槽（A2 槽 0-7）未确认（Agent02 未确认 #4）。v3 优化：行末一次标量读或向量化 rsqrt。 |
| R4 | ReduceSum count 上限：fp32 tile=2048（安全）；**half tile=4096 恰好贴在上限边界**（矛盾 2） | **部分有对策**：fp32 2048 在 4096 内；half 4096 需真机确认或降到 2048。workLocal=1024 远超公式所需（count=4096 → 64 元素），富余。 |
| R5 | D 非 32 倍数尾块写出 32B burst（矛盾 1） | **无对策（真机第一验证项）**。CopyOut 用 `DataCopyPad(dst[offset], src, {1, len*sizeof(T), 0,0,0})`；官方 A 级称精确写回，社区 C 级实测 padding 覆盖相邻行。备选：路线 10 DataCopyCustom（GetValue/SetValue 重排）。 |
| R6 | 标量 `sqrtf` 精度（bf16 风险） | **无对策**。Agent06 §2.4：rsqrt 误差 2^-14 → bf16 全挂；向量 Sqrt 文档保证 0 ulp。标量 sqrtf 是否正确舍入无文档；真机验证或改向量 Sqrt。首版顺序为 `sqrtf(sum/D+eps)` 再 `1/rms`（两次标量舍入），与 golden 的 `y/rms` 或 `1/rms` 同序问题需 CPU 同链对照（Agent06 附注）。 |
| R7 | 归约顺序：reduce 后除 N vs 官方 reduce 前 `Muls(avgFactor)` | **无对策**。Agent03 对照：官方前置缩放缩小动态范围，精度行为可能不同，真机对照。 |
| R8 | gamma/bias 每行每 tile 重搬（outer×tiles 次重复读 GM） | **无对策**。性能风险（访存放大，Agent07 §2.7）；v2 优化：D 允许时整体驻留（half D≤8192 32KB、fp32 D≤4096 32KB）或 tile 切片一次预载。 |
| R9 | 已有对策项（正确） | epsilon 位置 `sqrt(mean+eps)` ✓（题面公式）；FP32 归约 + CAST_RINT ✓（Agent06/09）；搬入补 0 的 padding ≤32B 满足文档 ✓；行分配负载均衡（each+extra）✓；blocks ≤ outer 防空核 ✓；V_S/S_V 事件成对无分支遗漏 ✓（Agent09 3.8）；UB 预算 ~68KB < 192KB ✓（Agent02 §4）；workLocal 富余 ✓；tile 8KB 在搬运高效区 ✓（Agent07 §2.2）。 |

---

## 8. 推荐路线

- **首版推荐路线（正确性基线）**：路线 1+7+9+11+13 组合（两遍扫描 + 行均分 + DataCopyPad 尾块 + ReduceSum 分块 ≤4096 + FP32 中间 + CAST_RINT 输出）。提交前仅两处改动：① 入口改 `__global__ __vector__` 与模板逐字对齐（R1）；② half tile 降至 2048 或真机确认 4096 安全（R4）。
- **第二候选路线**：小 D 形态单遍存 y（路线 2+6）——D ≤ 4096(fp32)/≤8192(half) 时 y 留 UB 省一次 GM 读（访存 5ts→3ts）；大 D 保持两遍。依赖判题机 A2 系 + DataCopyPad 写方向验证通过。
- **性能优化候选路线**：手工向量归约（路线 12：Add 折叠 + WholeReduceSum / 二分累加）+ BUFFER_NUM=2 双缓冲 + gamma/bias 驻留/预载（R8）+ 行末一次 GetValue 或全向量化 Sqrt+Duplicate（R3）。按真机 profiling（PipeUtilization.csv 的 MTE2 vs Scalar 占比，Agent07 §5.3）决定 v2/v3/v4 优先级。

---

## 9. 证据缺口清单（必须真机验证才能闭环，编号）

1. **判题机 SoC 型号**（dav-2201=910B/A2 系？具体 B1~B4 / 800I A2 / 910B2C？）。为什么重要：决定 DataCopyPad/ReduceSum/Cast 支持矩阵、核数、UB 预算、`--npu-arch` 取值，是全部方案的分叉点（Agent01 ⑤、Agent02 未确认 #1、Agent07 未确认 #1）。
2. **DataCopyPad UB→GM 写方向是否精确字节**（burst 32B padding 是否写入 GM 覆盖相邻行）。为什么重要：D 非 32 倍数（题面明确 D=192/576 等）尾块方案的生死线；A 级文档与 C 级实测直接冲突（矛盾 1，Agent09 案例 6.1）。
3. **ReduceSum(count) 实际能力上限**（4096？16320？仅受 UB 限制？）+ A2 count 版"方式二"累加在 bf16 D=32768 的实测 mismatch。为什么重要：决定 tile 大小与分块策略；上限争议（矛盾 2）+ 累加顺序与 golden pairwise 的逐位一致性未验证（Agent02 未确认 #5、Agent09 案例 7.1）。
4. **标量 sqrtf / 向量 Sqrt 的实际精度**（bf16 需 rsqrt 误差 ≤2^-20 或 Sqrt 0 ulp）。为什么重要：rsqrt 2^-14 → bf16 全挂（Agent06 §2.4 实测 0.4~1.2%）；tilelang issue #1225 是真实先例。
5. **CAST_RINT 与 numpy RNE 的逐位等价**（fp32→half/bf16 实际舍入行为）。为什么重要：bf16 通过依赖与 golden 逐位一致 ≥99.9%（Agent06 §2.5）；S8 案例证明中间 Cast 时机也影响匹配（Agent09 案例 2.2）。
6. **FetchEventID 是否耗用事件槽**（A2 槽 0-7，循环内反复调用是否溢出）。为什么重要：若耗用则首版每行 2 次 Fetch 可能溢出导致同步错乱（Agent02 未确认 #4）。
7. **判题端精度判定实现细节**（isclose 的 rtol/atol 组合、失配上限 0.1%、bf16 比较前是否转 fp32，是否与本地模板 verify_result.py 一致）。为什么重要：Agent06 全部结论建立在该假设上（Agent01 ⑤）。
8. **15 个测试点各自的 shape/dtype/epsilon 配置**。为什么重要：平台不开放测试用例 API，只能泛化覆盖；D∈{64,65,127,128,4096,4097,8192,32768}、outer∈{1,2,核数±1}、dtype×3、epsilon×{1e-5,1e-6} 需在真机逐一验证（Agent09 案例 10.1）。
9. **上传表单实际字段与提交方式**（直调 kernel.asc 单文件 vs 旧 4 文件字段 tiling_h/host_cpp/kernel_cpp）。为什么重要：提交格式错 = 直接 Compile Error/判错（Agent01 §5 ⑤、Agent09 案例 8.1）。
10. **双缓冲/大 tile 在真机的实际收益与 UB 预算确认**（每核 192KB；双缓冲 -60% 是训练营 C 级数据非本算子实测）。为什么重要：决定 v4 优化投入与 tile 上限（Agent07 未确认 #7）。
11. **核数 GetBlockNum 运行时值**（社区 20~48 矛盾）。为什么重要：性能预估上下界与多核扩展曲线（Agent07 未确认 #3）。
12. **9.0.0 安装版头文件中 DataCopyPad/ReduceSum/Cast 签名与 9.x master 差异**（blockLen 上界、sharedTmpBuffer 改名、ISASI 标注、dynUBufSize=nullptr 是否被接受）。为什么重要：编译期兼容性的最终裁决（Agent02 未确认 #2/#3）。

---

## 10. 本轮新增来源清单

本轮为审阅/合并轮，未新增外部 URL；新增"处理对象"为以下工作区文件（全部已读全文）：

| 文件 | 路径 | 用途 |
| --- | --- | --- |
| Agent01~Agent09 九份报告 | `/Users/sunyiyang/Desktop/Project/cann/调研/调研1/Agent0{1..9}-*.md` | 方案矩阵与矛盾清单的依据 |
| 当前候选 kernel | `/Users/sunyiyang/Desktop/Project/cann/提交/V001/kernel.asc` | 第 7 节评估对象 |
| 旧版调研报告（待更新） | `/Users/sunyiyang/Desktop/Project/cann/调研/调研1/research-report.md` | 交叉核对（第 7 节结论与之一致） |
| 来源总表 | `/Users/sunyiyang/Desktop/Project/cann/调研/调研1/sources.md` | 第 1 节去重/编号核对对象 |

**遗留登记任务（交给主 Agent）**：Agent02（34 条）、Agent03（5 条）、Agent06（11 条）、Agent07（20 条）、Agent08（28 条）、Agent09（21 条）的增量来源未并入 sources.md（主清单止于 #104）；第 1.1 节列出的重复条目需合并重编号。
