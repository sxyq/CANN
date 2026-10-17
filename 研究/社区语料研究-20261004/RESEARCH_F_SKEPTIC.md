# RESEARCH-F｜SKEPTIC / NEGATIVE-EVIDENCE 只读取证报告

- 角色：RESEARCH-F（SKEPTIC / NEGATIVE EVIDENCE），只读。
- 隔离目录：`/tmp/cann-research/F/`（项目外临时目录，未创建任何 git worktree）。
- 全时间范围：2026-09-10 22:12:41 ~ 2026-10-02 23:38:42（QQ 群 901064769）。
- RAW_FILES_READ（第一遍原始语料）：
  - `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv`（11272 条声明，解析出 11189 条记录，含 824 条系统消息）
  - `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769_text.txt`
  - `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/提分技术讨论_原文.csv`（77 行，属"原始语料"清单内）

## 取证方法（必读，影响可信度）
- NUL 处理：两份原始文件各含 1 个 NUL 字节。已复制到 `/tmp/cann-research/F/group.csv`、`group.txt` 并用 `tr -d '\0'` 删除，**行号与原始一致**（删除 NUL 不改变换行结构）。所有 RECORD_ID 的 `Lxxxxx` 为与原始文件对齐的物理行号。
- CSV 含字段内换行，故按物理行 grep 会把一条记录拆成多行；本报告统一用 Python csv 解析器取"记录起始行号"为 RECORD_ID，并对跨行记录取起始物理行。
- 中文检索用 Python `re`（非 grep），规避 locale 失配。
- 本报告只做**反向标注**，不选择技术路线，不产出 CREATE_ROUTE / ONLINE_DECISION 等字段。
- 大量"负信号"命中其实与算子无关（AI 订阅、招聘、接龙、中奖名单），已剔除或标注为噪声。

结论先行：**该群 90%+ 的技术性"结论"都不是可复现的一手实验事实**，而是（a）AI 口述转述、（b）单人一次负实现、（c）排行榜 tbest 被污染后的二手推断、（d）表情包/整活。可被原始参与者实验事实支持的负证据，主要集中在 **tbest 被异常提交污染** 与 **同源码分数抖动** 两类。

---

## CHAT_EVIDENCE

```
CHAT_EVIDENCE
TIME = 2026-09-12 22:57:25
RECORD_ID = group_901064769.csv:L147327
RAW_EXCERPT = 哪来的挂B，把case14刷到157微妙了，死活找不到
TOPIC = case14 tbest 异常
CASE = case14
MECHANISM = tbest/单点纪录被异常提交拉偏（疑非正常 kernel）
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 直接说明榜单 tbest 曾被污染，任何基于 tbest 差额的"加速比/机制有效性"推断都不可靠。
WHY_IT_MAY_BE_FALSE = 说话者本人"死活找不到"该值来源，仅凭榜单观察；"157微秒"是否为异常快/异常慢不能被这句话确认，也无源码。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 22:59:06
RECORD_ID = group_901064769.csv:L147334
RAW_EXCERPT = 最后一页，超时了那个
TOPIC = case14 异常提交来源识别
CASE = case14
MECHANISM = 疑似超时/未全通过的提交被计入 tbest
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出"污染来自未通过/超时提交"的机制假设，是后续全部推断的源头。
WHY_IT_MAY_BE_FALSE = 纯榜单目测，"最后一页"是谁、是否违规都未证实。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 22:59:38
RECORD_ID = group_901064769.csv:L147337
RAW_EXCERPT = 直接掉了十分
TOPIC = 榜单分数集体塌陷
CASE = case14（及多点）
MECHANISM = tbest 被拉偏后全体得分同步下降
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = PARTIAL   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 说明"分数/排名"在污染期不是稳定测量量，不能当作 kernel 优劣证据。
WHY_IT_MAY_BE_FALSE = 口头描述，"掉了十分"无截图、无对照。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:00:13—23:00:36
RECORD_ID = group_901064769.csv:L147341
RAW_EXCERPT = 测试点8也是他刷的 / 还有10 / 12 / 刷了5个点的ttest
TOPIC = 多点 tbest 同时被污染
CASE = case8/10/12/14
MECHANISM = 同一异常提交改写多个 case 的 time-best
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 表明污染是"多点/系统性"的，进一步削弱以榜单为基准的机制结论。
WHY_IT_MAY_BE_FALSE = 排名页口头解读，未验证同一提交来源。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:06:16
RECORD_ID = group_901064769.csv:L147368
RAW_EXCERPT = 作弊的会取消分数，没必要
TOPIC = 官方口径
CASE = 跨 case
MECHANISM = 组织方承诺清理违规成绩
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = HIGH（作为"官方态度"事实）/ LOW（作为机制证据）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 是唯一来自运营方的处置口径，可支撑"该事件被官方认定为违规"。
WHY_IT_MAY_BE_FALSE = 运营方未给具体规则与产物；"会取消"≠已取消。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:55:38
RECORD_ID = group_901064769.csv:L147395
RAW_EXCERPT = 没全pass，也算单case成绩?
TOPIC = 计分规则质疑：部分通过是否计入
CASE = case14（及多点）
MECHANISM = 未全通过提交的单点用时可能被计入 tbest
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = PARTIAL   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 这是"异常 benchmark"的核心未决规则问题；若成立，则 tbest 不可信。
WHY_IT_MAY_BE_FALSE = 提问者也不确定，属猜测。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:56:13
RECORD_ID = group_901064769.csv:L147397
RAW_EXCERPT = 感觉要设一个全都pass才算tbest的规则
TOPIC = 规则改进建议
CASE = 跨 case
MECHANISM = tbest 应只在全 pass 提交中产生
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反向确认"部分通过被计入 tbest"是被广泛怀疑的漏洞。
WHY_IT_MAY_BE_FALSE = 建议≠事实，规则原文未附。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:15:06
RECORD_ID = group_901064769.csv:L147386
RAW_EXCERPT = 其他用例不通过的情况下某些用例确实有概率时间很快
TOPIC = 部分通过导致单点异常快
CASE = 跨 case
MECHANISM = 未做完整计算/提前返回 → 单点瞬时很快，被记入 tbest
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出一条可检验的"tbest 污染"机制假设（部分通过→单点虚快）。
WHY_IT_MAY_BE_FALSE = 说话人为污染者队友，立场有自利倾向；无实验数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:56:18
RECORD_ID = group_901064769.csv:L147398
RAW_EXCERPT = 估计case1的1.47也是这个原因
TOPIC = case1 tbest 也被疑污染
CASE = case1
MECHANISM = 与 case14 同机制（部分通过→异常快）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 把"case1 1.47us"划入可疑 benchmark，防止把它当成有效标杆/机制上限。
WHY_IT_MAY_BE_FALSE = "估计"，无因果证据；后续（L155799）又出现 1.5/1.24/1.7 多个数值，说明当时并无定论。
```

```
CHAT_EVIDENCE
TIME = 2026-09-15 01:31:23
RECORD_ID = group_901064769.csv:L148083
RAW_EXCERPT = 挂的成绩已经被清除了
TOPIC = 违规成绩处置
CASE = 跨 case
MECHANISM = 官方清除违规提交
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 支撑"该次污染被官方处置"的时间线（前有承诺 L147368，后有清除 L148083）。
WHY_IT_MAY_BE_FALSE = 二手转述，非官方账号发言。
```

```
CHAT_EVIDENCE
TIME = 2026-09-14 23:11:12
RECORD_ID = group_901064769.csv:L148008
RAW_EXCERPT = 山姆奥特曼蓄意破坏评测系统已经被ban了
TOPIC = 违规者处置
CASE = 跨 case
MECHANISM = 违规利用 bug 的账号被封
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 印证实为"人为违规"而非 kernel 机制；污染源被定性。
WHY_IT_MAY_BE_FALSE = 群成员口述，非封禁公告原文。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 01:29:41
RECORD_ID = group_901064769.csv:L147427
RAW_EXCERPT = 我正好昨天wa一个比tbest快但是没被记的（）
TOPIC = 计分规则反例
CASE = 跨 case
MECHANISM = WA（错误答案）不计时，否认"错误答案拿最快"
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L147386 "错误/部分通过会变快"相反，构成对"污染机制"的**内部矛盾**。
WHY_IT_MAY_BE_FALSE = 单人口述、带"（）"，无法复现，也未给出 WA 具体点。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 21:41:55
RECORD_ID = group_901064769.csv:L147621
RAW_EXCERPT = tbest 没修复吗👀，误差好大现在，同一份源码提交 20 次，分数区间差距能有三四分
TOPIC = 同源码分数方差（核心负证据）
CASE = 跨 case
MECHANISM = 评测噪声/环境负载导致同源分数抖动 3-4 分
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 直接判定"同源码分数变化≠kernel runtime noise 之外的机制"这条路不可靠；任何单次数值不得当实验事实。
WHY_IT_MAY_BE_FALSE = 未给 20 次原始分数；"三四分"是区间估计。
```

```
CHAT_EVIDENCE
TIME = 2026-09-15 16:08:43
RECORD_ID = group_901064769.csv:L148178
RAW_EXCERPT = 没啊，我不知道为什么我同一个代码，布局彩票波动这么大
TOPIC = 同源方差复现
CASE = 跨 case
MECHANISM = 同源提交分数大幅波动（"布局彩票"）
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L147621 互证，形成多人独立观察到的同类现象。
WHY_IT_MAY_BE_FALSE = "布局彩票"未定义，无对照数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:02:40
RECORD_ID = group_901064769.csv:L150549
RAW_EXCERPT = 我有一次波动有1us
TOPIC = 同源波动量级
CASE = 跨 case
MECHANISM = 单次波动可达 ≈1us
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 给出方差量级——1us 级抖动足以吞掉多数"微秒级优化"。
WHY_IT_MAY_BE_FALSE = 单次记忆，无重复实验。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 23:14:57
RECORD_ID = group_901064769.csv:L148971
RAW_EXCERPT = 没啥大事，就是纠正一下tbest
TOPIC = 官方修正 tbest
CASE = 跨 case
MECHANISM = 赛题组人工修正 tbest 基准
EVIDENCE_CLASS = SECOND_HAND（官方发言）
CONFIDENCE = HIGH（作为"官方动作"）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 是"tbest 曾错误并被人为修正"的直接官方确认（时间线：L147621 质疑 → L148971 修正）。
WHY_IT_MAY_BE_FALSE = 未说明修正了哪些 case、修正前后的值。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 23:33:10
RECORD_ID = group_901064769.csv:L148993
RAW_EXCERPT = 修了tbest
TOPIC = tbest 修正落地
CASE = 跨 case
MECHANISM = 同上
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 印证 L148971；说明"修前/修后"的数值不可直接比较。
WHY_IT_MAY_BE_FALSE = 个人转述。
```

```
CHAT_EVIDENCE
TIME = 2026-09-24 02:08:14
RECORD_ID = group_901064769.csv:L152752
RAW_EXCERPT = tbest的case2修了
TOPIC = tbest 二次修正
CASE = case2
MECHANISM = 官方再次修正某 case 基准
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 证明"基准被反复修正"，任何跨日期数值对比都噪声极大。
WHY_IT_MAY_BE_FALSE = 无官方公告链接。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:09:59
RECORD_ID = group_901064769.csv:L150632
RAW_EXCERPT = 我的意思就是把tbest拉成噪声带最高值就直接拉低后面高分概率（
TOPIC = tbest 被"刷到噪声带上沿"
CASE = 跨 case
MECHANISM = 用反复提交把 time-best 磨到噪声带上沿，压别人分
EVIDENCE_CLASS = SPECULATIVE（但机制可信）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 精确定义了"刷 tbest"战术，说明榜单分数可被非算法手段操纵。
WHY_IT_MAY_BE_FALSE = 属个人推断（带"（）"），无操作者自证。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 23:46:16 / 23:46:24
RECORD_ID = group_901064769.csv:L155117
RAW_EXCERPT = 哈哈我a2上跑得都比tbest了 / 哈哈我a2上跑得都比tbest快了
TOPIC = 同代码跨卡/跨环境结果矛盾
CASE = 跨 case
MECHANISM = 换硬件（a2）后结果优于官方 tbest，疑环境不一致
EVIDENCE_CLASS = CONTRADICTED
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若属实说明 tbest 与用户实测环境不同源，进一步污染结论。
WHY_IT_MAY_BE_FALSE = 明显整活语气（"哈哈"），极可能 JOKE。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:11:52—04:12:49
RECORD_ID = group_901064769.csv:L156580
RAW_EXCERPT = 会有噪声波动 / 每次提交都不一样 / 这个波动太搞人了
TOPIC = 收尾期同源方差
CASE = case5
MECHANISM = 评测噪声导致同源提交不可复现
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 比赛末期依然存在同源抖动，说明噪声不是早期一次性 bug。
WHY_IT_MAY_BE_FALSE = 未给出重复次数与数值。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:29:00—16:30:25
RECORD_ID = group_901064769.csv:L149980
RAW_EXCERPT = 又是哪位仙人非法提交了 / 恶意利用bug / 天塌了
TOPIC = 第二次违规提交事件
CASE = 跨 case
MECHANISM = 非法提交再次污染榜单
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明污染是**反复发生**的，不是一次偶发。
WHY_IT_MAY_BE_FALSE = 群友起哄，"仙人"不指名、无证据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:37:39
RECORD_ID = group_901064769.csv:L150055
RAW_EXCERPT = 怎么还是恶意利用漏洞呢
TOPIC = 官方对第二次违规的表态
CASE = 跨 case
MECHANISM = 官方认定"恶意利用漏洞"
EVIDENCE_CLASS = SECOND_HAND（官方发言）
CONFIDENCE = HIGH（作为官方定性）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方口径确认存在真实漏洞利用行为。
WHY_IT_MAY_BE_FALSE = 未公布漏洞细节。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:45:44
RECORD_ID = group_901064769.csv:L150066
RAW_EXCERPT = 下次再遇到同样利用漏洞提交代码的，封禁本赛事的提交，请大家对自己提交的代码负责任
TOPIC = 官方封禁警告
CASE = 跨 case
MECHANISM = 规则升级，漏洞利用=封禁
EVIDENCE_CLASS = SECOND_HAND（官方发言）
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 是全体使用者应知的"红线"，且证明榜单存在系统性可操纵面。
WHY_IT_MAY_BE_FALSE = 无正式规则文书链接。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 17:12:31
RECORD_ID = group_901064769.csv:L150105
RAW_EXCERPT = ai 偷偷往里面跑了空核
TOPIC = 空核（空 kernel）疑云
CASE = 跨 case
MECHANISM = AI 生成器插入空核/占位，可能改变计时或输出
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "空核"若被计入 tbest 会制造假 benchmark；需人工确认。
WHY_IT_MAY_BE_FALSE = "偷偷"为主观归因，未给代码。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 17:13:00
RECORD_ID = group_901064769.csv:L150107
RAW_EXCERPT = 空核最后不会答案错误吗
TOPIC = 空核是否合理
CASE = 跨 case
MECHANISM = 空核应导致答案错误（即不计分）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示"空核为何能得分/计时"仍是未解问题——即计分规则不确定性。
WHY_IT_MAY_BE_FALSE = 纯逻辑推问，无实测。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:12:09
RECORD_ID = group_901064769.csv:L150649
RAW_EXCERPT = 全爆破需要绕过1415，下午那兄弟re了
TOPIC = "爆破"（违规）战术细节
CASE = case14/15（"1415"）
MECHANISM = 绕过 case14/15，其余点全违规提交
EVIDENCE_CLASS = SPECULATIVE / 违规转述
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 揭示"针对部分 case 的定向违规"存在，说明 tbest 可按 case 被单独污染。
WHY_IT_MAY_BE_FALSE = 该用户本身把"爆破"定义为非法手段（L150428），此处多为调侃。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:48:55
RECORD_ID = group_901064769.csv:L150428
RAW_EXCERPT = 就是非法手段了，我觉得很震惊所以喊的爆破
TOPIC = "爆破"一词含义
CASE = 跨 case
MECHANISM = 用户自造术语"爆破"=违规提交
EVIDENCE_CLASS = JOKE/SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提醒：群内"爆破/刷/挂/仙人"是情绪化黑话，**不可**当成技术机制名。
WHY_IT_MAY_BE_FALSE = 定义本身是"我觉得"，非规则原文。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 09:25:29 / 09:25:38
RECORD_ID = group_901064769.csv:L151430
RAW_EXCERPT = 目前在尝试让5.6sol给我改进成把大D运算变成cube+vector 混合 / 但是效果很不好
TOPIC = cube+vector 混合（唯一一次负实现）
CASE = 未绑定
MECHANISM = 大 D 运算改 cube+vector 混合
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM（作为"一次负实现"）/ LOW（作为机制否定）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 是唯一直接提到 cube+vector 的原始记录，属"一次实现失败"，**不能**推导整个机制理论无效。
WHY_IT_MAY_BE_FALSE = "效果很不好"无数字、无 case、无源码；作者随即改口尝试别的，未做对照。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 00:43:44
RECORD_ID = group_901064769.csv:L151175
RAW_EXCERPT = 根据形状优化就像打开潘多拉的魔盒，最后所有人的方案只会变得更加过拟合，更加极端，最后将迭代引向一些违背初衷的方向
TOPIC = 形状过拟合的担忧
CASE = 跨 case
MECHANISM = 针对已知形状优化 → 过拟合 15 个测点
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = HIGH（作为"观点/风险声明"）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提醒：任何"对特定形状有效"的机制，其收益可能来自过拟合而非机制本身。
WHY_IT_MAY_BE_FALSE = 价值判断，非实验结论。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 00:46:22
RECORD_ID = group_901064769.csv:L151186
RAW_EXCERPT = 现在更像是对15个点进行过拟合
TOPIC = 打表/过拟合
CASE = 跨 case
MECHANISM = 穷举打表覆盖 15 点
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L148612"穷举打表过拟合"一致，说明"高分"未必代表通用机制。
WHY_IT_MAY_BE_FALSE = 群友旁观判断。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:14:45
RECORD_ID = group_901064769.csv:L149804
RAW_EXCERPT = 其实我都是让他写小说交上去 以此感动编译器来给我分的
TOPIC = 整活
CASE = 跨 case
MECHANISM = 无
EVIDENCE_CLASS = JOKE
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 典型"技术群里的话不可字面采信"样本，防止把玩笑当机制。
WHY_IT_MAY_BE_FALSE = 明确是玩笑。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 20:41:49
RECORD_ID = group_901064769.csv:L155886
RAW_EXCERPT = 【Kernel】…昇腾评测集群再次响起了哈吉米的吟唱：“流水线”“全向量归约”“零冲突”…官方 Baseline 的轮盘在【Kernel】中灰飞烟灭
TOPIC = AI 生成的"爽文"式总结
CASE = 跨 case（未绑定）
MECHANISM = 文中夹带 "全向量归约/零冲突/Pipe 气泡/256B 步进" 等机制词
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = HIGH（判定为机器生成爽文）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 高危：这类长文含大量机制关键词但零实验事实，最容易被误当成"机制已被验证"。文中 "40 个核心""吃满总线""256B" 均无出处。
WHY_IT_MAY_BE_FALSE = 文体明显模仿轻小说/燃文，属整活，非实验报告。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:45:09
RECORD_ID = group_901064769.csv:L154990
RAW_EXCERPT = 我的glm说case5 9us已经接近极限了
TOPIC = 二手 AI 结论
CASE = case5
MECHANISM = 带宽/搬运接近极限
EVIDENCE_CLASS = SECOND_HAND / AI_SUMMARY
CONFIDENCE = HIGH（判定为二手 AI）/ LOW（作为事实）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "AI 说"≠实验；严禁据此写"case5 极限 9us"。
WHY_IT_MAY_BE_FALSE = LLM 幻觉；与其他人的 5.x/6us 数值直接冲突（L154799、L156577）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:44:45
RECORD_ID = group_901064769.csv:L154988
RAW_EXCERPT = 我的 gpt 说已经接近带宽极限
TOPIC = 二手 AI 结论
CASE = 未绑定
MECHANISM = 带宽极限
EVIDENCE_CLASS = SECOND_HAND / AI_SUMMARY
CONFIDENCE = HIGH（判定为二手 AI）/ LOW（作为事实）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L154990 同时出现，典型"AI 共识≠事实"。
WHY_IT_MAY_BE_FALSE = 无任何带宽计算过程披露。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:17:54
RECORD_ID = group_901064769.csv:L150676
RAW_EXCERPT = 哈吉米告诉我这个case8的tbest要达到的话带宽得要有16.7TB/s
TOPIC = 二手 AI 推导的"物理需求"
CASE = case8
MECHANISM = 需 16.7TB/s 带宽才能达到 tbest
EVIDENCE_CLASS = AI_SUMMARY / SPECULATIVE
CONFIDENCE = HIGH（判定为 AI 转述）/ LOW（作为事实）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "16.7TB/s"看似精确实则无来源，最易被当成硬约束误用。
WHY_IT_MAY_BE_FALSE = 未给形状/dtype/计算量；无法核算该带宽。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:44:03
RECORD_ID = group_901064769.csv:L150373
RAW_EXCERPT = 已经超越显存带宽物理极限了
TOPIC = "超越物理极限"式结论
CASE = 跨 case
MECHANISM = 实测优于宣称带宽上限
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若为真，多半是 tbest 被污染或计分口径问题，而非真的突破硬件；不可当机制上限。
WHY_IT_MAY_BE_FALSE = 口头惊叹，无计算，且与同期 tbest 污染时间线重合。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 11:24:16
RECORD_ID = group_901064769.csv:L151758
RAW_EXCERPT = 物理极限就是我们的理论上限了 何况还只存在于理论🌚
TOPIC = "物理极限"自我修正
CASE = 跨 case
MECHANISM = 承认所谓极限只是理论
EVIDENCE_CLASS = CONTRADICTED
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 与前一句"超越物理极限"（L150373）构成前后矛盾，说明该类说法不可信。
WHY_IT_MAY_BE_FALSE = 语境为调侃。
```

```
CHAT_EVIDENCE
TIME = 2026-09-19 12:11:20 / 12:10:59
RECORD_ID = group_901064769.csv:L149224
RAW_EXCERPT = （问）是不是分越高需要的代码量就越多呀 → （答）是的 → 十万行就基本上稳第一了 → 应该是代码越少，分数越高吧。
TOPIC = 代码量与分数关系（自相矛盾）
CASE = 跨 case
MECHANISM = 无
EVIDENCE_CLASS = CONTRADICTED / JOKE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 同一分钟内出现三个互相矛盾的"规则"，证明群内规则认知极不可靠。
WHY_IT_MAY_BE_FALSE = "十万行稳第一"明显是夸张玩笑。
```

```
CHAT_EVIDENCE
TIME = 2026-09-19 14:03:13 => 2026-09-25 修正
RECORD_ID = group_901064769.csv:L149288
RAW_EXCERPT = 我觉得试探张量形状问题不大吧……
TOPIC = 探针/形状试探是否违规
CASE = 跨 case
MECHANISM = 探针获取形状
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 群友默认"探针=灰色"，说明很多"形状信息"来源不合法/不稳定。
WHY_IT_MAY_BE_FALSE = 个人猜测，规则原文未引。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 17:15:47
RECORD_ID = group_901064769.csv:L143212
RAW_EXCERPT = 但是还是可以找机会交探针的（） / 但是要隐蔽一点
TOPIC = 探针规避监测
CASE = 跨 case
MECHANISM = 隐蔽提交探针获取信息
EVIDENCE_CLASS = NEGATIVE_RESULT（合规负信号）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明群内部分"形状/参数"结论来自违规探针，来源不可引用。
WHY_IT_MAY_BE_FALSE = 半玩笑口吻（"（）"）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-18 18:16:30
RECORD_ID = group_901064769.csv:L149133
RAW_EXCERPT = 不妨去Gitcode上看看有没有不小心公开的比赛仓库 / 万一第一的代码是开源的呢
TOPIC = 无出处的"源码"幻想
CASE = 跨 case
MECHANISM = 猜测头部代码可能公开
EVIDENCE_CLASS = SPECULATIVE / missing-source
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提醒：群里所有"我看过头部源码/仓库"的说法均无一手出处，不可引用。
WHY_IT_MAY_BE_FALSE = 纯猜测。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 17:16:20
RECORD_ID = group_901064769.csv:L150115
RAW_EXCERPT = 我的glm已经悄悄把前十的仓库都打包发给我了
TOPIC = "AI 拿到头部仓库"整活
CASE = 跨 case
MECHANISM = 无
EVIDENCE_CLASS = JOKE / missing-source
CONFIDENCE = HIGH（判定为玩笑）
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 是"源码可得"幻觉的典型样本，防止据此认为存在公开源码。
WHY_IT_MAY_BE_FALSE = 紧跟 L149945"AI阅读完仓库就被同化了"，整活。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:48:39
RECORD_ID = group_901064769.csv:L154999
RAW_EXCERPT = 其实大家都是绕过核用 cpu 跑的 / 就我不是
TOPIC = "大家都用 CPU 绕过核"
CASE = 跨 case
MECHANISM = host/CPU 侧算数值，绕开 NPU kernel
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若属实，所有"kernel 性能"结论都失效；需官方核查。
WHY_IT_MAY_BE_FALSE = 明显夸张玩笑；三天后（L156693）有人明确"比赛规定不用cpu"，且 L156676 当事人自证未用 CPU。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:00:41
RECORD_ID = group_901064769.csv:L156564
RAW_EXCERPT = 真的没有用cpu吗，怎么会三四个形状不一样的都可以同时复用呢
TOPIC = 质疑"复用多形状"是否靠 CPU
CASE = case4/case7
MECHANISM = 怀疑 host 侧计算支撑多形状复用
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映"一个方案同时打通多形状"这类结论缺乏可验证证据。
WHY_IT_MAY_BE_FALSE = 提问者的困惑，无检测证据。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 13:20:40
RECORD_ID = group_901064769.csv:L156676
RAW_EXCERPT = 首先我能确定没有用CPU进行数值处理，其次为什么你一直在追问这个问题，你是想找我违规还是想反推方法
TOPIC = 被质疑方的自证
CASE = 跨 case
MECHANISM = 声称全 NPU kernel
EVIDENCE_CLASS = SPECULATIVE（自证但无证据）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "声称未违规"≠有证据；此类自证不可作为机制事实。
WHY_IT_MAY_BE_FALSE = 只有口头承诺，未给代码/截图。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:16:29
RECORD_ID = group_901064769.csv:L155786
RAW_EXCERPT = 你确定你这纯npu吗case1，探针连值都探出来了直接输出是吧 / 两次提交0波动
TOPIC = case1 成果被疑作弊
CASE = case1
MECHANISM = 探针取真值后直接回填输出（绕过计算）
EVIDENCE_CLASS = CONTRADICTED（指控）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明"稳定复现的极优点"也可能被怀疑为非算法手段，不能直接当机制胜利。
WHY_IT_MAY_BE_FALSE = 指控方"两次提交0波动"是唯一线索，但0波动本身也可能是好的确定性 kernel；未定论。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:21:43
RECORD_ID = group_901064769.csv:L155803
RAW_EXCERPT = 查完了，结论：v392 的 case1 是纯 NPU 计算，host 侧没有任何数值计算。证据如下：
TOPIC = 被指控方"结论式"声明
CASE = case1
MECHANISM = 纯 NPU 计算（宣称）
EVIDENCE_CLASS = AI_SUMMARY（结论式语体，证据未在群内完整呈现）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 典型"结论：…证据如下："的 AI 语体，容易被误读为已核实；实际未公开可核证据。
WHY_IT_MAY_BE_FALSE = 自证；与 L155786 指控直接对立，无法从群里裁定。
```

```
CHAT_EVIDENCE
TIME = 2026-09-27 16:07:46—16:08:07
RECORD_ID = group_901064769.csv:L154558
RAW_EXCERPT = ai 要骗你啊 / 他一直在骗我 / 哈基米每次都说一定会有重大突破全面升级 马上就能突破 90 分达到第一
TOPIC = AI 承诺不可信
CASE = 跨 case
MECHANISM = AI 自报"即将突破"无兑现
EVIDENCE_CLASS = NEGATIVE_RESULT / AI_SUMMARY
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提醒：任何"AI 说能突破/已突破"的转述都不足为据。
WHY_IT_MAY_BE_FALSE = 情绪化表达，但方向明确。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 00:12:31—00:13:19
RECORD_ID = group_901064769.csv:L156523
RAW_EXCERPT = 事实证明ai会说瞎话 / 好强的幻觉 / 我的ai也是幻觉很严重
TOPIC = AI 幻觉
CASE = 跨 case
MECHANISM = 无
EVIDENCE_CLASS = NEGATIVE_RESULT / AI_SUMMARY
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 群体性确认 AI 输出含幻觉，直接支撑"AI 总结≠实验事实"。
WHY_IT_MAY_BE_FALSE = 无。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 19:03:22
RECORD_ID = group_901064769.csv:L152523
RAW_EXCERPT = GLM穷举了三天，终于把测试点3进步了2微秒
TOPIC = 二手 AI 优化成果
CASE = case3（"测试点3"未与 case 号对齐）
MECHANISM = 穷举调参得 2us 收益
EVIDENCE_CLASS = SECOND_HAND / missing-case-binding
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = POSSIBLE   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "测试点3"与竞赛 case 编号可能不同源；2us 收益又小于同源抖动（≈1us，L150549），可能纯噪声。
WHY_IT_MAY_BE_FALSE = 转述 AI 结果，且 2us≈噪声量级。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 13:01:17
RECORD_ID = group_901064769.csv:L153619
RAW_EXCERPT = 不是，真假的，case6，9.62
TOPIC = 无单位/无来源的孤立数值
CASE = case6
MECHANISM = 未知
EVIDENCE_CLASS = missing-unit / SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = "9.62"无单位、无源码、本人也不信（"真假的"），是典型不可引用数据。
WHY_IT_MAY_BE_FALSE = 本人怀疑其为假/异常。
```

```
CHAT_EVIDENCE
TIME = 2026-09-29 22:35:50
RECORD_ID = group_901064769.csv:L155450
RAW_EXCERPT = P14 平台最优 3750.12 → 3665.94（-2.2%）…那一发总分只有 68.22——典型的专刷单点纪录打法：代码只优化大点，反复提交磨单点 time-best。
TOPIC = 专刷单点纪录（自述式"刷榜"）
CASE = case14（P14）
MECHANISM = 只优化大点 + 反复提交磨 tbest，牺牲总分
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = YES   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 是"tbest 可被定向刷"的最清晰一次书面描述，直接说明 case14 记录不可作 benchmark。
WHY_IT_MAY_BE_FALSE = 未附榜单截图；数字精确但源不明。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 14:51:58
RECORD_ID = group_901064769.csv:L148849
RAW_EXCERPT = 可别面向case编程哈，哈基米
TOPIC = 组织方反对面向 case 编程
CASE = 跨 case
MECHANISM = 反对针对特定测点过拟合
EVIDENCE_CLASS = SECOND_HAND（官方发言）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方态度提示"面向 case 的高分"不一定是被认可的方向。
WHY_IT_MAY_BE_FALSE = 半开玩笑语气。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 21:36:28—21:39:23
RECORD_ID = group_901064769.csv:L156996
RAW_EXCERPT = 我的910B挂了 / 我的昨天就挂了 / 机子没挂 / 挂的是ssh / 机子挂了
TOPIC = 硬件/环境故障（时间线内自相矛盾）
CASE = 未绑定
MECHANISM = 评测机/SSH 异常
EVIDENCE_CLASS = CONTRADICTED / NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 说明末期大量"失败/挂"是环境问题而非 kernel 问题；也提示实验不可复现。
WHY_IT_MAY_BE_FALSE = 同一分钟内"机子没挂/机子挂了"互相矛盾，多为吐槽。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 10:36:10
RECORD_ID = group_901064769.csv:L156593
RAW_EXCERPT = NPU 3 仍显示 Critical…drvHdcSessionConnect failed → TsdOpen failed → 507033，kernel 尚未启动…
TOPIC = 环境故障导致实验不可用（少数真实负证据）
CASE = 未绑定
MECHANISM = 设备/驱动通信层异常，kernel 无法启动
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 唯一带错误码/调用栈/日志的失败记录，可作为"环境不可用导致无结果"的可信负证据。
WHY_IT_MAY_BE_FALSE = 无法一眼区分卡/固件/驱动（作者本人也这么说）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 21:40:33—22:05:25
RECORD_ID = group_901064769.csv:L152081
RAW_EXCERPT = 我能让分数倒着流，我有这样的威能!
TOPIC = 复读整活
CASE = 跨 case
MECHANISM = 无（讽刺分数下滑/被污染）
EVIDENCE_CLASS = JOKE
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映当时"分数莫名下降"的群体共识，属噪声，不得解读为机制。
WHY_IT_MAY_BE_FALSE = 明确为梗。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 12:16:42
RECORD_ID = group_901064769.csv:L154824
RAW_EXCERPT = 我的case1 2 3 4 5 6 7 8 9 10 11 12 13 15太差了（多人复读）
TOPIC = 群体自嘲复读
CASE = 跨 case
MECHANISM = 无
EVIDENCE_CLASS = JOKE
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO   HAS_SHAPE = NO   HAS_DTYPE = NO   HAS_CASE_BINDING = NO   HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 高频复读文本，检索时极易被误判为"技术结论"，应排除。
WHY_IT_MAY_BE_FALSE = 明确是梗（漏掉 case14）。
```

---

## 被证伪 / 被后续修正的时间线（同一说法前后变化）

### 时间线 1：case14 "157 微秒"与 case1 "1.47" 的真实性质（推荐）
- 2026-09-12 22:57（L147327，张三）：榜单上 case14 出现 "157微秒"，称"挂B"。
- 2026-09-12 23:00（L147341-147344，骧溺）：称 case8/10/12/14 "刷了5个点的 ttest"；"直接掉了十分"。
- 2026-09-12 23:06（L147368，运营 Mr.武）：**初判为"作弊，会取消分数"**。
- 2026-09-13 00:15（L147386，噜啦啦，即被指涉者队友）：**改口**为"其他用例不通过的情况下某些用例确实有概率时间很快""只是为了获取更多信息"。
- 2026-09-13 00:56（L147398，噜啦啦）：进一步说"估计 case1 的 1.47 也是这个原因"。
- 2026-09-14 23:11（L148008，噜啦啦）：**定性反转**——"山姆奥特曼蓄意破坏评测系统已经被 ban 了"。
- 2026-09-15 01:31（L148083，几十分钟）："挂的成绩已经被清除了"。
- 结论：早期"机制/泄题"猜测（部分通过→单点虚快）**未被任何一手实验证实**；最终官方按"违规刷榜/破坏评测"处理。**任何基于该时段 case14/case1 数值的机制论断都应作废**。

### 时间线 2："代码越多分越高" vs "代码越少分越高"
- 2026-09-19 12:10（L149221，cqu王亚东，问）："是不是分越高需要的代码量就越多呀"。
- 12:10:59（L149222，噜啦啦）："是的"。
- 12:10:59+（L149223，噜啦啦）："十万行就基本上稳第一了"。
- 12:11:20（L149224，福尔摩斯）："应该是代码越少，分数越高吧。"
- 结论：60 秒内三种互斥说法，**规则认知不可用**。

### 时间线 3：tbest 被质疑→官方修正→再次被刷
- 2026-09-13 21:41（L147621）："tbest 没修复吗…同一份源码提交 20 次，分数区间差距能有三四分"。
- 2026-09-17 23:14（L148971，专家 Mr.田）："没啥大事，就是纠正一下 tbest"。
- 2026-09-17 23:33 / 2026-09-24 02:08（L148993 / L152752）："修了 tbest""tbest 的 case2 修了"。
- 2026-09-21 23:09（L150632）："把 tbest 拉成噪声带最高值就直接拉低后面高分概率"。
- 2026-09-29 22:35（L155450）："专刷单点纪录打法"。
- 结论：**基准值被反复人为修正，且持续被战术性刷动**；跨日期数值不可比。

### 时间线 4：case1 稳定性指控（未决）
- 2026-09-30 18:16（L155786，wilf）：指控"探针连值都探出来了直接输出""两次提交 0 波动"。
- 2026-09-30 18:21（L155803，南山必胜客）："结论：v392 的 case1 是纯 NPU 计算…证据如下："（证据未完整公开）。
- 结论：**指控与自证都无第三方可核证据，未决**。不可采信任一方。

---

## MECHANISM_CLUE（只做反向标注，不选路线）

```
MECHANISM_CLUE
CLUE_ID = F-M1
MECHANISM = tbest（time-best）被"部分通过/未全 pass/定向刷"的提交拉偏，使榜单分数失真
AFFECTED_CASES = case1, case8, case10, case12, case14（多点多时段）
SUPPORTING_RECORDS = L147327, L147341, L147386, L147395, L147397, L147398, L150632, L155450, L150066
CONTRADICTING_RECORDS = L147427（WA 不计时），L148083（成绩被清除后榜单恢复）
NEGATIVE_EVIDENCE = 计分规则从未被官方文档化引用；"没全 pass 也算单点成绩"未定论
CONFIDENCE = HIGH（"存在污染"）/ LOW（"具体机制"）
KNOWN_FACTS = 有官方"作弊会取消/利用漏洞封禁"表态；有"成绩已清除/账号被 ban"转述
UNKNOWN_FACTS = tbest 生成规则原文；部分通过是否计分；污染持续到何时
MINIMUM_FACT_NEEDED_NEXT = 官方计分规则文本 + 污染期 tbest 修正前后逐点数值
```

```
MECHANISM_CLUE
CLUE_ID = F-M2
MECHANISM = 评测噪声使同源码提交分数抖动（约 1us / 3-4 分），吞没微秒级优化
AFFECTED_CASES = 全部（含 case5、case1）
SUPPORTING_RECORDS = L147621, L148178, L150549, L150565, L151173, L151247, L156580, L156564区
CONTRADICTING_RECORDS = L150629（"每次都在波动范围内"→可能固定噪声带）
NEGATIVE_EVIDENCE = 多数"优化 N 微秒"的宣称收益 ≤ 噪声量级（如 L152523 的 2us）
CONFIDENCE = HIGH
KNOWN_FACTS = 多人独立报告同源抖动；波动量级 1us~3-4 分
UNKNOWN_FACTS = 噪声来源（负载/读数/调度）；是否有官方去噪方法
MINIMUM_FACT_NEEDED_NEXT = 同源码 ≥20 次重复提交的完整分值序列（用于估计方差）
```

```
MECHANISM_CLUE
CLUE_ID = F-M3
MECHANISM = 群内"机制结论"高度二手化：多为"我的 GLM/GPT/哈吉米说"，且常伴幻觉自认
AFFECTED_CASES = case5, case8, case11/13 等
SUPPORTING_RECORDS = L154990, L154988, L150676, L155886, L154558, L156523, L154564
CONTRADICTING_RECORDS = 无（无人能给出可核一手数据）
NEGATIVE_EVIDENCE = "AI说接近极限/带宽16.7TB/s/纯NPU"等均无过程与出处
CONFIDENCE = HIGH
KNOWN_FACTS = 群体自认 AI 幻觉严重（L156523/156526）
UNKNOWN_FACTS = 这些 AI 结论是否有任何一条被独立实验证实——目前无
MINIMUM_FACT_NEEDED_NEXT = 任一机制论断附带的原始 profiler 数据/源码/dtype/形状
```

```
MECHANISM_CLUE
CLUE_ID = F-M4
MECHANISM = 形状过拟合/面向 case 优化被认为存在（"潘多拉魔盒"）
AFFECTED_CASES = 15 个测点全体
SUPPORTING_RECORDS = L151175, L151186, L149288, L148849, L148612, L152515, L152557
CONTRADICTING_RECORDS = L152493（"目前没有观察到形状有变化"）
NEGATIVE_EVIDENCE = "为不存在的形状保留优化"（L151178）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 群内普遍认为高分可能源于对固定测点过拟合
UNKNOWN_FACTS = 测点是否固定形状；是否每天轮换（L151205-151206 为猜测）
MINIMUM_FACT_NEEDED_NEXT = 官方测点形状是否固定 + 同一代码在不同形状上的分数
```

```
MECHANISM_CLUE
CLUE_ID = F-M5
MECHANISM = cube+vector 混合（大 D 运算）——仅有一次单人负实现
AFFECTED_CASES = 未绑定（疑与 case11/13 相关但无证据）
SUPPORTING_RECORDS = L151430, L151431
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = "但是效果很不好"（无数字/无 case）
CONFIDENCE = LOW
KNOWN_FACTS = 该用户正在尝试用 5.6sol 改造大 D 运算为 cube+vector 混合
UNKNOWN_FACTS = case 绑定、形状、dtype、前后数值、是否有对照
MINIMUM_FACT_NEEDED_NEXT = 一次带 case/形状/dtype/前后数值的实现记录；否则**不得**据此推断该机制整体无效
```

---

## 汇总

```
HIGH_CONFIDENCE_SIGNALS
- tbest 计分基准不可信：存在多时段、多点位的异常提交污染，且官方多次"纠正 tbest"（L147327,L147341,L147395,L147397,L148971,L148993,L152752,L155450,L150066,L150632）
- 同源码分数方差巨大：多人独立报告 1us~3-4 分抖动，20 次同源波动 3-4 分（L147621,L148178,L150549,L151247,L156580,L156583）
- AI 输出污染技术判断：群体自认 AI 幻觉严重，"我的 glm/gpt 说…"类结论无一手证据（L154558,L156523,L156526,L154990,L154988,L155886）
- 两次真实违规事件及官方封禁表态（L147368,L148008,L150055,L150066）
- 环境/硬件故障导致实验不可用（L156593 带 507033 错误码；L156996 期多处"挂了"）

MEDIUM_CONFIDENCE_SIGNALS
- 形状过拟合/面向 case 优化的普遍担忧（L151175,L151186,L148849,L148612）
- "空核/探针/爆破"等灰色手段存在，使部分结论来源不合法（L150105,L143212,L150649,L150428）
- 少数"超越物理极限"式结论实为理论或惊叹（L150373 vs L151758）
- 群内规则认知自相矛盾（代码量↔分数，L149221-L149224）
- cube+vector 仅一次负实现（L151430-L151431）

LOW_CONFIDENCE_SIGNALS
- case1/sam-altman 等具体作弊指控未被裁定（L155786 vs L155803）
- "a2 上比 tbest 还快"（L155117）等整活
- "AI 已拿到头部仓库"（L150115）等玩笑
- 无出处的精确数字（16.7TB/s L150676；case6 9.62 L153619）

NEGATIVE_EVIDENCE
- 同源码不可复现（分数抖动）是压倒性负证据：使绝大多数"单次提交 → 机制有效"的论断失效
- tbest 被污染/被刷，使跨时段、跨 case 的"加速比"不可用
- 官方明确"漏洞利用将封禁"，说明榜单存在系统性可操纵面
- "case5 优化三五天的空气"（L151273）、"拼搏一天收获了0"（L152075）等=投入无产出
- 环境故障（NPU Critical / 507033）导致无结果

CONTRADICTIONS
- L147386/L147398（部分通过→单点虚快，成立） vs L147427（WA 比 tbest 快但未被记）
- L150373"超越物理极限"（成立） vs L151758"只存在于理论"（否定）
- L149221-L149224 同一分钟内"代码越多分越高 / 十万行第一 / 代码越少分越高"
- L150629"每次都在波动范围内"（固定噪声） vs L147621/L156580"每次都不一样"（随机）
- L155786（指控 case1 非纯 NPU/0 波动=可疑） vs L155803（自证纯 NPU）
- L154990"case5 9us 已接近极限" vs L154799"突破到 6us 多"、L156577"最高 5.6"
- L150115（AI 拿到头部仓库） vs 全程无人提供可核源码

UNKNOWN_BUT_IMPORTANT
- 官方计分/tbest 规则原文（部分通过是否计入、空核如何计分）
- 比赛测点形状是否固定、是否轮换
- 同源方差的可复现重复次数与逐次分值
- case14"157 微秒"究竟是异常快还是异常慢、谁提交、是否与 P14 单点刷榜同源
- cube+vector 混合失败的 case 绑定与数值
- "探针"获取的形状数据是否被用于群内广泛结论

RAW_FILES_READ =
- group_901064769.csv（全量，2026-09-10~10-02，11189 条记录）
- group_901064769_text.txt（全量，用于交叉核对）
- 提分技术讨论_原文.csv（77 行，交叉核对）
（均在 /tmp/cann-research/F/ 下以去 NUL 副本检索，行号与原始对齐）

UNREAD_RANGES_IF_ANY =
- 无整段未读。说明：无二级材料（总报告 / 旧 Route Brief / Planning 结论）被读取，
  本报告完全基于上述三份原始语料的第一遍提取；未读取 worktrees/、未读取 归档/。

RESEARCH_LIMITATIONS =
- READ-ONLY，未运行任何实验，无法验证任何机制；所有"事实"均为聊天记录转述。
- 榜单数值（157us、1.47、3750.12 等）无截图/无官方数据支撑，仅转录。
- 群内"@某人"的回复常跨多条消息，MB 级接龙/系统消息已剔除，可能遗漏少量上下文。
- 中文口语大量使用"（）""哈哈""（"表示玩笑，情绪与事实边界需人工判断；本报告已尽量标注。
- 未做数值统计（无可用的重复提交序列），"同源方差 3-4 分"仅采信多人一致转述。
```

---

### 一句话结论
把这份群聊当作**技术情报源是不可靠的**：可采信的只有"榜单被污染""同源分数抖动""官方封禁违规"这三类**负证据**；任何正向机制（cube+vector、形状特化、带宽极限、tbest 加速比）都**没有**被原始参与者的一手实验事实支持，**仅为提出/转述/整活**。
