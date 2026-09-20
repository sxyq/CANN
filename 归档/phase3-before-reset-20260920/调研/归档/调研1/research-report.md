# 调研报告：AddRmsNormBias 方案空间深度调研（2026-09-11 第二轮）

> 本轮以 10 个并行子代理完成覆盖 8 类平台的深度调研（第一波 Agent 1–9 并行，第二波 Agent 10 证据审阅/反例/矩阵合并），主 Agent 汇总。工作区为 macOS，**无 CANN 编译器、无昇腾 NPU**——本报告全部结论均为资料核对、源码审阅、CPU 参考计算与静态分析，**一律未在真实 NPU 编译/精度/性能验证**；凡涉及"可编译/可跑/达标"均为预判，闭环条件见第 14 节。
> 证据等级：A=官方题面/官方 API/官方仓库原文；B=官方样例/源码/测试/官方权威培训；C=社区文章/论坛/个人仓库；D=仅搜索摘要未核验。
> 子代理原始报告：`Agent01~Agent10-*.md`（同目录）；来源总表：`sources.md`（本轮新增 119 条）。

## 1. 调研范围与平台覆盖

| 代理 | 职责 | 覆盖平台 | 输出文件 |
| --- | --- | --- | --- |
| 1 | 规则与提交接口 | CANNJudge 题面/提交/排名/API、GitCode 比赛页、cannjudge-submit skill | Agent01-rules-submission.md |
| 2 | 官方 Ascend C API | hiascend.com/.cn 9.0/8.x、asc.gitcode.com 9.x master | Agent02-ascendc-api.md |
| 3 | 官方昇腾开源仓库 | gitcode.com/cann/ops-nn、ops-transformer、cann-samples、learning-hub（均读源码） | Agent03-ascend-opensource.md |
| 4 | GPU/CUDA/Triton/PyTorch | NVIDIA/PyTorch/vLLM/flash-attention/Triton/llama.cpp/ROCm | Agent04-gpu-migration.md |
| 5 | 编译器/IR/算子生成 | TVM/IREE/MLIR/Triton-Ascend/TileLang/AKG/PyPTO/msopgen | Agent05-compiler-ir.md |
| 6 | 数值精度与验证 | 官方精度文档 + numpy CPU 实验（3 组，见 validation-host-notes.md） | Agent06-numerics.md |
| 7 | 性能/UB/硬件 | hiascend 硬件与 Profiling、asc.gitcode 性能指南、社区实测 | Agent07-ub-performance.md |
| 8 | Linux/环境/真机工程 | Linux DO、V2EX、SO、官方安装/编译文档 | Agent08-linux-env.md |
| 9 | 竞赛经验与失败案例 | GitHub/GitCode/官方论坛/往届算子赛（22 个真实案例） | Agent09-competition-cases.md |
| 10 | 证据审阅/合并/反例 | 复核 1–9 报告，产出矩阵/矛盾/缺口 | Agent10-merge-counterexamples.md |

覆盖完成情况：8 类主题全部产出报告；各代理均给出"输入范围/输出文件/验收标准/禁止范围"；互不重复搜索。遗漏/受限：CANNJudge 提交页与部分 hiascend 页面需登录不可达；GitHub 匿名 API 限流改用 raw/clone；`CANNCommunityEdition/900` 下 API 单页 404，用 9.x master + 8.x 手册推断 9.0 差异（未验证）。

## 2. 题目约束（已确认 / 未确认分开标注）

- **接口（A/B）**：模板为 CANN 9.0 Direct Invocation 直调工程；`run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, …, int64_t availableCoreNum, aclrtStream stream, float epsilon)`，内部 `<<<blocks,nullptr,stream>>>` 启动 `__global__ __vector__` 核函数；`TensorInfo.dtype` 枚举 0=fp32/1=fp16/2=bf16；epsilon 由判题端传入，默认 1e-5；模板 main.asc 与判题接口逐字段一致（Agent01 §1）。
- **15 个测试点 = 官方明文（A）**：题目 API desc 原文"共15个测试点，全部通过才计分"；排名页 15 列 + testcases 数组 15 条三方印证；无隐藏测试点证据（Agent01 §2）。
- **性能是唯一计分维度（A，已实测验证）**：单点得分 `100/(1+log₁.₅(t/T))`，T=该点全局最优时间，总分=15 点均值，同分按提交时间；用排名第 1 实测数据验证 78.79↔78.78（Agent01 §4）。`iterations=5`（每点执行/统计方式），`score_mode=1`。
- **精度表（A）**：fp32 相对/绝对 <1e-4；fp16/bf16 <1e-3（相对+绝对）。本地模板 `verify_result.py` 判题为 `np.isclose(rtol=0.001, atol=0.001, equal_nan=True)` 逐元素 + 失配容忍 0.1%（A/B，Agent06 读源码）；**判题端是否与本地模板一致：推断一致，未证实**。
- **上传格式（A/B）**：`code_template=npu_kernel_dev` 直调工程（beta），提交物至少为含 `run_kernel` 的 kernel.asc；旧 skill 文档的 `tiling_h/tiling_key_h/host_cpp/kernel_cpp` 四字段属旧 msopgen 算子工程格式，不适用；实际表单待登录提交页确认（Agent01 §5、Agent09 §8）。
- **SoC（推断，未证实）**：模板默认 `dav-2201` ↔ Ascend 910B1~B4/910B2C/910_93（cannbot-skills 映射 B 级）；`dav-1001`=老 910/910A 训练系、`dav-2002`=310P 推理、`dav-3002`=200I/500 A2、`dav-3510`=950 系。判题机具体型号未定（Agent01/08/10）。
- **每日 50 次提交额度：无任何公开页面依据**（C/D，用户提供信息，未证实）（Agent01 §8）。
- **Kernel 类型 vector / CANN 9.0.0 / 核心计算必须在 Kernel 内完成**：题面明文（A）。

## 3. Ascend C 官方 API（Agent02，全部 A/B 级，未真机验证）

- **直调模式（A）**：官方「Kernel 直调」；`__global__ __vector__` 为纯向量入口限定符；`<<<numBlocks, dynUBufSize, stream>>>` 第二参数是 UB 大小——模板传 `nullptr` 的语义待真机确认（Agent02 §2）。
- **入口限定符争议（矛盾 4）**：A 级文档称 `__aicore__` 与 `__vector__` 对纯向量算子均合法；C 级官方指南转述称"纯向量类必须 `__vector__`"。当前候选用 `__aicore__`，提交前建议与模板逐字对齐 `__global__ __vector__`（Agent10 矛盾 4）。
- **DataCopyPad（A）**：`DataCopyExtParams{blockCount, blockLen, srcStride, dstStride}` + `DataCopyPadExtParams`；GM 侧 1B 对齐、blockLen 单位字节、padding ≤32B、9.x 新增 NOP/SetPadValue/950 Compact。产品：Atlas A2 训练/800I A2/200-500 A2/A3/950 支持；**老「Atlas 训练系列」（910A/910B 老形态）与推理系列（310P 等）不支持**。9.0 手册标 **ISASI**（跨硬件不保证兼容）。**写方向（UB→GM）语义存在 A 级"精确写回"与 C 级"padding 覆盖相邻行"矛盾（见 §13 矛盾 1）——真机第一验证项**（Agent02 §1.1、Agent09 案例 6.1）。
- **ReduceSum（A）**：count 版 `ReduceSum(dst, src, work, count)`，结果在 `dst[0]`；`workLocal` 空间公式 fp32 `RoundUp(count/64,8)*8`；src 32B 对齐；A2 count 版=「方式二」累加（同 repeat 二叉树、不同 repeat 顺序累加）。**能力上限三处说法冲突：A 文档"受 UB 限制"、B 官方博客实测 ≈4096、B 官方源码注释 <255 repeat≈16320 fp32**——分块 ≤4096 规避（Agent10 矛盾 2）。9.x 参数改名 `sharedTmpBuffer`（含义相同）。
- **Cast（A）**：`CAST_NONE` 用于 fp16/bf16→fp32（精确）；**A2 上 fp32→bf16 无 CAST_NONE，需 RINT/FLOOR/CEIL/ROUND/TRUNC**；`CAST_RINT`=四舍五入（官方规则=RNE，与 numpy 一致，逐位等价未验证）。**Add/Mul 在 A2 不支持 bfloat16_t**——所以必须 Cast 到 fp32 域计算（与首版一致）。
- **Sqrt/Rsqrt（A/B）**：向量 Sqrt 文档保证正确舍入 0 ulp；Rsqrt 为近似（误差随指令；tilelang issue #1225 实证裸 rsqrt 在 rtol=1e-3 下失配）；**标量 sqrtf 精度无文档**——bf16 关键风险（Agent06 §2.4）。
- **GetValue/SetValue（A）**：官方文档原文"仅调试用，性能极低"；V_S/S_V 事件同步（A2 eventID 槽 0-7，FetchEventID 是否耗槽未确认）。
- **TPipe/TQue/TBuf/PipeBarrier/HardEvent/GetBlockIdx/GetBlockNum（A）**：语义与首版用法一致；PipeBarrier(ISASI) 不支持 PIPE_S。
- **9.0 vs 8.x（A）**：9.0 手册 DataCopyPad 标 ISASI；9.0.1 公告仅 aclnn 层拆出 aclnnAddRmsNorm（与 Kernel API 无关）；9.x master 新增 A3/950 型号；blockLen 上界 8.x=[1,2097151]、9.x=[0,]（以安装版头文件为准）。

## 4. 开源实现对比（Agent03，已读源码，未真机验证）

| 仓库 | 关键发现 | 与本题关系 |
| --- | --- | --- |
| ops-nn `add_rms_norm` 系列（commit 9b594837） | **AddRmsNormQuantV2 公式与本题完全一致**：`y_i = x_i·γ_i/Rms(x) + bias`（x=x1+x2 即 x+residual），仅输出多一步 int8 量化 | 最接近官方参考；bias 在 FP32 域 Add（位于 gamma 乘后、Cast 回目标前），与首版 ApplyAffine 位置一致 |
| ops-transformer AddRmsNorm 5 变体（e7019c29） | Tiling 模式：D>12288→**SPLIT_D**（D 分块两遍扫描，尾块余数 ≥16 元素不跨行覆盖）；D≤2000→MERGE_N；blockFactor==1→SINGLE_N；否则 NORMAL；归约链 `Mul 平方→Muls(avgFactor)→Add 折叠→WholeReduceSum→+eps→Sqrt→Div(1)→GetValue(0)` | 官方已验证的多模式 tiling 与归约链；**官方 reduce 前先 Muls(1/N)**，首版是 reduce 后除 N——精度行为可能不同，真机对照 |
| cann-samples `rms_norm_quant_story`（23c981c0） | 0_naive→2_multi_core→5_ub_utilization→6_binary_sum 优化链；`workLocal` 可复用输入 buffer 的 `ReinterpretCast<float>` | 多核/UB/归约优化样板；dav-3510 构建为 950 系专属，不能照搬 |
| cann-learning-hub qwen_ops rmsnorm 训练营 | Direct Invocation 模板 + cmake(-DSOC_VERSION -DASCEND_CANN_PACKAGE_PATH) | CANN 9.0 官方直调样板（partial 获取） |

明确不可迁移：Hccl 通信、量化路径（doScales/RoundFloat2Int8/SetDeqScale）、MoE 路由、VF RegAPI 寄存器版本（正确性风险高，仅作性能冲刺选项）。待真机验证差异：SPLIT_D 尾块 ≥16 元素规则的 910B 行为；half 域 Add vs FP32 域 Add 精度差异；`Muls(avgFactor)` 前置 vs 后置除 N。

## 5. GPU/NPU 迁移分析（Agent04，未真机验证）

已核实 6 个真实实现（均带链接与行号）：PyTorch `RowwiseMomentsCUDAKernel`（Welford+warp shuffle+smem 两级归约）、vLLM `fused_add_rms_norm_kernel`（residual 融合、y 就地写回 residual buffer、vec8）、flash-attention CUDA `ln_fwd_kernel`（编译期特化、save_x、跨 CTA 协作）、flash-attention Triton `_layer_norm_fwd_1pass_kernel`（**单遍存 x 在寄存器**、residual+RMS+bias 融合、mask 尾块）、Triton 官方教程 05-layer-norm、llama.cpp `rms_norm_f32`（最小两遍）。

| GPU 机制 | Ascend C 对应 | 可迁移 | 不可直接迁移 | 风险 |
| --- | --- | --- | --- | --- |
| warp shuffle / block_reduce | 无线程概念，归约走 SIMD ReduceSum/手工向量归约 | 树形归约思想 | 位级求和顺序 | 结果不可逐位移植 |
| atomicAdd / 自旋锁 | MTE 级 SetAtomicAdd（A2 可用性受限）；SIMT asc_atomic_add 仅 950 | 无 | 跨核归约会退化为串行写热点 | 行切分天然避免跨核归约 |
| cooperative groups / grid sync | 无 grid 级同步 | 无 | 同 one 行跨核归约需 GM 原子/二次 kernel | 直接否定 |
| 动态 shared memory | UB 需 tiling 期静态规划 | 无 | kernel 内不能动态分配 | 预算静态化 |
| mask 谓词"零成本不越界" | DataCopyPad 补 0 真实写 UB、占带宽；搬出无谓词 store | 尾块补 0 思路（Triton other=0 同构） | 搬出必须限长 | 尾块越界 |
| float4 / 16B 对齐快速路径 | 对齐粒度 32B/256B | 大块搬运思想 | 16B 对齐假设 | 非对齐行首只能 DataCopyPad |
| Welford 在线统计 | RMS 不需要均值 | 无 | 三状态更新是纯负担 | 直接 Cast→Mul→ReduceSum |
| Triton autotune(num_warps) | 调参对象为 repeat/双缓冲份数/tile | 调参方法论 | num_warps 无对应物 | 真机网格扫描 |

结论：**算法结构与数值策略可迁移（与昇腾官方训练营推荐交叉印证）；执行机制层必须按昇腾搬运/归约/UB 模型重写。任何"GPU kernel 直接翻译即用"的判断均否定**（Agent04 §4/§5）。

## 6. 数值精度方案（Agent06，含 CPU 实验）

**判题语义（A/B）**：golden = 输入各自量化 → FP32 内部计算 → cast 回原 dtype；判定 = 量化输出 vs golden 输出的 isclose（rtol/atol=1e-3，失配 ≤0.1%）。同链计算可逐位一致。

- **fp16 必过**（1 ulp=9.77e-4 < 1e-3 容差，实验 mismatch 恒 0）；**fp32 必过**（1e-4 容差余量 ~100 倍，rsqrt 误差 2^-14 时仍有 2 倍余量）。
- **bf16 是唯一风险**：必须 ① 归约分块/树形（整行顺序累加 D=32768 实测 0.16% > 0.1% 挂；分块 ≤4096 后 <0.01%）② sqrt 正确舍入（裸 rsqrt 误差 2^-14 时 mismatch 0.4~1.2% 全挂；误差 ≤2^-20 才安全；向量 Sqrt 0 ulp 最稳）。bf16 输出精度天花板 ~2^-8≈3.9e-3 由位宽决定，能过 1e-3 是因为判题基准同为 bf16 量化计算。
- **fp16 域累加 y² 必溢出**（|y|≥256 即 inf）——低精度中间方案直接否定。
- **epsilon 位置**：`sqrt(mean(y²)+eps)`（题面公式），实现按 FP32 链。
- **NaN/Inf**：输入 NaN/Inf 沿 rms 污染整行，判题 `equal_nan=True`；禁止 clamp/分支。
- **CAST_RINT = RNE**（A 级规则文档），与 numpy cast 一致；中间 Cast 时机也必须与 golden 同链（S8 案例：全程 fp32 反而对不上 golden 的 bf16 中间结果，Agent09 案例 2.2）。

**可复现测试矩阵**（Agent06 §4，本地/真机统一采用）：

| 维度 | 取值 |
| --- | --- |
| dtype | FP16 / BF16 / FP32 |
| rank | 2D / 3D / 4D |
| D | 1 / 31 / 32 / 33 / 64 / 127 / 128 / 129 / 1024 / 4096 / 32768 |
| outer | 小规模 / 多行 / 大规模 |
| 补充边界 | 全 0、NaN、±Inf、|y|≥256（fp16 溢出）、D 非 32 倍数、epsilon 1e-5/1e-6 |

**建议 Kernel 计算链**：`Cast(CAST_NONE)→Add(fp32)→Mul 平方→分块 ReduceSum(≤4096)→块间标量合并→Muls(1/D)→Add(eps)→Sqrt(向量)或正确舍入 rsqrt→Div 或 Muls(1/rms)→Mul(gamma fp32)→Add(bias fp32)→Cast(CAST_RINT)`。

## 7. UB 分块与性能方案（Agent07，未真机验证）

- **访存密集型**：算术强度 ~0.5 flop/B，瓶颈在 MTE2 搬运而非计算；优化主轴=减搬运量、大块搬运、双缓冲、避免非对齐。
- **UB 容量**：主口径 **192KB/核**（A2/A3 官方样例 + 多来源一致）；另有 1.5MB/2MB/256KB 矛盾说法已排除为主口径；核数社区 20~48 矛盾，代码用 `GetBlockNum()` 自适应。HBM 带宽 1.6TB/s（A 级彩页）。
- **ReduceSum 成本（A）**：归约延迟 ≈ Add 的 2~5 倍；二分累加 172 cycles < ReduceRepeat 242 cycles；ReduceSum 接口为软仿通常最慢。**GetValue/SetValue 标量同步是性能黑洞**——需行末合并取数/减少每 tile 同步。
- **DataCopyPad 非对齐有实价**：A2 实测仅尾块不齐即劣化 -21.6%（GM→UB）、-47.5%（GM→L1）；建议 512B 对齐、尾块尽量一次搬。
- **tiling 推荐（3 形态）**：

| 形态 | 推荐 tiling |
| --- | --- |
| 小 outer 大 D（[1,32768]） | 单核主跑 + tile D=8192 分块 + 双缓冲；ReduceSum 分块 ≤4096；跨块标量合并每行一次 |
| 大 outer 小 D（[8192,64]） | 按行均分多核；行间合并归约（多行拼一个 GetValue 批次），把 GetValue 次数从 8192 降到 ~256 次 |
| 中等（[512,1024]） | 全形态基准；D≤8192 时 gamma/bias 整体驻留 UB（half 32KB）；UB 余量充足 |

- **优化阶梯**：v1 两遍正确性优先（当前首版）→ v2 gamma/bias 驻留/预载（R8）→ v3 削减标量同步（行末一次 GetValue + 向量化 rsqrt，R3）→ v4 双缓冲 BUFFER_NUM=2（R2，C 级训练营数据收益 -50~60%，未本算子实测）。测量走 msKPP→msOpST→msprof op→msopprof（§7.5），看 PipeUtilization.csv 的 MTE2 vs Scalar 占比决定优先级。

## 8. 编译与真机环境方案（Agent08，未本机验证）

- **模板是 CANN ≥8.3（推荐 9.0.0）的新式 ASC 语言直调工程**：`find_package(ASC)` + `--npu-arch=dav-2201` + bisheng；**8.2 及以下不支持 `--npu-arch`**，不要用老 ccec/`-c ai_core-<soc>`/8.x kernel_operator.h 教程逐字套用（Agent08 §5、Agent10 §1.3）。
- **CANN 9.0.0 ↔ Ascend HDK 26.0.RC1/25.5.2/25.5.1 配套表**（gitcode.com/cann/release-management）；安装顺序 toolkit→910b-ops，首次驱动>固件、覆盖固件>驱动。
- **环境变量**：ASCEND_HOME_PATH（set_env.sh）、ASCEND_CANN_PACKAGE_PATH、ASCEND_OPP_PATH、DDK_PATH、NPU_HOST_LIB、LD_LIBRARY_PATH（作用表见 Agent08 §6）。
- **真机第一步**：`npu-smi info`（驱动/固件）+ `source set_env.sh`；模板 run.sh 已内置两层检查。
- **从零到跑通 run.sh 的 11 步清单**见 Agent08 §7（含容器化替代路径、`-DNPU_ARCH` 覆盖、HwHiAiUser 权限、/dev/davinci* 挂载）。
- **故障排查表**（12 行）见 Agent08 §8：507015/507035 超时、UB 越界 errorStr、地址未对齐、SoC 错配、多核死锁、libascendcl 缺失等。

## 9. 竞赛失败案例（Agent09，22 个真实案例）

对本题最重要的三条（均带链接，见 Agent09 报告与 sources.md 09-x）：

1. **Compile Error 头号来源 = 本地与判题环境版本差**：as_strided 实战因 CANNJudge 用 8.5、本地 9.0 命名空间差异首提交即 CE；本地编译通过≠判题通过。直调入口应为 `__global__ __vector__` + `.asc` 后缀。
2. **DataCopyPad 写方向 burst 32B 对齐**：有效数据不足 32B 时 padding 覆盖相邻数据——D 非 32 倍数且行间连续布局下，尾块写出会踩下一行开头（最隐蔽尾块炸弹；与官方 A 级文档"精确写回"冲突，真机第一验证项）。
3. **ReduceSum(count) 实际上限 4096**（XJTUACM S7 实测，官方博客 B 级）；BF16 输出必须 `CAST_RINT` 还原参考舍入；"中间更高精度"反而 WA（S8 案例）。

其余覆盖：RE/崩溃（UB 越界、WaitFlag 分支遗漏、栈上大数组）、TLE（GetValue 黑洞、死循环折叠点）、精度不稳定（FP16 溢出、归约顺序）、上传包错误（旧 4 字段 vs 直调）、隐藏测试点（平台不开放用例 API）、额度管理（无 CANNJudge 明文）。完整"失败→根因→排查"表与预防清单见 Agent09 §二/§三。

## 10. 方案矩阵（Agent10 汇总，14 条路线）

> 列：方案/算法 | 访存次数（每元素字节，ts=输入类型字节） | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别。所有评分 = 静态分析 + 反例证据，未真机验证。

| # | 方案/算法 | 访存 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 两遍扫描（Pass1 平方和归约 → Pass2 归一化+γ+b 输出） | 读 2×(x+res)+写 out = 5·ts | 一行/tile 常驻，任意 D 可跑 | 低（FP32+分块 ≤4096 安全） | 中（搬出依赖 DataCopyPad 写语义） | 中（memory-bound，访存翻倍） | 低 | 中（依赖 DataCopyPad，A2 系 OK） | **首选**（正确性基线，与官方 NORMAL/SPLIT_D 同构） |
| 2 | 单遍保存中间结果（y 驻留 UB） | 读 2·ts+写 out = 3·ts | y(fp32)=D·4B，D≤4096(fp32)/8192(half) 才现实 | 低 | 同路线 1 | 高（小 D 省一次 GM 读） | 中 | 同路线 1；D 大退化两遍 | **可作为第二路线**（仅小 D 形态） |
| 3 | FP32 全中间计算（cast→归约→cast 回） | 同载体 | 需额外 fp32 tile | 无（黄金法则）；唯一风险=与 golden Cast 时机不同步 | 无 | 中 | 低 | 低 | **首选**（强制前提） |
| 4 | 低精度中间计算（对照） | 同载体 | 省 fp32 tile | **高：fp16 累加必溢出；bf16 域加法 ~25% 元素翻转** | 无 | 高（省 Cast） | 低 | 中 | **不建议** |
| 5 | 大 tile（D≤8192 一行一块） | 同 1 或 2 | fp32 D=8192 ≈224KB 超 192KB；half ≈112KB | 中（count=8192 撞上限争议） | 中（大块尾块集中） | 高（大搬运高效区） | 中 | 中（需先解 count 上限） | **可作为第二路线**（性能向，half/bf16 且真机确认） |
| 6 | 小 tile（1024 元素块循环） | 同路线 1 | 最低 | 最低（chunk≤1024 实测 0.002~0.010%） | 低 | 中（4KB 在高效区边缘；块多→GetValue 多） | 低 | 低 | **可作为第二路线**（精度最稳、尾块最简单） |
| 7 | 以行分配 AI Core（outer 均分） | 无附加 | 无附加 | 无 | 无 | 高（多核并行） | 低 | 低 | **首选**（官方 blockFactor 同构，首版已实现） |
| 8 | 以 tile 分配 AI Core（跨核归约） | 无附加 | 无附加 | 中（跨核合并顺序） | 无 | 理论高，实际被跨核通信吃掉 | 高 | 高（无 grid sync，GM 原子/二次 kernel） | **不建议** |
| 9 | DataCopyPad 尾块（搬入补 0 + 搬出真实字节） | 无附加 | 补 0 ≤32B/tile | 低（0² 不污染归约） | **高：写方向 padding 覆盖相邻行（矛盾 1）**；非对齐代价 -21.6%/-47.5% | 中 | 低 | 中（判题机须 A2 系） | **首选（条件性）**：真机验证写方向通过后；否则降级路线 10 |
| 10 | 手工尾块（GetValue/SetValue 重排 / Duplicate / GatherMask） | 无附加 | 无附加 | 低 | 低（显式控制） | 低-中（标量黑洞；官方仅尾部 ≤32B 用） | 中-高 | 低（910 系官方 DataCopyCustom 在用） | **可作为第二路线**（DataCopyPad 不可用时的官方降级） |
| 11 | ReduceSum 方案（count 版 + workLocal） | 无附加 | workLocal 按公式 | 低-中（上限争议；A2 方式二 vs golden pairwise 逐位差异未验证） | 无 | 中（软仿最慢） | 低 | 低-中（分块 ≤4096 规避） | **首选**（分块 ≤4096 + 块间标量合并，首版已实现） |
| 12 | 手工向量归约（Add 折叠 + WholeReduceSum / 二分累加） | 无附加 | 可复用输入 tensor | 低（全二叉树，更接近 golden） | 无 | 高（二分 172 vs ReduceSum 242 cycles） | 中-高（折叠点/别名/UpdateMask 坑） | 低 | **可作为第二路线**（性能优化候选） |
| 13 | 纯 Ascend C 实现（直调模板手工 kernel） | — | — | — | — | — | — | — | **首选**（强制：唯一可提交形态） |
| 14 | 从 CUDA/Triton/PyTorch 迁移 | 算法结构可迁移，执行机制不可照搬 | — | 位级不可移植 | GPU mask 假设不成立 | 参考价值高 | 高 | 高（直接翻译必挂） | **仅作研究参考** |

## 11. 推荐实现路线

**首版推荐（正确性基线）= 路线 1+7+9+11+13 组合**：两遍扫描 + 按行均分 AI Core + DataCopyPad 搬入补 0/搬出 + ReduceSum 分块 ≤4096 + FP32 全中间 + CAST_RINT 输出 + 纯直调手工 kernel。这正是当前候选 `提交/V001/kernel.asc` 的形态（Agent10 §7.1 与官方 NORMAL/SPLIT_D 结构一致）。

**提交前仅两处改动（Agent10 §8）**：
1. 入口 `extern "C" __global__ __aicore__` → `extern "C" __global__ __vector__`，与模板逐字对齐（R1）；
2. half tile 4096 → 2048，或真机确认 ReduceSum count 上限后再定（R4）。

## 12. 第二候选路线

**小 D 形态单遍存 y（路线 2+6）**：D ≤ 4096(fp32)/8192(half) 时 y=x+residual 留 UB，ReduceSum(y²) 后直接归一化输出，访存 5ts→3ts；大 D 保持两遍。依赖：判题机 A2 系 + DataCopyPad 写方向验证通过。**性能优化候选**：手工向量归约（路线 12，二分累加）+ BUFFER_NUM=2 双缓冲 + gamma/bias 驻留/预载 + 行末一次 GetValue/向量化 Sqrt（Agent10 §8）。

## 13. 风险清单（含矛盾）

| # | 风险 | 级别 | 对策/状态 |
| --- | --- | --- | --- |
| 矛盾 1 | DataCopyPad UB→GM 写方向：A 级"精确写回" vs C 级"padding 覆盖相邻行" | 高（生死线） | 真机第一验证项；备选路线 10 DataCopyCustom 尾块重排 |
| 矛盾 2 | ReduceSum(count) 上限：4096 / 16320 / UB 限制三处冲突 | 高 | 分块 ≤4096 规避；half tile 降 2048 |
| 矛盾 3 | "910B 不支持 DataCopyPad" vs "A2 系支持"：产品系列命名混淆 | 高 | 判题机型号确认前保留路线 10 降级 |
| 矛盾 4 | 入口 `__aicore__` vs `__global__ __vector__` | 中 | 提交前按模板逐字对齐 |
| 矛盾 5 | UB 容量/核数/带宽社区数据矛盾（192KB vs 1.5MB；20~48 核） | 中 | 用 GetBlockNum()/真机 npu-smi 定案 |
| R2 | BUFFER_NUM=1 无双缓冲 | 中 | 正确性阶段可接受；v4 优化（收益未本算子实测） |
| R3 | 每 tile 一次 GetValue + V_S/S_V 同步 | 中-高 | v3 行末一次标量读/向量化 rsqrt |
| R5 | D 非 32 倍数尾块写出 burst | 高 | 同矛盾 1 |
| R6 | 标量 sqrtf 精度（bf16） | 高 | 改向量 Sqrt（0 ulp）或正确舍入 rsqrt |
| R7 | reduce 后除 N vs 官方 Muls(avgFactor) 前置 | 低-中 | 真机对照 |
| R8 | gamma/bias 每行每 tile 重搬 | 中 | v2 驻留/预载 |
| 其他 | 版本差 CE、FetchEventID 槽耗尽、A2 无 bf16 Add/Mul、NaN/Inf 传播 | 中 | 见 §3/§6/§9 |

## 14. 未确认事项（证据缺口，必须真机验证闭环）

1. **判题机 SoC 型号**（决定 DataCopyPad/ReduceSum/Cast 支持矩阵、核数、UB 预算、`--npu-arch` 取值）——全部方案的分叉点。
2. **DataCopyPad UB→GM 写方向是否精确字节**（D 非 32 倍数尾块方案的生死线）。
3. **ReduceSum(count) 实际能力上限 + A2「方式二」累加在 bf16 D=32768 的实测 mismatch**（决定 tile 大小与分块策略）。
4. **标量 sqrtf / 向量 Sqrt 实际精度**（bf16 需 rsqrt 误差 ≤2^-20 或 Sqrt 0 ulp）。
5. **CAST_RINT 与 numpy RNE 的逐位等价** + 中间 Cast 时机（S8 案例）。
6. **FetchEventID 是否耗用事件槽**（A2 槽 0-7，循环内反复调用是否溢出）。
7. **判题端精度判定实现细节**（是否与本地模板 verify_result.py 一致）。
8. **15 个测试点各自的 shape/dtype/epsilon 配置**（平台不开放，只能泛化覆盖）。
9. **上传表单实际字段与提交方式**（直调 kernel.asc 单文件 vs 旧 4 字段）。
10. **双缓冲/大 tile 真机收益与 UB 预算确认**（-60% 为训练营 C 级数据）。
11. **核数 GetBlockNum 运行时值**（社区 20~48 矛盾）。
12. **9.0.0 安装版头文件中 DataCopyPad/ReduceSum/Cast 签名与 9.x master 差异**（blockLen 上界、sharedTmpBuffer 改名、ISASI、dynUBufSize=nullptr）。

## 15. 下一阶段实验计划

1. **CPU 同链验证（无 NPU 可做）**：按 §6 测试矩阵，实现与 golden 同链的逐位对照（含 CAST_RINT 模拟、分块归约、正确舍入 sqrt），验证候选 kernel 算法语义与 bf16 分块策略（Agent06 脚本 /tmp/agent06_numerics/ 可复现）。
2. **真机环境准备**：Linux 昇腾服务器 → 安装 CANN 9.0.0 + 配套 HDK（§8）→ `npu-smi info` 确认 SoC/核数 → 跑通模板 run.sh 默认 FP16 [1,64]。
3. **第一验证批（按 §14 顺序）**：① SoC 型号与 DataCopyPad 支持；② 尾块搬出语义（D=67/129/1000 写回后逐字节核对相邻行）；③ ReduceSum count 上限（2048/4096/8192 扫描）；④ 向量 Sqrt vs 标量 sqrtf vs rsqrt 的 bf16 精度；⑤ CAST_RINT 舍入。
4. **精度矩阵**：真机按 §6 矩阵跑 15 点泛化覆盖，逐点记录相对/绝对误差与判定（提交/result-<date>.md）。
5. **性能优化**：先 v1 正确性 → msOpST 计时 → 按 §7 阶梯实施 v2（驻留）/v3（减同步）/v4（双缓冲），每步记录耗时与误差，防止优化损伤精度。
6. **上传**：所有验证完成、用户明确确认后，按 §2 上传格式提交；记录到提交/历史-<date>.md。

## 16. 本轮结论与边界声明

- **完成的调研**：规则/接口、官方 API、官方开源源码、GPU 迁移、编译器/自动生成、数值精度（含 CPU 实验）、UB/性能、真机环境、失败案例、证据审阅与方案矩阵——共 10 份子报告 + 本汇总。
- **有源码或官方 API 证据的方案**：路线 1/2/3/5/6/7/9/10/11/12/13（A/B 级证据支撑）；官方 ops-nn AddRmsNormQuantV2 为最接近参考。
- **仅为迁移参考的方案**：路线 14（GPU 迁移，仅算法结构）；自动生成器（TileLang-Ascend/Triton-Ascend 等）产物形态与直调模板不匹配，不能作为提交路径。
- **必须真机验证的结论**：§14 全部 12 项；本报告任何"可编译/精度达标/性能达标"均未发生。
- **本轮未执行**：CANNJudge 上传、任何提交、任何 NPU 编译/精度/性能声明。凭据零写入。
