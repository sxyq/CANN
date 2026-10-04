# RESEARCH-A（EARLY PERIOD）社区语料取证报告

- 研究范围：2026-09-12 00:00 ~ 2026-09-18 23:59（范围内消息 2111 条，其中人类发言 1620 条，其余为系统/接龙刷屏）
- 研究角色：RESEARCH-A（只读取证 Child），不选路线、不写项目文件
- 隔离上下文：`/tmp/cann-research/A/`
- 语料来源：QQ 群 901064769 原始导出（CSV/TXT）+「提分技术讨论_原文.csv」
- 二级材料（第一遍原始提取完成后才读，仅作遗漏/上下文核对）：`提分技术讨论_清单.md`、`CANN西南赛区_群聊情报交接文档.md`

## 关键前置判断（务必先读）

1. 早期（09-12~09-18）**技术机制密度极低**。全窗口几乎没有 `DataCopy / tiling / 分核 / 流水 / UB / GM / 精度 / 溢出` 等 kernel 级术语，没有共享源码、没有确认的 shape/dtype 细节。该窗口的价值集中在**评测可信度、tbest 污染、本地↔线上机器差异、方法论（探针/反推/打表）**，而不是可复现的 kernel 优化机制。
2. `case14 157us` 与 `case1 1.47us` 在本窗口内即被多名群友与官方人员判定为**异常/违规产物**，并被官方清理或列入核查。**不得当作有效 benchmark 或目标线**。
3. 09-17 23:xx 官方“纠正 tbest”。**任何基于 09-17 之前 tbest 数值的结论都需要复核**。
4. 本窗口的所有“提升/标杆”均为**口头数字**，无 source code、无 shape、无 before/after 配对，属 LOW~MEDIUM 置信度，不能作为机制成立证据。

---

## CHAT_EVIDENCE

CHAT_EVIDENCE
TIME = 2026-09-12 22:57:25
RECORD_ID = group_901064769.csv:L147327
RAW_EXCERPT = 哪来的挂B，把case14刷到157微妙了，死活找不到
TOPIC = case14 异常标杆值 / 评测污染
CASE = case14
MECHANISM = 疑似非正常实现（未全跑通/规避真实计算）产生超常单点时延
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 明确点名 case14 被“刷”到 157us 且“死活找不到”，是本窗口最早的 case14 异常值记录，后续成为社群标杆值噪声源。
WHY_IT_MAY_BE_FALSE = 发言人为旁观者，未给出该 157us 的来源代码与形状；数值本身无法验证。

CHAT_EVIDENCE
TIME = 2026-09-12 23:00:13
RECORD_ID = group_901064769.csv:L147341
RAW_EXCERPT = 测试点8也是他刷的 / 还有10 / 12 / 刷了5个点 的ttest
TOPIC = 多点被刷 / tbest 污染范围
CASE = UNKNOWN（多测试点）
MECHANISM = 同一异常提交者刷多个测试点的单点 tbest
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明污染不止 case14，涉及测试点 8/10/12 等至少 5 个点，是“单点时间被压低”而非单 case 事件。
WHY_IT_MAY_BE_FALSE = 逐点编号来自群友转述，未与榜单截图对齐。

CHAT_EVIDENCE
TIME = 2026-09-12 23:08:53
RECORD_ID = group_901064769.csv:L147375
RAW_EXCERPT = 先把case1那个1.47清除吧
TOPIC = case1 1.47us 疑似污染
CASE = case1
MECHANISM = 疑似与 case14 157us 同源的异常单点产物
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case1 1.47us 在窗口内被群友要求官方清除；后续 09-13 00:56 被归因为同一“未全通过”机制。
WHY_IT_MAY_BE_FALSE = 认作污染属群友推断，官方仅回复“核实后处理”，未在窗口内确认 case1 1.47 是否被删。

CHAT_EVIDENCE
TIME = 2026-09-13 00:15:06
RECORD_ID = group_901064769.csv:L147386
RAW_EXCERPT = 其他用例不通过的情况下某些用例确实有概率时间很快
TOPIC = 污染机理（部分用例未通过却计入单点）
CASE = UNKNOWN
MECHANISM = 用例未全通过时，个别用例计时异常偏快 → 被记入单点最优
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出了污染的可检验机理假设：单点 tbest 与“是否 15/15 全通过”解耦。是解释 157us/1.47us 的核心线索。
WHY_IT_MAY_BE_FALSE = 属推断（“确实有概率”），无官方规则或实验佐证；发言人本人也不确定。

CHAT_EVIDENCE
TIME = 2026-09-13 00:55:38
RECORD_ID = group_901064769.csv:L147395
RAW_EXCERPT = 没全pass，也算单case成绩?
TOPIC = 计分规则疑点（单 case 成绩是否要求全通过）
CASE = UNKNOWN
MECHANISM = tbest 记录规则可能只看单点、不看整体 pass
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L147386/L147397/L147398 构成“单点 tbest 与全通过解耦”的重复信号。
WHY_IT_MAY_BE_FALSE = 疑问句，非结论；官方未正面确认。

CHAT_EVIDENCE
TIME = 2026-09-13 00:56:18
RECORD_ID = group_901064769.csv:L147398
RAW_EXCERPT = 估计case1的1.47也是这个原因
TOPIC = case1 1.47us 归因
CASE = case1
MECHANISM = 与 case14 157us 同源（未全通过/异常计时的单点）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 把 case1 1.47us 与 case14 157us 明确串为同一类异常，支持“同机制污染多个 case”。
WHY_IT_MAY_BE_FALSE = “估计”二字表明是推测；无数据。

CHAT_EVIDENCE
TIME = 2026-09-13 01:15:33
RECORD_ID = group_901064769.csv:L147425
RAW_EXCERPT = 感觉是因为输出错误答案也会计算用时，也就是输入数据原封不动的输出的话，应该是最快的
TOPIC = 计时规则（错误输出仍计时）
CASE = UNKNOWN
MECHANISM = 计时与正确性解耦：错误结果也计时间，理想化“原样输出”最快
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出“时间测量可能不校验正确性”的风险假设，与污染问题直接相关。
WHY_IT_MAY_BE_FALSE = 被同群 09-13 01:28 反驳（见下条），属被部分否定的推断。

CHAT_EVIDENCE
TIME = 2026-09-13 01:28:03
RECORD_ID = group_901064769.csv:L147426
RAW_EXCERPT = wa跟re不会（被记时）
TOPIC = 对“错误输出也计时”的反驳
CASE = UNKNOWN
MECHANISM = WA/RE 状态不纳入计时，只有 pass 的点进入 tbest
EVIDENCE_CLASS = CONTRADICTED
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L147425 相反，说明“输出错误答案拿最快时间”说法不成立于 WA/RE；污染机理更可能是“部分点 pass、其余未通过”。
WHY_IT_MAY_BE_FALSE = 同群 09-13 01:29 又说“昨天 wa 一个比 tbest 快但是没被记”，细节仍矛盾。

CHAT_EVIDENCE
TIME = 2026-09-13 01:31:27
RECORD_ID = group_901064769.csv:L147428
RAW_EXCERPT = 意思应该是虽然没有15/15但是其中pass的会被记入tbest
TOPIC = tbest 记账规则
CASE = UNKNOWN
MECHANISM = 未 15/15 全通过时，其中 pass 的点仍可写入单点 tbest
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出污染记账层机理：单点 tbest 与“整体是否全通过”解耦，是 case14/1.47 类异常的直接前提。
WHY_IT_MAY_BE_FALSE = “意思应该是”，无官方确认。

CHAT_EVIDENCE
TIME = 2026-09-14 22:45:48
RECORD_ID = group_901064769.csv:L147957
RAW_EXCERPT = 针对异常提交已经封禁，现在成绩已显示正常
TOPIC = 官方处置异常提交
CASE = UNKNOWN
MECHANISM = 官方封禁异常提交并恢复成绩
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方运营/专家在窗口内二次确认异常提交已被封禁（另见 L147368/L147374/L147969），是“157us/1.47us 类数值不可用”的官方口径证据。
WHY_IT_MAY_BE_FALSE = 未逐 case 列出被封禁的具体数值。

CHAT_EVIDENCE
TIME = 2026-09-13 21:41:55
RECORD_ID = group_901064769.csv:L147621
RAW_EXCERPT = tbest 没修复吗👀，误差好大现在，同一份源码提交 20 次，分数区间差距能有三四分
TOPIC = 评测噪声量级（同源码重复提交）
CASE = UNKNOWN（全局）
MECHANISM = 平台测量噪声/抖动导致同源码分数区间达 3~4 分
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（同源码 20 次）
WHY_IT_MATTERS = 本窗口最强的“噪声地板”证据：同源码 20 次提交区间差 3~4 分。任何小于该量级的“提升”都不可信。
WHY_IT_MAY_BE_FALSE = 无原始提交记录佐证；3~4 分是主观描述的区间。

CHAT_EVIDENCE
TIME = 2026-09-16 18:19:29
RECORD_ID = group_901064769.csv:L148516
RAW_EXCERPT = 只是掉的时候你没看见，我这个同一个码有三四分四五分的浮动的
TOPIC = 同源码分数波动量级（重复信号）
CASE = UNKNOWN（全局）
MECHANISM = 同一份代码分数浮动 3~4~5 分
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 独立于 L147621 的第二次同量级噪声报告，强化“3~5 分噪声地板”判断。
WHY_IT_MAY_BE_FALSE = 与 L147621 可能为同一现象的社群复述。

CHAT_EVIDENCE
TIME = 2026-09-15 16:08:43
RECORD_ID = group_901064769.csv:L148178
RAW_EXCERPT = 没啊，我不知道为什么我同一个代码，布局彩票波动这么大
TOPIC = 同代码分数波动
CASE = UNKNOWN（全局）
MECHANISM = 代码不变而分数大幅波动（“彩票”）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 第三次同量级波动报告，且明确“同一代码”，排除“改代码导致”的解释。
WHY_IT_MAY_BE_FALSE = “布局”指代不明，可能混入平台侧变化。

CHAT_EVIDENCE
TIME = 2026-09-17 12:29:24
RECORD_ID = group_901064769.csv:L148660
RAW_EXCERPT = 我重复交了三次分一次比一次低
TOPIC = 重复提交分数单调下降
CASE = UNKNOWN（全局）
MECHANISM = 同源码重复提交分数逐渐降低（疑平台状态/负载漂移）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（3 次连续）
WHY_IT_MATTERS = 与“随机抖动”不同，描述的是方向性漂移，提示平台状态随负载/时间变化，影响 before/after 配对可信度。
WHY_IT_MAY_BE_FALSE = 样本仅 3 次，且未给出具体分差。

CHAT_EVIDENCE
TIME = 2026-09-17 12:27:32
RECORD_ID = group_901064769.csv:L148654
RAW_EXCERPT = 算分机制，越靠近tbest，彩票影响就更大，所以我现在都一直在抽奖
TOPIC = 计分机制与噪声耦合
CASE = UNKNOWN（全局）
MECHANISM = 得分越接近 tbest，随机抖动对得分占比越大
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出“越接近极限越吃噪声”的得分机理假设，对判定“微小提升是否真实”关键。
WHY_IT_MAY_BE_FALSE = 属玩家推断，未验证评分公式。

CHAT_EVIDENCE
TIME = 2026-09-17 12:25:40
RECORD_ID = group_901064769.csv:L148653
RAW_EXCERPT = 排行榜很多都是双峰分布的
TOPIC = 榜单分布形态
CASE = UNKNOWN（全局）
MECHANISM = 分数/时延呈双峰分布
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若属实，提示存在两类实现族（如“真实计算”与“异常绕过”）或两类运行状态，对解释污染与聚类有价值。
WHY_IT_MAY_BE_FALSE = 无分布数据，纯观感。

CHAT_EVIDENCE
TIME = 2026-09-14 16:49:20
RECORD_ID = group_901064769.csv:L147851
RAW_EXCERPT = Tbest问题我们晚点会修复
TOPIC = tbest 数据被官方确认有误待修
CASE = UNKNOWN（全局）
MECHANISM = tbest 记账存在缺陷
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方（运营）承认 tbest 有问题并将修复，直接支撑“窗口内 tbest 不可信”。
WHY_IT_MAY_BE_FALSE = 未说明缺陷范围。

CHAT_EVIDENCE
TIME = 2026-09-17 23:14:57
RECORD_ID = group_901064769.csv:L148971
RAW_EXCERPT = 没啥大事，就是纠正一下tbest
TOPIC = tbest 被官方纠正
CASE = UNKNOWN（全局）
MECHANISM = 历史 tbest 数值被修正
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（纠正前后）
WHY_IT_MATTERS = 09-17 23:14 官方专家纠正 tbest（配合 L148993 “修了tbest”），说明 09-17 前的 tbest 数字截面不可直接沿用。
WHY_IT_MAY_BE_FALSE = 未给出被修正的 case/数值清单。

CHAT_EVIDENCE
TIME = 2026-09-17 23:33:10
RECORD_ID = group_901064769.csv:L148993
RAW_EXCERPT = @西南交通大学 lambd （已报名） / 修了tbest
TOPIC = tbest 修复完成回执
CASE = UNKNOWN（全局）
MECHANISM = tbest 已修复
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 与 L148971 构成“09-17 晚 tbest 发生变更”的重复确认；本窗口前后的 tbest 需分开处理。
WHY_IT_MAY_BE_FALSE = 无具体变更细节。

CHAT_EVIDENCE
TIME = 2026-09-14 10:31:29
RECORD_ID = group_901064769.csv:L147731
RAW_EXCERPT = 比赛评测用的芯片是910B3嘛，还是B1，B2啊 …（10:31:41 运营答）B4
TOPIC = 评测机芯片型号
CASE = UNKNOWN（全局）
MECHANISM = 评测平台为 910B4
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 运营口头给出评测机为 910B4，是后续“开发机≠评测机”讨论的锚点。
WHY_IT_MAY_BE_FALSE = 由运营助理（非专家）给出，与 09-18 群内其它说法存在张力（见 CONTRADICTIONS）。

CHAT_EVIDENCE
TIME = 2026-09-14 23:32:15
RECORD_ID = group_901064769.csv:L148032
RAW_EXCERPT = 有些本地时间变短平台时间却变长
TOPIC = 本地加速≠线上加速（反例）
CASE = UNKNOWN（全局）
MECHANISM = 本地与平台硬件/环境差异导致优化方向相反
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（本地 vs 平台）
WHY_IT_MATTERS = 关键的负结果：本地测得的加速在平台上可能变慢，直接否定“本地测量可外推线上”的假设。
WHY_IT_MAY_BE_FALSE = 无具体改动与数值，属观察陈述。

CHAT_EVIDENCE
TIME = 2026-09-16 15:04:02
RECORD_ID = group_901064769.csv:L148351
RAW_EXCERPT = 其实我之前以为测评机和这个机器是差不多的…这个机器好像是一个机器切了很多份…有相当大差别
TOPIC = 评测机为切分后的机器
CASE = UNKNOWN（全局）
MECHANISM = 评测环境是把大机切分成多份，硬件参数与开发机显著不同
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 解释“本地↔线上差异”和“核数不确定”的可能来源（资源切分/共享），是关键机制假设。
WHY_IT_MAY_BE_FALSE = 由 福尔摩斯 个人推断（“好像”），无官方确认。

CHAT_EVIDENCE
TIME = 2026-09-16 15:08:39
RECORD_ID = group_901064769.csv:L148356
RAW_EXCERPT = 目前我探测了一下测评机的一些参数。但是还没有完全探出来。我感觉这个还是挺重要的。
TOPIC = 主动探测评测机参数
CASE = UNKNOWN（全局）
MECHANISM = 通过提交探测评测机带宽/核数等参数
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 早期最早的“探针/参数反推”行为自述，方法论线索起点。
WHY_IT_MAY_BE_FALSE = 未给出探得的具体参数值。

CHAT_EVIDENCE
TIME = 2026-09-17 12:23:05
RECORD_ID = group_901064769.csv:L148641
RAW_EXCERPT = 测评机和开发机这两个之间的区别还是有一定的。…硬件上的一些参数很多都不一样。
TOPIC = 评测机/开发机硬件参数差异（重复信号）
CASE = UNKNOWN（全局）
MECHANISM = 开发机与评测机硬件参数不同
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L148351/L148032 一致，多渠道确认本地测不等于线上测。
WHY_IT_MAY_BE_FALSE = 结论性口述，无参数表。

CHAT_EVIDENCE
TIME = 2026-09-17 13:25:10
RECORD_ID = group_901064769.csv:L148826
RAW_EXCERPT = 就是有很多比方说在开发机上好像有效但是到判题机上面没效的这些原因，我基本上找到了一些了
TOPIC = 本地有效线上无效（负结果）
CASE = UNKNOWN（全局）
MECHANISM = 优化在开发机有效但判题机无效
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（开发机 vs 判题机）
WHY_IT_MATTERS = 明确的“本地↔线上失效”负证据；任何只在本机验证的优化都不能宣称提分。
WHY_IT_MAY_BE_FALSE = “找到了一些原因”但未公开具体原因。

CHAT_EVIDENCE
TIME = 2026-09-18 18:22:15
RECORD_ID = group_901064769.csv:L149162
RAW_EXCERPT = @cqu manganese / 建议花几次提交改变代码里的核数实测
TOPIC = 用提交反推核数
CASE = UNKNOWN（全局）
MECHANISM = 通过改变代码核数并观察线上表现来推断平台核数
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 核数不确定性的应对方法论；是早期“核数线索”的核心（line 05）。
WHY_IT_MAY_BE_FALSE = 建议性发言，未给实测结果。

CHAT_EVIDENCE
TIME = 2026-09-18 18:21:48
RECORD_ID = group_901064769.csv:L149160
RAW_EXCERPT = 我想问这个线上评测平台真是一直保持40核的吗
TOPIC = 线上核数疑问
CASE = UNKNOWN（全局）
MECHANISM = 平台核数可能非固定 40
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出平台核数可能动态/非 40 的关键疑问。
WHY_IT_MAY_BE_FALSE = 疑问句，无实测。

CHAT_EVIDENCE
TIME = 2026-09-18 18:23:04
RECORD_ID = group_901064769.csv:L149165
RAW_EXCERPT = 左测右测得出结果是大概率整到32核（
TOPIC = 平台核数≈32（自称实测）
CASE = UNKNOWN（全局）
MECHANISM = 平台实际可用核数约 32
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L149172（32/40/更少混合）一起，构成“核数不固定”的社区共识雏形。
WHY_IT_MAY_BE_FALSE = 发言人随后 09-18 18:28 自行“收回我的话”，可信度低。

CHAT_EVIDENCE
TIME = 2026-09-18 18:22:47
RECORD_ID = group_901064769.csv:L149163
RAW_EXCERPT = 开发机好像是910 c …（18:22:53）测评机好像是910b4。
TOPIC = 开发机/评测机型号（与运营说法对照）
CASE = UNKNOWN（全局）
MECHANISM = 开发机 910C、评测机 910B4
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L149166-149167（wilf：“开发是b3 / a2卡”）直接冲突，是重要矛盾点，说明机器型号在群内并未收敛。
WHY_IT_MAY_BE_FALSE = 与 wilf 说法矛盾，且均为“好像”。

CHAT_EVIDENCE
TIME = 2026-09-18 18:52:52
RECORD_ID = group_901064769.csv:L149172
RAW_EXCERPT = 我感觉有一些用32核有一些用40核好像有一些用的核就比较少。
TOPIC = 核数混合（32/40/更少）
CASE = UNKNOWN（全局）
MECHANISM = 不同提交/不同形态实际用到核数不同
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 支持“核数不是固定常量”的假设，对分核类优化方向有判断价值。
WHY_IT_MAY_BE_FALSE = 纯主观感觉（“我感觉”）。

CHAT_EVIDENCE
TIME = 2026-09-18 18:16:18
RECORD_ID = group_901064769.csv:L149132
RAW_EXCERPT = 910b3环境上，起码够迭代到70
TOPIC = 910B3 环境可支撑迭代到 70 分
CASE = UNKNOWN（全局）
MECHANISM = 910B3 开发环境足以达到约 70 分
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出“本地 910B3 可用于迭代验证”的经验阈值，与“本地≠线上”张力并存。
WHY_IT_MAY_BE_FALSE = 由 wilf 口述，无对照实验。

CHAT_EVIDENCE
TIME = 2026-09-12 17:12:40
RECORD_ID = group_901064769.csv:L140889
RAW_EXCERPT = 甚至禁止违规探针提交
TOPIC = 违规探针提交被禁
CASE = UNKNOWN（全局）
MECHANISM = 平台禁止以探针方式获取测试信息
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明“探针/打表”是被规则限制的灰色手段，任何基于探针的参数反推都需合规评估。
WHY_IT_MAY_BE_FALSE = 转述规则，未给官方原文（原文 09-21 才被完整转贴，属窗口外）。

CHAT_EVIDENCE
TIME = 2026-09-12 17:15:04
RECORD_ID = group_901064769.csv:L143212
RAW_EXCERPT = @电子科技大学cc（已报名） / 但是还是可以找机会交探针的（） …（17:15:11）但是要隐蔽一点
TOPIC = 探针提交仍可进行（灰色）
CASE = UNKNOWN（全局）
MECHANISM = 群友认为仍可隐蔽提交探针获取信息
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示“参数反推/打表”在早期已是主流方法论；也提示合规红线。
WHY_IT_MAY_BE_FALSE = 玩笑语气（“（）”），非官方或确定事实。

CHAT_EVIDENCE
TIME = 2026-09-12 17:15:45
RECORD_ID = group_901064769.csv:L143794
RAW_EXCERPT = 其实大概可以推测出来一个大致的形状。…（17:15:49）完全精准是肯定不知道的。
TOPIC = 形状可粗略反推
CASE = UNKNOWN（全局）
MECHANISM = 由时间/行为可粗略推测测试点 shape，但无法精确
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 早期最早的“形状反推”线索，是被后续 09-17 讨论展开的方法论起点。
WHY_IT_MAY_BE_FALSE = “大概/推测”，无具体形状结论。原始群聊中没有任何真实 shape 数字。

CHAT_EVIDENCE
TIME = 2026-09-17 13:16:17
RECORD_ID = group_901064769.csv:L148808
RAW_EXCERPT = 比如像绝对带宽，l2，指令的时间，向量核数等
TOPIC = 可由时间反推的机器参数清单
CASE = UNKNOWN（全局）
MECHANISM = 通过实测时间反推带宽/L2/指令时间/向量核数
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 明确列出“向量核数”等参数作为可探测对象（line 48 核心线索）。是硬件参数化优化的方法论核心。
WHY_IT_MAY_BE_FALSE = 属方法论主张，未给任何实测参数值。

CHAT_EVIDENCE
TIME = 2026-09-17 13:24:16
RECORD_ID = group_901064769.csv:L148824
RAW_EXCERPT = 通过时间可以反推指令开销，从而下一步看是动结构还是动指令条数什么的
TOPIC = 反推指令开销以决定优化方向
CASE = UNKNOWN（全局）
MECHANISM = 时间 → 指令开销 → 结构 vs 指令条数的决策
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出“结构 / 指令条数”两类优化维度的决策方法，是本窗口最接近 kernel 层的思路线索。
WHY_IT_MAY_BE_FALSE = 无实验支撑的推理。

CHAT_EVIDENCE
TIME = 2026-09-17 14:45:33
RECORD_ID = group_901064769.csv:L148841
RAW_EXCERPT = 我原本6000的，对每个测试点单写一遍，6000*15=8w
TOPIC = 面向 case 的复制式“打表”策略
CASE = UNKNOWN（多测试点）
MECHANISM = 对每个测试点单独写一份实现（“多专家稀疏激活”隐喻）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 揭示早期主流策略是“逐点/逐 case 专用实现”，是评估“通用机制 vs 过拟合”时的重要背景。
WHY_IT_MAY_BE_FALSE = 声称代码量 8w 行，未验证。

CHAT_EVIDENCE
TIME = 2026-09-17 14:51:58
RECORD_ID = group_901064769.csv:L148849
RAW_EXCERPT = 可别面向case编程哈，哈基米
TOPIC = 官方警告“面向 case 编程”
CASE = UNKNOWN（全局）
MECHANISM = 组织方反对逐 case 专用优化
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方（运营）口头劝阻针对单 case 的过拟合，与“面向 case 编程”策略形成规则性张力；对“真实通用机制”取舍有参考价值。
WHY_IT_MAY_BE_FALSE = 半玩笑语气，非正式禁令。

CHAT_EVIDENCE
TIME = 2026-09-17 12:21:40
RECORD_ID = group_901064769.csv:L148636
RAW_EXCERPT = 搬运和 fp32 不同数据量时候的改造
TOPIC = 搬运 / fp32 数据量相关改造
CASE = UNKNOWN
MECHANISM = 针对搬运（数据搬运）与 fp32 数据量的实现改造
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = YES（fp32）
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 全窗口唯一明确提到 dtype（fp32）与“搬运”的 kernel 相关线索，可作为后续查证的锚点。
WHY_IT_MAY_BE_FALSE = 泛泛而谈，无实现、无数据、无 case。

CHAT_EVIDENCE
TIME = 2026-09-17 01:02:06
RECORD_ID = group_901064769.csv:L148524
RAW_EXCERPT = 奶蛙14us的测点7我真怕了
TOPIC = case7 测点7 标杆值 14us
CASE = case7
MECHANISM = 测点 7 最佳时延约 14us
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 本窗口唯一带 case 绑定（case7）的具体时延标杆（14us），可作参考线（非目标线）。
WHY_IT_MAY_BE_FALSE = 二手数字（“奶蛙”的记录），无源码/形状；且 09-17 前后 tbest 曾被修正，取值需复核。

CHAT_EVIDENCE
TIME = 2026-09-17 12:30:20
RECORD_ID = group_901064769.csv:L148666
RAW_EXCERPT = 但是我觉得case11与case13那个值也不知道是问题还是什么的 … 真的离群值
TOPIC = case11/case13 数值离群
CASE = case11, case13
MECHANISM = 该两 case 的 tbest 明显离群，疑为异常
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 指出 case11/case13 存在离群 tbest，配合 09-13 case13 出现 300 的现象，提示这两个 case 数据不可信。
WHY_IT_MAY_BE_FALSE = 主观判断，未做数据分析。

CHAT_EVIDENCE
TIME = 2026-09-17 12:31:03
RECORD_ID = group_901064769.csv:L148674
RAW_EXCERPT = @福尔摩斯 / 官方的不加偏置吧
TOPIC = 官方基线是否含偏置
CASE = UNKNOWN（算子本体）
MECHANISM = 官方 RmsNorm 基线可能不含偏置（AddRmsNormBias 的 bias 部分为增量项）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 早期唯一触及“算子语义/基线是否含 bias”的线索，对判断评测基线口径有潜在价值。
WHY_IT_MAY_BE_FALSE = 无实据；上句 福尔摩斯 说“官方是有这个算子的”，此句为 wilf 猜测。

CHAT_EVIDENCE
TIME = 2026-09-15 19:36:40
RECORD_ID = group_901064769.csv:L148241
RAW_EXCERPT = 云淡风轻自己也回不到三百多微妙了
TOPIC = 离群值不可复现
CASE = case13 相关（“三百多微妙”）
MECHANISM = 曾出现的超优数值无法复现
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES（推测为 case13）
HAS_BEFORE_AFTER = YES（曾达到 vs 现无法复现）
WHY_IT_MATTERS = 与“离群 tbest/污染”一致，说明个体最佳可能不可复现，支持 tbest 噪声结论。
WHY_IT_MAY_BE_FALSE = 未明确 case 编号；数值 300 与 09-13 case13 300 呼应但非确证。

CHAT_EVIDENCE
TIME = 2026-09-18 21:59:51
RECORD_ID = group_901064769.csv:L149207
RAW_EXCERPT = 这case9的tbest真能实现吗
TOPIC = case9 tbest 可达性存疑
CASE = case9
MECHANISM = case9 的 tbest 疑似不可实现
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 早期即出现对 case9 tbest 可达性的质疑，提示该值可能同样不可信。
WHY_IT_MAY_BE_FALSE = 单句疑问，无依据。

CHAT_EVIDENCE
TIME = 2026-09-15 16:34:26
RECORD_ID = group_901064769.csv:L148212
RAW_EXCERPT = 前两名的case4case5太厉害了，赶不到
TOPIC = case4/case5 头部领先
CASE = case4, case5
MECHANISM = case4/case5 头部队伍领先明显
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 早期唯一将 case4 与 case5 并列的信号，可作为“case4/case5 共同 cluster”的早期弱证据；但不能证明二者共享同一实现机制。
WHY_IT_MAY_BE_FALSE = 观感陈述，无数据；与具体实现机制无关。

CHAT_EVIDENCE
TIME = 2026-09-14 14:49:07
RECORD_ID = group_901064769.csv:L147796
RAW_EXCERPT = 有没有可能这种最佳case是官方投的 …（同期）“绝大部分检查点第一都比第二优3-5倍”
TOPIC = 头部数值异常突出的质疑
CASE = UNKNOWN（全局）
MECHANISM = 第一名较第二名优 3~5 倍，疑似非真实
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 描述头部数值与第二名存在数量级差距，是污染/分布异常的旁证（与“双峰”呼应）。
WHY_IT_MAY_BE_FALSE = 阴谋论式猜测，无数据支撑。

CHAT_EVIDENCE
TIME = 2026-09-14 23:09:05
RECORD_ID = group_901064769.csv:L148002
RAW_EXCERPT = case1的1.47us呢
TOPIC = case1 1.47us 再被点名
CASE = case1
MECHANISM = 该数值被视为异常/可疑标杆
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 09-14 再次点名 1.47us，与前一日“清除”诉求构成重复信号，强化该值不可用。
WHY_IT_MAY_BE_FALSE = 语境为提问，未给结论。

CHAT_EVIDENCE
TIME = 2026-09-15 12:16:32
RECORD_ID = group_901064769.csv:L148104
RAW_EXCERPT = 成绩无效是因为卡测试漏洞吗
TOPIC = 成绩无效归因（漏洞）
CASE = UNKNOWN
MECHANISM = 成绩被判无效疑因利用测试漏洞
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与官方“针对异常提交已封禁/不要利用代码漏洞”（L147957/L147983）呼应，说明窗口内确有违规处置。
WHY_IT_MAY_BE_FALSE = 疑问，且官方未逐一点名。

CHAT_EVIDENCE
TIME = 2026-09-12 17:12:21
RECORD_ID = group_901064769.csv:L140310
RAW_EXCERPT = @华为竞赛运营Mr.武 / 15个测试点可以看到具体是哪15种输入吗 …（答）不行
TOPIC = 测试点信息隐藏
CASE = UNKNOWN（全局）
MECHANISM = 15 个测试点输入不可见
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方确认 15 个测试点且输入隐藏，是所有“形状猜想/打表”讨论的规则前提。
WHY_IT_MAY_BE_FALSE = 无。

---

## MECHANISM_CLUE

MECHANISM_CLUE
CLUE_ID = A-EARLY-M1
MECHANISM = 单点 tbest 污染：未全部通过（或异常实现）时，个别测试点被记入超常偏快的单点最优，从而污染榜单
AFFECTED_CASES = case14（157us）、case1（1.47us）、测试点 8/10/12 等（至少 5 点）
SUPPORTING_RECORDS = L147327, L147341, L147375, L147386, L147395, L147398, L147428, L147957, L147969, L148002, L148104
CONTRADICTING_RECORDS = L147426（WA/RE 不计时）、L147427（wa 比 tbest 快未被记）——说明“输出错误也计时”不成立，需换机理（部分点 pass 记账 / 空 kernel 类绕过）
NEGATIVE_EVIDENCE = 有选手自述“队友一己之力把大家分数打低”（L147402/L147450）
CONFIDENCE = MEDIUM（污染事实 HIGH；具体机理 MEDIUM）
KNOWN_FACTS = 157us/1.47us 被多人+官方视为异常；官方称已封禁异常提交并会纠正 tbest；tbest 于 09-17 23:xx 被修正
UNKNOWN_FACTS = 具体绕过手段（窗口内未知；二级材料提到的“空 kernel / Host CPU 路由”分析出自 09-21，窗口外）；被污染的具体 case/点位集合
MINIMUM_FACT_NEEDED_NEXT = 官方是否已彻底清除 case14/case1 的异常 tbest；09-17 修正后 tbest 的实际取值

MECHANISM_CLUE
CLUE_ID = A-EARLY-M2
MECHANISM = 评测噪声地板：同一份源码重复提交，分数区间差 3~5 分（方向性漂移亦存在）
AFFECTED_CASES = 全 case（榜单排名整体压缩）
SUPPORTING_RECORDS = L147621, L148516, L148178, L148660, L148654, L148653
CONTRADICTING_RECORDS = 无直接反驳；L148660 的单调下降可计入同现象
NEGATIVE_EVIDENCE = 无
CONFIDENCE = HIGH
KNOWN_FACTS = 同源码 20 次 → 3~4 分区间（L147621）；同码 3~4~5 分浮动（L148516）
UNKNOWN_FACTS = 噪声是否随 case、时段、负载变化；精确分布
MINIMUM_FACT_NEEDED_NEXT = 以项目正式提交做同源码重复配对，量化本项目下的 noise floor

MECHANISM_CLUE
CLUE_ID = A-EARLY-M3
MECHANISM = 开发机与评测机不同构：芯片型号/切片/核数不同，导致本地加速可能线上变慢
AFFECTED_CASES = 全 case（本地→线上外推失效）
SUPPORTING_RECORDS = L147731, L148032, L148036, L148351, L148356, L148641, L148826, L149132, L149160, L149162, L149165, L149163, L149172
CONTRADICTING_RECORDS = L149132（“910b3 起码够迭代到 70”）——提示本地仍有一定参考性，但方向相反于 L148032/L148826
NEGATIVE_EVIDENCE = L148032（本地变短、平台变长）；L148826（开发机有效、判题机无效）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 运营称评测机 910B4；疑平台为“一机切多份”；核数疑为 32/40 不固定
UNKNOWN_FACTS = 真实可用核数、切片方式、带宽/L2 具体值
MINIMUM_FACT_NEEDED_NEXT = 以“改核数”的受控提交反推平台核数；确认开发机与评测机确切型号

MECHANISM_CLUE
CLUE_ID = A-EARLY-M4
MECHANISM = 通过时间反推机器参数（带宽/L2/指令时间/向量核数），据此决定“动结构”还是“动指令条数”
AFFECTED_CASES = 全 case（尤其非访存受限 case）
SUPPORTING_RECORDS = L143794, L144376, L148808, L148824, L148819, L148822, L148634
CONTRADICTING_RECORDS = L148630（“有些读写指令开销是不好算的”）、L148635（“纯推理不好算”）——承认方法有边界
NEGATIVE_EVIDENCE = L148821（“猜出来形状……也优化不出来啥东西”）
CONFIDENCE = MEDIUM（作为方法论）；具体参数值 LOW
KNOWN_FACTS = 群内主流方法论是“探针/反推 + 逐点专用”
UNKNOWN_FACTS = 任何实测参数；是否有真实 shape
MINIMUM_FACT_NEEDED_NEXT = 可复现的参数反推实验（带宽/核数），并验证与线上排名相关性

MECHANISM_CLUE
CLUE_ID = A-EARLY-M5
MECHANISM = “面向 case / 逐点专用实现”策略（自称“多专家稀疏激活”），与官方“别面向 case 编程”的规则张力
AFFECTED_CASES = 全 case（行为面，非单一机制）
SUPPORTING_RECORDS = L148841, L148853, L148855, L149154, L149155, L148849, L147403, L147406, L147410
CONTRADICTING_RECORDS = L147403-L147410（“针对性的优化没意义”“换个形状性能很拉”）——同群内部对逐点过拟合的价值存在否定；L148849 官方劝阻
NEGATIVE_EVIDENCE = L147405-L147410（针对性优化只对特定输入有效）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 有人声称对每个测试点单独写实现；官方口头劝阻
UNKNOWN_FACTS = 该策略是否真的提升全局 Official；是否有通用机制收益
MINIMUM_FACT_NEEDED_NEXT = 验证某“通用”改动在多个 case 上的 before/after（配 noise floor）

MECHANISM_CLUE
CLUE_ID = A-EARLY-M6
MECHANISM = 历史 tbest 曾误并 09-17 被官方修正 → 09-17 前的 tbest 截面不可直接用于标定
AFFECTED_CASES = case11, case13（离群），case9（可达性存疑），以及全 case 排名
SUPPORTING_RECORDS = L148971, L148993, L147851, L147957, L148666, L148915, L149207, L148711
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无
CONFIDENCE = HIGH（tbest 被修正这一事实）、LOW（各 case 具体影响）
KNOWN_FACTS = 09-14 官方承诺修复、09-17 23:xx 修复完成
UNKNOWN_FACTS = 被修正的 case/数值清单；修正是否消除了 11/13 离群
MINIMUM_FACT_NEEDED_NEXT = 获取 09-17 修正前后的 tbest 快照对比

---

## HIGH_CONFIDENCE_SIGNALS

- case14 157us / case1 1.47us 属异常产物，官方在窗口内确认“异常提交已封禁”“会清除/核实”，并且 tbest 于 09-17 晚被官方纠正。（L147327, L147341, L147375, L147957, L147969, L148971, L148993）
- 评测噪声地板约 3~5 分：同一份源码多次提交分数区间差 3~4 分（09-13）与 3~4~5 分（09-16）为两次独立同量级报告。（L147621, L148516）
- 开发机≠评测机且“本地加速可能线上变慢”（L148032, L148826, L148641, L148351）；评测机为 910B4（L147732）。
- 15 个测试点、输入隐藏为官方确认规则前提（L140310）。
- 早期几乎无 kernel 级机制证据：全窗口无源码、无确认 shape、无 before/after 配对。

## MEDIUM_CONFIDENCE_SIGNALS

- 污染机理更可能是“未全部通过时 pass 点仍计入单点 tbest”（L147386, L147428, L147395），而非“输出错误答案计时”（该说法被 L147426 部分否定）。
- tbest 被解读为“选手历史最优”而非官方最优（L148711, L148657, L148663），解释为何“最高分不是 100”。
- 核数不确定性：平台疑非固定 40 核（L149160, L149162, L149172）；但自称实测“32 核”的人随后收回（L149165→L149169）。
- 方法论：由时间反推带宽/L2/指令时间/向量核数，决定动结构或动指令条数（L148808, L148824）。
- case11/case13 数值离群（L148666），case9 tbest 可达性存疑（L149207）。
- 探针/打表为早期主流手段且处于规则灰色地带（L140889, L143212, L148799）。

## LOW_CONFIDENCE_SIGNALS

- case7 测点 7 = 14us 标杆（L148524, L148619）——二手、且窗口内 tbest 被修正。
- “面向 case / 多专家稀疏激活”策略（L148841, L148853, L149155）——自称，未见通用收益证据；且被官方劝阻（L148849）。
- “排行榜双峰分布”“第一名比第二优 3~5 倍”（L148653, L147796）——观感/阴谋论。
- “官方不加偏置”（L148674）、910C/910B3/A2 卡型号（L149163 vs L149166）——均为未经证实的口述。
- case4/case5 头部领先（L148212）——仅观感，无机制。

## NEGATIVE_EVIDENCE

- 本地时间变短但平台时间变长（L148032）；开发机有效、判题机无效（L148826）——直接否定“本地测量可外推线上”。
- 针对性/逐点优化“换个形状就性能很拉”“没有意义”（L147405-L147408）——不支持“单 case 专用=全局提分”。
- 曾出现的超优数值不可复现（L148241 “回不到三百多微妙”；L148713 “自己复现不了”）。
- 探针反推的边界：读写指令开销“不好算”“纯推理不好算”（L148630, L148635）。

## CONTRADICTIONS

- 计时规则：L147425（错误答案也计时、原样输出最快）↔ L147426（wa/re 不计时）+ L147427（wa 比 tbest 快但未记）。窗口内未收敛。
- 开发机/评测机型号：L149163（开发 910C / 评测 910B4）↔ L149166-149167（开发是 b3 / a2 卡）；且与 L147731（评测 910B4，由运营助理给出）并存。型号说法不统一。
- 核数：L149160（疑 40）↔ L149165（“大概率 32”）↔ L149172（32/40/更少混合）；L149165 随后被本人收回（L149169）。
- 本地参考价值：L149132（910B3 够迭代到 70）↔ L148032/L148826（本地有效线上无效）。

## UNKNOWN_BUT_IMPORTANT

- 污染的具体实现机理（空 kernel / Host CPU 路由等）在窗口内**未知**；二级材料中的相关分析出自 2026-09-21（窗口外），不得当作窗口内原始事实。
- 09-17 官方“纠正 tbest”的具体 case 与数值变更——未知，直接影响历史截面复核。
- 平台真实核数、是否内存/算力切片、带宽/L2 具体值——未知。
- 任何测试点的真实 shape / dtype——窗口内**完全缺失**（除 L148636 提到 fp32）。
- 逐 case 的“真实可实现”上限——窗口内仅有个体口述。

## RAW_FILES_READ

- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv`
  - 表头 L1；抽样 L2/L20000/L40000/L60000/L76528/L90024/L100000/L120000/L134607/L140000/L143794/L147000/L147327/L149210/L150000/L155000/L157067（确认文件含多行长消息、非严格一行一条）。
  - 全量日期切片：时间戳 `2026-09-12 00:00:00 ~ 2026-09-18 23:59:59`，共 2111 条（切片另存 `/tmp/cann-research/A/slice.csv`，人类发言 1620 条另存 `slice_human.csv`）。
  - 逐段精读：L147315-L147435（09-12 夜 case14 污染）、L147460-L147520（09-13/14 case13、芯片）、L147780-L148085（09-14 榜单/本地线上）、L148200-L148240（09-15）、L148300-L148520（09-16）、L148520-L148680（09-17 方法论）、L148790-L148860（09-17 反推/8w行）、L148960-L149210（09-17 晚 tbest 修正 ~ 09-18 核数）、L140300-L140315、L140880-L140915、L143200-L143232、L143780-L143810（09-12 探针/形状）。
- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769_text.txt`
  - 仅做覆盖度核验：按 `^[2026-09-1[2-8] ` 统计每日条数与 CSV 完全一致（354/261/413/208/208/477/190），未逐条精读正文（内容镜像 CSV）。
- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/提分技术讨论_原文.csv`
  - 表头 + 前 9 行抽样（确认含 case1/case11 早期条目）。
- 二级材料（第一遍后读，仅复核）：
  - `/Users/sunyiyang/Desktop/Project/cann/提分技术讨论_清单.md`（通读）
  - `/Users/sunyiyang/Desktop/Project/cann/CANN西南赛区_群聊情报交接文档.md`（部分：§2 规则与判例、§3 机器型号、§4 噪声与 tbest、§5.1 case14，及关键词定位）

## UNREAD_RANGES_IF_ANY

- `group_901064769_text.txt` 正文未逐条阅读（视为 CSV 镜像）。
- CSV 中长消息的续行（接龙名单、系统 JSON、转贴长文）未逐行审阅——这些非独立发言，不影响消息计数。
- 二级材料中 09-18 之后的章节未通读（超出本窗口且为二手信息）。
- 窗口边界严格为 09-12 00:00 起；09-11 及更早同类讨论未纳入。

## RESEARCH_LIMITATIONS

1. 本报告的“机制”多为**方法论主张与推断**，缺少源码/形状/dtype/before-after，不能当作已验证的性能机制。
2. 所有标杆值为**口头数字**，且 09-17 晚 tbest 曾被官方修正，数值截面存在系统性风险。
3. 群聊中存在大量复述与从众表达；“同一说法多次出现”不等于“多次独立验证”。本报告已尽量按 REPEATED_SIGNAL 与 DIRECT_RESULT 区分，但仍有残余风险。
4. 未进行任何 compile/correctness/local/NPU/online/hash 操作（受硬约束），因此无法把任何群聊数字与真实 kernel 行为对照。
5. 二级材料（清单、交接文档）中的窗口外分析（如 09-21 的“空 kernel/Host CPU 路由”、case8 16.7TB/s 等）**不属于本窗口原始事实**，本报告仅在其明确标注为 SECOND_HAND 时引用，未将其升格为窗口内结论。
