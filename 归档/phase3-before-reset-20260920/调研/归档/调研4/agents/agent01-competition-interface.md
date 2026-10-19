# Agent 01 报告：比赛题面、官方规则与提交接口（AddRmsNormBias）

> 调研日期：2026-09-11
> 代理：agent01（主题：比赛题面、官方规则与提交接口）
> 题面 ID：`6a9a9a99bf41025d6013eb85`（平台编号 `ID=1742`）；赛事 ID：`2094722165106008066`（平台内部 `contest_id=6a9a9295bf41025d601255a3`，`zone_id=2094722165106008066`）
> 方法：直接抓取 `cannjudge.cn` 公开 API 的 JSON（题面页为 JS-SPA，内容来自该 API）、`competition.gitcode.com` 官方赛事页，并结合用户提供的本地模板与既有 `文档/`。**未登录任何账号、未执行任何提交动作。**
> 标签体系：①官方页面明确写出　②用户提供但未找到页面依据　③页面可访问但内容不完整　④根据接口或源码推断　⑤当前无法确认
> 证据等级：A=官方题面/官方 API/官方仓库/官方赛事页　B=官方样例、测试、培训材料　C=社区/个人　D=仅摘要未核验

---

## 1. 结论摘要（≤10 条，每条带标签）

1. **【①/A】接口字段顺序**：x → residual → gamma → bias → epsilon(属性) → output；题目描述表格（§3.3）原文如此。CANNJudge 直调工程的 `run_kernel` C 签名为：`run_kernel(GM_ADDR x, info_x, residual, info_residual, gamma, info_gamma, bias, info_bias, output, info_output, int64_t availableCoreNum, aclrtStream stream, float epsilon)`（epsilon 作为末尾标量传入）。来源：题面 `desc` + 本地模板 `kernel.asc`/`main.asc`。
2. **【①/A】shape/rank/dtype 约束**：支持 2D(batch,D)/3D(batch,seq,D)/4D(batch,seq,heads,D)；`batch∈[1,8192]`、`seq_len∈[1,32768]`、`D∈[64,32768]`；dtype ∈ {float16, bfloat16, float32}，输出与输入同型。来源：题面 `desc` §3.4。
3. **【①/A】gamma/bias 形状**：均为 `(D,)`，沿最后一维对齐；`residual.shape == x.shape`。来源：题面 `desc` §3.3/§3.4。
4. **【①/A】epsilon 语义**：float 属性，默认 `1e-5`，取值通常 `1e-5 ~ 1e-6`（**可变**），加在 `mean(y²)` 之后、开方之前；与 PyTorch `rms_norm` 一致。来源：题面 `desc` §3.3/§3.5。
5. **【①/A】测试点数量为 15**：题面 `desc` §六明文"本次比赛共15个测试点，所有 case 点精度全部通过才会计分"，且公开 API `testcases` 数组长度恰为 15（ID 11456–11470）。`visible_testcase_count=0` → 各点具体 shape/dtype/epsilon 配置**平台未开放**。
6. **【①/A】每点执行/统计次数 iterations=5**：题面 API 顶层字段 `iterations:5` 实测返回。
7. **【①/A】精度判定口径**：fp32 相对<1e-4 且 绝对<1e-4；fp16/bfloat16 相对<1e-3 且 绝对<1e-3；int32"完全准确"（注：输入 dtype 仅列 fp16/bf16/fp32，int32 疑为模板/题面遗留，非真实输入类型）。题面**未给出**"允许失配元素百分比"口径；本地模板 `verify_result.py` 的 `tol=0.001`（0.1% 失配容忍）属本地校验，**判题端是否采用未证实**。来源：题面 `desc` §五。
8. **【①/A】性能评分公式**：单点 `100/(1+log₁.₅(t/T))`（T=该点全局最优时间，t=当前提交时间），最终分=15 点均值；同分按提交时间，越早排名越高。`score_mode=1`、`use_baseline=False` → **性能是唯一计分维度**（精度为前置门槛，不过则不计分）。来源：题面 `desc` §六 + API 字段。
9. **【①/A】上传形态字段 `code_template=npu_kernel_dev`**：题面 API 顶层字段实测为 `npu_kernel_dev`（直调单文件 `kernel.asc` 形态）；提交物为含 `run_kernel` 的单个 `kernel.asc`。"单文件"由 `code_template`+模板结构推断（④），实际上传字段需登录提交页最终确认。**旧 Skill 的 `tiling_h`/`host_cpp`/`kernel_cpp` 四字段属旧 msopgen 工程格式，不适用本题**。
10. **【①/A】每日提交 50 次 + 初赛/决赛/违规规则**：官方赛事页明文"每天最大提交次数为50次""取比赛期间最后一次提交的成绩作为最终成绩"；初赛前 32 强晋级决赛；"核心计算必须在昇腾 NPU 上通过 AscendC 实现，Host CPU 代算/空 kernel 占位 → 违规，取消当前提交成绩"。来源：`competition.gitcode.com` 赛事页。

**补充关键结论（SoC）【⑤/A】**：题面 API 中 `soc_version: null`、`ddk_version: null` —— **判题 SoC 型号官方未指定**（非 Atlas A2/dav-2201 官方要求；`dav-2201` 仅为本地模板 `CMakeLists.txt` 的编译默认，非平台约束）。此为对既有推断的**纠正**。

---

## 2. 接口与规则事实表

| 项 | 值 | 来源 URL | 标签 | 等级 |
| --- | --- | --- | --- | --- |
| 题面 ID / 平台编号 | `6a9a9a99bf41025d6013eb85` / `ID=1742` | cannjudge API problems | ① | A |
| 赛事 ID | 内部 `6a9a9295bf41025d601255a3`；zone `2094722165106008066` | cannjudge API contests / gitcode | ① | A |
| 接口字段顺序 | x, residual, gamma, bias, epsilon, output | 题面 desc §3.3 | ① | A |
| run_kernel 签名 | `run_kernel(x,info_x,residual,info_residual,gamma,info_gamma,bias,info_bias,output,info_output,availableCoreNum,stream,float epsilon)` | 本地模板 kernel.asc/main.asc | ④ | B |
| 支持 rank | 2D/3D/4D | 题面 desc §3.4 | ① | A |
| batch / seq / D 范围 | batch∈[1,8192], seq∈[1,32768], D∈[64,32768] | 题面 desc §3.4 | ① | A |
| dtype | float16, bfloat16, float32（输入）；输出同型 | 题面 desc §3.4 | ① | A |
| gamma/bias 形状 | 均为 (D,) | 题面 desc §3.3/§3.4 | ① | A |
| epsilon 默认/范围 | 默认 1e-5；通常 1e-5~1e-6；可变 | 题面 desc §3.3/§3.5 | ① | A |
| 数学语义 | y=x+residual; rms=sqrt(mean(y²,dim=-1)+eps); out=y/rms*gamma+bias | 题面 desc §3.2/§3.3 | ① | A |
| 测试点数量 | 15 | 题面 desc §六 + API testcases 长度15 | ① | A |
| 测试点通过规则 | 全部 15 点精度通过才计分 | 题面 desc §六 | ① | A |
| 每点 iterations | 5 | 题面 API `iterations:5` | ① | A |
| 测试点配置可见性 | visible_testcase_count=0（shape/dtype/eps 不公开） | 赛事 API | ① | A |
| 精度阈值 | fp32 相对<1e-4 且 绝对<1e-4；fp16/bf16 相对<1e-3 且 绝对<1e-3 | 题面 desc §五 | ① | A |
| 失配容忍百分比 | 题面未给；本地模板 tol=0.001（0.1%） | 题面 desc（无）/ 模板 verify_result.py | ③/② | A/B |
| 确定性要求 | 多次执行结果完全一致（无随机性） | 题面 desc §四 | ① | A |
| 特殊值 | NaN→NaN；Inf→按公式得 Inf/NaN；不崩溃 | 题面 desc §3.7 | ① | A |
| 评分公式 | 单点 100/(1+log₁.₅(t/T))；均值；同分早提交优先 | 题面 desc §六 | ① | A |
| 计分维度 | 性能唯一计分（精度为门槛） | 题面 desc §四/§六 + score_mode=1, use_baseline=False | ① | A |
| 排名取数 | ranking_submission_mode=latest（取最新提交） | 题面 API | ① | A |
| CANN 版本 | 9.0.0 | 题面 API `cann_version:9.0.0` | ① | A |
| Kernel 类型 | vector | 题面 API `kernel_pattern:vector` | ① | A |
| 上传 code_template | npu_kernel_dev（直调单文件 kernel.asc） | 题面 API `code_template` | ① | A |
| 上传为单文件 kernel.asc | 推断（code_template+模板） | 题面 API + 模板 | ④ | A/B |
| 判题 SoC 型号 | **未指定**（`soc_version:null`） | 题面 API | ⑤ | A |
| 初赛时间 | 2026-09-05 00:00 ~ 2026-10-17 18:00（北京时间） | 题面 API start/end（UTC+8 换算） | ① | A |
| 每日提交额度 | 50 次/天 | gitcode 赛事页 | ① | A |
| 最终成绩取数 | 取比赛期间最后一次提交成绩 | gitcode 赛事页 | ① | A |
| 封榜/截止 | 提交 10-17 17:30 封榜、18:00 截止；freeze_duration=30（约 30 分钟冻结） | gitcode 赛事页 + 赛事 API | ① | A |
| 晋级 | 初赛前 32 强晋级决赛 | gitcode 赛事页 | ① | A |
| 决赛安排 | 10-20 发题；10-20~11-06 打榜；11-07 现场决赛 | gitcode 赛事页 | ① | A |
| 违规规则 | Host CPU 代算/空 kernel 占位绕开 NPU → 取消当前提交成绩 | gitcode 赛事页 | ① | A |
| 报名/组队 | 西南五省高校在校生；同赛区可跨校组队；每日算力仅队长申请 | gitcode 赛事页 | ① | A |
| 公开排行榜样本 | last_submission 数组含 371 条（user_id/submission_id/score） | 题面 API | ③ | A |

---

## 3. 题面原文摘录（可核对关键句，来自 `desc` 字段）

> **§3.3 输入输出与属性总览**（表格节选）
> | 类型 | 参数名 | 类型 | 维度形状 | 支持数据类型 | 备注 |
> | INPUT(必选) | x | tensor | (..., D) | float16, bfloat16, float32 | 主输入张量 |
> | INPUT(必选) | residual | tensor | (..., D) | 与x一致 | shape与x完全一致 |
> | INPUT(必选) | gamma | tensor | (D,) | 与x一致 | RMS归一化的缩放系数 |
> | INPUT(必选) | bias | tensor | (D,) | 与x一致 | 逐通道偏置，加在RMS归一化结果之后 |
> | ATTR(属性) | epsilon | float | - | - | 默认值1e-5 |
> | OUTPUT(输出) | output | tensor | (..., D) | 与x一致 | 残差加+RMS归一化+偏置加法的结果 |

> **§3.4 关键输入约束**
> - 数据类型: float16, bfloat16, float32
> - 维度场景: 2维（batch, D）、3维（batch, seq, D）、4维（batch, seq, heads, D）
> - 维度取值范围: batch∈[1,8192]；seq_len∈[1,32768]；隐藏维度 D∈[64,32768]
> - 非对齐场景兼容: D 可能为非 32 整倍数（如 D=192, D=576）
> - 数值取值约束: 输入张量数值不超出各自数据类型的原生表达范围

> **§3.5 核心属性说明**
> - epsilon（float，默认 1e-5）: ……取值通常为 1e-5 至 1e-6

> **§五、精度判断规则**
> - float32：相对误差 < 1e-4，绝对误差 < 1e-4（双万分之一精度）
> - float16、bfloat16：相对误差 < 1e-3，绝对误差 < 1e-3（双千分之一精度）
> - int32：要求计算结果完全准确，无误差

> **§六、得分规则**
> - 本次比赛共15个测试点，所有case点精度全部通过才会计分。
> - 每个测试点单独计分，逻辑如下（T为最优性能，t为当前提交性能）：100/(1+log₁.₅(t/T))
> - 排行榜显示的最终分数为所有case得分的均值。若得分计算一致，则以提交时间进行排序，提交越早，排名越高。

> **§3.7 特殊值处理规则**
> - NaN 处理: 如果输入包含 NaN，输出对应位置为 NaN，算子不应崩溃
> - Inf 处理: 如果输入包含 Inf，输出按数学公式得出 Inf 或 NaN（Inf / Inf = NaN），算子应正常执行不崩溃

> **gitcode 赛事页（规则节选）**
> - "(每天最大提交次数为50次，取比赛期间最后一次提交的成绩作为最终成绩)"
> - "比赛分初赛和决赛两个赛段……本赛事要求核心计算在昇腾 NPU 上通过 AscendC 算子实现完成，任何将计算转移至 Host CPU、通过空 kernel 占位绕过 NPU 计算要求的行为，均构成违规，将取消当前提交成绩。"

---

## 4. 提交接口与上传格式

- **提交形态（直调工程）**：`code_template = npu_kernel_dev`（题面 API 实测字段）。即 CANNJudge 直调（Direct Invocation）模式，提交物为包含 `extern "C" void run_kernel(...)` 的单个 `kernel.asc` 文件。
- **上传字段推断（④）**：由 `code_template=npu_kernel_dev`、本地模板结构（`kernel.asc` 内含 `run_kernel`）、以及上一轮"旧 Skill 四字段不适用"的结论交叉支持；**实际提交表单字段名需登录 `…/submit` 页最终确认**（该页为登录态 SPA，本轮未登录、未提交）。
- **`run_kernel` 入参顺序（④/B，来自模板）**：
  `GM_ADDR x, const TensorGroupInfo& info_x, GM_ADDR residual, const TensorGroupInfo& info_residual, GM_ADDR gamma, const TensorGroupInfo& info_gamma, GM_ADDR bias, const TensorGroupInfo& info_bias, GM_ADDR output, const TensorGroupInfo& info_output, int64_t availableCoreNum, aclrtStream stream, float epsilon`
  - 注意：接口概念顺序为 x/residual/gamma/bias/epsilon/output（见 §3.3）；而 C 函数签名中 epsilon 是末尾标量参数，`output` 在 `availableCoreNum`/`stream` 之前。实现需严格对齐该签名。
  - `TensorInfo.dtype` 枚举（模板注释）：0=fp32,1=fp16,2=bf16,3=int8,…；`availableCoreNum` 由 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 取得。
- **不适用项**：旧 CANNJudge Skill 的 `tiling_h`/`tiling_key_h`/`host_cpp`/`kernel_cpp` 四字段属旧版 msopgen 算子工程格式，本题直调模式不适用。
- **提交时间窗口**：初赛 2026-09-05 00:00 ~ 2026-10-17 18:00（北京时间）；10-17 17:30 封榜、18:00 截止（与赛事 API `freeze_ranking=true, freeze_duration=30` 一致，约为 30 分钟冻结）。

---

## 5. 评分与精度判定机制

- **前置门槛（精度）**：15 个测试点**全部**达到 §五阈值后才进入计分；任一不达标则该题不计分（"所有 case 点精度全部通过才会计分"）。
- **精度阈值（官方，①/A）**：
  - fp32：相对误差 < 1e-4 **且** 绝对误差 < 1e-4
  - fp16 / bfloat16：相对误差 < 1e-3 **且** 绝对误差 < 1e-3
  - int32：完全准确（疑模板遗留，真实输入不含 int32）
  - **未明确**："相对误差"是逐元素 max 还是 mean？"失配元素百分比容忍"是否在判题端启用？题面 `desc` 未给出——本地模板的 `tol=0.001`（0.1%）属本地校验口径，判题端是否一致**未证实（③）**。
- **性能计分（官方，①/A）**：
  - 单点得分 `100 / (1 + log₁.₅(t / T))`，T 为该测试点全局最优（最快）提交耗时，t 为当前提交耗时。
  - 最终分 = 全部 15 点得分的均值。
  - `score_mode=1`、`use_baseline=False`：无基线、纯性能排名；性能是唯一计分维度，精度仅作通过性门槛。
  - 同分排序：提交时间早者排名高；排行榜取各队**最新一次**提交（`ranking_submission_mode=latest`，与 gitcode"取最后一次提交成绩"一致）。
- **公开统计（③/A）**：题面 API `last_submission` 暴露 371 条提交记录（含 user_id/submission_id/score，抽样 score 多为 0）→ 证明已有大量参赛提交，但**无法据此推算通过率**（score 字段样本为 0，且隐藏了各点明细）。

---

## 6. 与「用户提供信息 / 既有 capture」的差异清单

| # | 既有认知（用户提供 / 调研1） | 本轮复核结果 | 变化 |
| --- | --- | --- | --- |
| 1 | "每日最多 50 次"标为"用户提供；官网未明文，待确认" | gitcode 赛事页**明文"每天最大提交次数为50次"** | **升级 ②→①**（已找到页面依据） |
| 2 | SoC 推断为 Atlas A2 / `dav-2201`（B 级推断，似官方约束） | 题面 API `soc_version:null`、`ddk_version:null` —— **官方未指定 SoC**；`dav-2201` 仅为本地模板编译默认 | **纠正**：SoC 非官方要求，标记 ⑤；模板默认不可当作平台约束 |
| 3 | "15 测试点=官方明文 A 级 API 确认" | 复核确认：`desc` §六 + `testcases` 长度=15 双印证 | 维持 ①，证据更完整 |
| 4 | "code_template=npu_kernel_dev A 级确认" | 复核确认：API 顶层字段实测为 `npu_kernel_dev` | 维持 ① |
| 5 | "iterations=5" | 复核确认：API `iterations:5` | 维持 ① |
| 6 | "得分公式 100/(1+log₁.₅(t/T)) A 级" | 复核确认：`desc` §六原文 | 维持 ① |
| 7 | 精度"fp32<1e-4；f16/bf16<1e-3" | 复核确认：`desc` §五原文（相对**且**绝对双阈值） | 维持 ①；补充"双阈值"语义 |
| 8 | int32 列为真实输入类型（待澄清） | `desc` §3.4 输入 dtype 仅 fp16/bf16/fp32；§五 精度表含 int32 但属遗留 | 维持"非真实输入，模板遗留"判断 |
| 9 | 判题端精度失配容忍 0.1% 未证实 | 本轮仍未能在官方题面找到该口径；本地模板 `tol=0.001` 为模板自带 | 维持"未证实（③）" |
| 10 | 提交截止时间 2026-10-17 18:00 | 赛事 API end_time=2026-10-17T10:00:00Z（=北京 18:00）+ gitcode"17:30封榜/18:00截止"三方一致 | 维持 ①，新增封榜时间证据 |

---

## 7. 未闭环事项与 blocker

| # | 缺什么 | 去哪补 | 怎么验 |
| --- | --- | --- | --- |
| B1 | 15 个测试点的具体 shape / dtype / epsilon 配置 | 平台未开放（`visible_testcase_count=0`）；仅能通过登录提交后判题返回或官方样例反推 | 真机提交后读取判题回传日志；或向组委会/赛事交流群（gitcode discussions/12）索取配置范围 |
| B2 | 判题端是否启用"失配元素百分比容忍"及具体值 | 题面 `desc` 未写；本地模板 `tol=0.001` | 以"全元素满足相对+绝对双阈值"为最高标准实现；提交后看判题 PASS/FAIL 反推 |
| B3 | 上传表单确切字段名 / 是否需 zip 工程包 | 提交页为登录态 SPA，未登录不可见 | 登录 `…/submit` 页查看表单；或等团队提供提交页 DOM/网络请求（不自行登录） |
| B4 | 判题 SoC 具体型号 | 平台 `soc_version=null`，未指定 | 向组委会确认；以模板默认 `dav-2201` 编译但做好换 SoC 回退（DataCopyPad 支持矩阵差异） |
| B5 | 决赛题目/规则细节（是否换题、禁用优化手段） | 初赛未结束，决赛页未发布 | 关注 gitcode 赛事页 10-20 前后更新 |
| B6 | "相对误差"计算口径（max vs mean）与采样方式 | 题面未细化 | 实现按"每个元素均满足双阈值"最严口径；提交后反推 |
| B7 | 公开通过率 / 各队得分分布 | `last_submission` 仅暴露 score（样本 0），无逐点明细 | 排行榜页（登录态）可能有更细统计；或赛事群内讨论 |

---

## 8. 来源表

| 编号 | 名称 | URL | 版本/日期 | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- | --- |
| S1 | CANNJudge 题面 API（JSON） | `https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85` | 题面 v1（version_label=v1） | 2026-09-11 | 接口字段、testcases、iterations、code_template、cann_version、soc_version、desc 原文 | A |
| S2 | CANNJudge 赛事 API（JSON） | `https://cannjudge.cn/api/contests/6a9a9295bf41025d601255a3` | 赛事 op_challenge_xinan_prelim | 2026-09-11 | 赛事时间、visible_testcase_count、freeze_ranking、ranking_mode、submit 配置、zone_id | A |
| S3 | GitCode 官方赛事页 | `https://competition.gitcode.com/competition/2094722165106008066/intro` | 2026 CANN 挑战赛西南赛区 | 2026-09-11 | 每日50次、最终成绩取最后一次、初赛/决赛日程、违规规则、报名组队 | A |
| S4 | CANNJudge 题面页（SPA） | `https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias` | — | 2026-09-11 | HTTP 200 但 JS 渲染，正文内容等同 S1 的 desc | A（内容）/ D（页面本身不可读） |
| S5 | CANNJudge 提交页（SPA，登录态） | `https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit` | — | 2026-09-11 | 未登录；仅记录为需登录的 blocker（B3） | —（未访问内容） |
| S6 | 本地模板工程 | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/`（kernel.asc, main.asc, run.sh, CMakeLists.txt, scripts/） | 用户提供（2026-09-10 下载） | 2026-09-11 | run_kernel 签名、dtype 枚举、本地验证 harness、CMake 默认 SoC | B |
| S7 | 既有项目文档 | `文档/problem-add-rms-norm-bias.md`、`文档/competition-rules.md`、`文档/submission-checklist.md` | 调研1（2026-09-10/11） | 2026-09-11 | 上一轮 capture 对照与差异比对 | A/B（派生） |

---

### 备注
- 所有结论均来自公开 API/页面与用户提供的本地模板；**未登录、未提交、未消耗任何提交次数**。
- 凡 NPU 侧运行结论（编译/精度/性能）均不在本报告范围，且不声称已验证；本机 macOS 无 CANN/NPU，仅做资料核对。
- 与"用户提供信息"相比，本轮**纠正**了 2 处：① 每日 50 次现已找到官方页面依据（gitcode）；② 判题 SoC 官方未指定（`soc_version=null`），`dav-2201` 仅为模板编译默认，不可当作平台约束。
