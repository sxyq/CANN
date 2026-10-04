# RESEARCH-D（CASE-CENTRIC）取证报告

- 研究类型：只读社区语料取证（QQ 群 901064769 全时段原始聊天）
- 聚焦 case：case1 / case4 / case7 / case14
- 主证据源：`group_901064769.csv`（1-based 行号为 RECORD_ID）
- 隔离目录：`/tmp/cann-research/D/`
- 方法：先只用原始聊天做多形态检索（`caseN` / `case N` / `caseNN` / `测点N` / `测试点N` / `PN` / 昵称别名），完成原始提取后，再读二级材料做遗漏/上下文校验。
- 关键别名映射（raw 内自证）：`测试点1`=`case1`（L148708）；`测点7/测试点7`=`case7`（L148524/150826）；`测点14`=`case14`（L148527）；`P14`=`case14` 平台最优项（L155449）。raw 中未出现 `Tiny`、`c1/c4/c7/c14` 等写法（检索为空）。

---

## 一、case1 证据

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:08:53
RECORD_ID = group_901064769.csv:L147375
RAW_EXCERPT = 先把case1那个1.47清除吧
TOPIC = tbest 记录存疑/清理请求
CASE = case1
MECHANISM = tbest（历史最优）记录本身可疑
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = UNKNOWN  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case1 的 1.47us 在全时段被当作标杆引用，但此处首次被要求“清除”，说明它可能不是干净成绩。
WHY_IT_MAY_BE_FALSE = 只是某人主观请求，无平台裁决证据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:56:18
RECORD_ID = group_901064769.csv:L147398
RAW_EXCERPT = 估计case1的1.47也是这个原因
TOPIC = 1.47us 归因于“未全通过也能刷单点快”的漏洞
CASE = case1
MECHANISM = 部分通过 + 计时漏洞刷单点 tbest
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 直接质疑 case1 1.47us 的有效性（与 09-12 的 case14 事件同因）。
WHY_IT_MAY_BE_FALSE = 说话者用“估计”，无复现实证；且 149987 又说“挂都没打破 case1 的 tbest”，暗示 1.47 未必来自漏洞。
```

```
CHAT_EVIDENCE
TIME = 2026-09-14 23:09:05
RECORD_ID = group_901064769.csv:L148002
RAW_EXCERPT = case1的1.47us呢
TOPIC = 1.47us 作为对标数值被反复引用
CASE = case1
MECHANISM = 标杆值引用
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明 1.47us 已在群里固化为 case1 的“目标线”。
WHY_IT_MAY_BE_FALSE = 引用者只是问“呢”，未确认来源是否干净。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 12:38:14
RECORD_ID = group_901064769.csv:L148708
RAW_EXCERPT = 为啥测试点 1 的 Tbest 在榜上找不到
TOPIC = 测点1=case1 的 tbest 在榜单不可见
CASE = case1
MECHANISM = tbest 展示/修复问题
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 自证 `测试点1 == case1`；同时暴露 tbest 显示与修复时间线（官方 147851 “Tbest 问题晚点修复”）。
WHY_IT_MAY_BE_FALSE = 仅说明榜单显示问题，不代表成绩本身。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:17:45
RECORD_ID = group_901064769.csv:L149849
RAW_EXCERPT = case1是真的那个唯一一个2以下的达到的，奶蛙的case7，case9我不知道，真的摸不到门道啊
TOPIC = case1 是唯一 sub-2us，且 case7 属“奶蛙”
CASE = case1（并牵连 case7/case9）
MECHANISM = 无（结果描述）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 支持“case1 sub-2us 是真实存在”的旁证，并把 case7 与“奶蛙”队绑定。
WHY_IT_MAY_BE_FALSE = 个人观察，无原始榜单截图。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:30:25
RECORD_ID = group_901064769.csv:L149987
RAW_EXCERPT = 笑点解析：挂都没打破case1的tbest
TOPIC = 作弊提交未能超过 case1 tbest
CASE = case1
MECHANISM = 计时/空kernel 漏洞（当日 421 队事件）
EVIDENCE_CLASS = JOKE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 147398 矛盾：若 1.47 来自漏洞，则新漏洞“都打不破”说明 case1 tbest 可能另有其因（真实或更强漏洞）。
WHY_IT_MAY_BE_FALSE = 调侃语气，且未指明“挂”在 case1 上具体跑了多少。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 21:43:11
RECORD_ID = group_901064769.csv:L152652
RAW_EXCERPT = case1是舍弃局部换整体的吗？
TOPIC = case1 是否“牺牲局部换整体”的策略
CASE = case1
MECHANISM = 全局/局部取舍（策略猜测）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 群内对 case1 优化取向的少数机制性猜测。
WHY_IT_MAY_BE_FALSE = 纯提问，无实现与数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 21:50:45
RECORD_ID = group_901064769.csv:L152662
RAW_EXCERPT = 这个case1的tbest太高了，按公式算下来我case1就六十多
TOPIC = case1 tbest 太高导致个人分被压
CASE = case1
MECHANISM = 计分公式（越接近 tbest 分越高）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明 case1 的 tbest 被他人刷高后，普通队伍得分被拉低，是“tbest 泛滥”问题的具体受害陈述。
WHY_IT_MAY_BE_FALSE = 无截图，公式细节未给。
```

```
CHAT_EVIDENCE
TIME = 2026-09-24 13:20:53
RECORD_ID = group_901064769.csv:L152855
RAW_EXCERPT = ggbond 的 case1 好厉害
TOPIC = ggbond 的 case1 成绩被认可
CASE = case1
MECHANISM = 无（结果描述）
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 154981/155798 一致，把 case1 强解归到 ggbond。
WHY_IT_MAY_BE_FALSE = 第三方赞叹，非 ggbond 本人实验陈述。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 00:44:51
RECORD_ID = group_901064769.csv:L153356
RAW_EXCERPT = 我感觉我的case1case2又成短板了，case4好不容易掉下来，这边又上去了
TOPIC = case1/case2 回退为短板；case4 有进步
CASE = case1（并 case2/case4）
MECHANISM = 不同点之间此消彼长
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 同时给出 case4“掉下来（时间下降=变好）”的 before/after 方向，和 case1 回退信号。
WHY_IT_MAY_BE_FALSE = 主观“感觉”，同源码波动大（见 147621）会污染判断。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 03:20:34
RECORD_ID = group_901064769.csv:L153429
RAW_EXCERPT = 我觉得最难打的就这45679了，那个1.7的cae1除外，那是特级
TOPIC = 最难打点簇 45679；case1 1.7 属“特级”
CASE = case1（并 4/5/6/7/9）
MECHANISM = 点簇难度分层
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 将 case1 与 case4/6/7 一并列为高难，且给 case1 一个 1.7us 稳定值。
WHY_IT_MAY_BE_FALSE = 个人难度感受，非测量。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:43:57
RECORD_ID = group_901064769.csv:L154982
RAW_EXCERPT = 前20就他一个case1 2us以内的
TOPIC = 前20名中仅一人 case1 sub-2us
CASE = case1
MECHANISM = 无（榜单观察）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 强证据：case1 sub-2us 极稀缺；配合 L154981“我估计就是ggbond刷的”。
WHY_IT_MAY_BE_FALSE = 依赖说话者读榜准确性。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:16:29
RECORD_ID = group_901064769.csv:L155785
RAW_EXCERPT = 你确定你这纯npu吗case1，探针连值都探出来了直接输出是吧
TOPIC = 质疑 case1 成绩是“探针直接输出”
CASE = case1
MECHANISM = 探针（probe）推断输入 + 可能直接输出
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 把 case1 的“异常稳定/超低”与探针/直接输出关联，是有效性负面证据。
WHY_IT_MAY_BE_FALSE = 是质疑不是结论；同段 L155802 的自查反而称“纯 NPU、host 无数值计算”。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:20:58
RECORD_ID = group_901064769.csv:L155798
RAW_EXCERPT = 第一次修之前case1是1.5，grok爆破1.24，后面就是ggbond的稳定1.7左右，彩票1.6左右
TOPIC = case1 tbest 历史数值链
CASE = case1
MECHANISM = 无（数值史）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 全时段最有信息量的一条：给出 case1 的 1.5(修前)→1.24(grok爆破)→1.7(ggbond稳定)→1.6(彩票) 序列，是 case1 标杆值的唯一完整口述史。
WHY_IT_MAY_BE_FALSE = 单一参与者复述，可能有记忆/口径误差；未标单位（应为 us）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:21:43
RECORD_ID = group_901064769.csv:L155802
RAW_EXCERPT = 查完了，结论：v392 的 case1 是纯 NPU 计算，host 侧没有任何数值计算。证据如下：
TOPIC = 对 v392 版本 case1 的自动自查结论
CASE = case1
MECHANISM = 纯 NPU 计算、host 无数值计算
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提供“case1 非 CPU 作弊”的一种反证；但来源是 AI 自查。
WHY_IT_MAY_BE_FALSE = AI 总结≠原始实验事实；说话者自己随后也说“感觉有问题”（L155453）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:48:24
RECORD_ID = group_901064769.csv:L155855
RAW_EXCERPT = 是吗？我认可你的实力了。但接下来的case1 2这连续3个tbest会很疯狂
TOPIC = case1/case2 tbest 将被连续刷
CASE = case1（并 case2）
MECHANISM = tbest 泛滥
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L155856 完全复读，是 case1 “tbest 被反复刷新”的重复信号代表。
WHY_IT_MAY_BE_FALSE = 半玩笑口气，无实际数值。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 11:34:49
RECORD_ID = group_901064769.csv:L156605
RAW_EXCERPT = 你踏马到底怎么爆破的case1
TOPIC = 众人追问 case1 的“爆破”方法
CASE = case1
MECHANISM = “爆破”（探针/穷举式探测）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 表明到 10-02 仍无人能复现 case1 解法；L156607 哈吉米称“case1 确实可以做到”。
WHY_IT_MAY_BE_FALSE = 对话式追问，方法仍未公开。
```

case1 结论段落：
case1 是全群讨论量最大的点之一，也是唯一被反复称为“sub-2us 独苗”的点。原始聊天里 case1 的数值链条清晰（L155798）：1.5(修前) → 1.24(grok 爆破) → 1.7(ggbond 稳定) → 1.6(彩票)。有效性上存在两条互相冲突的线索：一条把 1.47us 归因于“未全通过也能刷单点快”的漏洞（L147398、L147375），另一条称“挂都没打破 case1 的 tbest”（L149987）、且 case1 是“唯一 2us 以下”（L149849、L154982）。因此 case1 的 1.47/1.7 不能简单判为无效，也不能简单判为干净；它更可能是“真实强解 + 历史漏洞污染 + 高噪声”叠加。群里从未出现 case1 的源码、形状、dtype 或可复现机制，唯一机制性讨论是把超低稳定成绩怀疑为“探针直接输出”（L155785），但又被 AI 自查（L155802）部分反驳。case1 的机制绑定几乎为空：没有 core/row/tile/vector/cube/fusion/pipeline 与 case1 的直接绑定记录。

---

## 二、case4 证据

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:02:41
RECORD_ID = group_901064769.csv:L147358
RAW_EXCERPT = case4就没有低于9的吧 / 低于8
TOPIC = case4 早期下界（无低于 9，后改口低于 8）
CASE = case4
MECHANISM = 无（下界陈述）
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case4 最早的定量锚点：群里认为 case4 极难低于 8~9us。
WHY_IT_MAY_BE_FALSE = “吧”表推测，且说话者紧接着自我修正（9→8），口径不稳。
```

```
CHAT_EVIDENCE
TIME = 2026-09-15 16:34:26
RECORD_ID = group_901064769.csv:L148212
RAW_EXCERPT = 前两名的case4case5太厉害了，赶不到
TOPIC = 头部队伍 case4/case5 领先
CASE = case4（并 case5）
MECHANISM = 无
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case4 与 case5 在早期被并列提到，是后续“case4case7”之外的另一种共现。
WHY_IT_MAY_BE_FALSE = 只谈排名差距，无数值。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 00:29:04
RECORD_ID = group_901064769.csv:L153288
RAW_EXCERPT = case4case7好难打啊
TOPIC = case4/case7 难以攻破
CASE = case4（与 case7 连写）
MECHANISM = 无
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = “case4case7”连写的代表性记录之一，是 case4/7 同簇信号的语言层证据。
WHY_IT_MAY_BE_FALSE = 连写不等于共享机制，可能只是两个都难。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 00:37:48
RECORD_ID = group_901064769.csv:L153333
RAW_EXCERPT = 但是今天打一天case4没下7的方案
TOPIC = 打一天无 sub-7us 方案
CASE = case4
MECHANISM = 无（穷举/尝试失败）
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = case4 的一天实验负结果：无法做到 <7us；给 case4 一个“现状≈7us 及以上”的锚点。
WHY_IT_MAY_BE_FALSE = 单队单日，不代表全群极限。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 13:20:45
RECORD_ID = group_901064769.csv:L153694
RAW_EXCERPT = 昨天就是打了一天的case4case6case7，基本都是中性改动或者反向
TOPIC = 一整天 case4/6/7 改动全部中性或反向
CASE = case4（并 6/7）
MECHANISM = 无（改动无效）
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 强负证据：case4/6/7 的随机/中性改动无效，说明这簇对“瞎改”不敏感。
WHY_IT_MAY_BE_FALSE = 仅代表该队当日的改动方向。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 13:21:06
RECORD_ID = group_901064769.csv:L153695
RAW_EXCERPT = 4567真的比其他点难打
TOPIC = 点簇 4567 比其余点更难
CASE = case4（并 6/7）
MECHANISM = 点簇难度分层
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L153429 的 45679 呼应，4/6/7 属同一“难打簇”。
WHY_IT_MAY_BE_FALSE = 主观难度感受。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:00:41
RECORD_ID = group_901064769.csv:L156563
RAW_EXCERPT = 真的没有用cpu吗，怎么会三四个形状不一样的都可以同时复用呢
TOPIC = 疑惑“三四个不同形状可同时复用一套方案”
CASE = case4（上下文指向 4 及其簇）
MECHANISM = 跨形状通用复用
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = YES（笼统，无具体 shape）  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 引出“一队同时拿下多个不同 shape 点”的疑问，是 case4/7 同簇讨论的触发句。
WHY_IT_MAY_BE_FALSE = 未点名 case4，靠下一条对话锚定。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:06:12
RECORD_ID = group_901064769.csv:L156567
RAW_EXCERPT = 同时做到4和7说明他们这俩大概走的一种类似处理方式 这两个按特征看 理论上也可以类似处理
TOPIC = 4 与 7 “类似处理方式”的机制猜测
CASE = case4（并 case7）
MECHANISM = 相似分发/处理方式（shape 特征相近）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 群里对 case4/7 同簇的唯一“机制性”表述，但仅停留在“按特征看理论上”。
WHY_IT_MAY_BE_FALSE = 说话者用“大概/理论上/🤔”，无实现或 profiling。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:06:35
RECORD_ID = group_901064769.csv:L156569
RAW_EXCERPT = 他说是是4和7是一块攻破的
TOPIC = “4 和 7 是一块攻破的”转述
CASE = case4（并 case7）
MECHANISM = 同簇一并攻破
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case4/7 同簇的关键但二手证据链（“他说是”）。
WHY_IT_MAY_BE_FALSE = 双跳转述，无一手实证；后续 L156618“我自己都不知道”。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:13:52
RECORD_ID = group_901064769.csv:L156583
RAW_EXCERPT = 搞4和7是对的 / 这两个现在分相差的太大了 / 而且破了一个对另外一个也有帮助
TOPIC = “攻 case4/7 可互相受益”
CASE = case4（并 case7）
MECHANISM = 收益迁移（破一助另一）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 把“同簇”从相关性升级为“收益可迁移”的行动假设。
WHY_IT_MAY_BE_FALSE = 说话者自述仍在搞 case5、4/7“还不知道啥时候能弄”，属预判非结论。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 11:35:36
RECORD_ID = group_901064769.csv:L156610
RAW_EXCERPT = 你怎么爆破的4和7
TOPIC = 直接追问 case4/7 的攻破手段
CASE = case4（并 case7）
MECHANISM = “爆破”（探测式求解）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 确认确实有人（他队）同时推进 4 和 7。
WHY_IT_MAY_BE_FALSE = 问答式，方法未落地。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 11:35:50
RECORD_ID = group_901064769.csv:L156612
RAW_EXCERPT = 因为他们同簇 / 爆破手段一样
TOPIC = case4 与 case7 “同簇、爆破手段一样”
CASE = case4（并 case7）
MECHANISM = 同簇 + 相同爆破手段
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = “同簇”原话出处，是全群最直接的同簇表述。
WHY_IT_MAY_BE_FALSE = 紧接着 L156618-156620 承认“我自己都不知道 / 我队友爆破了这么多点”，属其对队友行为的二手归因。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 11:50:20
RECORD_ID = group_901064769.csv:L156632
RAW_EXCERPT = 4和7等你接近了 / 你也做到了
TOPIC = 4/7 存在“接近即自然拿到”的门槛效应
CASE = case4（并 case7）
MECHANISM = 阈值/门槛式收敛
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 暗示 4/7 一旦“接近”则另一自动达成，增强同簇假设。
WHY_IT_MAY_BE_FALSE = 玄学式鼓励语，无数据。
```

case4 结论段落：
case4 的原始证据全部指向“难且不敏感”：早期有人断言“没有低于 9（后改低于 8）”（L147358）；09-25 有队伍“打一天没下 7us 方案”（L153333），同一天另一队“打一天 case4/6/7，基本中性改动或反向”（L153694），说明 case4 对随机/中性改动不响应。case4 本身没有一条机制绑定（无 core/row/tile/vector/cube/fusion/pipeline 的直接绑定）。case4 的价值主要来自它与 case7 的强共现/同簇信号：语言层有反复的“case4case7”（L153288）、点簇“4567/45679”（L153695/L153429）；机制层只有 10-02 的一串推测与二手转述（L156566/567/569/583/585/610/612/632），其中“同簇、爆破手段一样”出自一个承认自己都不清楚、是队友所为的人（L156618-620）。因此：“case4/7 同簇”应作为值得验证的假设（MEDIUM-LOW 置信），不能当成已证实的机制事实；“破一个帮另一个”更只是预判。

---

## 三、case7 证据

```
CHAT_EVIDENCE
TIME = 2026-09-17 01:02:06
RECORD_ID = group_901064769.csv:L148524
RAW_EXCERPT = 奶蛙14us的测点7我真怕了
TOPIC = 测点7=case7，奶蛙成绩 14us
CASE = case7
MECHANISM = 无（标杆值）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case7 的核心标杆值 14us 的原始出处，并自证 `测点7 == case7`。
WHY_IT_MAY_BE_FALSE = 二手赞叹，非奶蛙本人陈述；与项目实测 bestTimeUs 13.99 吻合，可信度较高。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 12:18:42
RECORD_ID = group_901064769.csv:L148619
RAW_EXCERPT = 比如奶娃的14us的测点7
TOPIC = 14us case7 复述（“奶娃”/“奶蛙”混写）
CASE = case7
MECHANISM = 无
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 14us case7 的重复信号，昵称“奶蛙/奶娃”指同一队。
WHY_IT_MAY_BE_FALSE = 同上，转述。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 12:27:32
RECORD_ID = group_901064769.csv:L148654
RAW_EXCERPT = 算分机制，越靠近tbest，彩票影响就更大，所以我现在都一直在抽奖
TOPIC = 计分机制：越接近 tbest 波动影响越大
CASE = 未点名（一般机制）
MECHANISM = 计分公式 + 同源码波动（彩票）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 解释为何 case7 这类“贴着 tbest”的点分数高噪声，影响对其数值有效性的判断。
WHY_IT_MAY_BE_FALSE = 个人推断，非官方公式。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 00:03:50
RECORD_ID = group_901064769.csv:L150826
RAW_EXCERPT = 奶蛙的测试点7还是太逆天了
TOPIC = 测点7=case7 再次被称“逆天”
CASE = case7
MECHANISM = 无
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case7 高难度/强解的重复信号。
WHY_IT_MAY_BE_FALSE = 情绪化表述。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 09:43:35
RECORD_ID = group_901064769.csv:L151512
RAW_EXCERPT = 我发现我现在最烂的是case6和7
TOPIC = 个人自评 case6/case7 最差
CASE = case7（并 case6）
MECHANISM = 无
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case6/7 作为“个人短板”的独立陈述，支持 4/6/7 同簇难度。
WHY_IT_MAY_BE_FALSE = 自评，无分数。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 00:48:25
RECORD_ID = group_901064769.csv:L153371
RAW_EXCERPT = 明天看看试试奶蛙的case7
TOPIC = 打算尝试奶蛙的 case7 思路
CASE = case7
MECHANISM = 无（模仿意图）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明 case7 的先进解属于“奶蛙”，他人想复刻。
WHY_IT_MAY_BE_FALSE = 只是计划，非结果。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 18:45:36
RECORD_ID = group_901064769.csv:L153828
RAW_EXCERPT = 大肥鱼，我命令你没有npu的情况下打过case7
TOPIC = 玩笑：无 NPU 也要打过 case7
CASE = case7
MECHANISM = 无
EVIDENCE_CLASS = JOKE
CONFIDENCE = N/A（调侃）
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映 case7 被当作“炫耀性难点”，无技术信息。
WHY_IT_MAY_BE_FALSE = 纯玩笑，不可作事实。
```

```
CHAT_EVIDENCE
TIME = 2026-09-27 01:39:17
RECORD_ID = group_901064769.csv:L154419
RAW_EXCERPT = 奶娃的 case7 偷了
TOPIC = 声称“偷到”奶娃 case7 方案
CASE = case7
MECHANISM = 无（抄袭式复刻声明）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若为真，说明 case7 存在可复制的“奶蛙解”；但未见数值。
WHY_IT_MAY_BE_FALSE = “偷了”多为口号/玩笑，无证据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-27 14:41:31
RECORD_ID = group_901064769.csv:L154526
RAW_EXCERPT = 我代码基本稳85就是抽不上去，想不通奶蛙神的case67
TOPIC = 稳定 85 分但上不去，卡在 case6/7
CASE = case7（并 case6）
MECHANISM = 抽卡/波动天花板
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出 case6/7 是“分数天花板”的直接体感。
WHY_IT_MAY_BE_FALSE = 个人分数陈述，无原始提交。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 23:44:10
RECORD_ID = group_901064769.csv:L155100
RAW_EXCERPT = 下一步就偷看奶蛙神的case67
TOPIC = 继续想复刻奶蛙 case6/7
CASE = case7（并 case6）
MECHANISM = 无
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = “奶蛙 case6/7”反复出现的代表记录（另见 L154428/L154995）。
WHY_IT_MAY_BE_FALSE = 意图陈述。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 23:17:01
RECORD_ID = group_901064769.csv:L157056
RAW_EXCERPT = 刚把case7的给探测出来了
TOPIC = 用探针探出 case7 的形状
CASE = case7
MECHANISM = 探针/probe 推断 shape
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = YES（声称已探出，未公开）  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 全时段 case7 的唯一机制绑定：通过探测获知形状，用于针对性优化。
WHY_IT_MAY_BE_FALSE = 未公布形状/数值；无法核实“探出来”是否可靠（且探针本身被指违规，见 L149274）。
```

case7 结论段落：
case7 在全群有稳定的“标杆值 14us + 属于奶蛙队 + 极难”三重共识（L148524/L148619/L150826/L149849），并与 case6 常被连写为“case67”（L154526/L154995/L155100），与 case4 连写为“case4case7”（L153288）。case7 的机制绑定同样薄弱，唯一直接绑定是 10-02 “刚把 case7 的给探测出来了”（L157056），即用探针推断形状再针对性优化；未见任何源码、shape 具体值、dtype、core/row/tile/pipeline 绑定。关于“偷奶娃 case7”的说法（L154419/L154428/L155100）多为口号，非实证。14us 与项目实测 bestTimeUs 13.99 高度一致，可作为 case7 的可靠目标线。

---

## 四、case14 证据

```
CHAT_EVIDENCE
TIME = 2026-09-12 22:57:25
RECORD_ID = group_901064769.csv:L147327
RAW_EXCERPT = 哪来的挂B，把case14刷到157微妙了，死活找不到
TOPIC = case14 被“刷到 157us”
CASE = case14
MECHANISM = 挂/漏洞刷单点（疑似）
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = HIGH（“无效”判定的置信度高）
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 157us 的原始出处，且同一句即称其为“挂B”。
WHY_IT_MAY_BE_FALSE = 157us 不是干净 benchmark：当日该账号被指作弊、提交将被清除（L147374）；且与平台最优 3750.12 口径完全不同（L155449）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:00:13
RECORD_ID = group_901064769.csv:L147341
RAW_EXCERPT = 测试点8也是他刷的 / 还有10 / 12 / 刷了5个点 / 的ttest
TOPIC = 同一账号刷多个点（含 case14）
CASE = case14（并 case8/10/12）
MECHANISM = 批量刷 tbest/ttest
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 证明 157us 属于一次跨多点作弊事件，而非 case14 的真实能力。
WHY_IT_MAY_BE_FALSE = 目击式陈述，但多人在场互证。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 23:13:47
RECORD_ID = group_901064769.csv:L147378
RAW_EXCERPT = 谁把case14刷了一个数量级
TOPIC = case14 被拉低“一个数量级”
CASE = case14
MECHANISM = 异常跳变
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = “一个数量级”的异常幅度，进一步坐实 157us 是污染值而非正常成绩。
WHY_IT_MAY_BE_FALSE = 非精确倍数，口语夸张。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:15:06
RECORD_ID = group_901064769.csv:L147386
RAW_EXCERPT = 其他用例不通过的情况下某些用例确实有概率时间很快
TOPIC = 漏洞机制：部分用例不通过时，通过的点可能异常快
CASE = case14（一般机制）
MECHANISM = 部分通过 + 计时漏洞抬升单点速度
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 全群对“为什么会出现 157us 这类怪值”的最清楚机制描述，是解释 case14/case1 异常值的核心线索。
WHY_IT_MAY_BE_FALSE = 说话者自身参与讨论，属推断但随后被多人认同（L150025）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-13 00:56:13
RECORD_ID = group_901064769.csv:L147397
RAW_EXCERPT = 感觉要设一个全都pass才算tbest的规则
TOPIC = 呼吁“全部通过才算 tbest”的规则
CASE = case14（一般机制）
MECHANISM = 计分规则漏洞（未全通过也记 tbest）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 规则层面对“异常低值”的应对提议；与 L147428“pass 的会被记入 tbest”互证。
WHY_IT_MAY_BE_FALSE = 建议未落地。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 01:13:38
RECORD_ID = group_901064769.csv:L148527
RAW_EXCERPT = 被测点14卡的死死的 / 决定穷举
TOPIC = 测点14=case14 卡住，改用穷举
CASE = case14
MECHANISM = 穷举（brute force）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 自证 `测点14 == case14`，并说明 case14 是难点、走穷举路线。
WHY_IT_MAY_BE_FALSE = 穷举未必有效（同群多次称穷举需理论支撑）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-29 22:35:50
RECORD_ID = group_901064769.csv:L155449
RAW_EXCERPT = P14 平台最优 3750.12 → 3665.94（-2.2%），是队伍 cd（ID 685）今晚 21:39 刷出来的。注意他们那一发总分只有 68.22——典型的专刷单点纪录打法
TOPIC = P14(=case14) 平台最优被刷新
CASE = case14
MECHANISM = 专刷单点 time-best（只优化大点、反复磨）
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 提供 case14 的平台口径数值 3750.12→3665.94（与 157us 完全不同量纲），并暴露“专刷单点纪录”打法。
WHY_IT_MAY_BE_FALSE = 说话者当场承认这是“AI 的总结”（L155450-155451），且自称“感觉有问题”（L155453）；单位口径不明。
```

```
CHAT_EVIDENCE
TIME = 2026-09-29 19:50:53
RECORD_ID = group_901064769.csv:L155379
RAW_EXCERPT = 另外注意到平台最优 T 被别的队刷新了（P9 67.98→67.81、P15 8272.9→8249.6）
TOPIC = “平台最优 T”被刷新（含 P9/P15 口径参考）
CASE = case14（上下文，P14 同批）
MECHANISM = 平台最优 T 指标
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 说明“平台最优 T”是跨点指标且量纲混（67 到 8272），提示 P14 的 3750 也非纯 us。
WHY_IT_MAY_BE_FALSE = AI 汇总，未核对。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 20:42:15
RECORD_ID = group_901064769.csv:L156925
RAW_EXCERPT = case 14 大概是个什么形状 / 我这儿出奇的高
TOPIC = case14 形状未知，个人成绩偏高
CASE = case14
MECHANISM = shape（未知）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = UNKNOWN（本人也问）  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 到 10-02 case14 的 shape 仍无人掌握，且他人成绩“出奇的高”（差），说明 case14 是公认高提升空间点。
WHY_IT_MAY_BE_FALSE = 提问，无数据。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 20:50:46
RECORD_ID = group_901064769.csv:L156935
RAW_EXCERPT = 探针探出来形状了，做针对性优化
TOPIC = 用探针探出形状做针对性优化（上下文含 case14 形状讨论）
CASE = case14（上下文）
MECHANISM = 探针 + 针对性优化
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = YES（声称探出，未公开）  HAS_DTYPE = NO  HAS_CASE_BINDING = YES  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case14 与“探针探形状”机制绑定的唯一弱信号。
WHY_IT_MAY_BE_FALSE = 未明确点名 case14，且“我也没完全搞下来”（L156927）。
```

case14 结论段落：
case14 的数字证据非常“脏”。唯一广传的数值是 157us，但它出自 09-12 的一次作弊事件（L147327 称“挂B”、L147341 称同账号刷了 5 个点、L147374 官方称提交记录将被清除、L147378 称“刷了一个数量级”），因此 157us 必须判为 INVALID_BENCHMARK。与之量纲完全不同的是 09-29 的平台最优 T：3750.12 → 3665.94（L155449），但那条是说话者自认的“AI 总结”（L155450-451）且被本人怀疑（L155453），单位口径不明，只能算 AI_SUMMARY/低可信。case14 的“为什么会出现怪值”有明确机制解释：部分用例不通过时，通过的点有概率异常快（L147386），配合“空 kernel 绕过占位 / 文件末尾再启动一次 kernel”（L150018/L150034），构成 157us 一类异常值的成因。case14 本身仍是公认高提升空间难点（L148527“卡的死死的”、L156925“出奇的高”），形状未知，机制绑定仅有一条弱信号“探针探形状”（L156935）。

⚠️ 二级材料《CANN西南赛区_群聊情报交接文档.md》§5.1 把“13 受限于显存带宽”归到 case14，属错误：raw 原文 L156636 是“13受限于显存带宽”，13≠14。

---

## 五、MECHANISM_CLUE（机制线索）

```
MECHANISM_CLUE
CLUE_ID = MC-01
MECHANISM = 部分通过 + 计时/空kernel 漏洞刷单点 tbest（未全通过也能让通过点异常快）
AFFECTED_CASES = case14（157us 直接触发）；case1（1.47us 被怀疑）；case8/10/12（同一作弊事件）
SUPPORTING_RECORDS = L147386, L147389, L147390, L147428, L150018, L150025, L150034, L147397
CONTRADICTING_RECORDS = L149987（挂都没打破 case1 的 tbest）
NEGATIVE_EVIDENCE = 官方 L147374“他的提交记录会清除掉”
CONFIDENCE = MEDIUM
KNOWN_FACTS = 存在“不全 pass 也能记 tbest / 让通过点很快”的机制（多人认同）；当年该账号被封/清除
UNKNOWN_FACTS = 该漏洞的具体实现（末尾再启动 kernel vs 空 kernel 绕过），是否已修复
MINIMUM_FACT_NEEDED_NEXT = 一条可复现的、带 before/after 数值的漏洞提交记录
```

```
MECHANISM_CLUE
CLUE_ID = MC-02
MECHANISM = 探针/probe 探测测试 shape（据 profile 反推形状/参数）
AFFECTED_CASES = case7（L157056 直接绑定）；case1（L155785 怀疑直接输出）；case14（L156935 弱绑定）
SUPPORTING_RECORDS = L148356, L148633, L150346, L150347, L151713, L155785, L156935, L157033, L157056, L157057
CONTRADICTING_RECORDS = L149274（探针要么无效要么违规）, L140889（禁止违规探针提交）
NEGATIVE_EVIDENCE = 探针被视为违规/无效；平台可能检测探针
CONFIDENCE = MEDIUM
KNOWN_FACTS = 多人用探针/GLM 探测；case7 声称已探出；指令耗时“大多数可探测”（L148633）
UNKNOWN_FACTS = 探出的具体 shape；探测是否可靠；平台是否已封堵
MINIMUM_FACT_NEEDED_NEXT = case7/case14 探出的具体 shape 与对应优化前后数值
```

```
MECHANISM_CLUE
CLUE_ID = MC-03
MECHANISM = case4 与 case7 同簇（相似 shape 特征 / 相同“爆破手段”），收益可迁移
AFFECTED_CASES = case4, case7
SUPPORTING_RECORDS = L153288, L153695, L156566, L156567, L156569, L156583, L156585, L156610, L156612, L156614, L156632
CONTRADICTING_RECORDS = L153694（一天 4/6/7 改动全中性/反向）, L156618-156620（爆料者自称不清楚，是队友所为）
NEGATIVE_EVIDENCE = 长期“打一天没下 7”“中性改动/反向”说明该簇对普通改动不敏感
CONFIDENCE = LOW
KNOWN_FACTS = 语言层反复共现（case4case7、4567、45679）；有一队同时推进 4/7
UNKNOWN_FACTS = 是否共享同一 kernel 机制；shape 是否相似；“破一助二”是否真实
MINIMUM_FACT_NEEDED_NEXT = case4 与 case7 的 shape/dtype 对比 + 一份可迁移的改动前后数值
```

```
MECHANISM_CLUE
CLUE_ID = MC-04
MECHANISM = cube + vector 混合处理大 D（负实现）
AFFECTED_CASES = UNKNOWN（说话者只说“大D运算”，未点名 case）
SUPPORTING_RECORDS = L151429, L151430（把大D运算变成cube+vector混合，但是效果很不好）, L152673（D/B 形状通过 metadata 换正方形）, L152665（把 D 合并再裁切）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = “效果很不好”（一次负实现）
CONFIDENCE = LOW（case 绑定 UNKNOWN）
KNOWN_FACTS = 至少一次 cube+vector 混合尝试失败
UNKNOWN_FACTS = 该尝试对应哪个 case；失败原因
MINIMUM_FACT_NEEDED_NEXT = 该尝试的 case 归属与 profiling
```

```
MECHANISM_CLUE
CLUE_ID = MC-05
MECHANISM = 计分公式：越接近 tbest，同源码波动（彩票）影响越大；tbest 被刷高会压低他人分
AFFECTED_CASES = case1（tbest 过高）、case7（贴近 tbest 高噪声）
SUPPORTING_RECORDS = L147621（同源码提交20次差3~4分）, L148654, L152662, L154238, L155798, L153346-348（前几个 case 波动 0.5~数 us）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 同源码分数波动大（3~4 分）；case1 tbest 高会压制普通分
UNKNOWN_FACTS = 官方公式细节与 noise floor
MINIMUM_FACT_NEEDED_NEXT = 同源码多次提交的分数分布
```

---

## 六、汇总标签

### HIGH_CONFIDENCE_SIGNALS
- case1 是“唯一 sub-2us 点”，且 1.47/1.7us 长期作为标杆被引用（L149849, L154982, L155798）。
- case1 tbest 数值链：1.5→1.24(grok)→1.7(ggbond 稳定)→1.6(彩票)（L155798）。
- case7 标杆 14us，属“奶蛙”队，极难（L148524, L148619, L150826）；（与项目实测 13.99 吻合）。
- case14 的 157us 出自作弊事件，非有效 benchmark（L147327, L147341, L147374, L147378）。
- 计分机制：同源码波动可达 3~4 分（L147621）。
- 部分通过 + 计时漏洞可让单点异常快（L147386, L150025）。

### MEDIUM_CONFIDENCE_SIGNALS
- 经探针探测 shape 再针对性优化，被用于 case7（L157056）及疑似 case1（L155785）。
- case4 是难点，长期无 sub-7us 方案（L153333），早期下界 8~9us（L147358）。
- case4/6/7 属“难打簇”，普通改动无效（L153694, L153695, L153429）。
- case14 是高提升空间难点，形状未知（L148527, L156925）。
- case1 成绩被怀疑“探针直接输出”（L155785）与“纯 NPU 自查”（L155802）并存。

### LOW_CONFIDENCE_SIGNALS
- “case4 与 case7 同簇、爆破手段一样、破一助二”（L156612, L156585）——二手+自认不清。
- P14 平台最优 3750.12→3665.94（L155449）——AI 总结、口径不明。
- cube+vector 混合“效果很不好”（L151430）——未绑定 case。
- case1 是否“舍弃局部换整体”（L152652）。
- “奶娃 case7 偷了”（L154419）——口号式。

### NEGATIVE_EVIDENCE
- 一天 case4/6/7 中性改动或反向（L153694）；case4 打一天无 sub-7（L153333）。
- case1 1.47 被要求清除并被疑为漏洞产物（L147375, L147398）。
- case14 157us 被官方清除、作弊账号（L147374）。
- 探针“要么无效要么违规”（L149274）。
- case14“被测点14卡的死死的”（L148527）。
- case7 个人稳定 85 分抽不上去（L154526）。

### CONTRADICTIONS
- case1 1.47us：既被归为漏洞产物（L147398），又被称“挂都没打破 case1 的 tbest”（L149987）、且是唯一 sub-2（L149849）。
- case1 是否 sub-2：L149849“2 以下唯一” vs L156628“case1 从来没进2”（后者或指说话者自己队）。
- case1 有效性：L155785 疑“探针直接输出” vs L155802 AI 自查称“纯 NPU、host 无数值计算”。
- case14 数值口径：157us（L147327）vs 平台最优 3750.12（L155449），量纲不一致，二者不可混用。
- case4/7 同簇：L156612“同簇、手段一样” vs L156618-620“我自己都不知道 / 是队友”。

### UNKNOWN_BUT_IMPORTANT
- case1 的真实干净最好值（1.24/1.47/1.6/1.7 哪些有效）。
- case4 与 case7 的 shape/dtype，以及是否真共享机制。
- case14 的 shape 与真实平台口径最好值。
- “部分通过漏洞”是否已被修复（决定历史异常值能否排除）。
- 探针探测能否稳定复现，是否违规。
- case1“唯一 sub-2”的具体做法。

### RAW_FILES_READ
- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv`（全文件文本模式检索；对 case 相关窗口逐行精读）
- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769_text.txt`（用作文本模式交叉计数校验）
- `/Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/提分技术讨论_原文.csv`（全读，作 case 标签交叉校验）
- 二级材料（第二遍，只读）：`/Users/sunyiyang/Desktop/Project/cann/提分技术讨论_清单.md`、`/Users/sunyiyang/Desktop/Project/cann/CANN西南赛区_群聊情报交接文档.md`

### UNREAD_RANGES_IF_ANY
- CSV 采用目标窗口精读 + 全文件正则检索策略：未逐行通读全部 157067 行，仅在检出的 case 相关窗口（约 147320–157070 内若干段）逐行阅读；其余区段仅经正则匹配与计数检索，未逐行人工核验。
- `group_901064769_text.txt` 未逐行通读，仅用于计数交叉校验。
- CSV 约 5.31MB 偏移处存在一个 \0 字节，默认二进制判定会截断普通 grep；本次已用文本模式（treat-as-text）检索规避，判定覆盖完整。

### RESEARCH_LIMITATIONS
- 群聊为二手/口语语料，绝大多数数值无原始榜单截图、无源码、无 shape/dtype，无法直接复现。
- 昵称混写（奶蛙/奶娃、wil/wilf、sxbf/南山必胜客 等）可能造成同一主体/事件的对齐误差。
- 大量撤回消息（“撤回了一条消息(没收到)”）导致上下文缺失。
- 157us、3750.12、14us 等数字来自不同口径（作弊值 / 平台最优 T / 标杆），本报告已分别标注有效性，但无法在只读约束下重测验证。
- 二级材料存在可核实的错误（如 §5.1 把 case13 的“显存带宽”误归 case14），提醒二级结论需回原始聊天复核。
- 本报告仅做证据整理，未做路线选择、不输出任何决策字段。
