# RESEARCH-B (MID PERIOD) — 社区语料只读取证报告

- 角色：RESEARCH-B（MID PERIOD）独立研究 Child
- 时间范围：2026-09-19 00:00 ~ 2026-09-25 23:59
- 语料范围：`group_901064769.csv` 行 149209–154013（区间内匹配约 4697 条）；`提分技术讨论_原文.csv`
- 隔离目录：`/tmp/cann-research/B/`（项目外临时目录，未触碰任何项目/git 文件）
- 性质：READ-ONLY 取证，未选择路线，未输出任何决策字段
- 说明：所有 RECORD_ID 为 `group_901064769.csv:L<1-based行号>`；时间取自该行首列

---

## 第一部分：CHAT_EVIDENCE

### E01
```
CHAT_EVIDENCE
TIME = 2026-09-19 13:52:27
RECORD_ID = group_901064769.csv:L149270
RAW_EXCERPT = 试了一堆方法全因为全局效应被ai回退了，所有新尝试全被否了
TOPIC = AI 优化被全局效应回退
CASE = 未知（疑跨 case 全局）
MECHANISM = 局部改动触发全局副作用 → AI 将改动整体回退
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = UNKNOWN
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明"局部优化→全局回退"是该赛题社区公认的负反馈机制，可作为负结果先验
WHY_IT_MAY_BE_FALSE = 说话人未给出具体 case/代码，可能是 AI agent 自身工作流问题而非算子机制
```

### E02
```
CHAT_EVIDENCE
TIME = 2026-09-19 14:00:07
RECORD_ID = group_901064769.csv:L149281
RAW_EXCERPT = 你的文件里不能出现probe这个词
TOPIC = 探针/probe 命名触发代码违规
CASE = 未知
MECHANISM = 提交代码中含 "probe" 等字样被平台判违规
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示命名/字面量可能触发合规判定，属流程坑而非性能机制
WHY_IT_MAY_BE_FALSE = 无官方规则原文佐证，可能是群内以讹传讹
```

### E03
```
CHAT_EVIDENCE
TIME = 2026-09-19 14:03:24
RECORD_ID = group_901064769.csv:L149290
RAW_EXCERPT = 如果你不知道他这台机器内部的微结构。那怎么设计一个好的程序呢
TOPIC = 按机器微结构反向设计
CASE = 未知
MECHANISM = 需探测机器内部微结构参数才能定向优化（供方法论）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 中期"参数反推"方法论的原始表述，指向 probing→定向优化路径
WHY_IT_MAY_BE_FALSE = 纯方法论主张，无实测数据支撑
```

### E04
```
CHAT_EVIDENCE
TIME = 2026-09-21 16:17:45
RECORD_ID = group_901064769.csv:L149849
RAW_EXCERPT = case1是真的那个唯一一个2以下的达到的，奶蛙的case7，case9我不知道，真的摸不到门道啊
TOPIC = case1 / case7 / case9 标杆
CASE = case1,case7,case9
MECHANISM = 社区标杆：case1 唯一 <2；case7 由"奶蛙"取得低值
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 中期对 case7 难点与 case1 标杆的表述，可与后期 case4/7 同簇情报对照
WHY_IT_MAY_BE_FALSE = 转述他人成绩，无数值来源；case1 "2以下"未指明单位
```

### E05
```
CHAT_EVIDENCE
TIME = 2026-09-21 16:19:11
RECORD_ID = group_901064769.csv:L149867
RAW_EXCERPT = 11我之前吃彩票达到的，波动很大
TOPIC = 分数波动（彩票/噪声）
CASE = case11
MECHANISM = 同一方案在不同提交时机分数波动大
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case11 早期就存在大幅波动的自述，支持"噪声地板"判断
WHY_IT_MAY_BE_FALSE = "吃彩票"为口语比喻，未给波动区间与重复次数
```

### E06
```
CHAT_EVIDENCE
TIME = 2026-09-21 16:25:12
RECORD_ID = group_901064769.csv:L149934
RAW_EXCERPT = 其实形状是可以知道的，我之前理解错了
TOPIC = 测试形状可探测
CASE = 全局
MECHANISM = 通过探测可获知测试点张量形状（形状非保密）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 149946"有很多不整齐的形状"共同指向形状探测可行
WHY_IT_MAY_BE_FALSE = 未给出任何具体形状/探测方法
```

### E07
```
CHAT_EVIDENCE
TIME = 2026-09-21 16:30:25
RECORD_ID = group_901064769.csv:L149987
RAW_EXCERPT = 笑点解析：挂都没打破case1的tbest
TOPIC = case1 tbest 极高
CASE = case1
MECHANISM = case1 的 tbest 高到外挂也打不破
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 侧面说明 case1 tbest 标杆极低（性能极高），对应 Tiny 类 launch 开销主导
WHY_IT_MAY_BE_FALSE = 调侃语气，且当时 tbest 曾处于"待修"争议期
```

### E08
```
CHAT_EVIDENCE
TIME = 2026-09-21 22:58:09
RECORD_ID = group_901064769.csv:L150509
RAW_EXCERPT = 67.50分是违规所得。TP1-7和TP8的"NPU时间"只测量了空marker kernel，实际计算在Host CPU上完成。刷新的4个best是TP1/TP2/TP3/TP8这些小规模TP——它们N*D≤64M元素，被路由到Host CPU计算。
TOPIC = 空 kernel + Host CPU 自动路由（违规）
CASE = TP1,TP2,TP3,TP7,TP8（小形状）
MECHANISM = 小规模 N*D≤64M 元素被框架自动路由 Host CPU；NPU 侧只剩空 marker kernel，计时极短→虚高 67.5 分
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = UNKNOWN
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 本窗口最高价值证据：给出小形状自动回退 Host CPU 的具体触发阈值(N*D≤64M)与判定机理，直接对应合规自查与小形状 case1/2/3 优化风险
WHY_IT_MAY_BE_FALSE = 为当事人自述/AI 分析，未附官方公告；"64M元素"阈值未经官方确认
```

### E09
```
CHAT_EVIDENCE
TIME = 2026-09-21 22:45:27
RECORD_ID = group_901064769.csv:L150397
RAW_EXCERPT = 本赛事要求核心计算在昇腾 NPU 上通过 AscendC 算子实现完成，任何将计算转移至 Host CPU、通过空 kernel 占位绕过 NPU 计算要求的行为，均构成违规，将取消当前提交成绩。
TOPIC = 官方违规规则原文转贴
CASE = 全局
MECHANISM = 核心计算必须 NPU+AscendC，禁止 Host CPU / 空 kernel 绕过
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 权威规则口径（群友转贴官方），是 E08 判定的合规依据
WHY_IT_MAY_BE_FALSE = 转贴，非官方账号直发；措辞可能非逐字
```

### E10
```
CHAT_EVIDENCE
TIME = 2026-09-21 22:57:02
RECORD_ID = group_901064769.csv:L150497
RAW_EXCERPT = AI偷偷写了个AICAIV双核绕过
TOPIC = AIC/AIV 双核绕过（违规手法）
CASE = 未知
MECHANISM = 疑用 AIC/AIV 双核分工掩盖真实计算位置/时间
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提供"绕过"手法的旁证；同段 L150492-150494 自述"整了个空核，没破纪录都被封了"
WHY_IT_MAY_BE_FALSE = "好像是"、猜测口吻；"AICAIV双核绕过"含义模糊，可能是误解
```

### E11
```
CHAT_EVIDENCE
TIME = 2026-09-21 23:03:16
RECORD_ID = group_901064769.csv:L150555
RAW_EXCERPT = 同一套代码不同时间交 / 是不是结果也会不一样 → 是 / 机器状态不同 / 精度舍入啥的有波动都很正常
TOPIC = 同源码跨时刻分数波动
CASE = 全局
MECHANISM = 机器负载/状态差异 + 精度舍入 → 同源码结果不同
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = UNKNOWN
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 明确"同源码≠同结果"，是噪声地板的核心判据
WHY_IT_MAY_BE_FALSE = 未量化波动大小，未区分 tbest 变动与运行时噪声
```

### E12
```
CHAT_EVIDENCE
TIME = 2026-09-21 23:05:33
RECORD_ID = group_901064769.csv:L150587
RAW_EXCERPT = 我感觉是因为每次跑的时候机器负载不同，所以每小时用自己最佳跑一次作为baseline，应该就能减少波动的影响
TOPIC = 以最佳跑作 baseline 降噪
CASE = 全局
MECHANISM = 周期性取自身最佳成绩作基线以抵消负载波动
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 社区提出的测量方法论，与本机"配对改善需超 noise floor"要求一致
WHY_IT_MAY_BE_FALSE = 个人经验，无实测对比
```

### E13
```
CHAT_EVIDENCE
TIME = 2026-09-21 23:08:02
RECORD_ID = group_901064769.csv:L150611
RAW_EXCERPT = 跟tbest有关系 / tbest是历史最佳 / 每次tbest改变，就改了成绩 / 把tbest拉成噪声带最高值就直接拉低后面高分概率
TOPIC = tbest = 历史最佳，刷高会压低后来者
CASE = 全局
MECHANISM = tbest 取历史最优；若被噪声刷到高位，其他提交的高分概率被拉低
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 解释"同一源码分数变化"的机制之一（计分侧而非 kernel 侧）
WHY_IT_MAY_BE_FALSE = 说话人推断，未获官方计分公式确认
```

### E14
```
CHAT_EVIDENCE
TIME = 2026-09-21 23:17:54
RECORD_ID = group_901064769.csv:L150675
RAW_EXCERPT = 哈吉米告诉我这个case8的tbest要达到的话带宽得要有16.7TB/s
TOPIC = case8 tbest 需求带宽 16.7TB/s
CASE = case8
MECHANISM = 达到 case8 tbest 需要 16.7TB/s 带宽，疑超物理极限
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E08 的"空 kernel/CPU 绕过"互文：异常低 tbest 可能非真实 NPU 实现
WHY_IT_MAY_BE_FALSE = 来源是 AI("哈吉米")推算，非实测；16.7TB/s 数字未验证
```

### E15
```
CHAT_EVIDENCE
TIME = 2026-09-21 22:36:14
RECORD_ID = group_901064769.csv:L150332
RAW_EXCERPT = case8，1.34微妙
TOPIC = case8 标杆值 1.34us
CASE = case8
MECHANISM = 社区公开的 case8 标杆时延
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E14 的"需 16.7TB/s"冲突：1.34us 的 case8 若为真则极可能违规或特殊形状
WHY_IT_MAY_BE_FALSE = 口头数字，未说明单位（us/μs），未区分本地/线上
```

### E16
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:35:59
RECORD_ID = group_901064769.csv:L151143
RAW_EXCERPT = 分数怎么又整体下来了 / 被爆破了 / 被我们的一位兄弟爆破了 / 我看我少了十几分
TOPIC = 违规刷分被清算导致全体掉分
CASE = 全局（TP1-3/8）
MECHANISM = 群内选手"爆破"（空 kernel/CPU）抬高 tbest，修正后多人分数整体回落
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = E08 违规链的后果实证：一次违规会污染多人 tbest 与总榜
WHY_IT_MAY_BE_FALSE = 群聊归因，"被爆破"是调侃式说法，未必对应平台真实修正
```

### E17
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:40:44
RECORD_ID = group_901064769.csv:L151172
RAW_EXCERPT = 我case5能有3us的波动
TOPIC = case5 波动 3us
CASE = case5
MECHANISM = case5 单点测量存在约 3us 抖动
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出噪声地板的绝对量级参考（us 级），可用于剔除伪改善
WHY_IT_MAY_BE_FALSE = 未说明测量方式/重复次数，3us 可能与 case5 量级本身接近
```

### E18
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:43:44
RECORD_ID = group_901064769.csv:L151174
RAW_EXCERPT = 根据形状优化就像打开潘多拉的魔盒，最后所有人的方案只会变得更加过拟合，更加极端，最后将迭代引向一些违背初衷的方向
TOPIC = 形状过拟合风险警告
CASE = 全局
MECHANISM = 针对形状特判的优化会走向过拟合与极端化
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 社区对 shape-specialization 路线的主流反对意见，影响技术取舍判断
WHY_IT_MAY_BE_FALSE = 价值判断而非实测，且与"形状可探测"派共存
```

### E19
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:49:21
RECORD_ID = group_901064769.csv:L151224
RAW_EXCERPT = 我的GLM就在不断地更改，回退。虽然有时候我觉得他优化的挺好，但使测试点退化了就还是要回退
TOPIC = AI 迭代中的回退
CASE = 全局
MECHANISM = 一旦导致测试点退化，AI 工作流即回退改动
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E01 呼应，构成"局部回退"重复信号
WHY_IT_MAY_BE_FALSE = 归因于 AI 工具行为，非算子机制
```

### E20
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:53:04
RECORD_ID = group_901064769.csv:L151241
RAW_EXCERPT = 因为case5老是有些时候触发我的降级策略
TOPIC = case5 触发降级策略（fallback）
CASE = case5
MECHANISM = case5 的某种输入/条件反复命中代码中的降级/兜底分支
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E29(福尔摩斯"静默降级/分支未走预期")构成同一机制的参赛者侧证据
WHY_IT_MAY_BE_FALSE = 未说明"降级策略"是自身代码还是框架行为
```

### E21
```
CHAT_EVIDENCE
TIME = 2026-09-22 00:58:16
RECORD_ID = group_901064769.csv:L151272
RAW_EXCERPT = 哈哈其实我之前前对着case5优化了三五天的空气
TOPIC = case5 长期优化无收益
CASE = case5
MECHANISM = 对 case5 优化数天无实际得分
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case5 负结果样本，佐证 Tiny 类单点硬啃 ROI 低
WHY_IT_MAY_BE_FALSE = "优化空气"为自嘲，可能因测量噪声掩盖了真实改善
```

### E22
```
CHAT_EVIDENCE
TIME = 2026-09-22 09:25:29
RECORD_ID = group_901064769.csv:L151429
RAW_EXCERPT = 目前在尝试让5.6sol给我改进成把大D运算变成cube+vector 混合 / 但是效果很不好
TOPIC = 大 D 运算改编为 cube+vector 混合（负结果）
CASE = 大 D 相关（未绑定具体 case）
MECHANISM = 将大 D 的 vector 计算改为 cube(matmul)+vector 混合，以期提升
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = UNKNOWN
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 本窗口唯一明确的 cube/vector 机制尝试，且为负结果——是"有人提出/实现"但"效果不好"的典型
WHY_IT_MAY_BE_FALSE = "效果很不好"未给数值；仅为一次实现，不能据此外推 cube+vector 机制整体无效
```

### E23
```
CHAT_EVIDENCE
TIME = 2026-09-22 09:43:35
RECORD_ID = group_901064769.csv:L151512
RAW_EXCERPT = 我发现我现在最烂的是case6和7 / 4.5.6.7都不太行 / 但是4.6.7特别不行
TOPIC = case4/5/6/7 集体短板
CASE = case4,case5,case6,case7
MECHANISM = 4/6/7 属同一难打群体（小形状/短行特征）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 中期即出现 4/6/7 共簇信号，与后期"case4/7 同簇可迁移"情报的早期版本
WHY_IT_MAY_BE_FALSE = 属个人主观"最难打"排序，非机制证明
```

### E24
```
CHAT_EVIDENCE
TIME = 2026-09-22 09:44:51
RECORD_ID = group_901064769.csv:L151524
RAW_EXCERPT = 我 case3 45 打不上去
TOPIC = case3 与 case4/5 打不上去
CASE = case3,case4,case5
MECHANISM = Tiny/Small 混合群体难提升
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 补充 case 难度分布的中期样本
WHY_IT_MAY_BE_FALSE = 简写"45"含义需确认（case4/5 连读），无数据
```

### E25
```
CHAT_EVIDENCE
TIME = 2026-09-22 11:24:16
RECORD_ID = group_901064769.csv:L151757
RAW_EXCERPT = 物理极限就是我们的理论上限了 何况还只存在于理论🌚
TOPIC = 物理极限作为理论天花板
CASE = 后段若干 case
MECHANISM = band-width/L2 类物理极限决定理论上限，且实际难以触及
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = UNKNOWN
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E06/E14 共同构成"带宽墙"重复信号，影响后段 case 收益预期
WHY_IT_MAY_BE_FALSE = 泛指，无 case 与数值
```

### E26
```
CHAT_EVIDENCE
TIME = 2026-09-22 18:28:08
RECORD_ID = group_901064769.csv:L151972
RAW_EXCERPT = 重新提交一波，上升四个排名，波动那么大
TOPIC = 重复提交引致排名大幅波动
CASE = 全局
MECHANISM = 同方案重复提交排名跃升 4 位，揭示榜单噪声量级
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 排名层面的噪声实证；运营随后索要提交 ID 核查
WHY_IT_MAY_BE_FALSE = 也可能确有改动，未言明是否同源码
```

### E27
```
CHAT_EVIDENCE
TIME = 2026-09-23 00:51:05
RECORD_ID = group_901064769.csv:L152236
RAW_EXCERPT = 据说这个会回退。以前只有fable会回退。现在opus5.5也会回退。我的GLM一直在左右脑互搏然后回退
TOPIC = 不同 LLM 的优化回退行为
CASE = 全局
MECHANISM = 模型在优化中自发回退改动
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 佐证"AI 回退"是跨模型现象，属工作流而非算子机制
WHY_IT_MAY_BE_FALSE = "据说"、模型版本口径模糊
```

### E28
```
CHAT_EVIDENCE
TIME = 2026-09-23 15:06:44
RECORD_ID = group_901064769.csv:L152479
RAW_EXCERPT = 就是比方说设计好了一个分支，希望他走这个分支，但是其实他实际走了另一个分支。
TOPIC = 分支未按预期执行（规格/形状判定错）
CASE = 全局
MECHANISM = 期望分支与实际执行分支不一致，可能因真实形状与假设不符
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出"宿主形状≠真实形状"疑点，是 case 判定类机制线索的上游
WHY_IT_MAY_BE_FALSE = 纯假设场景，无代码/日志
```

### E29
```
CHAT_EVIDENCE
TIME = 2026-09-23 15:09:03
RECORD_ID = group_901064769.csv:L152487
RAW_EXCERPT = 还有一种就是即便是真正的形状，但是在某些代码的里面就是我们可能写了一些降级方案然后就导致了静默报错。
TOPIC = 降级方案导致静默报错
CASE = 全局（疑 case5 等）
MECHANISM = 兜底/降级分支静默接管，导致"未按预期路径计算"而无显式报错
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = NO
HAS_CASE_BINDING = UNKNOWN
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E20 互证，指向"静默回退"这一隐蔽机制；对正确性与性能都可能影响
WHY_IT_MAY_BE_FALSE = 假设语气；cqu ai 同段反驳"目前我没有观察到形状有变化"(L152492)
```

### E30
```
CHAT_EVIDENCE
TIME = 2026-09-23 19:03:22
RECORD_ID = group_901064769.csv:L152522
RAW_EXCERPT = GLM穷举了三天，终于把测试点3进步了2微秒
TOPIC = 穷举三天使 case3 进步 2us
CASE = case3
MECHANISM = 通过 AI 穷举得到 case3 上 2us 的改善
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 唯一给出"投入-产出"的 case 级正结果；但 2us 是否超噪声需警惕（对比 E17 3us 抖动）
WHY_IT_MAY_BE_FALSE = 2us 可能小于/接近噪声地板，且"测试点3"与平台 case3 是否同名待确认
```

### E31
```
CHAT_EVIDENCE
TIME = 2026-09-23 19:38:54
RECORD_ID = group_901064769.csv:L152566
RAW_EXCERPT = 看来已经是趋同进化了 → 可能已经到了硬件的上限了 → case11和13应该可以一起来看 / 160/560 和 130/420 两种耗时 / 11耗时快的13也大概率快
TOPIC = case11/case13 耗时耦合（双台阶）
CASE = case11,case13
MECHANISM = case11/13 耗时集中在两组台阶（~160/560 与 ~130/420），11 快则 13 大概率快
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出 case11/13 可能同族、耗时呈台阶的观察，是"case 聚类"线索
WHY_IT_MAY_BE_FALSE = 说话人自称"疑惑""个例咋解释"，未确认机制；数字为口头近似
```

### E32
```
CHAT_EVIDENCE
TIME = 2026-09-23 21:50:46
RECORD_ID = group_901064769.csv:L152663
RAW_EXCERPT = 分析prof后，我发现随着D的增大，UB吞吐也就越高 / 我就想 / 能不能把D给合并 / 然后被裁切 / 然后计算
TOPIC = UB 吞吐随 D 增大而升高 → D 合并再裁切
CASE = 大 D 相关（未绑定具体 case）
MECHANISM = profiling 显示 UB 吞吐随 D 增大提高；猜测将 D 合并后裁切可提高 UB 利用率
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = UNKNOWN
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 本窗口最具可执行性的机制线索：以真实 profiling 为据，指向 D 维合并/裁切的 tile 策略
WHY_IT_MAY_BE_FALSE = 只有定性趋势，无数据；"合并-裁切"仅为设想，未证涨分；同段随后改口提"D和B换正方形"属猜测
```

### E33
```
CHAT_EVIDENCE
TIME = 2026-09-23 21:53:08
RECORD_ID = group_901064769.csv:L152673
RAW_EXCERPT = 从我的几何直觉来看，或许D和B的形状应该通过metadata给换为正方形？
TOPIC = 通过 metadata 将 D×B 视为正方形
CASE = 大 D 相关
MECHANISM = 借 metadata 重塑 D/B 为方阵以利于 cube/vector 划分
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = E22/E32 的延伸假说，说明社区在探索形态重塑
WHY_IT_MAY_BE_FALSE = "几何直觉"、问句形式，无任何验证
```

### E34
```
CHAT_EVIDENCE
TIME = 2026-09-24 13:02:35
RECORD_ID = group_901064769.csv:L152847
RAW_EXCERPT = 这 case15 能怎么优化啊 / 感觉是进入搬运平台期了 / 8.4 的好厉害 / 其他人统一在 9 / 接近带宽极限了
TOPIC = case15 进入搬运/带宽平台期
CASE = case15
MECHANISM = case15 优化卡在搬运/带宽平台，社区分化在 8.4 与 9
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = UNKNOWN
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case15 中期状态：已达带宽墙、收益趋平，与交接文档"case15 低 ROI"一致
WHY_IT_MAY_BE_FALSE = 未说明单位与是否 total/单点；"8.4/9"含义（分数或耗时）不明确
```

### E35
```
CHAT_EVIDENCE
TIME = 2026-09-25 00:29:04
RECORD_ID = group_901064769.csv:L153288
RAW_EXCERPT = case4case7好难打啊 / 还没有抽卡收益来的快
TOPIC = case4/case7 难打
CASE = case4,case7
MECHANISM = case4 与 case7 被并提为难点，纯优化收益不如"抽卡"（噪声）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 中期"case4/7 同提"的最早明确记录之一，是后期"同簇"结论的弱前身
WHY_IT_MAY_BE_FALSE = 仅共同提及难度，不等于同簇；"抽卡收益快"暗示其观察到的是噪声而非真实优化
```

### E36
```
CHAT_EVIDENCE
TIME = 2026-09-25 00:37:48
RECORD_ID = group_901064769.csv:L153333
RAW_EXCERPT = 但是今天打一天case4没下7的方案
TOPIC = 一天优化 case4 未获得 <7 的方案
CASE = case4
MECHANISM = case4 单点优化一天无突破
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case4 明确的负结果（配 E38 的"中性/反向"）
WHY_IT_MAY_BE_FALSE = "没下7"指目标数值不明（分数还是时延），"方案"含义模糊
```

### E37
```
CHAT_EVIDENCE
TIME = 2026-09-25 00:44:51
RECORD_ID = group_901064769.csv:L153356
RAW_EXCERPT = 我感觉我的case1case2又成短板了，case4好不容易掉下来，这边又上去了
TOPIC = case1/2 与 case4 此消彼长
CASE = case1,case2,case4
MECHANISM = 改善 case4 时 case1/2 变差，存在取舍/全局耦合
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 中期对"全局耦合/不可同时优化"的直接观察，是"全局效应"的参赛者侧证据
WHY_IT_MAY_BE_FALSE = 也可能是噪声造成的错觉（与噪声地板难区分）
```

### E38
```
CHAT_EVIDENCE
TIME = 2026-09-25 13:20:45
RECORD_ID = group_901064769.csv:L153694
RAW_EXCERPT = 昨天就是打了一天的case4case6case7，基本都是中性改动或者反向 / 4567真的比其他点难打
TOPIC = case4/6/7 一天优化多为中性/反向
CASE = case4,case6,case7
MECHANISM = 该簇改动大量中性（无效果）或负向
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 E23/E35 汇成稳定的"4/6/7 难打"重复信号；直接指出负向改动
WHY_IT_MAY_BE_FALSE = "中性/反向"可能部分是噪声误判而非真实退化
```

### E39
```
CHAT_EVIDENCE
TIME = 2026-09-25 13:18:53
RECORD_ID = group_901064769.csv:L153692
RAW_EXCERPT = 不是哥，刷了tbest又回去了啊（？）
TOPIC = tbest 被刷又回退
CASE = 全局
MECHANISM = tbest 数值出现被刷高后回退的波动
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 邻近窗口末尾再次出现 tbest 不稳信号，支持"tbest 可信度有限"
WHY_IT_MAY_BE_FALSE = 疑问语气，未确认是官方修正还是选手行为
```

### E40
```
CHAT_EVIDENCE
TIME = 2026-09-25 18:47:34
RECORD_ID = group_901064769.csv:L153851
RAW_EXCERPT = 一下午没npu的情况给我提升了5分
TOPIC = 无 NPU 情况下分数提升 5 分
CASE = 全局
MECHANISM = 未在真机验证却出现 5 分提升，指向分数漂移/噪声而非真实优化
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 强烈暗示 5 分级别变动可来自噪声/榜单漂移，支撑"小改善等于噪声"
WHY_IT_MAY_BE_FALSE = 也可能提交的是之前已跑好的成果；语带玩笑
```

（注：另有大量 JOKE/灌水条目，如 L152080–L152092「我能让分数倒着流，我有这样的威能!」接龙、L153828「没有npu的情况下打过case7」等，均判为 JOKE，未单列。）

---

## 第二部分：MECHANISM_CLUE

### MC-01
```
MECHANISM_CLUE
CLUE_ID = MC-01
MECHANISM = 小形状（N*D≤64M 元素）被框架自动路由到 Host CPU，NPU 侧只剩空 marker kernel，导致计时虚高
AFFECTED_CASES = 小形状拼测点 TP1/TP2/TP3/TP7/TP8；对应平台小 case1/2/3
SUPPORTING_RECORDS = L150509, L150397, L151143
CONTRADICTING_RECORDS = 无（未见直接反驳）
NEGATIVE_EVIDENCE = L150492 "整了个空核，没破纪录都被封了"；L151143 分数被整体清算回落
CONFIDENCE = HIGH
KNOWN_FACTS = 存在明确违规判例；触发规模 N*D≤64M；用空 kernel 占位
UNKNOWN_FACTS = 该 64M 阈值是否为官方定义；小形状是否在所有平台版本都会 CPU 回退
MINIMUM_FACT_NEEDED_NEXT = 官方对 Host CPU 路由阈值的原文；本机真机小形状是否实际走 NPU
```

### MC-02
```
MECHANISM_CLUE
CLUE_ID = MC-02
MECHANISM = 大 D 运算采用 cube(matmul)+vector 混合执行
AFFECTED_CASES = 大 D 相关（case13/14/15 候选，未绑定）
SUPPORTING_RECORDS = L151429
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = L151430 "但是效果很不好"（一次负实现）
CONFIDENCE = LOW
KNOWN_FACTS = 有人尝试，且自称效果不好
UNKNOWN_FACTS = 无源码/shape/dtype/before-after；是否只是 LLM 生成未跑通
MINIMUM_FACT_NEEDED_NEXT = 该实现的 shape/D 范围、dtype、以及"效果很不好"的具体前后数值
```

### MC-03
```
MECHANISM_CLUE
CLUE_ID = MC-03
MECHANISM = UB 吞吐随 D 增大而升高 → 将 D 维合并后裁切以提升 UB 利用率
AFFECTED_CASES = 大 D 相关
SUPPORTING_RECORDS = L152663, L152667
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无（仅设想，未失败）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 有 prof 分析支撑"D↑→UB吞吐↑"趋势；作者提出 D 合并-裁切思路
UNKNOWN_FACTS = 趋势的绝对数值；合并裁切是否涨分；是否引入额外搬运开销/精度问题
MINIMUM_FACT_NEEDED_NEXT = prof 原始数据（UB 吞吐 vs D 曲线），以及合并裁切实现的 before/after
```

### MC-04
```
MECHANISM_CLUE
CLUE_ID = MC-04
MECHANISM = 代码中的兜底/降级分支被静默触发（或分支未走预期路径），导致按非预期路径计算而无显式报错
AFFECTED_CASES = case5（参赛者明确提及触发降级）；全局
SUPPORTING_RECORDS = L152479, L152487, L151241
CONTRADICTING_RECORDS = L152492（cqu ai："目前我没有观察到形状有变化"）
NEGATIVE_EVIDENCE = L152487（静默报错本身即负向）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 多名参与者独立提到"降级/分支未按预期"
UNKNOWN_FACTS = 触发条件；是自身代码还是框架；是否影响最终分数或仅影响可调试性
MINIMUM_FACT_NEEDED_NEXT = 触发降级的具体 case/输入（形状，dtype）与可复现日志
```

### MC-05
```
MECHANISM_CLUE
CLUE_ID = MC-05
MECHANISM = case4/6/7（及 5）属同一"难打簇"，改动多为中性/反向；case4 与 case7 关联尤其强
AFFECTED_CASES = case4,case5,case6,case7
SUPPORTING_RECORDS = L151512, L151514, L153288, L153356, L153694, L152836
CONTRADICTING_RECORDS = L153356（case4 改善时 case1/2 变差，提示并非单纯独立簇）
NEGATIVE_EVIDENCE = L153333（一天未下 7）、L151272（case5 优化空气）、L153694（中性/反向）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 中期反复共提；case4/7 仅在"难度"层面共现
UNKNOWN_FACTS = 是否真"同簇"（同簇主张见 10-02，超出本窗口）；共同机制是什么
MINIMUM_FACT_NEEDED_NEXT = case4 与 case7 的具体形状/耗时构成，验证是否存在共享处理路径
```

### MC-06
```
MECHANISM_CLUE
CLUE_ID = MC-06
MECHANISM = 后段 case 已接近/超越显存带宽物理极限（case8 tbest 需 ~16.7TB/s）
AFFECTED_CASES = case8；后段若干
SUPPORTING_RECORDS = L150675, L149878, L150372, L151750, L151757, L152851
CONTRADICTING_RECORDS = L150332（case8 1.34us 若真则更可能来自 MC-01 违规路径）
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 多人多次提出带宽墙；case8 异常低 tbest 伴随违规事件
UNKNOWN_FACTS = 各 case 真实带宽需求；极限判定基于何模型
MINIMUM_FACT_NEEDED_NEXT = 910B4 实测显存带宽与各 case 数据量，核算理论下限
```

### MC-07
```
MECHANISM_CLUE
CLUE_ID = MC-07
MECHANISM = 评测噪声地板：同源码重复提交分数波动约 1–3 分甚至 3–4 分；case5 单点约 3us 抖动
AFFECTED_CASES = 全局（case5/case11 有数值）
SUPPORTING_RECORDS = L150555, L150564, L150587, L151172, L151972, L151251, L153851, L153619
CONTRADICTING_RECORDS = L150575（福尔摩斯："高于噪声比例即为真正改进"，提示可用比例滤除）
NEGATIVE_EVIDENCE = 无
CONFIDENCE = HIGH
KNOWN_FACTS = 波动量级被多人独立观察到；机器负载/时段是主因
UNKNOWN_FACTS = 精确噪声分布与置信区间；各 case 噪声是否不同
MINIMUM_FACT_NEEDED_NEXT = 同源码多次重复提交的分数样本，估计 case 级噪声
```

### MC-08
```
MECHANISM_CLUE
CLUE_ID = MC-08
MECHANISM = tbest 为"历史最佳"，被刷高会拉低后续高分概率；tbest 数值曾不稳/被修
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L150611, L150613, L150631, L151317, L153692, L149987
CONTRADICTING_RECORDS = L151434（"这个tbest修了吗"显示修复状态不明）
NEGATIVE_EVIDENCE = L151317（"爆破tbest会被通缉"）
CONFIDENCE = MEDIUM
KNOWN_FACTS = tbest 语义为历史最佳；曾与违规刷分交织
UNKNOWN_FACTS = 计分公式中 tbest 的确切作用；当前 tbest 是否可信
MINIMUM_FACT_NEEDED_NEXT = 官方计分公式说明；当前 effective 榜单 tbest 快照
```

### MC-09
```
MECHANISM_CLUE
CLUE_ID = MC-09
MECHANISM = case11 与 case13 耗时呈双台阶耦合（~130/420 与 ~160/560），11 快则 13 大概率快
AFFECTED_CASES = case11,case13
SUPPORTING_RECORDS = L152571, L152577, L152579, L152583, L152585
CONTRADICTING_RECORDS = L152583（"怎么会不一起快或者一起慢"的自相矛盾，存在个例）
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 观察到两档耗时与耦合趋势
UNKNOWN_FACTS = 台阶成因；对应形状；个例如何解释
MINIMUM_FACT_NEEDED_NEXT = case11/13 的形状与各自耗时分布样本
```

### MC-10
```
MECHANISM_CLUE
CLUE_ID = MC-10
MECHANISM = AI 迭代中因"全局效应"将局部改动整体回退；测试点退化即回退
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L149270, L151224, L152236, L152243
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = L149270（所有新尝试被否）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 多个独立参与者报告回退现象
UNKNOWN_FACTS = "全局效应"的量化定义；是 AI 工具行为还是算子真实耦合
MINIMUM_FACT_NEEDED_NEXT = 一次被回退改动的 before/after 分数与对应 case
```

### MC-11
```
MECHANISM_CLUE
CLUE_ID = MC-11
MECHANISM = 优化投入与产出严重不成比例（负/中性结果密集）：一天 case4 无果、case5 数天无果、大 D cube+vector 效果差
AFFECTED_CASES = case3,case4,case5,case6,case7
SUPPORTING_RECORDS = L153333, L153694, L151272, L151429, L152522（唯一正结果 +2us）
CONTRADICTING_RECORDS = L152522（case3 穷举三天 +2us 属正结果，但幅度近噪声）
NEGATIVE_EVIDENCE = 上述多条
CONFIDENCE = MEDIUM
KNOWN_FACTS = 小/中形状单点硬啃 ROI 低
UNKNOWN_FACTS = 是否存在被噪声掩盖的真实改善
MINIMUM_FACT_NEEDED_NEXT = 各 case 的噪声地板，用于区分"真无果"与"被噪声掩盖"
```

### MC-12
```
MECHANISM_CLUE
CLUE_ID = MC-12
MECHANISM = 按机器微结构参数（绝对带宽/L2/指令耗时/向量核数）反向设计算子（参数反推方法论）
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L149290, L149934, L149946, L149867
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 社区核心方法论：探测→反推形状/参数→定向优化
UNKNOWN_FACTS = 探测手段与精度；评测机（疑 910B4）参数是否与开发机一致
MINIMUM_FACT_NEEDED_NEXT = 评测机型号与关键参数实测值
```

---

## 第三部分：汇总标签

```
HIGH_CONFIDENCE_SIGNALS
- MC-01 小形状(N*D≤64M)自动回退 Host CPU + 空 kernel 违规链（L150509/L150397/L151143）：有具体阈值、有被判违规后果
- MC-07 评测噪声地板 1–3 分（个别 3–4 分）、case5 约 3us（L150555/L151172/L151972/L153851）
- case4/6/7 在中期反复被并提为"最难打"（L151512/L153288/L153694，重复信号稳定）
```

```
MEDIUM_CONFIDENCE_SIGNALS
- MC-03 UB 吞吐随 D 增大而升高 + D 合并裁切设想（L152663）
- MC-05 case4/6/7 同簇难打、case4 与 case7 关联（中期仅难度共现，非同簇证明）
- MC-09 case11/case13 耗时双台阶耦合（L152571–L152583）
- MC-02 大 D cube+vector 混合尝试（负结果，L151429）
- MC-04 降级分支静默触发（L152479/L152487/L151241）
- MC-06 后段 case 接近显存带宽物理极限（L149878/L151750/L152851）
- MC-10 AI 全局效应回退（L149270/L151224/L152236）
- MC-11 小/中形状单点优化 ROI 低（负结果密集）
- MC-12 参数反推方法论（L149290/L149934）
```

```
LOW_CONFIDENCE_SIGNALS
- E02 "probe" 字样触发违规（无官方依据）
- E10 AIC/AIV 双核绕过（"好像是"猜测）
- E14 case8 需 16.7TB/s（AI 二手推算）
- E18 形状过拟合警告（价值判断）
- E33 D/B 换正方形（几何直觉问句）
- E27 各 LLM 回退行为（"据说"）
```

```
NEGATIVE_EVIDENCE
- L151430 大 D cube+vector 混合"效果很不好"
- L153333 一天优化 case4 未下 7
- L153694 case4/6/7 一天改动多为"中性或反向"
- L151272 对着 case5 优化"三五天的空气"
- L149270 所有新尝试因全局效应被 AI 回退
- L150492 "整了个空核，没破纪录都被封了"
- L152487 降级方案导致静默报错
```

```
CONTRADICTIONS
- case8：E14 称 tbest 需 16.7TB/s（超极限）vs E15 称 case8=1.34us（极低）——两者若同真则 1.34us 更可能来自 MC-01 违规路径而非真实 NPU 实现
- case4/7 共簇：中期只有"难点共提"（E35/E38），未见"同簇/可迁移"证据；"同簇"主张出现在 10-02（超出本窗口）——不得把中期弱信号当作同簇证明
- case1/2 与 case4 此消彼长（E37）vs "case4/7 同簇独立攻击"——前者提示全局耦合，后者提示可共享，方向相反
- 形状判定：E28/E29 假设"实际走了别的分支/静默降级" vs L152492 cqu ai "没有观察到形状有变化"
- 噪声量级：case3 穷举三天 +2us（E30）落在 case5 观测到的 3us 抖动范围内 → 该正结果可能被噪声淹没
```

```
UNKNOWN_BUT_IMPORTANT
- 64M 元素 Host CPU 路由阈值是否为官方定义，以及本机真机小形状是否真的走 NPU（合规 + 性能双重关键）
- 各 case 的真实形状与 dtype（本窗口内无人给出可靠数据）
- 噪声地板是否 case 相关、评测机型号（疑 910B4）与开发机差异
- case11/13 双台阶、UB 吞吐随 D 的成因（缺 prof 原始数据）
- case14（本窗口内几乎无讨论）——最大提升空间 case 的中期情报空白
- 早期出现的"第一名 84.7"说法（本窗口内 L151757 附近上下文），与本地 Champion 量级差距需确认
```

```
RAW_FILES_READ
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv （行 149209–154013，按关键词/日期分段检索）
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/提分技术讨论_原文.csv （全量 77 行，首遍原始语料）
- （第二遍上下文核对，只读）提分技术讨论_清单.md、CANN西南赛区_群聊情报交接文档.md
```

```
UNREAD_RANGES_IF_ANY
- group_901064769_text.txt 未逐行读取（其内容与 .csv 同源，本报告 RECORD_ID 统一用 .csv 行号）
- csv 中大量图片/表情/nudge 系统消息与被撤回消息（多为噪声）未逐条展开
- 窗口内非技术灌水段（如 09-24/09-25 的接龙、"金榜题名"等）仅抽样，未穷举
```

```
RESEARCH_LIMITATIONS
- 仅覆盖 2026-09-19~09-25 单窗口；case14 等 case 在本窗口几乎无社区讨论
- 全部为群聊文本，无源码/无 shape/dtype 字段；多数条目 HAS_SOURCE_CODE=NO
- 大量条目是 AI 辅助工作流叙述与情绪化表达，需与"实验事实"严格区分
- "同源码分数变化"无法在本报告内区分是 kernel runtime noise 还是 tbest 计分侧变化
- case14 157us（虽在 09-12，超出窗口）、case4/7 同簇（10-02）等关键结论均来自窗口外，本报告不作采信，仅作对照
- 未做任何 git/compile/correctness/local/NPU/online 操作，亦未进入 worktrees
```
