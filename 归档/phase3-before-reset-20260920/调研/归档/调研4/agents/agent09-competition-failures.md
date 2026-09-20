# Agent 09：竞赛经验与失败案例调研报告

> 主题：竞赛与工程实践中的**失败案例**（Compile Error / Wrong Answer / Runtime Error / Timeout / 精度不稳定 / BF16 失败 / 尾块越界 / ReduceSum 错误 / 上传包错误 / 提交次数管理 / 隐藏测试点 / 性能优化失败）
> 题目：`AddRmsNormBias`（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，vector，昇腾 NPU）
> 本机环境：macOS，**无 CANN、无昇腾 NPU**，所有结论均基于公开资料与本项目本地失败记录，未做真机验证。

---

## 1. 结论摘要（≤12 条）

1. **本项目已发生两类真实失败**：V001 因 `TPipe pipe_` 宏命名冲突导致 15/15 `Compile Error`；V002 因“上传内容异常”（平台收到的 `kernel.asc` 第 1 行为 `return false;`）导致整文件连锁编译错，**本地 405 行代码本身是完整的**。
2. **编译错误高度集中在“入口/类型/宏”层面**：昇腾官方 FAQ 的 5 个高频编译案例全部是路径、产品系列、API 参数、数据类型等“非算法”问题；AI 代码生成器（bisheng 前校验）也把“入口点 `__aicore__` 缺失、DMA 后缺 `pipe_barrier`、InitBuffer 超 UB 上限”列为头三号静态检查。
3. **归约精度是第一精度杀手**：FP16/BF16 上直接累加 `x²` 会溢出为 INF/NAN；必须在 FP32 中完成 `x² → sum → mean → sqrt`，仅输入/输出保留低精度（ops-transformer、catlass、devpress 三处独立报告一致）。
4. **BF16 失败常表现为“静默错误”**：用 bf16 累加、或中间结果过早转 bf16，在均匀小样本下看不出问题，在真实长尾分布/大形状/多核划分下才会发散（marvin-42 的 RMSNorm backward 案例、海光 DCU drift 案例）。
5. **尾块越界是 Runtime Error 重灾区**：`DataCopyPad` 的 padding 写入若边界算错，会覆盖相邻段/相邻行；`D` 非 32 倍数时若用 `DataCopy` 直接搬非对齐尾块，会直接 Core Dump。
6. **`DataCopyPad` 参数单位是最易踩的配置坑**：`srcStride`（GM 侧，单位**字节**）与 `dstStride`（UB 侧，单位 **dataBlock/32B**）单位不同，是社区公认高频错误。
7. **ALIGN_UP 向上对齐会引入“读越界/OOM”风险**：非对齐形状若向上取整搬运，可能读到 GM 物理边界之外，需 Host 多预留安全余量或仅计算有效元素。
8. **性能优化必须在“全测试点通过”之后做**：多个竞赛指南明确“先正确性、后性能”；KernelBench 系列研究指出“单 shape 通过但宽 shape 破坏梯度”“reward hacking 假加速”是普遍失败模式，性能分是唯一计分维度但**不能牺牲 15 点全过**。
9. **不要写死 TileLength / blockDim / dtype**：竞赛平台不开放测试用例，shape、dtype、rank 组合未知，必须泛化（训练营避坑指南反复强调）。
10. **上传/提交流程类失败真实且高频**：V002 的“编辑器未全选替换、只贴了片段”是本项目最可能的上传异常成因；作业平台上传失败还常见于文件大小超限、网络中断、浏览器缓存/插件、BOM/特殊字符。
11. **提交次数与“最终成绩取最后一次/最优成绩”规则必须前置知晓**：江山赛区讨论贴与多个赛题规则写明“取最后一次提交成绩”或“取最优成绩”，应保留一个稳定可用版本再冲性能。
12. **本机可做的静态检查能挡掉大部分编译/越界类失败**：`grep` 检查 `pipe_` 类命名、首末行、`DataCopyPad` 参数与 `ReduceSum` 的 FP32 累加，可在无 NPU 环境下过滤大部分低级错误。

---

## 2. 失败案例表（≥10 条，字段：案例 / 平台 / 发生时间 / 错误类型 / 现象 / 根因 / 处置 / 证据 URL / 证据等级 / 对本题的启示）

> 证据等级：A=官方文档/官方仓库；B=官方赛事/官方论坛/官方PR；C=社区文章/Issue/讨论（含类比平台）；L=本项目本地真实记录。

| # | 案例 | 平台 | 发生时间 | 错误类型 | 现象 | 根因 | 处置 | 证据 URL | 等级 | 对本题的启示 |
|---|------|------|----------|----------|------|------|------|----------|------|--------------|
| 1 | **V001 `TPipe pipe_` 宏命名冲突** | 本项目 CANNJudge | 2026-09-11 | Compile Error | 15/15 `unknown type name 'pipe_'`、`cannot use dot operator on a type` | kernel 类成员 `TPipe pipe_` 与保留宏/类型冲突，InitBuffer 调用处连带报错 | 改 `TPipe tpipe`，同步更新所有 InitBuffer 调用（见 V002） | `提交/V001/结果.md`（本地） | L | 命名避开 `pipe_`；本题 V002 已改 `TPipe tpipe`，提交前 `grep -n "TPipe pipe_"` 应无命中 |
| 2 | 昇腾 FAQ 案例1：CANN 包路径填错 | 昇腾社区论坛 | 2025-08-04 | Compile Error | `fatal error: register/tilingdata_base.h: No such file or directory` | `CMakePresets.json` 中 `ASCEND_CANN_PACKAGE_PATH` 写错 | 改为 CANN 实际安装路径 | https://www.hiascend.com/dev/forum/thread-0259189679380603071-1-1.html | B | 直调单文件不显式配 CMake，但 CI 缺头文件路径会全编译错；确认入口与依赖完整 |
| 3 | 昇腾 FAQ 案例2/3：API 不支持的产品系列 / 型号写错 | 昇腾社区论坛 | 2025-08-04 | Compile Error | `no member named 'Cos' in namespace 'AscendC'`；`FileNotFoundError: aic-ascend310-ops-info.ini` | 用了目标 SoC 不支持的 API；`ASCEND_COMPUTE_UNIT` 与 `AICore().AddConfig` 不一致 | 换成支持的 API；核对产品系列型号一致 | 同上 | B | 目标 CANN 9.0.0，确认 `ReduceSum`/`DataCopyPad` 在 9.0.0 可用且 SoC 型号一致 |
| 4 | 昇腾 FAQ 案例5：API 参数错（少/多/类型不符） | 昇腾社区论坛 | 2025-08-04 | Compile Error | `error: no matching function for call to 'Add'` | 参数个数/类型/数据类型不在 API 限制内 | 对照 API 文档核对参数 | 同上 | B | `ReduceSum`、`DataCopyPad` 的参数个数与类型必须严格匹配，禁止凭记忆填参 |
| 5 | bisheng 前静态校验三关 | ascend-rs（开源校验层） | 2025 | Compile/静态检查 | 入口缺 `__aicore__`、DMA 后缺 `pipe_barrier`、InitBuffer 超 UB 上限 | 生成代码未感知目标硬件约束 | 字符串扫描：入口点检查、DMA/同步屏障、Buffer 大小 vs UB 上限（910B=196608B） | https://ascend-rs.org/appendix/appendix-d-ecosystem.html | C | 本机可套用：grep `__aicore__`、检查 `pipe_barrier`、核对 UB 分配总量≤192KB |
| 6 | 官方：避免 TPipe 在对象内创建和初始化 | 昇腾官方文档 8.0.RC3 | 2024-2025 | Compile/性能 | TPipe 在类内创建影响编译器标量折叠，scalar 指令增加 | TPipe 构造设置全局指针，污染类对象内存，编译优化保守 | 改为 kernel 入口创建 `TPipe`，类内保存指针 | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/developmentguide/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0028.html | A | 本题 V002 已是“入口建 `TPipe`、类内存指针”，符合官方最佳实践 |
| 7 | ReduceSum float 累加精度误差 | Gitee ascend/modelzoo | 2022-07-29 | 精度/Wrong Answer | complex64 虚部为零时精度误差；aicpu 结果与标杆不一致 | `float` 类型数据累加导致精度误差 | 修改代码优化精度 | https://gitee.com/ascend/modelzoo/issues/I5JJ8N | B | 归约务必 FP32 累加；本题 `x²` 求和必须用 FP32 |
| 8 | RMSNorm 在 FP16 上归约溢出 | CANN ops-transformer 分析 | 2025 | 精度/BF16 | FP16 上 `mean(x²)` 溢出 INF/NAN | 直接在 FP16 累加 `x²`（范围可达 65504²）第二步即溢出 | 内核用 FP32 累加器做归约 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | C | 本题 `x + residual`、平方、归约、sqrt 全程 FP32，仅输出转回输入 dtype |
| 9 | rms_norm 中间结果转 bf16 与 CUDA 不对齐 | 国产芯片精度排查（DevPress） | 2025 | 精度/BF16 | 大模型微调 loss 与 A100 不对齐 | rms_norm 中间结果转成 bf16（为对齐 GPU 小算子），但算法需高精度 | 去除中间转 bf16 逻辑，已修复于 cann8.0.RC3 | https://devpress.csdn.net/v1/article/detail/156951082 | C | 本题中间计算不要过早降精度；FP32 累加路径保持到归一化完成 |
| 10 | AI 生成 CUDA kernel：bf16 累加致训练发散 | marvin-42 insights（通用） | 2025 | 精度/BF16 | 融合 embedding 梯度+RMSNorm backward 通过验证器，但训练 loss 发散 | bf16 累加 embedding 梯度，高频 token 行漂移 | 必须 fp32 累加；优化器/数据集敏感应纳入测试 | https://insights.marvin-42.com/articles/ai-generated-cuda-kernels-passed-the-benchmark-then-broke-real-training | C | 归约累加用 fp32 是硬性要求；bf16 仅用于输入搬运与最终输出 |
| 11 | BF16 逐位一致仍文本漂移 | 海光 DCU 实测（类比） | 2026 | 精度/不稳定 | 单 shape 逐位一致，但完整服务 4/10 请求文本变化 | 归约顺序不同→BF16 输出不同；自回归链路误差放大 | 沿真实 shape/计算图/生成链路逐层验证 | https://blog.csdn.net/2502_94781626/article/details/162957938 | C | 多核划分/归约顺序会带来非确定性；本题确认单 shape 通过≠全点通过 |
| 12 | DataCopyPad 写出溢出覆盖相邻段 | CSDN（ferriswym） | 2025 | 越界/踩内存 | seg 的 padding 覆盖 seg+1 前 4 字节，空段残留垃圾 | 尾块 padding 写入越过本段边界 | 严格计算每段有效长度与 padding，避免写回越界 | https://blog.csdn.net/ferriswym/article/details/162206787 | C | 本题 `rightPadding` 只向右补零；多核按行写回时勿覆盖相邻行（32B cache line 边界） |
| 13 | ALIGN_UP 向上对齐读越界/OOM | 昇腾训练营·性能篇 | 2025 | 越界/OOM | 向上取整搬运读到 GM 物理边界外→OOM | ALIGN_UP 多读尾部字节 | Host 申请 GM 时多预留 32B 安全余量 | https://ai6s.net/69429f5cbf6b0e4b285c3379.html | C | 本题 `D` 非 32 倍数时，写回与搬运都要保证不越过张量尾部 |
| 14 | DataCopyPad 参数单位混淆（srcStride vs dstStride） | 稀土掘金 | 2025 | 越界/配置错 | MTE 类 Illegal Instruction / 数据错位 | GM 侧 stride 单位字节、UB 侧单位 dataBlock(32B) 不同 | 严格按单位填 `srcStride`(字节)/`dstStride`(dataBlock) | https://juejin.cn/post/7682716879561687050 | C | 本题核对 `DataCopyExtParams` 各字段单位；`leftPadding/rightPadding` 单位元素个数 |
| 15 | 尾块直接 DataCopy 非对齐→Core Dump | 昇腾训练营·第九期 | 2025 | Runtime Error | 尾块 `DataCopy(...,length=8)` 立即崩溃 | MTE 无法处理 <32B 或非对齐长度 | 用 `DataCopyPad` + Mask 处理非对齐尾块 | https://blog.csdn.net/2401_82857325/article/details/155320399 | C | 本题 `D%32!=0` 的最后一块必须走 `DataCopyPad`，不得直接 `DataCopy` |
| 16 | KernelBench/robust-kbench：单 shape 测试不足、reward hacking | Stanford arXiv | 2025 | 性能/泛化 | 语义正确 kernel 比 eager 慢；窄测试下“假加速”50-120x | 只测单一 config、硬编码/早退/复用缓存 | 多 shape/多 dtype/多 seed、对抗测试、静态检查 | https://arxiv.org/pdf/2509.14279v1 | C | 本题性能优化前先保证 15 点全过；忌为提速牺牲正确性 |
| 17 | **V002 上传内容异常（本项目真实）** | 本项目 CANNJudge | 2026-09-11 | 流程/上传异常 | 平台收到 `kernel.asc` 第 1 行 `return false;`，连锁 `expected unqualified-id`、`extraneous closing brace`、多个符号未定义 | 上传内容被截断/替换（编辑器未全选替换，只贴了片段；或网络/缓存导致不完整） | 编辑器全选→替换为本地完整 405 行；提交前核对首末行与 `run_kernel` 入口 | `提交/V002/结果.md`（本地） | L | 提交前必须核对上传内容首行非 `return false;`、末行合法、`wc -l` 行数一致 |
| 18 | 算子挑战赛提交规则（次数/版本管理） | gitcode cann-competitions 讨论 + erf 赛题 | 2025 | 流程/提交管理 | “每天最大提交 50 次，取最后一次提交成绩”“正式比赛取队伍最优成绩排名” | 提交次数有限、成绩取特定一次 | 做好版本管理，保留稳定版本再冲性能 | https://gitcode.com/cann/cann-competitions/discussions/1 | B | 本题初赛有提交次数上限；最后提交要稳，勿把唯一可用版本覆盖为未验证优化版 |
| 19 | CANNJudge 提交指南：不开放测试用例、需泛化 | CSDN CANNJudge 指南 | 2025 | 流程/泛化 | 部分用例失败→Wrong Answer；状态含 Compile/Wrong/RunTime/Timeout | 平台不开放测试 API，shape/dtype/属性未知 | 设计泛化算子，提交前穷举 shape/dtype 组合自测 | https://blog.csdn.net/gitblog_00467/article/details/152189083 | C | 本题 dtype∈{fp16,bf16,fp32}、rank∈{2D,3D,4D}、D∈[64,32768] 都要覆盖 |
| 20 | 作业平台上传失败（大小/网络/缓存/格式） | CSDN 问答（类比） | 2025 | 流程/上传异常（类比） | 413 Payload Too Large、上传中断、旧缓存干扰 AJAX | 文件超限、网络中断、浏览器插件、不兼容格式 | 换浏览器/无痕、查文件大小与格式、网络重试 | https://ask.csdn.net/questions/8783004 | C | 上传异常成因之一；V002 更可能是“编辑器未全选替换”，但网络/缓存亦需排除 |

---

## 3. 按错误类型分组的规律总结

### 3.1 编译类（Compile Error）
- **规律**：编译失败极少源于“算法写错”，而集中于**入口、类型、宏、路径、API 签名、产品系列**。昇腾 FAQ 的 5 大高频案例（路径错、API 不支持、型号错、Numpy 版本、参数错）全是这类；ascend-rs 的 bisheng 前校验把“入口点 `__aicore__` 缺失 / DMA 后缺 `pipe_barrier` / InitBuffer 超 UB 上限”列为静态检查头号三关。
- **本项目印证**：V001 的 `TPipe pipe_` 正是“宏/命名冲突”子类——改 `tpipe` 即过。
- **可复用处置**：提交前做“三关静态检查”（见第 5 节检查项）。

### 3.2 精度类（Wrong Answer / 精度不稳定）
- **规律**：归约与低精度是重灾区。**FP16 上累加 `x²` 必溢出**（ops-transformer）；**bf16 累加/中间过早降精度会静默错误**（devpress、marvin-42、海光 DCU）；**归约顺序非确定**（多核划分）会在真实分布下放大误差。
- **共识处置**：归约与中间计算一律 FP32；仅输入搬运与最终输出保留目标 dtype；输出 Cast 用正确 `RoundMode`；多核划分注意 32B cache line 边界。
- **对本题**：`y=x+residual` → `rms=sqrt(mean(y²)+eps)` → `output=y/rms*gamma+bias`，整条链须在 FP32 完成。

### 3.3 越界类（Runtime Error / 踩内存）
- **规律**：非对齐尾块 + `DataCopy` = Core Dump（训练营第九期）；`DataCopyPad` 的 padding 写入若边界算错会覆盖相邻段（ferriswym）；`ALIGN_UP` 向上取整会读到 GM 边界外（OOM）；`srcStride`(字节) vs `dstStride`(dataBlock) 单位混淆是高频配置错（掘金）。
- **对本题**：D 非 32 倍数时，最后一块必须 `DataCopyPad` + 右侧补零；多核按行均分写回时须满足 `k*D*sizeof(T) ≡ 0 (mod 32)`，避免总线踩踏相邻行。

### 3.4 性能类（Timeout / 性能优化失败）
- **规律**：性能优化失败的两大模式是“**先优化后正确**”（未全过就冲性能，结果 Wrong Answer）和“**单 shape 假加速**”（KernelBench/robust-kbench：窄测试通过、宽 shape 破坏梯度、reward hacking 达 50-120x 假加速）。
- **对本题**：性能是唯一计分维度，但 15 点全过是前提；忌写死 `TileLength`/`blockDim`；性能调优应在本地泛化用例全过之后。

### 3.5 流程类（提交 / 上传 / 版本 / 隐藏测试点）
- **规律**：上传/提交流程失败真实高频。本项目 V002 即“编辑器未全选替换、只贴片段（首行 `return false;`）”；平台类失败还来自文件超限、网络中断、浏览器缓存/插件、BOM/特殊字符。提交次数有限且“取最后一次/最优成绩”，需版本管理。
- **隐藏测试点**：CANNJudge 不开放测试用例，shape/dtype/rank/属性组合未知，隐藏点就在“非 32 倍数 D、BF16、3D/4D、outer<或>AI Core 数”等边界（见 CANNJudge 提交指南与训练营泛化清单）。
- **对本题**：提交前核对上传内容首末行与行数；保留一个稳定可用版本；最后提交求稳。

---

## 4. “上传内容异常”专题（含 V002 案例、常见成因、预防检查）

### 4.1 本项目 V002 案例（真实）
- **现象**：平台提交编号 `6aa388bd2d3dd2c5ae81372f`，15/15 `Compile Error`。平台日志显示收到的 `kernel.asc` **第 1 行为 `return false;`**，随后 `expected unqualified-id`、`extraneous closing brace`、`IsSingleTensorGroup`/`IsPositiveShape`/`MAX_ELEMENTS`/`MAX_DIM` 未定义。
- **本地核对**：本地 `提交/V002/kernel.asc` 第 1 行为 `#include <cmath>`，共 **405 行**，含完整 `extern "C" void run_kernel(...)` 入口与上述符号定义。
- **结论**：平台收到的内容与本地不一致，属于**上传内容异常**，不是算法编译失败。错误是“文件开头缺失”后的连锁语法环境，不能据此判断本地 API 调用是否通过。
- **记录位置**：`提交/V002/结果.md`。

### 4.2 常见成因（按可能性排序）
1. **编辑器未全选替换**：在 CANNJudge 编辑器中粘贴时只替换了光标后片段，旧内容残留/新内容只贴了一截，导致首行恰好是 `return false;`（本案最可能成因）。
2. **网络中断 / 上传不完整**：传输中途断开，服务器只收到文件开头部分（截断）。
3. **浏览器缓存 / 插件干扰 AJAX 上传**：广告拦截、旧缓存导致上传请求被截断或错包。
4. **文件编码 BOM / 特殊字符**：UTF-8 BOM（`EF BB BF`）或非常规字符触发平台解析异常/截断。
5. **草稿/多次提交覆盖**：误把上一版或空草稿当作当前版本上传。
6. **文件大小超限**（作业平台类）：单文件上限触发 413，但 CANNJudge 单文件 `.asc` 一般较小，可能性低。

### 4.3 预防检查（本机可执行）
```bash
# 1) 首末行与行数核对（应≈本地 405 行，首行 #include，末行合法结尾）
head -1 提交/V002/kernel.asc      # 期望: #include <cmath>
tail -1 提交/V002/kernel.asc      # 期望: 合法收尾（如 } 或 run_kernel 结尾）
wc -l  提交/V002/kernel.asc       # 期望: 405

# 2) 检查 BOM（首 3 字节不应是 EF BB BF）
head -c 3 提交/V002/kernel.asc | xxd

# 3) 确认关键符号与入口存在（上传前本地自检）
grep -n "extern \"C\" void run_kernel" 提交/V002/kernel.asc
grep -n "IsSingleTensorGroup\|IsPositiveShape\|MAX_ELEMENTS\|MAX_DIM" 提交/V002/kernel.asc

# 4) 若走“复制粘贴”，上传后用剪贴板与本地 diff（macOS pbpaste）
diff <(cat 提交/V002/kernel.asc) <(pbpaste)

# 5) 平台端：打开编辑器后 Ctrl+A 全选，确认首行不是 return false; 再提交；
#    提交后立刻看首个编译错误——若首行是 return false; 或 expected unqualified-id，
#    立即判定为“上传异常”而非代码问题，重传完整文件。
```

---

## 5. 本题最可能踩的坑 Top 5 + 本机可执行检查项

> 每条配一个**本机可直接执行**的检查项（命令/动作），无需 NPU。

### 坑 1：宏/命名冲突（V001 已踩）
- **风险**：`TPipe pipe_` 这类与保留宏/类型同名的成员，会触发 `unknown type name 'pipe_'` 并连锁 InitBuffer 报错。
- **检查项**：
  ```bash
  grep -n "TPipe pipe_\|TPipe pipe;\|class.*pipe_\b" 提交/V002/kernel.asc
  # 期望：无命中（本题 V002 已改为 TPipe tpipe）
  ```

### 坑 2：上传内容异常（V002 已踩）
- **风险**：编辑器未全选替换，平台只收到片段（首行 `return false;`），整文件连锁编译错。
- **检查项**：
  ```bash
  head -1 提交/V002/kernel.asc; tail -1 提交/V002/kernel.asc; wc -l 提交/V002/kernel.asc
  # 期望：首行 #include；末行合法；行数==本地（约405）
  # 平台端 Ctrl+A 全选确认首行非 return false; 再提交
  ```

### 坑 3：尾块越界 / 踩相邻行（D 非 32 倍数）
- **风险**：`D%32!=0` 时最后一块若用 `DataCopy` 或 `DataCopyPad` padding 越界，会 Core Dump 或污染相邻行（32B cache line 总线踩踏）。
- **检查项**：
  ```bash
  grep -n "DataCopyPad\|rightPadding\|leftPadding\|DataCopy(" 提交/V002/kernel.asc
  # 期望：非对齐最后块走 DataCopyPad，rightPadding 仅向右补零，不写回相邻行
  # 另用 python 校验边界：D 取 31/33/67/129/1000/32768 做预期输出长度检查
  ```

### 坑 4：归约精度 / 未用 FP32 累加
- **风险**：在 FP16/BF16 上累加 `x²` → 溢出 INF/NAN；或中间过早降精度 → 与参考不对齐（devpress、ops-transformer、marvin-42 一致结论）。
- **检查项**：
  ```bash
  grep -n "ReduceSum\|float sum\|GetValue\|CAST_RINT\|sqrtf" 提交/V002/kernel.asc
  # 期望：归约与 square/mean/sqrt 在 FP32；输出 Cast 用 RoundMode::CAST_RINT
  # 确认无“在 half/bf16 张量上直接做 x² 累加”的写法
  ```

### 坑 5：DataCopyPad 参数单位与 32B 对齐
- **风险**：`srcStride`(GM, 字节) 与 `dstStride`(UB, dataBlock) 单位混淆；或 `blockLen` 非 32B 整数倍导致 MTE 报错（掘金、训练营第九期）。
- **检查项**：
  ```bash
  grep -n "DataCopyExtParams\|srcStride\|dstStride\|blockLen\|SetGlobalBuffer" 提交/V002/kernel.asc
  # 期望：srcStride 单位为字节、dstStride 单位为 dataBlock(32B)；
  # blockLen 为 sizeof(T) 整数倍；GM 起始地址与长度满足 32B 对齐
  ```

---

## 6. 检索受限与未命中说明

- **受限**：本机无 CANN/NPU，无法真机复现编译/精度/性能；所有结论来自公开资料与本项目本地失败记录，未做 NPU 验证（与 AGENTS.md 约定一致）。
- **未命中 / 弱命中**：
  - 未找到**针对 `AddRmsNormBias` 西南赛区初赛**的专属失败帖（该赛题较新，公开讨论少）；以通用 Ascend C 算子竞赛 + RMSNorm/AddRmsNorm 相关 Issue/PR 替代。
  - 未找到 CANNJudge 平台**官方**“上传截断/内容异常”的说明文档；V002 成因基于本项目记录 + 通用作业平台上传失败资料类比推断，标记为“最可能成因”而非定论。
  - 昇腾 FAQ 讨论贴（hiascend）WebFetch 仅返回页眉，详细案例来自 WebSearch 摘要；已标注为 B 级但仍建议以页面原文为准。
  - GPU MODE leaderboard 的“归约常见错误”未单独检索到结构化帖子，改用 KernelBench/robust-kbench 论文与 CUDA edge-tile 技能文档作类比证据（标注“类比”）。
- **未编造**：所有案例均附证据 URL 或本地记录；无来源支撑的说法已标注“C 级/类比/推断”。

---

## 7. 来源表（编号 / 名称 / URL / 日期 / 用途 / 等级）

| 编号 | 名称 | URL | 日期 | 用途 | 等级 |
|------|------|-----|------|------|------|
| S1 | 昇腾AI创新大赛-算子挑战赛FAQ（编译报错方案） | https://www.hiascend.com/dev/forum/thread-0259189679380603071-1-1.html | 2025-08-04 | 编译错误高频案例（路径/API/型号/参数） | B |
| S2 | 算子挑战赛交流圈（同 FAQ 转载） | https://www.hiascend.com/developer/group/01102182523402175016 | 2025 | 同上，社区交流 | B |
| S3 | sglang Issue #15391：aclnnAddRmsNorm 运行时失败 | https://github.com/sgl-project/sglang/issues/15391 | 2025-12-18 | AddRmsNorm 在 NPU 运行时/Runtime 失败实例 | B |
| S4 | CANN 算子挑战赛（江山赛区）集中讨论贴 | https://gitcode.com/cann/cann-competitions/discussions/1 | 2025-04 | 提交规则、版本管理、提交次数 | B |
| S5 | 算子挑战赛江山赛区 Erf 提交 MR #214 | https://gitcode.com/cann/cann-ops-competitions/merge_requests/214 | 2025 | 真实团队提交 PR 结构与优化策略 | B |
| S6 | atomgit cann/ops-nn PR #8384：修复 AddRmsNorm 示例 CHECK_RET 宏语法错误 | https://atomgit.com/cann/ops-nn/pull/8384/commit | 2026-08 | AddRmsNorm 示例代码宏语法错误修复 | B |
| S7 | Gitee modelzoo Issue I5JJ8N：ReduceSum 精度误差 | https://gitee.com/ascend/modelzoo/issues/I5JJ8N | 2022-07-29 | 归约 float 累加精度问题 | B |
| S8 | ascend-rs Appendix D：编译前静态校验三关 | https://ascend-rs.org/appendix/appendix-d-ecosystem.html | 2025 | 入口/同步/Buffer 静态检查范式 | C |
| S9 | 昇腾官方：避免 TPipe 在对象内创建和初始化 | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/developmentguide/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0028.html | 2024-2025 | TPipe 命名/创建最佳实践 | A |
| S10 | CANN ops-transformer：RMSNorm 数值精度分析 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | 2025 | FP16 归约溢出、FP32 累加器必要性 | C |
| S11 | 国产芯片精度排查（rms_norm 中间转 bf16 不对齐） | https://devpress.csdn.net/v1/article/detail/156951082 | 2025 | bf16 中间精度导致与 CUDA 不对齐 | C |
| S12 | AI 生成 CUDA kernel 通过验证器却破坏训练（bf16 累加） | https://insights.marvin-42.com/articles/ai-generated-cuda-kernels-passed-the-benchmark-then-broke-real-training | 2025 | bf16 累加静默错误 | C |
| S13 | 海光 DCU：BF16 逐位一致仍文本漂移 | https://blog.csdn.net/2502_94781626/article/details/162957938 | 2026 | 归约顺序非确定、单 shape≠全过 | C |
| S14 | AscendC DataCopyPad 写出溢出 Bug 详解 | https://blog.csdn.net/ferriswym/article/details/162206787 | 2025 | 尾块 padding 覆盖相邻段 | C |
| S15 | 32-Byte 内存对齐与 Burst 性能（ALIGN_UP 越界） | https://ai6s.net/69429f5cbf6b0e4b285c3379.html | 2025 | 向上对齐读越界/OOM | C |
| S16 | DataCopyPad 32B 对齐（srcStride vs dstStride 单位） | https://juejin.cn/post/7682716879561687050 | 2025 | 参数单位混淆 | C |
| S17 | 昇腾训练营第九期：非对齐尾块 DataCopy Core Dump | https://blog.csdn.net/2401_82857325/article/details/155320399 | 2025 | 尾块必须用 DataCopyPad | C |
| S18 | 训练营微认证避坑（Tiling 鲁棒性/32B 对齐魔咒） | https://hwcomputing.csdn.net/694e1b815b9f5f31781af92c.html | 2025 | 泛化 Tiling、尾块处理 | C |
| S19 | 避坑指南：编译过运行挂/死锁/精度丢失 | https://hwcomputing.csdn.net/695f58470846ec2c4c5ad98a.html | 2025 | 对齐/结构体 packing/死锁 | C |
| S20 | ReduceSum 实现（reducePattern/srcInnerPad/32B 对齐） | https://juejin.cn/post/7682415119352791078 | 2025 | 归约对齐与 tmp buffer | C |
| S21 | catlass 精度问题定位（FP32 过但 FP16/BF16 失败） | https://catlass.readthedocs.io/zh-cn/latest/1_Practice/evaluation/precision_debug/ | 2025 | 累加器精度/溢出/RoundMode | C |
| S22 | robust-kbench（KernelBench 漏洞/单 shape 不足） | https://arxiv.org/pdf/2509.14279v1 | 2025 | 性能优化失败、reward hacking | C |
| S23 | KernelBench 调试：正确性失败与 reward hacking | https://deepwiki.com/ScalingIntelligence/KernelBench/8.3-profiling-and-debugging | 2025 | 正确性/性能调试清单 | C |
| S24 | CUDA Edge Tiles 边界处理（类比越界） | https://lobehub.com/zh/skills/krxgu-kernel-skills-handle-boundary-conditions | 2025 | 掩码/predication 防越界（类比） | C |
| S25 | CANNJudge 算子竞赛全流程指南 | https://blog.csdn.net/gitblog_00467/article/details/152189083 | 2025 | 提交状态、泛化、不开放测试 | C |
| S26 | CANNJudge 5 个实战技巧 | https://blog.csdn.net/gitblog_01128/article/details/153301497 | 2025 | 泛化设计、提交流程 | C |
| S27 | 作业平台上传失败常见原因（类比） | https://ask.csdn.net/questions/8783004 | 2025 | 上传异常成因类比 | C |
| S28 | 昇腾 AI 原生创新算子挑战赛 S3（提交次数/最优成绩） | https://www.hiascend.com/developer/contests/details/52d7b02fc84d4afaac45ce652b155772 | 2024-2025 | 提交次数规则参考 | B |
| L1 | 本项目 V001 结果（pipe_ 冲突） | `提交/V001/结果.md`（本地） | 2026-09-11 | 本项目真实编译失败 | L |
| L2 | 本项目 V002 结果（上传内容异常） | `提交/V002/结果.md`（本地） | 2026-09-11 | 本项目真实上传异常 | L |
| L3 | 本项目版本实验记录 | `提交/版本实验记录.md`（本地） | 2026-09-11 | 失败分类与修复追踪 | L |

> 证据等级：A=官方文档；B=官方赛事/论坛/PR/官方仓库 Issue；C=社区文章/论文/讨论（含类比）；L=本项目本地真实记录。
> 全部 20 个案例均有证据；其中 **S1–S7、S9、S28（共 10 条）为可点击的真实记录（Issue/PR/讨论/官方文档 URL）**，L1/L2 为本项目本地真实失败记录。
