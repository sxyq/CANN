# Agent 1 报告：竞赛规则与提交接口调研（AddRmsNormBias）

> 调研代理：Agent 1（竞赛规则与提交接口）｜日期：2026-09-11｜工作区：macOS（无 CANN/NPU，全部结论不涉及真机编译/精度/性能验证）
> 结论标签：①官方页面明确写出 ②用户提供但未找到页面依据 ③页面可访问但内容不完整 ④根据接口或源码推断 ⑤当前无法确认
> 证据等级：A=官方题面/官方页面原文；B=官方 skill/仓库源码；C=社区转述；D=仅搜索摘要未核验

---

## 0. 覆盖范围与访问情况

| 目标 | 路径 | 访问结果 |
| --- | --- | --- |
| 题面页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | ✅ 公开可访问，全文抓取 |
| 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | ❌ 需登录（"需要先登录才能访问这个页面"） |
| 提交记录页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/status | ❌ 需登录（图形验证码+密码） |
| 排名页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | ✅ 公开可访问（235 条记录，12 页） |
| 比赛列表 | https://cannjudge.cn/contests | ✅ 公开可访问 |
| 西南赛区详情 | https://cannjudge.cn/public/op_challenge_xinan_prelim | ✅ 公开可访问 |
| 杭厦赛区详情（对照） | https://cannjudge.cn/public/op_challenge_hangxia_prelim | ✅ 公开可访问 |
| 题目 API | `GET https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85` | ✅ **无需登录**，返回完整 JSON（含 desc 全文、得分规则、testcases） |
| 提交结果 API | `GET https://cannjudge.cn/api/submissions/6aa202b62d3dd2c5aef620a8` | ✅ **无需登录**，返回逐点 time/precision_ratio/best_time |
| 模板下载 API | `GET /api/problems/{id}/package` | ❌ 未授权失败（需登录） |
| GitCode 报名页 | https://competition.gitcode.com/competition/2094722165106008066/intro | ✅ 可访问但**内容为空**（无赛事描述/时间线） |
| cannjudge-submit skill | https://gitcode.com/cann/cann-learning-hub/tree/master/skills/cannjudge-submit | ✅ README/SKILL.md/cannjudge_cli.py 均已读取 |

**重要突破**：题目 API 与提交结果 API 均无需登录即可访问，官方 JSON 里直接含有"得分规则"原文——这是本轮最有价值的一手证据，多项"待确认"据此闭环。

---

## 1. 输入输出接口（x/residual/gamma/bias/output/epsilon）

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| x / residual | tensor，(..., D)，2D/3D/4D，ND 格式；residual shape 与 x 完全一致 | ① | A（题面 3.3/3.4 表） |
| gamma / bias | tensor，(D,)，与 x 同 dtype；两 shape 一致但数值独立 | ① | A |
| output | tensor，shape 与 x 完全一致，dtype 与 x 一致 | ① | A |
| epsilon | ATTR float，默认 1e-5，取值通常 1e-5~1e-6 | ① | A |
| dtype | float16 / bfloat16 / float32 三选一，各输入输出 dtype 一致 | ① | A |
| rank 约束 | batch ∈ [1,8192]、seq_len ∈ [1,32768]、D ∈ [64,32768]（均正整数）；D 可非 32 倍数（例：D=192、576） | ① | A |
| 数值范围 | 不超出各自 dtype 原生表达范围 | ① | A |
| 与模板一致性 | 模板 main.asc 中 x/residual shape {1,64}、gamma/bias {64}、dtype 枚举 0=fp32/1=fp16/2=bf16、epsilon=1e-5、`run_kernel(...)` 签名与题面参数一一对应；availableCoreNum 取自 `ACL_DEV_ATTR_VECTOR_CORE_NUM` | ④ | B（本地模板） |
| int32 精度行 | 题面精度表含 int32"完全准确"行，但输入类型仅 fp16/bf16/fp32 → 判定为模板遗留，不影响实现 | ④ | A（对照题面 3.3 与五） |

**回答验收问题"是否与模板 main.asc 一致"**：是。接口参数顺序、shape 语义、dtype 枚举与模板完全一致。

---

## 2. 测试点数量、执行次数、隐藏点

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| **15 个测试点** | 官方 API desc 明文："本次比赛共15个测试点，所有case点精度全部通过才会计分"；排名页表格正好"测试点 1"~"测试点 15"共 15 列；API `testcases` 数组 15 条（ID 11456~11470，type=default）；实测一份 Pass 提交的 `result` 数组正好 15 项 | **① 官方明文** | A（来源 #58/#60/#59） |
| 每点执行次数 | problems JSON 有 `"iterations": 5`（每测试点执行 5 次）；但**time 字段取 5 次的何种统计（最优/均值/中位）官方未说明** | ④（字段存在）+⑤（统计方式） | A |
| 隐藏测试点 | 无任何证据：15 个 testcase 全部 type=default、baseline=null；Pass 提交 result 恰好 15 项；skill 文档称"平台不开放测试用例 API"（指配置不可见，非判点多于 15）→ 判题就是这 15 点 | ④（推断无隐藏，待登录最终确认） | B |
| 本地模板 | 本地 verify_result.py 只有 1 个 case（case0），AddRmsNormBias.py 注释写"与 JSON npu_cases 对应的 15 个测试用例"但仅列 1 例——本地模板只做单点冒烟，与平台 15 点无关 | ④ | B（本地模板） |

**回答验收问题"15 点是否页面明文"**：是。题面页本身未写数量，但**官方 API desc（等价官方页面数据源）明文写出 15 点**，且排名页、提交结果、testcases 数组三方互相印证。

---

## 3. 精度判定公式与阈值

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| 阈值 | fp32：相对误差<1e-4 且绝对误差<1e-4；fp16/bf16：相对<1e-3 且绝对<1e-3 | ① | A（题面五、精度判断规则） |
| 判定语义 | 官方只写"相对误差+绝对误差"双阈值，**未写明逐元素 isclose 还是全局指标、失配元素比例上限**；本地 verify_result.py 采用 `np.isclose(rtol=0.001, atol=0.001, equal_nan=True)` 逐元素判定 + 允许 0.1% 元素失配（tol=0.001） | ①（阈值）+⑤（判定实现细节） | A/B（本地脚本） |
| precision_ratio | 提交结果 API 每点有 `precision_ratio`（1.0=完全匹配，实测 Pass 点均为 1）；Pass 时不计分细项、每点仅记录 time | ①（字段存在） | A（来源 #59） |
| NaN/Inf | 题面 3.7 明文：输入含 NaN → 对应输出 NaN 且算子不崩溃；含 Inf → 按数学公式得 Inf/NaN，不崩溃 | ① | A |
| 确定性 | 相同输入多次执行结果必须完全一致（不含随机性） | ① | A |
| epsilon 判定端传入 | 由平台判题端调用 `run_kernel(..., float epsilon)` 传入；默认 1e-5；15 点是否各自不同 epsilon **无法确认** | ④/⑤ | A/B（模板签名） |

---

## 4. 性能评分方式（官方 API desc 原文，已实测验证）

**官方原文（题面 API desc 第六节"得分规则"）**：
> - 本次比赛共15个测试点，所有case点精度全部通过才会计分。
> - 每个测试点单独计分，逻辑如下（T为最优性能，t为当前提交性能）：100 / (1 + log₁.₅(t/T))
> - 排行榜显示的最终分数为所有case得分的均值。若得分计算一致，则以提交时间进行排序，提交越早，排名越高。

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| 单点计分 | `score_i = 100 / (1 + log₁.₅(tᵢ/Tᵢ))`，T 为该点**全局最优**（best_time，即排名页 TBest 行），t 为本次提交时间 | ① | A |
| 总分 | 15 点得分**算术平均**；同分按提交时间早者优先 | ① | A |
| 是否按排名计分 | 是：排行榜按"分数"列降序排列（分数=均值），无独立排名分 | ① | A |
| 性能占比 | 100 分制；精度全通过是计分前提，通过后分数完全由性能决定 → 性能=唯一计分维度 | ④ | A |
| 每点是否单独计时 | 是：每点独立 time（API 单位 **μs**，排名页显示为 μs/ms），并有独立 best_time；15 点耗时差异极大（1.47μs ~ 8.66ms），表明各点数据规模差异大 | ①（字段）+④（单位） | A |
| **公式实测验证** | 用排名第 1（分数 78.79）的 15 点时间套公式计算得 78.78（差 0.01 仅为 TBest 小数位截断），与排名页一致；另一份提交 API 数据算得 46.97，合理。**公式与"T=全局最优""均值得分"全部验证通过** | ①（经实测） | A（本轮验证） |
| use_baseline=false | 题目无基线（不同于天梯赛类），T 只来自参赛者提交 | ①（字段存在） | A |

---

## 5. 上传字段与上传格式

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| 模板类型 | problems JSON `"code_template":"npu_kernel_dev"`，题面标注"算子核函数工程(beta)" → **Direct Invocation 直调工程**（无 msopgen op_host/op_kernel 结构） | ① | A |
| 模板内容 | kernel.asc（含 `run_kernel(...)`，`__global__ __vector__` 核函数，被 main.asc #include）+ main.asc + run.sh + CMakeLists.txt + data_utils.h + scripts/{gen_data,verify_result,AddRmsNormBias}.py | ④ | B（本地模板） |
| skill 旧格式（不适用） | cannjudge-submit skill 的提交字段是 `tiling_h / tiling_key_h / host_cpp / kernel_cpp`，CLI 按 op_kernel/*_tiling.h、op_kernel/tiling_key_*.h、op_host/*.cpp、op_kernel/*.cpp 找文件——这是**旧式算子工程格式**，与本题直调模板结构不符 | ④ | B（来源 #66） |
| 提交结果字段 | submission API 返回含 `tiling_h/tiling_key_h/tiling_key_cpp/host_cpp/kernel_cpp` 与 `files` 数组（未登录显示"你没有权限查看该代码"）→ 服务端按文件内容存储提交 | ①（字段存在） | A（来源 #59） |
| 实际上传表单 | **需登录提交页才能确认**：是网页代码编辑器直接编辑 kernel.asc 后点"提交代码"（社区转述），还是 API 上传工程文本；本题 beta 直调格式是否新增 kernel_asc/files 字段——登录前无法确认 | ⑤（核心）+③（社区描述） | C（来源 #69） |
| 与题面/模板匹配度 | 判题直接调用 `run_kernel(x, info_x, residual, info_residual, gamma, info_gamma, bias, info_bias, output, info_output, availableCoreNum, stream, epsilon)` 并以 `<<<blocks,nullptr,stream>>>` 启动核函数 → **提交物应至少包含实现该 run_kernel 的 kernel.asc 内容** | ④ | B（本地模板） |

**回答验收问题"上传格式到底是什么"**：公开侧只能确认"npu_kernel_dev 直调工程（beta），提交内容为代码文件（至少 kernel.asc）"，skill 文档描述的 4 文件 tiling 字段属于旧格式、与本题不匹配；**最终上传表单字段必须登录提交页核实**（⑤）。

---

## 6. 判题 CANN 版本与 SoC

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| CANN 版本 | 9.0.0（题面标注 + API `cann_version`） | ① | A |
| Kernel 类型 | vector（题面标注 + API `kernel_pattern`）；模板 `__global__ __vector__`，经 AIV（Vector 核）执行 | ① | A |
| 模板默认 SoC | CMakeLists.txt 默认 `SOC_ARCH = "dav-2201"`（未设 NPU_ARCH 时） | ④ | B（本地模板） |
| dav-2201 是什么 | 官方 cannbot-skills npu-arch 映射：**DAV_2201 ↔ Ascend910B1~B4 / Ascend910B2C / Ascend910_93**，NPU_ARCH=2201；910B 系 UB≈192KB（社区旁证一致）→ 模板默认对应 Atlas A2 训练产品（910B 系列） | B | B（来源 #64）+C（来源 #68） |
| 实际判题 SoC 型号 | 平台未公开具体型号；最可能是 910B 系列（dav-2201），但**不能 100% 确认**（也可能运行时覆盖 NPU_ARCH） | ⑤（具体型号）+④（推断 910B 系） | — |
| 可用核数 | 模板运行时 `aclrtGetDeviceInfo(ACL_DEV_ATTR_VECTOR_CORE_NUM)` 获取，判题侧把值传入 run_kernel | ④ | B（本地模板） |

**回答验收问题"SoC 能否推断"**：能推断到 **Ascend 910B 系列（DAV_2201）**，依据是官方模板默认值 + 官方 skill 的架构映射表（B 级）；具体型号（B1/B2/B3/B4/910B2C/910_93）无法从公开页面区分（⑤）。

---

## 7. 初赛/决赛/违规规则

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| 初赛时间 | 2026/09/05 00:00 ~ 2026/10/17 18:00（题面/API start_time 2026-09-04T16:00Z = 北京 9/5 00:00；end 2026-10-17T10:00Z = 北京 10/17 18:00） | ① | A |
| 报名截止 | 10/16 23:59（社区报道；GitCode 官方页为空无法佐证） | ③ | C（来源 #67） |
| 决赛赛程 | 社区报道：10/20 决赛赛题发布、10/20-11/6 在线打榜、11/7 区域决赛（线下）、晋级名单 10/18-19 确认、另有冠军挑战赛阶段；GitCode 官方页无内容 | ③ | C（来源 #67） |
| 晋级规则 | 前多少名晋级/决赛是否换题：未公开 | ⑤ | — |
| 禁用手段/违规 | 官方无"禁用手段清单"页面；题面仅要求：确定性（多次执行一致）、与 PyTorch 组合实现对齐、性能要求；用户侧约束（AGENTS.md：禁止 Host 代算/空 Kernel/写死结果等）为最高优先，比官方更严 | ⑤（官方无明文）+①（题面确定性/对齐） | A |
| 各赛区题目不同 | 西南=AddRmsNormBias（vector）；杭厦=QuantMatmulReluQuant（cube）→ 本调研结论只适用于西南赛区 | ① | A（来源 #63） |
| 赛事规模（西南） | 344 人报名、12065 次提交、1 题（2026-09-11 抓取）；排名 235 条；题面页与排名页通过率数字不一致（53%/39/74 vs 64%/220/344），疑为页面缓存/口径差异 | ①（数据） | A |
| 奖金 | 社区报道：区域一等奖 20,000 元（1 支）、二等奖 10,000（5 支）、三等奖 2,000（15 支）；华为校招 QuickPass 权益 | ③ | C（来源 #67） |

---

## 8. 每日提交额度

| 项 | 结论 | 标签 | 证据等级 |
| --- | --- | --- | --- |
| 每天 50 次 | 用户提供；**所有公开页面/API/skill 文档均未见"50 次/日"字样**；cannjudge-submit skill 仅含糊提示"竞赛存在限制" | ② + ⑤ | C/D |
| 验证途径 | 登录提交页/提交后服务端返回（额度耗尽时的报错信息）；或平台帮助页 | ⑤ | — |

---

## 9. 仍无法确认清单（提交前必须闭环）

1. **上传表单实际字段**（登录提交页）：直调 beta 工程是网页编辑器提交 kernel.asc，还是 API 传文件内容；字段名是否复用 tiling_h/host_cpp/kernel_cpp 或新增 files。
2. **每日 50 次提交额度**：无任何公开页面依据，需登录后确认（含额度计数口径：通过/编译失败是否计入）。
3. **iterations=5 的时间统计方式**：每点 5 次执行取最优/均值/中位数未知（影响性能优化目标）。
4. **precision_ratio 判定实现细节**：isclose 的 rtol/atol 组合、失配比例上限（本地模板 0.1%，判题端是否一致未知）。
5. **15 点各自的 shape/dtype/epsilon 具体配置**：平台不开放测试用例 API（skill 明文），只能按题面约束范围做泛化实现。
6. **判题 SoC 具体型号**：推断为 910B 系列（dav-2201），具体 B1~B4/910B2C/910_93 未知；直接影响 msopgen `-c ai_core-<soc>` 与 UB 分块参数。
7. **晋级线（前 N 名）与决赛规则**：官方未公开；GitCode 报名页为空。
8. **是否存在排名之外的"作弊检测"**（如判题端对 Host 代算/写死结果的检测机制）：无公开信息。
9. **epsilon 是否每点不同**：默认 1e-5，判题端可传任意值（通常 1e-5~1e-6），实现须按传入值计算。

---

## 10. 本轮新增来源清单

| # | 名称 | URL / 仓库 | 版本 | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- | --- |
| 58 | CANNJudge 题目 API（AddRmsNormBias） | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 2026-09-11 抓取 | 2026-09-11 | 得分规则原文、15 测试点、iterations、code_template、testcases 15 条 | A |
| 59 | CANNJudge 提交结果 API（示例提交） | https://cannjudge.cn/api/submissions/6aa202b62d3dd2c5aef620a8 | 2026-09-11 抓取 | 2026-09-11 | 逐点 time/precision_ratio/best_time；提交文件字段；时间单位 μs | A |
| 60 | CANNJudge AddRmsNormBias 排名页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | 2026-09-11 抓取 | 2026-09-11 | 15 列测试点、TBest、分数；得分公式实测验证 | A |
| 61 | CANNJudge 比赛列表页 | https://cannjudge.cn/contests | 2026-09-11 抓取 | 2026-09-11 | 各赛区统计；西南 344 人/12065 次提交 | A |
| 62 | CANNJudge 西南赛区详情页 | https://cannjudge.cn/public/op_challenge_xinan_prelim | 2026-09-11 抓取 | 2026-09-11 | 赛区信息、GitCode 报名链接 | A |
| 63 | CANNJudge 杭厦赛区详情页（对照） | https://cannjudge.cn/public/op_challenge_hangxia_prelim | 2026-09-11 抓取 | 2026-09-11 | 各赛区题目不同（杭厦=cube 题） | A |
| 64 | cannbot-skills npu-arch skill（官方仓） | https://gitcode.com/cann/cannbot-skills（CSDN 转述：blog.csdn.net/gitblog_01165/article/details/151219465） | master，2026-06 | 2026-09-11 | DAV_2201=Ascend910B 系列映射 | B |
| 65 | cannjudge-submit SKILL.md | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/SKILL.md | master | 2026-09-11 | 提交流程、API、"平台不开放测试用例 API" | B |
| 66 | cannjudge_cli.py 源码 | https://raw.gitcode.com/cann/cann-learning-hub/raw/master/skills/cannjudge-submit/cannjudge_cli.py | master | 2026-09-11 | 旧格式提交字段（tiling_h 等）、模板结构 | B |
| 67 | CSDN：2026 CANN 挑战赛报名启动 | https://blog.csdn.net/csdn_codechina/article/details/164369653 | 2026-09-04 | 2026-09-11 | 赛制三阶段时间表、奖金、晋级概述 | C |
| 68 | CSDN：CANN学习中心 as_strided 实战 | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 2026-05-20 | 2026-09-11 | 旁证：erf 初赛 15/15；910B UB 192KB | C |
| 69 | CSDN：CANN 竞赛作品提交规范 | https://blog.csdn.net/gitblog_07213/article/details/151463322 | 2026-05-19 | 2026-09-11 | 提交流程（代码编辑页→提交→排名） | C |
| 70 | GitCode 2026CANN挑战赛西南赛区报名页 | https://competition.gitcode.com/competition/2094722165106008066/intro | — | 2026-09-11 | 官方页无赛事描述（空） | A(空) |
| 71 | CANNJudge 模板下载 API（尝试） | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/package | — | 2026-09-11 | 未授权失败；本地模板由用户提供 | A(受限) |

> 已同步追加至 `sources.md`（编号 58-71，避开并行 Agent 已占用的 54-57）。

---

## 11. 给主 Agent 的关键结论速览

1. **15 点=官方明文**（API desc 原文+排名页 15 列+testcases 15 条三方印证）；全部精度通过才计分。
2. **得分公式已闭环**：每点 `100/(1+log₁.₅(t/T))`，T=全局最优（best_time），总分=15 点均值，同分按提交时间；公式经排名第 1 实测验证（78.79↔78.78）。**性能是唯一计分维度**，正确性只是门槛。
3. **上传格式**：`code_template=npu_kernel_dev`（Direct Invocation，beta），提交物至少含实现 `run_kernel` 的 kernel.asc；旧 skill 的 4 文件字段不适用；实际表单待登录确认。
4. **SoC**：推断 Ascend 910B 系列（模板默认 dav-2201，官方映射 DAV_2201=910B1~B4/910B2C/910_93），UB≈192KB；具体型号待确认。
5. **判题 CANN 9.0.0、vector 类型**；接口与模板 main.asc 完全一致（含 epsilon 入参）。
6. **每日 50 次额度无公开依据**，仍待登录确认；精度判定实现细节、iterations=5 统计方式、15 点具体配置亦未公开。
