# Agent 01 调研报告：题面、规则与提交接口（AddRmsNormBias）

> 子代理：Agent 1（题面、规则与提交接口）｜访问日期：2026-09-12（北京时间）
> 方法：CANNJudge 公开页面抓取 + CANNJudge 公开只读 GET API + 前端 JS 逆向分析 + GitCode 赛事页探测 + 网络检索 + 本地模板逐字段核对。
> 约束遵守：全程未调用任何写操作 API（未提交、未上传、未登录），仅 GET 与页面抓取。
> 结论状态标注约定：【官方页面明确写出】｜【用户提供但未找到页面依据】｜【页面可访问但内容不完整】｜【根据接口或源码推断】｜【当前无法确认】。

---

## 1. 调研范围与平台覆盖情况

| 平台/入口 | URL | 覆盖结果 | 状态 |
| --- | --- | --- | --- |
| CANNJudge 题面页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | 完整抓取（题面全文+元信息+统计） | 【官方页面明确写出】 |
| CANNJudge 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | 跳转登录页，未登录不可见 | 【页面可访问但内容不完整】 |
| CANNJudge 排名页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | 完整抓取（15 列测试点+TBest+前 20 名） | 【官方页面明确写出】 |
| CANNJudge 赛事页 | https://cannjudge.cn/public/op_challenge_xinan_prelim | 完整抓取（1 题/344 报名/12065 次提交） | 【官方页面明确写出】 |
| CANNJudge 赛事列表 | https://cannjudge.cn/contests | 完整抓取（26 个赛事，含五大赛区对照） | 【官方页面明确写出】 |
| CANNJudge 提交记录页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/status | 跳转登录页 | 【页面可访问但内容不完整】 |
| CANNJudge 公开 API（GET） | `/api/contests/name/{name}`、`/api/problems/{id}`、`/api/problems/{id}/ranking`、`/api/submissions/contest/{id}/stats` | 全部成功返回 JSON（端点由前端 JS `services/shared.js`、`open.js` 逆向获得） | 【官方页面明确写出】 |
| GitCode 赛事页 intro/publish/ranking/qa | https://competition.gitcode.com/competition/2094722165106008066/intro 等 | 页面可达但内容为客户端异步渲染，未登录时全部显示"暂无赛事描述/暂无时间线/暂无QA"；HTML 内无业务数据；常见 API 路径探测均 301 回落页面 | 【页面可访问但内容不完整】 |
| 赛事公告（媒体通稿） | https://blog.csdn.net/csdn_codechina/article/details/164369653（AtomGit 官方账号 2026-09-04 发布） | 完整抓取（三阶段赛制+奖金+时间线） | C 级来源，标注为【根据接口或源码推断】级别以下的旁证 |
| 本地模板 | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/`（8 个文件全读） | 与平台 API 数据逐字段核对（见第 8 节） | B 级证据 |
| 本地既有文档 | `文档/competition-rules.md`、`文档/problem-add-rms-norm-bias.md` | 已读，与本次网页核实结果互证（差异见第 9 节） | 内部文档 |

**关键逆向发现（前端 JS，B 级证据）**：CANNJudge 为 SPA（`cannjudge.cn/app.js` 入口），公开只读 API 端点（提取自 `js/services/shared.js`、`js/pages/open.js`、`js/pages/problem/detail.js`、`js/pages/problem/editor.js`）：
- `GET /api/contests/name/{name}`、`GET /api/contests/{id}`
- `GET /api/problems/{token}`、`GET /api/problems/name/{name}`、`GET /api/problems/contest/{contestId}`
- `GET /api/problems/{problemId}/ranking`
- `GET /api/problems/{problemId}/template`、`GET /api/problems/{problemId}/package`（模板/工程包下载）
- `GET /api/submissions/contest/{contestId}/stats`（summary/problem 分组）
- `GET /api/submissions/{submissionId}`（提交详情轮询）
- `POST /api/submissions/submit`（提交，写操作，本次未调用）
- `POST /api/submissions/check-cla`、`POST /api/submissions/submit-to-gitcode`（GitCode PR 通道，写操作，本次未调用）
- `POST /api/submissions/{submissionId}/ban`（管理员封禁，写操作，本次未调用）

---

## 2. 题面数学语义

来源：题面 desc 原文（`GET https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85` 返回的 `desc` 字段，Markdown 原文，比 HTML 渲染页多出"六、得分规则"一节）。

1. **三步公式**（【官方页面明确写出】，URL 同上，2026-09-12）：
   - Step 1（残差加法）：`y_i = x_i + residual_i, ∀ i ∈ [0, D)`
   - Step 2（RMS 归一化）：`rms(y) = sqrt( (1/D)·Σ_{i=1..D} y_i² + ε )`；`z_i = y_i / rms(y) · gamma_i`
   - Step 3（偏置加法）：`output_i = z_i + bias_i`
   - ε 加在 mean(y²) 之后、开方之前；bias 加在归一化之后。
2. **参考实现**（【官方页面明确写出】）：PyTorch 组合 `rms = torch.sqrt(torch.mean(y**2, dim=-1, keepdim=True) + epsilon); normalized = y / rms * gamma; output = normalized + bias`；等价调用 `torch.nn.functional.rms_norm(x + residual, normalized_shape, weight=gamma, eps=epsilon) + bias`。要求"算子行为、计算结果需与上述 PyTorch 组合实现完全对齐"。
3. **确定性**（【官方页面明确写出】）：给定相同输入和属性，多次执行结果完全一致（不含随机性）。
4. **特殊值**（【官方页面明确写出】）：NaN 输入 → 输出对应位置 NaN，不崩溃；Inf 输入 → 按数学公式得 Inf 或 NaN（Inf/Inf=NaN），不崩溃。
5. **应用背景**：题面自述为 LLM 推理融合算子（AddRmsNorm + Channel Bias，提及 Gemma、部分 Qwen 变体）（【官方页面明确写出】）。

---

## 3. 接口与 dtype/shape 约束

来源：题面 desc 3.3–3.7 节（A 级）+ 模板 `kernel.asc`/`main.asc`（B 级）。

| 项 | 结论 | 状态 |
| --- | --- | --- |
| 输入张量 | `x (...,D)`、`residual (...,D)`（shape 与 x 完全一致）、`gamma (D,)`、`bias (D,)`（两者 shape 一致数值独立）；输出 `output (...,D)` 与 x 同 shape 同 dtype | 【官方页面明确写出】 |
| dtype | x/residual/gamma/bias/output：float16、bfloat16、float32；输出与输入一致；数据格式 ND | 【官方页面明确写出】 |
| rank | 2D(batch,D)、3D(batch,seq,D)、4D(batch,seq,heads,D) | 【官方页面明确写出】 |
| 维度取值 | batch ∈ [1,8192]；seq_len ∈ [1,32768]；D ∈ [64,32768]（均为正整数） | 【官方页面明确写出】 |
| epsilon | 属性 float，默认 1e-5，"取值通常为 1e-5 至 1e-6"；由判题端传入（模板 `run_kernel(..., float epsilon)` 末位参数） | 【官方页面明确写出】（默认值）+【根据接口或源码推断】（判题端可传不同值，题面未给出取值集合） |
| 非对齐 | D 可能为非 32 整倍数（题面举例 D=192、D=576），需兼容非 32B 对齐 | 【官方页面明确写出】 |
| 数值范围 | 输入数值不超出各自 dtype 原生表达范围 | 【官方页面明确写出】 |
| 直调入口签名 | `extern "C" void run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, GM_ADDR residual, const TensorGroupInfo& info_residual, GM_ADDR gamma, const TensorGroupInfo& info_gamma, GM_ADDR bias, const TensorGroupInfo& info_bias, GM_ADDR output, const TensorGroupInfo& info_output, int64_t availableCoreNum, aclrtStream stream, float epsilon)`，内部 `<<<blocks, nullptr, stream>>>` 启动 `__global__ __vector__` 核函数 | 【官方页面明确写出】（模板注释与题面模板，B 级；模板 URL 即工程包来源 `/api/problems/{id}/package`） |
| TensorInfo.dtype 枚举 | 0=float32, 1=float16, 2=bfloat16（模板 main.asc/kernel.asc 注释，另 3..11 为 int/uint/bool，本题不会用到） | 【官方页面明确写出】（模板内注释，B 级） |
| 每输入的张量组结构 | `TensorGroupInfo{tensors, numTensors}`，本题每个输入组 numTensors=1 | 【根据接口或源码推断】（模板 main.asc 用例仅示范单张量） |

---

## 4. 测试点与评分

### 4.1 测试点

- **15 个测试点**（【官方页面明确写出】）：题面 desc"六、得分规则"原文："本次比赛共15个测试点，所有case点精度全部通过才会计分。"（URL: `https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85`，2026-09-12）。排名页 15 列测试点与该陈述互证。
- 题目 API `testcases` 数组恰 15 条，全部 `type=default`、`baseline=null`，仅含内部 ID 11456–11470，**不含 shape/dtype/epsilon 参数**（【官方页面明确写出】接口结构；参数本身【当前无法确认】）。
- **每点执行 iterations=5**（【官方页面明确写出】）：题目 API 元数据 `iterations: 5`（同上 URL）。该值的统计口径（取均值/最优/中位数）页面未说明（【当前无法确认】）。
- `score_mode: 1`、`use_baseline: false`、`ranking_submission_mode: "latest"`（排名取该队最新一次提交）（【官方页面明确写出】，contest/problem API）。
- 15 个测试点的具体 shape/dtype/epsilon 组合：平台不开放（cann-learning-hub 官方 Skill 文档亦明示"平台不开放测试用例 API，必须设计泛化算子"，B 级，见来源登记 S08）。

### 4.2 精度判定标准

题面 desc"五、精度判断规则"（【官方页面明确写出】）：

| dtype | 相对误差 | 绝对误差 |
| --- | --- | --- |
| float32 | < 1e-4 | < 1e-4 |
| float16、bfloat16 | < 1e-3 | < 1e-3 |
| int32 | 完全准确，无误差（题面原文；本题输入不含 int32，模板遗留表述） |

- 判题端逐测试点返回 `precision_ratio`（通过率比率，全过=1）与 `testcase_status`（排名 API `result` 数组字段，【官方页面明确写出】）。
- 判题端是否采用与本地模板 `verify_result.py` 相同的 `np.isclose(rtol, atol)` + 0.1% 失配容忍算法：【当前无法确认】（本地模板仅保证 case0 的本地判定，见第 8 节）。

### 4.3 性能评分公式

题面 desc"六、得分规则"（【官方页面明确写出】）：

- 单测试点得分：`100 / (1 + log_{1.5}(t/T))`，其中 **T=最优性能（该测试点全局最优时间），t=当前提交性能**。
- 排行榜最终分数 = 15 个 case 得分的**均值**；得分一致时按**提交时间**排序，越早排名越高。
- 排名页实测 TBest 行（2026-09-12 抓取，μs/ms 级）：测试点 1–15 依次 1.47μs、2.16μs、2.54μs、6.80μs、5.70μs、12.31μs、16.70μs、31.36μs、50.61μs、47.59μs、133.40μs、76.68μs、411.34μs、3.75ms、8.66ms（URL: 排名页，见来源登记 S03）。
- 排名页第一名（馒头卡的队伍）总分 78.79（2026-09-08 提交）；共 235 条上榜记录（同页页脚），ranking API `total=258`（含更多状态的行）（统计口径差异见第 9 节）。
- 性能是唯一计分维度：精度全过是计分前置条件（【官方页面明确写出】）。

---

## 5. 上传格式与提交接口

来源：前端 JS 逆向（`js/pages/problem/editor.js`、`js/pages/problem/detail.js`，B 级）+ 模板工程（B 级）+ cann-learning-hub 官方 Skill 文档（B 级，S08）。

1. **工程形态**（【官方页面明确写出】）：题目 `code_template = "npu_kernel_dev"`（算子核函数工程 beta，题面页头部"算子核函数工程(beta)vector CANN：9.0.0"）；`kernel_pattern = "vector"`；`cann_version = "9.0.0"`。
2. **提交方式 A：在线编辑器提交**（主路径，【根据接口或源码推断】——接口行为由前端代码逆向，页面文案"提交代码"按钮已见）：
   - `POST /api/submissions/submit`，请求体 `{ problemId, userId, files: [{path, content}, ...], tiling_h, tiling_key_h, host_cpp, kernel_cpp }`。
   - `files` 为文件路径+内容数组；legacy 四字段（`tiling_h/tiling_key_h/host_cpp/kernel_cpp`）由文件树自动映射（`projectFilesToLegacyPayload`），直调工程下通常映射为 kernel 侧文件内容或空串。
   - **npu_kernel_dev 文件规则**（editor.js 前端校验，与后端 `src/utils/projectFiles.ts` 声明同步）：文件名仅允许根级 `*.asc` 与 `*.h`（1–128 字符，`[\w][\w.-]*` 模式）；**受保护不可改文件：`judge.asc`、`data_utils.h`、`main.asc`**；用户自建文件上限 20 个。
   - 提交后轮询 `GET /api/submissions/{submissionId}`，状态含 Running/Pass/Wrong Answer/Time Limit Exceeded 等（S08）。
3. **提交方式 B：GitCode PR 提交通道**（【根据接口或源码推断】）：`POST /api/submissions/submit-to-gitcode`，请求体 `{ problemId, gitcodeToken(PAT), teamInfo }`；前置 `POST /api/submissions/check-cla` 校验全体成员邮箱 CLA 签署；成功后创建/更新 PR。注意：本题 contest 元数据 `submit_enabled = false`、`submit_repo_url` 为空（contest API，【官方页面明确写出】字段值；含义为该通道在本赛事未启用，判断为【根据接口或源码推断】）。
4. **模板/工程包下载**（【官方页面明确写出】接口存在）：`GET /api/problems/{problemId}/package?userId=...`（页面按钮"下载空工程"）；本地模板目录名 `addrmsnormbias_problem_1742_template` 中的 `1742` 即题目 API 的 `ID` 字段。
5. **单文件 kernel.asc 语义**（【根据接口或源码推断】）：本地模板中 `kernel.asc` 是被 `main.asc` `#include` 的实现文件；提交时以 `files` 数组中的 `kernel.asc` 内容上传；不存在"上传 zip"的表单证据（cann-learning-hub 旧版四字段接口与 msopgen 工程配套，本题直调模板走 files 数组）。
6. **提交内容与判题统计展示**（【官方页面明确写出】）：每次提交逐测试点返回 `time`（μs）、`precision_ratio`、`testcase_status`（ranking API `result` 字段实测）。

---

## 6. 时间线与赛制规则

### 6.1 平台登记时间（A 级）

| 项 | 值 | 来源/状态 |
| --- | --- | --- |
| 赛事 | 2026年CANN挑战赛_西南赛区（初赛），`name=op_challenge_xinan_prelim`，`zone_id=2094722165106008066`（与赛事 ID 一致） | 【官方页面明确写出】（contest API + 赛事页） |
| 赛事开始 | 2026-09-04T16:00:00Z = 北京时间 2026-09-05 00:00:00 | 【官方页面明确写出】（contest API `start_time`；题面页"题目开始 2026/09/05 00:00:00"互证） |
| 赛事结束 | 2026-10-17T10:00:00Z = 北京时间 2026-10-17 18:00:00 | 【官方页面明确写出】（同上） |
| 报名 | `signup_required=true`，`signup_mode=external_link`，报名外链即 GitCode 赛事页 | 【官方页面明确写出】（contest API） |
| 报名截止 | 2026-10-16 23:59 | 【根据接口或源码推断】以下调级：C 级媒体通稿明写（S09），平台自身页面未明示报名截止时刻 |
| 组队模式 | `organization_mode=gitcode_team`（GitCode 团队），`huawei_contest_type=ict` | 【官方页面明确写出】（contest API） |
| 封榜 | `freeze_ranking=true`，`freeze_duration=30`（分钟）：前端逻辑 `freezeTime = endTime - 30*60000`，即**北京时间 10-17 17:30–18:00 冻结排名** | 【官方页面明确写出】（contest API 字段）+【根据接口或源码推断】（分钟语义由 `services_shared.js` 547–555 行逻辑确认） |
| 排名模式 | `ranking_mode=score`、`show_total_ranking=true`、`scoring_rule=default` | 【官方页面明确写出】（contest API） |
| 题目创建/修订 | contest 创建 2026-09-04 09:42 UTC；题目创建 2026-09-04 10:16 UTC（soc_version=null, ddk_version=null）；desc 于 09-04/09-05 更新 3 次（内容微调，公式未变）；09-05 至 09-11 新增 4 名赛事管理员 | 【官方页面明确写出】（contest API `edit_logs`，11 条） |
| 赛事统计（09-12） | 1 题；344 人报名；12065 次提交；stats summary：participantCount=380（提交人数）、submissionCount=14299（判题次数，与页面 12065 口径不同，疑含非计入提交）；题面页另一口径：通过率 53%（39/74）；排名页口径：64%（220/344） | 【官方页面明确写出】各数值本身；**多口径不一致**，谁为准【当前无法确认】 |

### 6.2 三阶段赛制与决赛（C 级：AtomGit 官方账号通稿，S09）

| 阶段 | 时间 | 事项 |
| --- | --- | --- |
| 线上初赛 | 9/5 赛题发布；9/5–10/16 在线答题开发优化；10/16 23:59 报名截止；10/17 18:00 初赛作品提交截止；10/18–19 晋级名单陆续确认 | 【用户提供但未找到页面依据】→ 已找到 C 级依据（官方账号通稿），平台页未明示决赛安排 |
| 区域决赛 | 10/20 赛题发布；10/20–11/6 在线打榜与持续优化；11/7 区域决赛；线下安排另行通知 | 同上 |
| 冠军挑战赛 | 区域决赛优秀队伍受邀参加，时间另行通知 | 同上 |
| 奖金（每赛区） | 一等奖 20000 元×1 队、二等奖 10000 元×5 队、三等奖 2000 元×15 队；五大赛区（杭厦、西南、西北、京津东北、上合）合计 50 万 | 同上 |
| 人才权益 | 华为校招 QuickPass（机考绿卡/面试绿卡，以最终通知为准） | 同上 |

### 6.3 判题环境

- **CANN 版本 9.0.0**（【官方页面明确写出】：题目 API `cann_version`、题面页头部、tags `['vector','CANN：9.0.0']`）。
- **Kernel 类型 vector**（【官方页面明确写出】：`kernel_pattern=vector`，题面页头部）。
- **SoC 型号**：题目与 contest API 的 `soc_version`/`ddk_version` 均为 null，官方未明示判题芯片（【当前无法确认】）；唯一官方物料证据是模板 `CMakeLists.txt` 默认 `SOC_ARCH="dav-2201"`（B 级，【根据接口或源码推断】判题按此架构编译）。

---

## 7. 违规与每日额度

1. **平台明文规则**（题面 desc"四、规则要求"，【官方页面明确写出】）仅有两条：数值一致性（确定性）与性能要求；**题面无独立"违规/作弊"条款**。
2. **平台反作弊机制**：存在管理员封禁提交接口 `POST /api/submissions/{id}/ban` 与管理端页面（problem_detail.js 引用）（【根据接口或源码推断】机制存在；触发条件未公开，【当前无法确认】）。
3. **每日提交额度"每天最多 50 次"**：【用户提供但未找到页面依据】。且存在**反向证据**：contest stats API（09-12）显示单队最高已提交 558 次（LO_Net 的队伍）、537/476/458/333 次者众，比赛自 09-05 起仅 8 天，平均约 70 次/天 > 50/天，说明在线判题提交**不受**"每日 50 次"限制（该推断为【根据接口或源码推断】D+级：由公开统计反推）。真实限额若存在，数值【当前无法确认】。
4. **决赛阶段禁用手段**（算子融合/多核策略/特殊指令限制）：【当前无法确认】（决赛细则未公开）。
5. 本项目合规边界（禁止 Host 代算、空 Kernel、写死结果等）来自 `AGENTS.md` 用户约束，非平台题面条款（用户约束最高优先，照常执行）。

---

## 8. 与本地模板的逐字段核对表

模板目录：`/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/`（8 文件全读）。核对基准：题面 desc（A 级）+ contest/problem API（A 级）。

| 模板字段 | 模板值 | 平台依据 | 一致性 |
| --- | --- | --- | --- |
| 工程名（CMake project） | `add_rms_norm_bias_custom` | 题面 `name=addrmsnormbias`、`title=AddRmsNormBias`、`ID=1742`（目录名 `problem_1742`） | 一致（命名惯例加 `_custom`） |
| `kernel_pattern` | 模板注释 `__global__ __vector__` 核函数 | 题目 API `kernel_pattern=vector`、tags | 一致【官方页面明确写出】 |
| CANN 版本 | run.sh 要求 `ASCEND_HOME_PATH`；CMake 链接 tiling_api/register/platform 等 | `cann_version=9.0.0` | 一致【官方页面明确写出】 |
| SOC_ARCH | CMake 默认 `dav-2201`（可用 `NPU_ARCH` 覆盖） | API `soc_version=null` | 模板默认即唯一官方线索（B 级） |
| `run_kernel` 签名 | `(x, info_x, residual, info_residual, gamma, info_gamma, bias, info_bias, output, info_output, availableCoreNum, stream, epsilon)` | 题面 3.3 输入输出表（x/residual/gamma/bias/epsilon/output） | 参数集合与顺序一致【官方页面明确写出】 |
| dtype 枚举 | `0=fp32 1=fp16 2=bf16`（main.asc 注释，3–11 为整数类） | 题面 3.3 支持数据类型 fp16/bf16/fp32 | 一致（前三即本题 dtype 集） |
| 默认用例（main.asc） | x/residual/output shape [1,64] fp16（dtype=1）；gamma/bias shape [64] fp16；epsilon=1e-5 | 题面：D∈[64,32768] 下界 64；epsilon 默认 1e-5；fp16 合法 | 一致（示例仅覆盖最小 D） |
| 核数获取 | `ACL_DEV_ATTR_VECTOR_CORE_NUM` → availableCoreNum | 题面 vector 类型 | 一致 |
| 同步超时 | `aclrtSynchronizeStreamWithTimeout(stream, 3000)` 3 秒 | 无平台依据（模板自带） | 判题端超时阈值【当前无法确认】 |
| golden 实现（AddRmsNormBias.py impl） | FP32 内部计算：y=x+r；rms=sqrt(mean(y²)+eps)；out=y/rms*g+b；输出 cast 回原 dtype | 题面 3.1/3.2 公式与 3.6 数值等价 | 逐式一致【官方页面明确写出】 |
| 本地验证容差（verify_result.py case0） | output fp16，rtol=0.001、atol=0.001、tol=0.001（失配元素比例容忍 0.1%，`np.isclose(..., equal_nan=True)`） | 题面 fp16 相对/绝对 <1e-3 | rtol/atol 一致；**额外 0.1% 失配容忍为模板本地逻辑，判题端是否同样【当前无法确认】**；`equal_nan=True` 与题面 NaN→NaN 一致 |
| 本地数据生成（gen_data.py） | 仅 1 个 case（case0）；x/residual~U(-2,2) fp16 [1,64]；gamma~U(0.8,1.2)；bias~U(-0.3,0.3)；eps=1e-5；seed=42 | 判题 15 点不公开 | 本地模板≠判题全集，泛化实现必须（S08 亦强调） |
| run.sh 流程 | set_env → cmake/make → gen_data → 拷 case0 → 120s 超时运行 → verify | 无平台依据（本地自测用） | 一致用途 |
| data_utils.h | ReadFile/WriteFile（fileSize 必须等于 bufferSize） | 无平台依据（模板自带） | — |
| 受保护文件 | 模板无 judge.asc（仅 kernel/main/data_utils/run.sh/CMake/scripts） | 前端受保护集合 {judge.asc, data_utils.h, main.asc}（editor.js） | data_utils.h/main.asc 在模板中存在；judge.asc 为判题端在线工程独有文件【根据接口或源码推断】 |

**核对结论**：本地模板与平台题面在公式、接口签名、dtype、D 下界、epsilon 默认值、精度阈值上**全部一致**；模板自带的 3s 超时、0.1% 失配容忍、单 case 数据均为本地逻辑，不能外推为判题端行为。

---

## 9. 结论汇总

### 9.1 已确认（A 级为主）

1. 题面三步公式、PyTorch 参考实现、特殊值与确定性要求（题面 desc API 原文）。
2. 接口：4 输入 + 1 属性 + 1 输出；fp16/bf16/fp32；ND；2D/3D/4D；batch∈[1,8192]、seq∈[1,32768]、D∈[64,32768]；epsilon 默认 1e-5（题面 desc）。
3. 15 个测试点、全部通过才计分；单点 `100/(1+log₁.₅(t/T))`；总分=均值；同分早提交优先；iterations=5；ranking_submission_mode=latest（题面 desc + problem API）。
4. 精度阈值：fp32 <1e-4；fp16/bf16 <1e-3；int32 完全准确（题面 desc）。
5. CANN 9.0.0；vector 核函数；直调工程 npu_kernel_dev；平台编号 ID=1742；题目 ID/contest ID/zone_id 与项目既有记录一致（contest/problem API）。
6. 时间：2026-09-05 00:00 至 2026-10-17 18:00（北京时间）；封榜为结束前 30 分钟（10-17 17:30–18:00）（contest API + 前端逻辑）。
7. 提交主路径为在线编辑器 `POST /api/submissions/submit`（files 数组，.asc/.h 文件，judge.asc/data_utils.h/main.asc 受保护，自建文件≤20）；模板下载 `/api/problems/{id}/package`（前端 JS 逆向，B 级）。
8. GitCode PR 通道存在但本赛事 `submit_enabled=false`（contest API）。

### 9.2 修正既有文档的两点

1. `文档/competition-rules.md` 中"每天最多提交 50 次（用户提供）"：**未找到任何页面依据，且与公开统计数据矛盾**（单队 8 天 558 次）。建议提交前以一次试探性提交实测或登录提交页确认，勿以 50/天做额度规划。
2. 该文档"SoC 型号未在题面明确给出"：本次进一步确认 contest/problem API 的 `soc_version`/`ddk_version` 显式为 null，模板默认 `dav-2201` 是唯一官方物料证据——维持"推断"级，真机编译按 dav-2201。
3. `文档/problem-add-rms-norm-bias.md` 第 8 节"得分公式（A 级，已实测验证）"：本次在题面 desc 原文中找到**逐字依据**（`100/(1+log₁.₅(t/T))`、均值、同分按提交时间），升级为 A 级页面明文，与既有结论一致，无需修改。
4. 题面页头部统计（53%、39/74）与赛事页/排名页（64%、220/344）及 stats API（380 人/14299 次）三个口径不一致：均为平台渲染值，以 API 原始值为准做记录，口径含义【当前无法确认】。

### 9.3 无法确认项（提交前需登录或实测闭环）

1. `/submit` 提交页登录后的实际表单与文案（额度提示、文件上传方式是否有"上传文件"入口）。
2. 判题端 15 个测试点的 shape/dtype/epsilon 配置（API 不含参数）。
3. iterations=5 的统计口径（均值/最优）。
4. 判题端精度判定算法是否等同本地 `verify_result.py`（含 0.1% 失配容忍）。
5. 判题 SoC 型号官方名称与 `dav-2201` 的对应产品（Atlas A2 系列推断见 agent-02/07 范围）。
6. 每日提交限额的真实数值（若存在）。
7. GitCode 赛事页（intro/publish/qa/ranking）登录后的公告内容。
8. 决赛/冠军挑战赛的正式规则文本（当前仅 C 级通稿）。

---

## 10. 来源登记

| # | URL | 标题/内容 | 访问日期 | 用途 | 证据分级 | 证据状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S01 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | CANNJudge AddRmsNormBias 题面页（HTML） | 2026-09-12 | 题面全文、元信息、通过率统计 | A | verified |
| S02 | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 题目详情 API（desc 原文 Markdown、testcases 15 条、iterations/score_mode/kernel_pattern 等） | 2026-09-12 | 题面权威原文、测试点与计分元数据 | A | verified |
| S03 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | 提交排名页（15 列测试点、TBest、前 20 名、共 235 条） | 2026-09-12 | 计分与 15 测试点互证、TBest 数据 | A | verified |
| S04 | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/ranking | 题目排名 API（result: time/precision_ratio/testcase_status 逐点） | 2026-09-12 | 判题结果字段结构 | A | verified |
| S05 | https://cannjudge.cn/api/contests/name/op_challenge_xinan_prelim | 赛事元数据 API（时间/封榜/报名/zone_id/edit_logs 11 条） | 2026-09-12 | 赛事规则、时间线、配置字段 | A | verified |
| S06 | https://cannjudge.cn/public/op_challenge_xinan_prelim 与 https://cannjudge.cn/contests | 赛事页与全部赛事列表（26 赛事、五大赛区对照） | 2026-09-12 | 赛事规模、赛区结构 | A | verified |
| S07 | https://cannjudge.cn/api/submissions/contest/6a9a9295bf41025d601255a3/stats | 赛事提交统计 API（summary: 380 人/14299 次；逐队 submissionCount，最高 558） | 2026-09-12 | 额度反证、参与规模 | A | verified |
| S08 | https://blog.csdn.net/gitblog_00410/article/details/143789539（原仓库 https://gitcode.com/cann/cann-learning-hub） | CANN/cann-learning-hub：CANNJudge 算子提交 Skill（提交 API、四字段、状态枚举、平台不开放测试用例） | 2026-09-12 | 提交接口旁证、泛化必要性 | B（官方仓库文档的媒体转载，原文仓库未直接抓取） | partial |
| S09 | https://blog.csdn.net/csdn_codechina/article/details/164369653 | 最高 2 万元奖金＋华为校招绿卡！2026 CANN 挑战赛报名启动（AtomGit 官方账号，2026-09-04） | 2026-09-12 | 三阶段赛制、奖金、报名/决赛时间线 | C（官方社区账号通稿转载） | verified |
| S10 | https://cannjudge.cn/app.js 及 js/services/shared.js、js/pages/open.js、js/pages/problem/editor.js、js/pages/problem/detail.js、js/pages/problem/submission.js（前端源码） | CANNJudge 前端模块（API 端点、npu_kernel_dev 文件校验、提交 payload、封榜逻辑、GitCode 提交表单） | 2026-09-12 | 提交接口与限额逻辑逆向 | B（平台自身源码） | verified |
| S11 | https://competition.gitcode.com/competition/2094722165106008066/intro（及 /publish、/ranking、/qa） | GitCode 赛事页（未登录渲染为空，无业务数据；API 探测 301） | 2026-09-12 | 报名入口存在性确认 | A（页面本身）/内容 | unavailable（内容需登录/异步加载） |
| S12 | 本地：/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/（kernel.asc、main.asc、run.sh、CMakeLists.txt、data_utils.h、scripts/*.py） | 官方下载模板（工程包） | 2026-09-12（本地读取） | 接口签名、dtype 枚举、本地判定逻辑核对 | B | verified |
| S13 | 本地：文档/competition-rules.md、文档/problem-add-rms-norm-bias.md | 项目既有规则/题面分析备忘 | 2026-09-12（本地读取） | 与网页核实结果互证、差异修正 | 内部文档 | verified |

> 注：S02/S04/S05/S07 为对 CANNJudge 公开只读 GET API 的直接调用返回，均无副作用；S10 为公开静态 JS 文件下载分析；全程未执行 POST/登录/上传。
