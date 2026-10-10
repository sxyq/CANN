# CANN AddRmsNormBias

## W4 历史归档

W4 的 15 条路线已在证据归档后退役。当前成绩表记录了 26 次真实 Judge 提交；旧 30 次目标保留为历史目标，不约束新五路线阶段。已受理任务的最终结果、源码、提交号和失败材料继续保存在原位置。

当前任务按最新用户指令重建五条路线。N01-N04 仍需源码去重核验，N05 的替代方向尚待研究结果；此处不预先登记路线名、Candidate 或实验成绩。W4 的研究、成绩和工作树记录只作历史证据。

## 当前唯一活动路线：N01-N05

当前唯一活动路线固定为：

- `N01 REGISTER-LIVE-RANGE-X`：研究宽行 Vector 临时 FP32 值的寄存器生命周期，首版聚焦 `ProcessWideLowPrecision` 与 `ProcessWideFp32FullCacheRows` 的定义、最后使用和复用位置。
- `N02 CODE-FOOTPRINT-ICACHE-X`：研究融合内核热点代码体积与指令取指，首版只比较辅助函数的 inline/noinline 与布局变化，不重复模式分派。
- `N03 VECTOR-ILP-DUALCHAIN-X`：研究 Vector 独立操作的指令级并行，保持算术、访存、同步和计算量不变，复用现有可工作的构建入口。
- `N04 GM-CACHE-STREAMING-X`：研究大工作集 GM/L2 访问策略，首版使用已确认可用的 `SetL2CacheHint` 做单因素比较，不调整参数驻留、DMA 描述符或跨行预取。
- `N05 SCALAR-ADDRESS-GENERATION-X`：研究 `ProcessWideLowPrecision` 的 Scalar 地址生成，使用递增索引与行列进位减少可避免的整除和重复地址计算，保持 Tile 顺序、算术和 DMA 语义。

五条路线均以 `R31B V011 / Official 45.16` 作为性能对照。W2、W3、W4 只作为历史研究资料，不再进入当前活动路线池。

Main 负责协调、跨路线去重和状态汇总；Route Agent 负责自己的源码、真实 Compile、Correctness、Local、结果保存和版本提交。每条路线固定为 `1 Route = 1 Agent = 1 Context = 1 Branch = 1 Local Worktree`。Agent 只能在自己的本地工作树和明确授权的公共只读资料中读取、修改和提交，禁止读取其他 Route 的工作树、分支源码或未公开实验；跨路线比较由 Main 使用授权的只读审计完成。

五条路线的主要优化机制不得重叠。完整源码相似度和有效修改相似度目标均不超过 80%，必须报告真实数据，不得通过变量改名、格式变化、注释或无意义重写制造差异。当前任务不恢复已取消的额外 Official 提交条件，不新增调度器、Dashboard、常驻隔离工程或重复等待。普通 Route Revision 直接沿用当前上下文；任务恢复或规则变化时按现行规则刷新。

本仓库服务于 2026 CANN 挑战赛西南赛区 `AddRmsNormBias`。正式主线在仓库根目录；`归档/` 只保存历史资料，不承担当前执行入口。

## 规则优先级

1. 当前会话与用户明确指令；
2. 本文件与当前角色 Skill；
3. `项目规则/` 中对应主题的唯一说明；
4. `技术路线/`、`调度/` 中的正式事实记录；
5. `归档/` 中的历史材料只作证据参考。

当前主题入口：

- Main：`.agents/skills/cann-main-orchestrator/SKILL.md`
- Route：`.agents/skills/cann-route-executor/SKILL.md`
- Support：`.agents/skills/cann-support-research/SKILL.md`
- Record：`.agents/skills/cann-record-owner/SKILL.md`
- Online：`.agents/skills/cann-online-owner/SKILL.md`
- 兼容路由：`.agents/skills/cann-mainline/SKILL.md`

## Historical Portfolios

W4 已退役；W3 与 W2 也只作历史来源。旧 W4 安排仅保留在归档记录中，不能覆盖当前用户指令或新五路线研究结果。

`ACTIVE PORTFOLIO W3` 保留为历史记录来源，仅包含以下五条路线：

- `R1 UB-BANK-LAYOUT-CHAMPION-X`
- `R2 ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X`
- `R3 TINY-MINIMAL-KERNEL-CHAMPION-X`
- `R4 MULTIROW-PANEL-RMS-CHAMPION-X`
- `R5 CROSSROW-FULL-PIPELINE-CHAMPION-X`

`OLD W2 HISTORICAL ONLY`：`SYNC`、`CASE47`、`CASE14`、`SELECTIVE`、`TINY-FIXED-OVERHEAD` 只属于历史 W2，不能用于当前状态、路线选择或执行判断。

历史 W3 事实仍为 Local facts only，不能写成 Official：R1 `V001-V010` 无稳定 Local Best；R2 至 `V016`，Best `V015`，target `-18.0207%`，control `-19.7413%`；R3 `V001-V010` 无稳定 Best；R4 至 `V013`，Best `V009`，FP16 `128x12288`，`-0.4954%`；R5 至 `V016`，Best `V012`，`10.040 us`，`-2.994%`。

## W4 Archive Reference

旧 W4 阶段的 worktree、版本额度、回执和实验记录继续保留在归档位置；旧 30 次 Judge 目标不作为新五路线启动条件。

## Online Experimental Submission Policy

本节是本机 CANN Online 实验提交的唯一执行规则。它只调整 Online 提交流程，不改变服务器资源安全、用户数据保护、Route worktree 隔离及其他实验流程。

### 用户授权与范围

- 用户在当前任务中明确点名 Candidate 并要求 Online 提交，即授权本次提交；不再二次询问，也不需要 Planning、Main、Route Agent 或其他角色另行批准。
- 只提交用户明确指定的 Candidate，不扩展版本或路线范围。用户未指定的 Candidate 不提交。
- 当前任务执行者可按用户指令直接调用现有官方提交脚本。角色分工不增加其他执行步骤。

### 本地结果说明

除非用户明确要求，本机 Online 提交不要求预先完成 Compile、Correctness、Golden 对比、Parent/Candidate 比较、Local Score、性能复测、噪声验收、全 shape 覆盖、CANN/NPU 硬件验证、`VALID_LOCAL_BEST=YES`、`ONLINE_CANDIDATE_READY=YES`、超过历史 Official Champion、历史 Git 来源核对、完整实验文档或人工审阅。

上述结果可作为背景信息保留；不得仅因结果为 `NOISY`、`LOCAL_REJECTED`、`PARTIAL_LOCAL_ONLY`、`MEASUREMENT_BLOCKED` 或 `ONLINE_READY=NO` 而跳过用户明确指定的提交。官方评测中的编译、正确性或分数结果均如实保存。

### 唯一提交入口

- 仅使用 `工具/cannjudge-submit.mjs`；不得建立第二套脚本或提交链，也不得为提交修改 Candidate 源码。
- 按用户指定的文件和顺序调用脚本。保留脚本自身的鉴权、源码格式与来源身份处理，以及 Judge 平台强制执行的要求；平台直接拒绝时记录真实错误，不绕过平台。
- 用户指定文件可来自 ZIP 或工作区现有文件。只提取提交所需文件，提交内容不得混入其他版本。
- 工作区已有未提交改动时，仍可提交用户明确指定的 Candidate；不清理、覆盖或重置已有改动。官方脚本自身产生的提交事件记录属于该提交流程，其余 Dashboard 内容保持不动。

### 执行与结果记录

1. 定位用户指定的源码文件，并按既有脚本参数执行。
2. 获取并保存真实提交响应、Submission ID、提交时间、官方状态、Official Score（若已返回）及完整命令日志。
3. 依照用户指定顺序继续后续 Candidate。单项失败不影响其余指定项。
4. 评测仍在进行时记为 `PENDING`；不得重复提交，除非已确认官方未受理首次请求。
5. 按用户要求保存官方结果到既有结果位置。Local Score 仅作本地参考，不替代 Official Score。

本节与角色表中限制用户授权 Online 操作的内容不一致时，以本节和用户当前明确指令为准；Route Candidate 编辑权、服务器保护及 Git 安全规则不因此改变。

## 角色表

| 角色 | 负责 | 不负责 |
|---|---|---|
| Planning / Review Layer | Route 生命周期、路线组合、当前授权范围之外的 Online 决策 | — |
| Main | C2C 协调、Child 管理、状态汇总；本地只读 Git、源码和实验证据审计；按用户明确请求修改根 `AGENTS.md` 或调用官方 Online 提交脚本 | 其他文件写入；运行 Compile、Correctness、Local、NPU 或身份计算 |
| Route Agent | 一个 Route 的 Candidate、Compile、Correctness、Local、实验 Git | 其他 Route、共享记录、Route 生命周期、正式 Online |
| Support Agent | 跨 Route 研究、群聊和公开资料取证、硬件/API 分析 | Candidate、Route ownership、共享记录、正式 Online |
| Record Owner | SLOT-3；主工作树内获准的规则及四份共享记录 | Candidate、实验、Route ownership/决策、Dashboard、归档及其他工作树 |
| Online Owner | 按用户明确授权调用官方提交脚本并转交评测结果 | Candidate、Route 决策、共享成绩写入 |

Main 的项目状态来自 Child/Record/Online receipt、权威共享状态及获准的本地只读审计；用户明确授权或 Planning 指令规定范围，测量数字仍须真实结果支持。

用户指定的 Online Candidate、授权、执行和结果记录按本文 `Online Experimental Submission Policy` 处理；本地测量值不用于跨路线排序或代替官方分数。

Main 的当前执行说明见 `项目规则/执行约定.md` 及相邻章节。新五路线按当前用户任务和本轮更新后的有效规则执行。

## 状态读取

Main 在开始任务、恢复上下文或收到新指令时，按需读取适用规则和共享状态。W4 控制文件只在核对旧路线时作为历史来源；涉及 server3、Local 或 Git 时读取对应规范。来源冲突时报告 `STATE_SYNC_GAP`，不得猜测或写入共享记录。

Main 可本地只读核对 Git、源码、实验证据和共享状态；按用户明确要求可编辑根 `AGENTS.md`，并通过现有官方提交脚本执行指定 Online。其他文件写入、Compile、Correctness、Local、NPU 和身份计算仍不属于 Main 的常规职责。本轮无需 workspace_info，工作区由本地 Git 确认。

Route 内的普通下一 Revision 复用当前 owner/context/branch/worktree；结束本次 Route 任务后按安全交接关闭旧 Agent，下一 Route 使用 fresh context；Main 不逐项微管理 Revision。

## PERSIST EXECUTION DISCIPLINE

Route 执行必须闭合为：

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

正常完成记录 numeric Local score/delta、Parent/Candidate raw samples 与 medians、shape/dtype、device、free HBM、load note 和 current best。Compile 或 Correctness 失败也保留证据、Git commit/status，并记录 `VERSION_RECORD_EVENT`。Record Owner 记录每个版本事件，包括负结果、失败结果和非 Local Best，不得发明缺失数据。

新五路线的 Route Agent 在每个新 Revision、resume、Child restart、reconnect、context restore 或 long interruption 后，必须在 Candidate edit 前重新读取本文件、对应 Skill、`项目规则/实验总则.md`、`项目规则/执行约定.md`、`项目规则/服务器实验规范.md`、`项目规则/本地性能测试规范.md` 和 `项目规则/Git工作流程.md`，先发送 `RULE_REFRESH_RECEIPT`。W4 控制文件只用于历史研究。未收到回执不得开始新的 Candidate edit。

### MAIN_CONTINUATION_RULES

继续任务时记录下一步判断、路线交接和未完成事项。平台要求让出本轮时保存路线、动作、阶段、回执和待办；不得创建 timer、automation、cron 或后台循环。

一次状态回复只是 `CHECKPOINT_ONLY`。只要还有本轮指定的路线研究、证据整理、规则同步、Git 集成、worktree 处置、隔离能力核验或 Agent 启动事项，Main 都继续执行下一项，不把状态回复当作任务结束。

## Route ownership

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

每个 Route Agent 的命令 workdir 固定在自己的工作树。新五路线 Agent 不得进入其他路线目录，不得用 Git 命令读取其他路线对象，也不得通过软链接或脚本访问其他路线；仅可读公共规则、Direct Parent 和 Main 发布的只读审计摘要。Record Owner 的唯一工作树是 `/Users/sunyiyang/Desktop/Project/cann`、分支 main；Route Agent 不写主目录。Main 可做获准的本地只读审计。目录分离和 Agent 自述不能证明底层访问隔离或完整访问日志。

## 执行主链

```text
ONE CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT → NEXT CHANGE
```

单 Revision 只表达一个小变化。失败和负结果保留。记录异步写入，不能成为实验前置条件。

## server3 资源安全

对 Compile、Correctness、Local、Profile 使用同一个资源条件：

```text
目标 NPU FREE_HBM >= 100 MB → 允许立即执行
```

以下只记录为负载上下文，不影响执行：AICore utilization 非零、Vector/Core busy、VLLM 驻留、其他用户进程、device 非 idle、load 较高、没有 exclusive lease、没有 exclusive authorization、旧 lease、未知 lease owner、不存在 clean window。

lease 只用于 coordination / bookkeeping；不得为了运行实验删除、覆盖、伪造或重写他人的 lease。不得因上述事实要求等待、暂停 Route、拒绝 Correctness/Local、要求独占 NPU 或要求资源负责人批准。

唯一有效的设备资源停止条件：所有可用 NPU FREE_HBM < 100 MB；真实执行出现 OOM / allocation failure / runtime resource failure；启动自己的任务会实际破坏或终止其他用户任务；server3 不可连接。

## 绝对安全规则

- 禁止 force push、reset、clean、历史改写和删除失败证据；
- 不杀、不暂停、不迁移其他用户进程、服务、数据或设备任务；
- 不让多个写入者同时修改同一 Route；
- 新建或改变 Route 按当前用户明确给出的数量与范围执行，不把旧 W4 路线状态带入新阶段；
- Route Agent 按用户指定的 Online Candidate 执行；Online 执行按本文 `Online Experimental Submission Policy` 进行；
- Dashboard 只展示状态，不控制实验；不直接修改展示内容。用户明确授权调用官方提交脚本时，允许该脚本按既有行为追加提交事件；
- 本轮用户已授权集成提交与普通推送；只提交本任务明确范围，并保留其他未提交改动。其他任务没有当前用户明确要求时不提交、不推送；
- 未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。

## 正式事实位置

- Official Score：`技术路线/路线成绩表.tsv` 与 `线上结果/`；
- Route / Revision：`技术路线/全版本记录.tsv`；
- 当前调度：`调度/当前任务.tsv`；
- Local ↔ Official：`调度/本地线上校准.tsv`。

版本记录：人类可读版本树和 Markdown 表唯一位于 `技术路线/技术路线图.md`；结构化账本唯一位于 `技术路线/全版本记录.tsv`。Route Agent → `VERSION_RECORD_EVENT` → Record Owner 异步同步；Main 只读允许的共享状态，不写入这两份 shared files。

历史证据、旧字段和归档控制文件保持原样；它们不重新成为当前规则入口。群聊提取细节只在 `工具/提取/README.md` 维护，本文不重复操作说明。
