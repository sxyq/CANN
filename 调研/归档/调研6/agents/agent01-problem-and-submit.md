# Agent 1 调研报告：AddRmsNormBias 题面与提交接口

> 调研角色：Agent 1（题面与提交接口），10 个并行子代理之一，不越界检索其他主题。
> 调研性质：仅方案空间核实，**不写新提交代码**。
> 访问日期：2026-09-12
> 数据源：CANNJudge 题目 API / 公开题面页 / 赛事 API / GitCode 赛事规则页 / 平台下发模板（本地）/ 社区公告。
> 标签定义：`[官方页面明确写出]`(URL 原文) / `[用户提供但未找到页面依据]` / `[页面可访问但内容不完整]`(需登录/JS) / `[根据接口或源码推断]` / `[当前无法确认]`。

---

## 1. 执行摘要（每条带标签）

1. **接口契约已由官方题面(desc)完整写明**：输入 `x`/`residual` 为 `(..., D)`，输出 `output` 同 `x`；`gamma`/`bias` 为 `(D,)`；属性 `epsilon` 默认 `1e-5`；支持 `float16/bfloat16/float32`；维度覆盖 2D/3D/4D（batch, [seq], [heads], D）。`[官方页面明确写出]`
2. **共 15 个测试点，全部精度通过才计分**；计分公式 `100/(1+log_1.5(t/T))`，`T`=最优性能，`t`=当前提交性能，最终分数为各 case 得分均值；同分时按提交时间早者优先。`[官方页面明确写出]`
3. **官方精度阈值比本地模板更严**：fp32 双 `1e-4`、fp16/bf16 双 `1e-3`、int32 完全精确；而本地 `verify_result.py` 单用例仅 `rtol=atol=1e-3`、`tol=1e-3`（fp16）。fp32 必须按 `1e-4` 自测。`[官方页面明确写出]`＋`[根据接口或源码推断]`(本地对比)
4. **每日最多 50 次提交，取最后一次提交成绩作为最终成绩**；平台 `ranking_submission_mode=latest` 印证取最新；但 GitCode 页同段又写"把最优成绩作为最终成绩"，两处自相矛盾，以平台 API 字段为准。`[官方页面明确写出]`
5. **提交类型标识 `code_template=npu_kernel_dev`**（题目 API 字段）；形态为直调单文件 `kernel.asc`（模板结构）。`[官方页面明确写出]`
6. **环境**：`cann_version=9.0.0`、`kernel_pattern=vector`、`iterations=5`（每点跑 5 次）；**判题 SoC 型号未在任意公开接口出现**（`soc_version` 字段缺失），模板默认 `dav-2201` 仅本地默认、可被 `NPU_ARCH` 覆盖，真实判题芯片未知。`[官方页面明确写出]`＋`[当前无法确认]`(SoC)
7. **公开题面页可见统计**：通过率 **61%**、通过人数 **240**、尝试人数 **391**。`[官方页面明确写出]`
8. **无法确认项（≥4）**：各测试点具体 shape/dtype/epsilon、判题端是否含"允许失配比例 tol"、5 次 iterations 的聚合方式（最好/平均）、判题 SoC 型号、核数具体值、编译器版本、单文件/包与编辑器大小限制、总提交次数上限、是否有 15 之外的隐藏测试点。`[当前无法确认]`
9. **违规判定**：核心计算须在 NPU 上以 AscendC 实现；将计算转移至 Host CPU、以空 kernel 占位绕过 NPU 均属违规、取消当前提交成绩；抄袭/写死结果等细则仅泛述（"独立完成""以正式规则为准"）。`[页面可访问但内容不完整]`
10. **赛程衔接**：初赛前 32 强晋级决赛；初赛作品 **10/17 18:00 提交截止**（平台 `end_time=2026-10-17T10:00:00Z`）、17:30 封榜；报名截止约 10/16。`[官方页面明确写出]`

---

## 2. 接口事实表

| 字段 | 值 | 标签 | 证据 |
|---|---|---|---|
| 算子语义 | `y=x+residual` → `rms=sqrt(mean(y*y,axis=-1)+eps)` → `out=y/rms*gamma` → `out+bias` | `[官方页面明确写出]` | S001 desc §3.1/§3.2；S009 golden |
| 输入 x | tensor `(..., D)`，dtype fp16/bf16/fp32，ND | `[官方页面明确写出]` | S001 desc §3.3；S002 |
| 输入 residual | tensor `(..., D)`，与 x 同 shape 同 dtype | `[官方页面明确写出]` | S001 §3.3/§3.4 |
| 输入 gamma | tensor `(D,)`，与 x 同 dtype | `[官方页面明确写出]` | S001 §3.3/§3.4 |
| 输入 bias | tensor `(D,)`，与 x 同 dtype，数值独立于 gamma | `[官方页面明确写出]` | S001 §3.3/§3.4 |
| 属性 epsilon | float，默认 `1e-5`，典型取值 `1e-5~1e-6` | `[官方页面明确写出]` | S001 §3.3/§3.5 |
| 输出 output | tensor `(..., D)`，shape 与 dtype 同 x | `[官方页面明确写出]` | S001 §3.3/§3.6 |
| rank 支持 | 2D(batch,D)、3D(batch,seq,D)、4D(batch,seq,heads,D) | `[官方页面明确写出]` | S001 §3.4 |
| 维度取值范围 | batch∈[1,8192]，seq∈[1,32768]，D∈[64,32768] | `[官方页面明确写出]` | S001 §3.4 |
| 非对齐 D | D 可非 32 整倍数（如 192/576），需适配非 32 字节对齐 | `[官方页面明确写出]` | S001 §3.4 |
| dtype 支持 | float16、bfloat16、float32（desc §3.4 仅列此三者） | `[官方页面明确写出]` | S001 §3.4 |
| epsilon 默认值 | `1e-5`（main.asc 中 `epsilon=1e-05f` 印证） | `[官方页面明确写出]`＋`[根据接口或源码推断]` | S001 §3.5；S006 |
| 特殊值 | 输入含 NaN→输出对应 NaN 不崩溃；Inf 按公式得 Inf/NaN 不崩溃 | `[官方页面明确写出]` | S001 §3.7（WebFetch 补全） |
| 确定性 | 同输入同属性多次执行结果须完全一致（不含随机性） | `[官方页面明确写出]` | S001 §四 规则要求 |
| run_kernel 形态 | `extern "C" void run_kernel(GM_ADDR x, info_x, residual, info_r, gamma, info_g, bias, info_b, output, info_o, int64 availableCoreNum, aclrtStream stream, float epsilon)`；内部 `add_rms_norm_bias_custom<<<blockNum,nullptr,stream>>>` | `[官方页面明确写出]`(模板) | S005；S006 |
| 元信息传入 | `TensorGroupInfo`/`TensorInfo` 运行时传入（无 Host 侧 shape 常量、无 tiling 结构体） | `[官方页面明确写出]`(模板) | S005；S006 |
| 提交类型标识 | `code_template = npu_kernel_dev` | `[官方页面明确写出]` | S001 API 字段 |
| 核数获取 | `ACL_DEV_ATTR_VECTOR_CORE_NUM`（运行时取，传入 availableCoreNum） | `[根据接口或源码推断]` | S006 main.asc |

> 关于 int32：精度规则文本列出"int32 要求完全准确"，但 §3.4 支持的 dtype 仅 fp16/bf16/fp32，且测试点 dtype 枚举（模板 `TensorInfo.dtype` 0/1/2=fp32/fp16/bf16）未含 int32。判定为模板/题面残留，**真实测试点 dtype 应为 fp16/bf16/fp32**。`[官方页面明确写出]`(文本存在)＋`[根据接口或源码推断]`(实际范围)

---

## 3. 测试点与评分

### 3.1 测试点数量与配置
- **数量 = 15**：题目 API `testcases` 数组恰含 15 项（`ID` 11456–11470，`type=default`，`baseline=null`）；题面 §六 明确"本次比赛共15个测试点"。`[官方页面明确写出]`
- **各点 shape/dtype/epsilon 不可见**：testcase 详情端点（`/api/testcases/11456`、`/api/problems/.../testcases`）均返回 **403 Forbidden**；题目 API 的 testcase 对象仅含 `_id/type/baseline/ID`，不含 shape/dtype/epsilon。`[当前无法确认]`
- **隐藏测试点**：API 暴露恰好 15 个且题面写"15 个测试点"，无公开证据表明另有隐藏点；但无法 100% 排除（判题端可能私有扩展）。`[当前无法确认]`
- **可见性**：赛事 API `visible_testcase_count = 0`，即公开排行榜仅显示总分均值，不展示逐 case 通过情况。`[官方页面明确写出]`(API)＋`[根据接口或源码推断]`

### 3.2 每点执行次数
- 题目 API `iterations = 5`：每个测试点执行 5 次。`[官方页面明确写出]`
- 5 次结果如何聚合成 `t`（取最好/平均/中位数）**未在任何公开文本说明**。`[当前无法确认]`

### 3.3 精度判定
- 官方口径（题面 §五）：fp32 相对/绝对误差 `< 1e-4`；fp16/bf16 `< 1e-3`；int32 完全精确。`[官方页面明确写出]`
- 是否允许"失配比例 tol"：**题面 §五 仅给逐元素阈值，未提允许失配比例**；本地 `verify_result.py` 另有 `tol=1e-3`（允许 0.1% 元素不达标）。判题端是否套用 tol **无法确认**。`[当前无法确认]`
- 与本地模板一致性：本地仅 1 个 fp16 用例、阈值 `1e-3`，与官方 fp16/bf16 口径**一致**；但与官方 **fp32 的 `1e-4` 更严**——本地若只用 `1e-3` 自测，fp32 case 可能漏判。`[根据接口或源码推断]`
- 是否全部通过才计分：**是**，15 点全过才计分，否则该次提交计 0。`[官方页面明确写出]`

### 3.4 性能评分
- 公式：`score_case = 100 / (1 + log_1.5(t / T))`，`T`=最优性能，`t`=当前提交性能。`[官方页面明确写出]`
- 最终分数 = 所有 case 得分的均值；同分时按提交时间早者排名高。`[官方页面明确写出]`
- `T` 的解读：题面写"T 为最优性能"，题目 API `use_baseline = False` → **T 不是固定基线，而是实时最优（排行榜当前最佳）**，你登顶时该 case 得 100。`[根据接口或源码推断]`
- 是否唯一计分维度：**性能是唯一显式计分维度**；功能正确性为"准入门槛"（不过精度则 0 分）。`[官方页面明确写出]`＋`[根据接口或源码推断]`
- 多次取最好/平均：见 3.2，无法确认。`[当前无法确认]`

---

## 4. 上传与判题环境

| 项 | 结论 | 标签 | 证据 |
|---|---|---|---|
| 提交形态 | 直调单文件 `kernel.asc`（被 `#include` 进 `main.asc`，不得含 `main()`） | `[官方页面明确写出]`(模板) | S005；S006 |
| 类型标识 | `code_template = npu_kernel_dev` | `[官方页面明确写出]` | S001 API |
| 上传字段/单文件还是包/编辑器大小限制 | 提交页需登录（不尝试），公开接口未暴露字段与大小限制 | `[页面可访问但内容不完整]` | S002 仅给"开始答题"链接；S015 需登录 |
| CANN 版本 | `9.0.0`（API `cann_version`；公开页"vector CANN：9.0.0"） | `[官方页面明确写出]` | S001；S002；S003 tags |
| kernel 类型 | `vector`（API `kernel_pattern=vector`） | `[官方页面明确写出]` | S001；S002 |
| 判题 SoC 型号 | **未公开**（API 无 `soc_version`；公开页无） | `[当前无法确认]` | S001（无该字段）；S002 |
| 核数 | 运行时经 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 取得，传 `availableCoreNum`；具体值依赖硬件 | `[根据接口或源码推断]` | S006 |
| 编译器 | Ascend C（`find_package(ASC)`，`--npu-arch=`），版本随 CANN 9.0.0；具体版本未单列 | `[根据接口或源码推断]` | S007 |
| 本地超时 | `run.sh` 内 `timeout 120` 秒（kernel 运行） | `[官方页面明确写出]`(模板) | S008 |
| 判题执行次数 | 每点 5 次（`iterations=5`） | `[官方页面明确写出]` | S001 |
| 排行榜冻结 | 赛事 API `freeze_ranking=True`，`freeze_duration=30`（分钟） | `[官方页面明确写出]` | S003 |

---

## 5. 规则与额度

| 项 | 结论 | 标签 | 证据 |
|---|---|---|---|
| 每日提交额度 | 每天最多 **50 次** | `[官方页面明确写出]` | S004 GitCode 参赛答题要求 |
| 最终成绩取用 | 取比赛期间**最后一次提交**成绩（平台 `ranking_submission_mode=latest` 印证）；GitCode 同段亦称"把最优成绩作为最终成绩"，两者矛盾，以平台 API 为准 | `[官方页面明确写出]`(GitCode＋API) | S004；S001 |
| 总提交次数上限 | 仅规定每日 50 次，**未设总额上限**（公开文本无"总次数"数值） | `[用户提供但未找到页面依据]`／`[页面可访问但内容不完整]` | S004（未提总额） |
| 初赛截止 | 10/17 18:00 提交截止、17:30 封榜（平台 `end_time=2026-10-17T10:00Z`） | `[官方页面明确写出]` | S001；S003；S004 |
| 报名截止 | 约 10/16（GitCode：10/16 00:00；CSDN：10/16 23:59，存在出入） | `[官方页面明确写出]`(GitCode)＋差异 | S004；S011 |
| 初赛/决赛衔接 | 初赛前 **32 强**晋级决赛；决赛现场决一二三等奖；优秀队进全国总决赛 | `[官方页面明确写出]` | S004 GitCode |
| 赛段结构 | 初赛/决赛各含"在线训练+正式比赛"；在线训练提交不计入评奖 | `[官方页面明确写出]` | S004 |
| 违规：Host 代算 | 将计算转移至 Host CPU 属违规，取消当前提交成绩 | `[官方页面明确写出]` | S004 比赛机制第3条 |
| 违规：空 kernel 占位 | 以空 kernel 占位绕过 NPU 计算要求属违规，取消成绩 | `[官方页面明确写出]` | S004 |
| 违规：抄袭/写死 | 仅泛述"独立完成作品""作品查验及违规处理以正式规则为准"，未给抄袭/写死结果的细化判定 | `[页面可访问但内容不完整]` | S004 公平参赛 |
| 公开统计 | 通过率 61%、通过人数 240、尝试人数 391 | `[官方页面明确写出]` | S002 |
| 参赛对象 | 四川/重庆/云南/贵州/西藏高校在校学生；同赛区组队 | `[官方页面明确写出]` | S004；S003 desc |

---

## 6. 「平台未开放 / 无法确认」清单（≥4 项，硬性验收）

1. **各测试点具体配置**：15 个测试点的 shape、dtype、epsilon 取值——testcase 详情端点 403，题目 API 不暴露。`[当前无法确认]`
2. **判题端是否含"允许失配比例 tol"**：题面 §五 只给逐元素阈值，未提 tol；本地有 `tol=1e-3`，判题端是否套用无法确认。`[当前无法确认]`
3. **5 次 iterations 的聚合方式**：`iterations=5` 已确认，但 `t` 取最好/平均/中位数未公开。`[当前无法确认]`
4. **判题 SoC 型号**：API 无 `soc_version`、公开页无；真实芯片未指定（模板默认 `dav-2201` 仅本地默认）。`[当前无法确认]`
5. **核数具体值**：仅 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 运行时取，硬件相关，不公开。`[当前无法确认]`
6. **编译器具体版本**：随 CANN 9.0.0，但未单列 cce 版本号。`[当前无法确认]`
7. **上传表单字段 / 单文件还是包 / 编辑器大小限制**：提交页需登录，公开接口未暴露。`[页面可访问但内容不完整]`→归为无法确认
8. **总提交次数上限**：仅每日 50 次，无总额证据。`[用户提供但未找到页面依据]`
9. **是否有 15 之外的隐藏测试点**：API 恰 15 个、题面写 15，但判题端私有扩展无法排除。`[当前无法确认]`
10. **抄袭/写死结果的具体判定细则**：GitCode 仅泛述。`[页面可访问但内容不完整]`

---

## 7. 对实现的直接影响

- **shape 只能运行时拿到**：`TensorGroupInfo` 运行时传入、无 Host 常量、无 tiling 结构体 → kernel 必须写**通用分支**，按 `numDims` 与每维 `shape` 解析 `(..., D)`，不能写死 `[1,64]`。rank 需覆盖 2D/3D/4D，且 D、batch、seq 范围广（D∈[64,32768]）。
- **D 非 32 对齐必须支持**：题面明确要求适配 D=192/576 等非 32 整倍数字节对齐场景 → 内存/数据搬运不能假设 32B 对齐。
- **fp32 精度要按 1e-4 自测**：官方 fp32 阈值（双 1e-4）严于本地模板示例（1e-3）。本地 `verify_result.py` 仅 fp16 用例，需**自行补充 fp32 用例并以 1e-4 校验**，否则线上 fp32 点可能不过。
- **必须处理 NaN/Inf 且不崩溃**：题面 §3.7 要求输入含 NaN/Inf 时按数学公式产出且不崩溃（Inf/Inf=NaN）。kernel 不应因特殊值异常退出（否则判题端 0 分甚至判异常）。
- **确定性要求**：同输入多次结果须完全一致 → 不能用随机、不能用未初始化 UB、归约顺序须固定。
- **性能即分数**：精度是准入门槛，性能决定得分；`t` 越小分越高，登顶（t=T）得 100/点。应优先保证 15 点全过，再压时间。
- **最后一次提交定成绩**：务必让**最后一次**提交 15 点全过且性能尽可能好（平台 `latest`）。临时调试提交会覆盖成绩。
- **单文件直调约束**：只能改 `kernel.asc` 内的 `run_kernel` 与核函数，不能加 `main()`、不能改 `main.asc` 接口；`epsilon` 已是独立 float 参数，无需从 TensorInfo 解析。
- **dtype 三选**：仅需处理 fp16/bf16/fp32（int32 视为题面残留，实际不测），但内部建议统一 FP32 计算后整体 cast 回原 dtype（与 golden 一致）。

---

## 8. 来源清单（编号段 S001–S030）

- [S001] CANNJudge 题目 API（AddRmsNormBias）| https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：接口/测试点/评分/环境原始字段 | 可支持结论：接口表、15 测试点、计分公式、iterations=5、code_template、cann_version=9.0.0、use_baseline=False、ranking_submission_mode=latest、start/end_time、testcase 配置不暴露
- [S002] CANNJudge 公开题面页（AddRmsNormBias）| https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：题面全文+公开统计 | 可支持结论：通过率61%/通过240/尝试391、vector CANN:9.0.0、题面 §3.3–§七 全文（含 §3.7 Inf/NaN、§四 确定性）
- [S003] CANNJudge 赛事 API（西南赛区初赛）| https://cannjudge.cn/api/contests/6a9a9295bf41025d601255a3 | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：赛事级配置与规则字段 | 可支持结论：title=2026年CANN挑战赛_西南赛区（初赛）、zone_id=2094722165106008066、visible_testcase_count=0、freeze_ranking=True(30)、signup_url、submit_enabled=False、scoring_rule=default、start/end_time
- [S004] GitCode 赛事规则/报名页（西南赛区）| https://competition.gitcode.com/competition/2094722165106008066/intro | GitCode（官方外部报名页，由 S003 signup_url 指向）| 访问 2026-09-12 | 等级 A | 状态 verified | 用途：提交额度/违规/赛程/晋级 | 可支持结论：每日最多50次、取最后一次成绩、Host CPU代算/空kernel占位违规、前32强晋级、10/17封榜18:00截止、参赛对象与组队
- [S005] 平台下发模板 kernel.asc | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：run_kernel 签名与直调形态 | 可支持结论：单文件直调、run_kernel 参数顺序与 epsilon 末位、核函数启动方式
- [S006] 平台下发模板 main.asc | .../main.asc | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：元信息结构与核数获取 | 可支持结论：TensorGroupInfo/TensorInfo 运行时传入、ACL_DEV_ATTR_VECTOR_CORE_NUM 取核数、epsilon=1e-5、单用例[1,64]fp16
- [S007] 平台下发模板 CMakeLists.txt | .../CMakeLists.txt | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：编译环境与 SoC 默认 | 可支持结论：find_package(ASC)、默认 SOC_ARCH=dav-2201（可被 NPU_ARCH 覆盖）、链接库
- [S008] 平台下发模板 run.sh | .../run.sh | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：本地执行与超时 | 可支持结论：timeout 120、单 case 校验
- [S009] 平台下发 golden 脚本 AddRmsNormBias.py | .../scripts/AddRmsNormBias.py | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：golden 计算口径 | 可支持结论：FP32 内部计算→整体 cast 回原 dtype、`y/rms*gamma` 除法语义、epsilon 默认 1e-5
- [S010] 平台下发 verify_result.py | .../scripts/verify_result.py | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：本地判定口径 | 可支持结论：rtol=atol=1e-3、tol=1e-3、仅 1 个 fp16 用例（与官方 fp32 1e-4 更严形成差异）
- [S011] CSDN 公告《2026 CANN 挑战赛报名启动》| https://blog.csdn.net/csdn_codechina/article/details/164369653 | 社区/官方发布 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛制三阶段与日程佐证 | 可支持结论：线上初赛/区域决赛/冠军挑战赛；9/5 发布、10/17 18:00 截止、10/16 23:59 报名截止
- [S012] 西南交通大学本科生院 赛事通知 | https://bksy.swjtu.edu.cn/info/1431/94591.htm | 高校通知 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛程与奖项佐证 | 可支持结论：报名至10/16 23:59、初赛9/5、作品10/17、区域决赛11/7；西南赛区面向高校学生
- [S013] 上海工程技术大学 赛事通知 | https://www.sues.edu.cn/91/d0/c26790a299472/page.htm | 高校通知 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛程佐证 | 可支持结论：报名7/29–10/16、初赛9/5–10/19、决赛10/20–11/7；聚焦算子性能优化
- [S014] 测试点详情端点（批量探测）| https://cannjudge.cn/api/testcases/11456 等 | 平台 | 访问 2026-09-12 | 等级 A（接口行为）| 状态 unavailable(403 Forbidden) | 用途：验证测试点配置是否公开 | 可支持结论：各测试点 shape/dtype/epsilon 不公开 → [当前无法确认]
- [S015] CANNJudge 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | 平台 | 访问 2026-09-12 | 等级 A（页面存在）| 状态 partial(需登录，未尝试) | 用途：上传字段/大小限制 | 可支持结论：提交表单字段与编辑器限制未公开 → [页面可访问但内容不完整]

> 注：S005–S010 为本地 `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/` 下平台下发文件，属官方仓库模板（A 级）。全程未登录、未模拟提交、未消耗提交次数、未修改 `源码/` 与 `提交/` 下任何文件，仅产出本报告。
