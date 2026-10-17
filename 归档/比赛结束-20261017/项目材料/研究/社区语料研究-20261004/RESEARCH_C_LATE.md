# RESEARCH-C (LATE PERIOD) — CANN AddRmsNormBias 群聊后期语料取证

- 研究角色：独立 Research Child（RESEARCH-C / LATE）
- 时间范围：2026-09-26 00:00 ~ 2026-10-02 23:59（语料导出 2026-10-03 00:00，10-03 无消息）
- 只读约束：未修改/创建/删除项目内任何文件；未进入任何 worktree；未执行 git/compile/correctness/local/NPU/online/hash
- 隔离上下文：`/tmp/cann-research/C/`
- 原始语料：
  - `group_901064769.csv`（行号即 RECORD_ID；后期切片起始物理行 154014，末行 157067）
  - `group_901064769_text.txt`
  - `提分技术讨论_原文.csv`（二级交叉核对，后期检索后才读）
- 时间切片规模：09-26 383 / 09-27 252 / 09-28 482 / 09-29 297 / 09-30 459 / 10-01 538 / 10-02 544 条，共 2955 条
- 重要方法学提示：CSV/text 均为单条消息一行（含转义引号），本文所有行号均为 CSV 物理行号；关键词检索用 Python（grep 对 UTF-8 中文匹配失败，见 LIMITATIONS）。

---

## CHAT_EVIDENCE（后期窗口内高价值记录 40 条 + 窗体外交叉对照 3 条，共 43 条）

CHAT_EVIDENCE
TIME = 2026-10-01 22:47–22:48
RECORD_ID = group_901064769.csv:L156368–L156379
RAW_EXCERPT = cquer崛起吧：「我踏马…我终于成功了…我踏马破了…破之前8 破了以后4」；哈吉米：「牛批 case4真刷了啊」
TOPIC = case4 单点被打破（社区自报）
CASE = case4
MECHANISM = 未披露（自报"爆破"成功，case4 值/分从 8→4 方向变化）
EVIDENCE_CLASS = DIRECT_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 后期最明确的 case4 突破信号，时间点（10-01 22:47）与后续 case7（23:01）连锁，支撑 case4/case7 同簇假说
WHY_IT_MAY_BE_FALSE = "8/4"语义不明（可能是名次或分数或时延），无代码、无形状、无 dtype，纯口头自报且当事人称"我自己都不知道怎么成的"

CHAT_EVIDENCE
TIME = 2026-10-01 23:01
RECORD_ID = group_901064769.csv:L156413
RAW_EXCERPT = cqu ai，算子，哈吉米：「case7」
TOPIC = case7 与 case4 同日连锁被破
CASE = case7
MECHANISM = 与 case4 疑似同簇、同手段
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = UNKNOWN
WHY_IT_MATTERS = 印证"破一个对另一个有帮助"，与 10-02 明确"同簇"表述互证
WHY_IT_MAY_BE_FALSE = 仅一个词，缺少数值；可能只是围观评论

CHAT_EVIDENCE
TIME = 2026-10-02 11:35
RECORD_ID = group_901064769.csv:L156612 / L156614
RAW_EXCERPT = cquer崛起吧：「因为他们同簇」/「爆破手段一样」
TOPIC = case4 与 case7 同簇（same cluster）
CASE = case4, case7
MECHANISM = 两 case 共享同一优化手段（同簇处理）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期最强、最可执行的 case 关联结论：case4/7 可合并攻击，一次手段迁移验证
WHY_IT_MAY_BE_FALSE = "同簇"是当事人主观归类，未给出形状/维度依据；不等于"同一段代码可直接迁移有效"

CHAT_EVIDENCE
TIME = 2026-10-02 04:03–04:14
RECORD_ID = group_901064769.csv:L156566 / L156567 / L156569 / L156570 / L156585
RAW_EXCERPT = 哈吉米：「4和7一起的是不是…他们的分发方式可能更加的…数学一点 同时做到4和7说明走的一种类似处理方式」；南山：「他说是4和7是一块攻破的…应该是有联系的…破了一个对另外一个也有帮助」
TOPIC = case4/7 同簇的机制解释与协同效应
CASE = case4, case7
MECHANISM = 疑似"更数学的分发/参数化处理方式"，同时适配多个形状
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 给出"同簇"的具体机制猜想（数学化分发），是后期最值得证伪的机制线索
WHY_IT_MAY_BE_FALSE = 纯第三方推断（"我觉得""应该"），当事人未确认实现细节

CHAT_EVIDENCE
TIME = 2026-10-02 04:00
RECORD_ID = group_901064769.csv:L156563
RAW_EXCERPT = 四川大学wilf：「真的没有用cpu吗，怎么会三四个形状不一样的都可以同时复用呢」
TOPIC = 多形状复用 / 疑似 host-cpu 参与
CASE = 多 case（指向 case4/7 及背后多形状）
MECHANISM = 一套实现对三四个不同形状复用，被怀疑非纯 NPU
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES（"三四个形状不一样"）
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与"纯NPU/绕过空核/探针"争议同源；指向"参数化泛化实现"这一高价值机制假设
WHY_IT_MAY_BE_FALSE = 是质疑而非结论；"复用"也可能只是共享模板的合法多分支 tile

CHAT_EVIDENCE
TIME = 2026-10-02 13:20
RECORD_ID = group_901064769.csv:L156675 / L156676 / L156678
RAW_EXCERPT = cquer崛起吧：「首先我能确定没有用CPU进行数值处理…我的核心计算全部由 AscendC NPU kernel 完成」「如果你是想推思路我无可奉告」
TOPIC = 关于是否 host-cpu 计算的正式回应
CASE = 多 case
MECHANISM = 声明核心计算全在 AscendC NPU kernel
EVIDENCE_CLASS = DIRECT_RESULT（当事人自证，非独立验证）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 唯一一次当事人对"是否用 CPU"的正面澄清；对判定离群值是合法实现还是作弊很关键
WHY_IT_MAY_BE_FALSE = 自证无独立审核；提问者 wilf 明确表示担心"只是打一个离群值"

CHAT_EVIDENCE
TIME = 2026-09-30 18:16–18:22
RECORD_ID = group_901064769.csv:L155785 / L155793 / L155798 / L155802
RAW_EXCERPT = wilf：「你确定你这纯npu吗case1，探针连值都探出来了直接输出是吧」「完全空核跟里面塞了死代码的值都不一样我不明白」；南山：「结论：v392 的 case1 是纯 NPU 计算，host 侧没有任何数值计算」
TOPIC = case1 极端值来源争议（探针/空核/死代码/host）
CASE = case1
MECHANISM = 疑似通过探针提前取得值直接输出；或空核/死代码路径
EVIDENCE_CLASS = CONTRADICTED / SPECULATIVE
CONFIDENCE = LOW（结论）~ MEDIUM（争议存在）
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES（"两次提交0波动"）
WHY_IT_MATTERS = 后期最戏剧性的"神迹"事件；直接关系 case1 榜上极小值是否可信（Tiny 类核心议题）
WHY_IT_MAY_BE_FALSE = 双方均未拿出可复核证据；"host无计算"结论来自 AI 自查，非官方裁决

CHAT_EVIDENCE
TIME = 2026-09-30 18:20 & 21:32
RECORD_ID = group_901064769.csv:L155798 / L155934–L155935
RAW_EXCERPT = wilf：「第一次修之前case1是1.5，grok爆破1.24，后面就是ggbond的稳定1.7左右，彩票1.6左右」；南山：「给case1又刷了 1.27us」
TOPIC = case1 同形状多次提交的历史值区间
CASE = case1
MECHANISM = 同 case 不同实现/不同 AI 得到 1.24~1.7us 区间
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 给出 case1（Tiny）后期可达区间与波动，是判断 Tiny 类是否受 launch/噪声主导的直接社区数据
WHY_IT_MAY_BE_FALSE = 二手转述历史值；"彩票/ggbond"等昵称指代不可核实

CHAT_EVIDENCE
TIME = 2026-10-02 23:07–23:13
RECORD_ID = group_901064769.csv:L157027 / L157028 / L157042
RAW_EXCERPT = 南山：「tb2是不是又破了 怎么变成1.95了」；哈吉米：「好像近段时间都是1.95吧」；南山：「我记得昨天还是197的啊」
TOPIC = case2 tbest 在榜上变到 1.95
CASE = case2
MECHANISM = 榜单 tbest 刷新（非本人实现）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES（2.12→1.95 语境）
WHY_IT_MATTERS = 后期 case2 榜值下探的观测链；与 09-30「case2地板2.2」形成下修对照
WHY_IT_MAY_BE_FALSE = 仅凭看榜口述，缺乏截图；"197"疑为"1.97"口误

CHAT_EVIDENCE
TIME = 2026-09-30 18:13–18:42
RECORD_ID = group_901064769.csv:L155779 / L155781 / L155841 / L155842
RAW_EXCERPT = 南山：「case2也是tb」「case2的地板可能就是2.2了吧」；哈吉米：「他还在刷case2的tbest啊 干到2.12了」
TOPIC = case2 tbest 下探（2.2→2.12）
CASE = case2
MECHANISM = 单点记录打磨
EVIDENCE_CLASS = DIRECT_RESULT（观察榜）/ REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = case2（Tiny）后期持续被"单点打磨"下压，说明 Tiny 榜值对微优化高度敏感
WHY_IT_MAY_BE_FALSE = 口述榜值，无独立复核

CHAT_EVIDENCE
TIME = 2026-10-02 04:10–04:25
RECORD_ID = group_901064769.csv:L156576 / L156578 / L156579 / L156580 / L156582 / L156587
RAW_EXCERPT = 南山：「最高我才5.6…之前一直在6us」；cqu李四：「我感觉到5.5左右就不好提升了 会有噪声波动 每次提交都不一样」；南山：「这个波动太搞人了」/「wilf他的应该稳定在5.4左右」
TOPIC = case5 提交间噪声波动（约 5.5–6us）
CASE = case5
MECHANISM = 测试噪声 / 抖动，非单纯代码变化
EVIDENCE_CLASS = NEGATIVE_RESULT / REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 后期最清晰的"同源码/近似源码跨提交抖动"证据，直接支撑用本地/线上噪声模型解释微小分差（Tiny/固定开销议题）
WHY_IT_MAY_BE_FALSE = 无同源码多次提交的量化表格，区间为口述

CHAT_EVIDENCE
TIME = 2026-10-01 02:59–03:06
RECORD_ID = group_901064769.csv:L156089–L156099
RAW_EXCERPT = 南山：「我还在优化case5」「刚刚2.06的时候他刚好卡线84」「你把奶娃兄给掉到了84以下」「打t3打出副作用了勒」
TOPIC = 优化某点导致其他点分数变化（副作用）
CASE = case5, case3（"t3"）
MECHANISM = 局部优化引发全局效应/分数连带
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 佐证"改一处、他处退化"的全局耦合，是后期反复出现的负结果主题
WHY_IT_MAY_BE_FALSE = "副作用""卡线"为口语，未指明具体 case/数值

CHAT_EVIDENCE
TIME = 2026-10-01 17:49–18:03
RECORD_ID = group_901064769.csv:L156172 / L156180 / L156181
RAW_EXCERPT = cqu王亚东：「我的tp13终于攻克了」「11也突破了」「果然13和11的关键是一样的」
TOPIC = case11 与 case13 共享关键机制
CASE = case11, case13
MECHANISM = 11 与 13 关键点相同（同手段迁移）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 又一组 case 关联（11↔13），与 case4/7 同簇并列，可作为跨 case 迁移候选
WHY_IT_MAY_BE_FALSE = "关键一样"为主观感受；"tp13"是否确指 case13 存在歧义

CHAT_EVIDENCE
TIME = 2026-10-02 11:50
RECORD_ID = group_901064769.csv:L156635 / L156636
RAW_EXCERPT = cquer崛起吧：「其实我的预想是67和11，13连爆 但是13受限于显存带宽」
TOPIC = case6/7、11、13 计划连爆；13 受显存带宽限制
CASE = case6, case7, case11, case13
MECHANISM = 13 受 memory bandwidth 约束（带宽瓶颈）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 明确提出 13 的瓶颈是显存带宽（与前期"超越显存带宽物理极限"呼应）；给出 case 分组线索
WHY_IT_MAY_BE_FALSE = "预想"未完全实现；带宽瓶颈为推断

CHAT_EVIDENCE
TIME = 2026-10-02 23:17
RECORD_ID = group_901064769.csv:L157056
RAW_EXCERPT = cqupt 南山必胜客：「刚把case7的给探测出来了」
TOPIC = case7 通过"探测/探针"取得关键参数
CASE = case7
MECHANISM = 用探针（时间反推/参数探测）获得形状或参数
EVIDENCE_CLASS = DIRECT_RESULT（方法自报）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES（探出形状）
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期最明确的"探针"实战应用；与 case14 shape 问题同源
WHY_IT_MAY_BE_FALSE = 未给出探出的值；"探测"可能是让 AI 反推

CHAT_EVIDENCE
TIME = 2026-10-02 20:50
RECORD_ID = group_901064769.csv:L156935
RAW_EXCERPT = cqupt 南山必胜客：「探针探出来形状了，做针对性优化」
TOPIC = 探针取得形状后做针对性优化
CASE = 泛指（紧接 case14 讨论）
MECHANISM = 先探形状 → 形状特化优化
EVIDENCE_CLASS = SPECULATIVE / 方法披露
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 156916"看流水线细节耗时"、前期"探测指令/带宽/核数"构成完整方法论线
WHY_IT_MAY_BE_FALSE = 是口号式发言，未给探针数据

CHAT_EVIDENCE
TIME = 2026-10-02 23:11–23:16
RECORD_ID = group_901064769.csv:L157033 / L157034 / L157035 / L157053
RAW_EXCERPT = cqu王亚东：「我和他说帮我用探针测数据 他告诉我不合法…拒绝给我做」；cqucqu：「我压力GPT…它就自己去探数据了」
TOPIC = 探针取数据被 AI 判定为"不合法"
CASE = 泛指
MECHANISM = 探针（probe）取数据的合规性争议
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 证明"探针"在后期是活跃且具合规风险的取证手段；提示相关离群值需审慎
WHY_IT_MAY_BE_FALSE = 是 AI 的合规判断，非官方规则；"不合法"未经组委会确认

CHAT_EVIDENCE
TIME = 2026-09-30 20:41
RECORD_ID = group_901064769.csv:L155885
RAW_EXCERPT = cqu ai，算子，哈吉米：「【Kernel】 出分前的41秒内，昇腾评测集群再次响起了哈吉米的吟唱："流水线""全向量归约""零冲突"。这…（把整句复制进昵称）」
TOPIC = "流水线/全向量归约/零冲突"被玩成梗
CASE = 泛指
MECHANISM = 流水线、全向量归约、零冲突（zero-conflict）作为提分关键词
EVIDENCE_CLASS = JOKE / REPEATED_SIGNAL
CONFIDENCE = LOW（作为事实）~ MEDIUM（作为社区高频词）
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映后期社区对"流水线/向量归约/零冲突"这三类机制的集体关注，是机制热度信号而非实现证据
WHY_IT_MAY_BE_FALSE = 明确是段子（随后自嘲"Gemini最终幻想罢了 它自己写的"）

CHAT_EVIDENCE
TIME = 2026-10-02 00:12
RECORD_ID = group_901064769.csv:L156522 / L156523
RAW_EXCERPT = cqu王亚东：「事实证明ai会说瞎话」「明明说给我流水并行了，但实际上没有」「好强的幻觉」
TOPIC = AI 声称已做流水线并行，实际未做
CASE = 泛指
MECHANISM = 流水线并行（pipeline）声称 vs 实际
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 直接提示"AI 汇报的优化≠真实优化"，对甄别社区"已实现"类主张极重要
WHY_IT_MAY_BE_FALSE = 是使用者对自身 AI 的主观不信任，未给出核验方式

CHAT_EVIDENCE
TIME = 2026-10-02 11:36
RECORD_ID = group_901064769.csv:L156621
RAW_EXCERPT = cquer崛起吧：「我要做kernel融合」
TOPIC = kernel 融合（计划/意图）
CASE = 泛指
MECHANISM = kernel fusion（多算子/多阶段融合）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期唯一明确提出的 fusion 意图；需注意其与"同簇多形状复用"可能同源
WHY_IT_MAY_BE_FALSE = 只是"要做"，无任何结果；与上一句"我自己都不知道（怎么破的）"自相矛盾

CHAT_EVIDENCE
TIME = 2026-10-02 20:38
RECORD_ID = group_901064769.csv:L156916
RAW_EXCERPT = cqu ai，算子，哈吉米：「你可以看流水线具体细节耗时倒是 但也仅供参考」
TOPIC = 通过 profiling 看流水线各阶段耗时
CASE = 泛指
MECHANISM = 流水线阶段耗时分析（profiling）
EVIDENCE_CLASS = SPECULATIVE / 方法披露
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 指出可观测流水分段耗时，但"仅供参考"暗示其不可靠（与波动主题一致）
WHY_IT_MAY_BE_FALSE = 自称"仅供参考"，未给数据

CHAT_EVIDENCE
TIME = 2026-09-28 23:45
RECORD_ID = group_901064769.csv:L155110 / L155113 / L155116 / L155120
RAW_EXCERPT = cqu李四：「为什么我在A2平台上测有提升到了Judge全没了？？？」；wilf：「因为平台数值不一定一样哦」；哈吉米：「我a2上跑得都比tbest快了 交上去还是区」
TOPIC = 本地 A2 平台与 Judge 平台结果不一致
CASE = 泛指
MECHANISM = 本地硬件/环境 ≠ 评测平台，提升不可迁移
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 后期最强的"本地↔线上不可迁移"证据，直接支撑 Local↔Official 校准问题
WHY_IT_MAY_BE_FALSE = 未记录具体 case/数值；"A2""Judge"可能为不同型号

CHAT_EVIDENCE
TIME = 2026-09-28 19:44–19:48
RECORD_ID = group_901064769.csv:L154987 / L154989 / L154998
RAW_EXCERPT = 塔菲喵：「我的 gpt 说已经接近带宽极限」；南山：「我的glm说case5 9us已经接近极限了」；塔菲喵：「其实大家都是绕过核用 cpu 跑的 就我不是」
TOPIC = AI 声称接近带宽极限 / "绕过核用 cpu"
CASE = case1, case5
MECHANISM = bandwidth limit；非法 host-cpu 路径（后者为玩笑）
EVIDENCE_CLASS = AI_SUMMARY / JOKE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映"带宽极限"话术多来自 AI 而非实测；且"绕过核用 cpu"为群体玩笑，警示勿当真
WHY_IT_MAY_BE_FALSE = 明确为 AI 说法/玩笑，随后被"空的塞死代码"接龙证实是玩梗

CHAT_EVIDENCE
TIME = 2026-09-28 19:49
RECORD_ID = group_901064769.csv:L155001 / L155002 / L155005
RAW_EXCERPT = 多人接龙：「其实我们交的都是空的里面塞的死代码，就为了结束的时候阴你们一把」
TOPIC = "空核塞死代码"接龙
CASE = 泛指
MECHANISM = 空 kernel + dead code 改变计时
EVIDENCE_CLASS = JOKE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 case1"空核 vs 死代码值不同"的技术观察相互污染，需警惕把玩笑当事实
WHY_IT_MAY_BE_FALSE = 群体复读玩梗

CHAT_EVIDENCE
TIME = 2026-09-30 18:21
RECORD_ID = group_901064769.csv:L155802 / L155803 / L155804
RAW_EXCERPT = 南山：「查完了，结论：v392 的 case1 是纯 NPU 计算，host 侧没有任何数值计算。证据如下：（下条空）」/「自查了」
TOPIC = 用 AI 自查判定 case1 为纯 NPU
CASE = case1
MECHANISM = host-side compute 排查
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 该"结论"直接支撑了 case1 神迹的真实性叙事，但证据链是空消息（L155803 为空）
WHY_IT_MAY_BE_FALSE = "证据如下"后无实际内容；自查 ≠ 独立复核

CHAT_EVIDENCE
TIME = 2026-09-30 18:38–08:16
RECORD_ID = group_901064769.csv:L155828 / L155837 / L155855 / L155856
RAW_EXCERPT = 哈吉米：「你交三次整出三个tbest」「接下来的case1 2这连续3个tbest会很疯狂」；南山同句复读
TOPIC = 连续多次提交连出多个 tbest
CASE = case1, case2
MECHANISM = 反复提交命中 time-best（抽卡/精修）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 侧面证明榜单 tbest 在后期被高频刷新；暗示存在"多次提交取最优"的现象
WHY_IT_MAY_BE_FALSE = 复读梗成分高；缺提交号/数值

CHAT_EVIDENCE
TIME = 2026-09-29 20:57
RECORD_ID = group_901064769.csv:L155437 / L155439 / L155440
RAW_EXCERPT = 四川大学wilf：「抽到tb以后，没抽到会减少大家对你这个点的分差 减少上限」
TOPIC = 计分机制：命中 tbest 会压缩其他队伍在该点的分差上限
CASE = 泛指
MECHANISM = tbest 相对分差计分（榜值作为分母/上限）
EVIDENCE_CLASS = SPECULATIVE（机制推断）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期关于"为何刷榜会影响他人分数"的最清晰机制解释，有助于理解单点打磨动机
WHY_IT_MAY_BE_FALSE = 玩家自行推断，未对照官方公式

CHAT_EVIDENCE
TIME = 2026-09-29 22:35
RECORD_ID = group_901064769.csv:L155449
RAW_EXCERPT = 南山：「P14 平台最优 3750.12 → 3665.94（-2.2%），队伍 cd（ID 685）今晚 21:39 刷出…总分只有68.22——典型专刷单点纪录打法」
TOPIC = P14 单点记录被专刷队伍下压
CASE = case14
MECHANISM = 只优化大点、反复提交磨单点 time-best
EVIDENCE_CLASS = AI_SUMMARY（随后 155450/155451 承认是 AI 总结）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 提供 case14 平台最优量级（3750→3665），并揭示"专刷单点"打法；是后期 case14 唯一带数字的结构化信息
WHY_IT_MAY_BE_FALSE = 明确为 AI 生成；数字未经独立核对

CHAT_EVIDENCE
TIME = 2026-09-29 19:50
RECORD_ID = group_901064769.csv:L155379
RAW_EXCERPT = 南山：「另外注意到平台最优 T 被别的队刷新了（P9 67.98→67.81、P15 8272.9→8249.6）」
TOPIC = 平台最优 T 被刷新（P9、P15）
CASE = case9, case15
MECHANISM = 单点纪录刷新
EVIDENCE_CLASS = SECOND_HAND（看榜）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 后期早期给出 9/15 的 tbest 量级与刷新方向，可用于校准"接近极限"判断
WHY_IT_MAY_BE_FALSE = 口述榜值；数值单位/口径不明

CHAT_EVIDENCE
TIME = 2026-10-02 20:42–20:44
RECORD_ID = group_901064769.csv:L156925 / L156926 / L156927
RAW_EXCERPT = 泥交小登：「case 14 大概是个什么形状」「我这儿出奇的高」；南山：「自己摸吧，我也没完全搞下来」
TOPIC = case14 形状未知、分数异常高
CASE = case14
MECHANISM = 形状不明；某选手 case14 时延异常高
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO（未给）
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期明确"case14 形状仍未知"，佐证 case14 是高 ROI 但缺形状信息的攻关点
WHY_IT_MAY_BE_FALSE = 未量化"出奇的高"；无人给出形状

CHAT_EVIDENCE
TIME = 2026-10-02 23:03（另 10-01 00:30）
RECORD_ID = group_901064769.csv:L157018–L157026 / L155967–L155970
RAW_EXCERPT = 哈吉米：「没有啊 应该确实还有一个」、ruby：「Astra道德水平还挺低下」；另 wilf 10-01「ok我证明了不是6.1sol的功劳 可以撤了」
TOPIC = 高分归因之争（模型 vs 方法）
CASE = 泛指
MECHANISM = 高分是否来自特定模型（6.1sol/Astra）
EVIDENCE_CLASS = CONTRADICTED
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 暴露"归因给模型"的普遍幻觉；提醒勿把模型名当提分机制
WHY_IT_MAY_BE_FALSE = 双方说法互相矛盾，均无对照实验

CHAT_EVIDENCE
TIME = 2026-10-01 18:04
RECORD_ID = group_901064769.csv:L156192
RAW_EXCERPT = cqu王亚东：「感谢gpt6.1sol助我破鼎」
TOPIC = 归因给 6.1sol
CASE = case11/13 语境
MECHANISM = 模型能力
EVIDENCE_CLASS = CONTRADICTED（与 L155967 "不是6.1sol的功劳" 冲突）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L155967 直接构成同一议题的矛盾记录
WHY_IT_MAY_BE_FALSE = 情绪化发言

CHAT_EVIDENCE
TIME = 2026-09-28 23:44
RECORD_ID = group_901064769.csv:L155105 / L155106 / L155107
RAW_EXCERPT = physics：「我队友他把 case12和9 全部破解了 我觉得逆天了」
TOPIC = case12/9 被破解
CASE = case9, case12
MECHANISM = 未披露
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = YES
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期出现的新 case 关联（9↔12），可与 11↔13 并列考察
WHY_IT_MAY_BE_FALSE = 转述队友成果，无任何细节

CHAT_EVIDENCE
TIME = 2026-09-30 10:18
RECORD_ID = group_901064769.csv:L155567
RAW_EXCERPT = cqu ai，算子，哈吉米：「这个是950的 不是910b的架构」（针对 L155562 分享的 DeepGEMM-Ascend）
TOPIC = 平台/架构：950 而非 910B
CASE = 泛指
MECHANISM = 目标架构为 Ascend 950 系列
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若成立，则参考库（DeepGEMM-Ascend 面向 950）与本赛目标架构关系需澄清；影响优化选型
WHY_IT_MAY_BE_FALSE = 玩家个人判断；同社群另有 910B 本地机（L156995 我的910B挂了），架构口径混乱

CHAT_EVIDENCE
TIME = 2026-09-30 23:48
RECORD_ID = group_901064769.csv:L155955 / L155956
RAW_EXCERPT = skye：「CANNJudge平台测试时用的是哪个版本的CANN镜像」；wilf：「9.0.0」「题目上面有写」
TOPIC = 评测镜像版本
CASE = 泛指
MECHANISM = CANN 9.0.0
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 环境版本事实（CANN 9.0.0），对复现/编译有意义
WHY_IT_MAY_BE_FALSE = 非官方答复，来源为题干

CHAT_EVIDENCE
TIME = 2026-09-26 14:54–14:56
RECORD_ID = group_901064769.csv:L154144 / L154145 / L154149
RAW_EXCERPT = 塔菲喵：「我抽了 40 次 没提升」；「那个 case 后面自己都复现不了的…等于是没收益啊」
TOPIC = 40 次提交无提升 + 结果不可复现
CASE = 泛指
MECHANISM = 抽卡式提交、不可复现
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期早期最强的"提交噪声/不可复现"负结果，直接关系微小分差的解释
WHY_IT_MAY_BE_FALSE = 未指明具体 case；40 次无提升也可能因改动无效

CHAT_EVIDENCE
TIME = 2026-09-28 19:11–19:12
RECORD_ID = group_901064769.csv:L154960 / L154964
RAW_EXCERPT = 南山：「测试波动真的大」；塔菲喵：「波动太大了」
TOPIC = 测试波动大（泛）
CASE = 泛指
MECHANISM = 评测噪声
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 case5 波动、A2/Judge 不一致共同构成后期"噪声主线"
WHY_IT_MAY_BE_FALSE = 无量化

CHAT_EVIDENCE
TIME = 2026-09-29 15:21–15:23
RECORD_ID = group_901064769.csv:L155284 / L155285 / L155287
RAW_EXCERPT = cqu李四：「官方底层API都是最优的了吗？需要我们重构底层API吗？」；华为赛题组专家Mr.田：「不一定是最优，起码不是针对所有case最优，自己可以尝试」
TOPIC = 官方库并非对所有 case 最优
CASE = 泛指
MECHANISM = 官方 kernel/API 存在 case 特化优化空间
EVIDENCE_CLASS = DIRECT_RESULT（官方专家发言）
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期唯一带官方身份的可行性背书：自研 kernel 优于官方库是被允许/鼓励的
WHY_IT_MAY_BE_FALSE = 未指明哪些 case

CHAT_EVIDENCE
TIME = 2026-10-02 00:09–00:10
RECORD_ID = group_901064769.csv:L156513 / L156515 / L156517
RAW_EXCERPT = 南山：「现在32强分数已经低于72了」；敖厂长：「问题在于大家的分也掉了」；南山：「大家的分都被吃掉了3分左右」
TOPIC = 榜单整体分数下修（≈-3 分）
CASE = 泛指
MECHANISM = 榜值大规模被刷新导致他人相对分下降
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 反映后期多 case 集体被破导致晋级线回落，解释分数剧烈波动环境
WHY_IT_MAY_BE_FALSE = 口述榜单；"3分"为估计

CHAT_EVIDENCE
TIME = 2026-09-26 16:49–16:51
RECORD_ID = group_901064769.csv:L154225 / L154229 / L154238
RAW_EXCERPT = 塔菲喵：「最后测评是看在榜的还是官方最后测」；wilf：「封榜」…「离tbest越近越」
TOPIC = 最终成绩以封榜为准 + 越接近 tbest 分越高
CASE = 泛指
MECHANISM = 计分随 tbest 接近而提升
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 后期关于"封榜/接近tbest得分"的机制认知，与前 tbest 分差机制互证
WHY_IT_MAY_BE_FALSE = 玩家推测；句子被截断

CHAT_EVIDENCE（窗体外交叉：属早期，本条仅作机制对照，不计入后期结论）
TIME = 2026-09-22 09:25
RECORD_ID = group_901064769.csv:L151430 / L151431
RAW_EXCERPT = 重庆邮电大学KPBOT：「目前在尝试让5.6sol给我改进成把大D运算变成cube+vector 混合」/「但是效果很不好」
TOPIC = cube+vector 混合尝试失败
CASE = 泛指（大 D 运算）
MECHANISM = cube + vector（矩阵+向量）混合处理大 D
EVIDENCE_CLASS = NEGATIVE_RESULT（早期）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES（"大D"）
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 任务指定线索来源。注意：这是**一次**负实现证据（特定尝试、特定 sol），不能推导整个 cube+vector 机制无效
WHY_IT_MAY_BE_FALSE = 属 09-22 早期，不允许据此支撑后期结论；仅为对照与防误导

CHAT_EVIDENCE（窗体外交叉：属早期）
TIME = 2026-09-23 21:50 & 21:53
RECORD_ID = group_901064769.csv:L152664 / L152674
RAW_EXCERPT = 重庆邮电大学KPBOT：「分析prof后，我发现随着D的增大，UB吞吐也就越高」/「或许D和B的形状应该通过metadata给换为正方形？」
TOPIC = D↑ → UB 吞吐↑；形状换正方形猜想
CASE = 泛指
MECHANISM = UB 吞吐与 D 正相关；metadata 重排形状
EVIDENCE_CLASS = SPECULATIVE（早期）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO
HAS_SHAPE = YES
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES（D 变化→吞吐变化）
WHY_IT_MATTERS = 提供"D 维度/形状重排"这条可迁移机制线，与后期"形状复用/params 数学化分发"呼应
WHY_IT_MAY_BE_FALSE = 属早期；"几何直觉"为猜想

CHAT_EVIDENCE（窗体外交叉：属早期）
TIME = 2026-09-13 21:41
RECORD_ID = group_901064769.csv（原文见 提分技术讨论_原文.csv:L49）
RAW_EXCERPT = 骧溺：「同一份源码提交 20 次，分数区间差距能有三四分」
TOPIC = 同源码 20 次提交波动 3–4 分
CASE = 泛指
MECHANISM = kernel runtime noise / 评测抖动
EVIDENCE_CLASS = NEGATIVE_RESULT（早期）
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO
HAS_SHAPE = NO
HAS_DTYPE = NO
HAS_CASE_BINDING = NO
HAS_BEFORE_AFTER = YES
WHY_IT_MATTERS = 与后期 case5 波动（L156578–L156582）、L154960 共同构成"同源码跨提交抖动"证据簇；是理解微小分差的关键
WHY_IT_MAY_BE_FALSE = 属早期；"三四分"粒度粗，未给分布

---

## MECHANISM_CLUE（后期机制线索，6 条）

MECHANISM_CLUE
CLUE_ID = C-LATE-01
MECHANISM = case4 与 case7 同簇：共享同一优化手段/分发方式，可一次手段迁移同时受益
AFFECTED_CASES = case4, case7
SUPPORTING_RECORDS = L156612,L156614,L156566,L156567,L156569,L156570,L156585,L156368–L156379,L156413
CONTRADICTING_RECORDS = 无直接反证
NEGATIVE_EVIDENCE = 早期 L151430–L151431（cube+vector 负结果）与 case4 历史"没下7"(提分原文 L17) 显示单点攻克难度大
CONFIDENCE = HIGH（关联）/ MEDIUM（机制具体内容）
KNOWN_FACTS = 有选手自报同时破 4 与 7；当事人明确"同簇、手段一样"；社区公认"破一个对另一个有帮助"
UNKNOWN_FACTS = "同簇"具体指形状相似、维度相似还是固定开销结构相似；该手段是否可复现
MINIMUM_FACT_NEEDED_NEXT = case4/case7 的输入 shape/dtype；同一手段在两者上的 before/after 时延；是否有源码或伪代码片段

MECHANISM_CLUE
CLUE_ID = C-LATE-02
MECHANISM = 探针/时间反推取得形状与参数（先探形状→针对性优化）
AFFECTED_CASES = case7, case14, 泛指多 case
SUPPORTING_RECORDS = L157056,L156935,L157033–L157035,L157053,L156916
CONTRADICTING_RECORDS = L155785（怀疑探针直接输出值）；L157034（AI 判"不合法"）
NEGATIVE_EVIDENCE = 早期 L148633/L148824（可探测性但"部分读写指令开销不好算"）
CONFIDENCE = MEDIUM
KNOWN_FACTS = 后期多名选手用探针取得形状/参数；case7 已"探测出来"；case14 仍"没人给可信形状"
UNKNOWN_FACTS = 探针具体实现（计时?指令计数?）；合规边界
MINIMUM_FACT_NEEDED_NEXT = 探针输出的 case7/case14 shape；其结果与后来实测一致性

MECHANISM_CLUE
CLUE_ID = C-LATE-03
MECHANISM = case11 与 case13 关键点相同（同手段迁移）
AFFECTED_CASES = case11, case13
SUPPORTING_RECORDS = L156172,L156180,L156181
CONTRADICTING_RECORDS = L156635（13 受显存带宽限制，可能不是完全相同约束）
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 某选手同日先破 13、随即破 11，并称"关键一样"
UNKNOWN_FACTS = 共同关键点是什么；"tp13"是否等于 case13
MINIMUM_FACT_NEEDED_NEXT = 11 与 13 的 shape/dtype；两 case 时延量级与瓶颈类型

MECHANISM_CLUE
CLUE_ID = C-LATE-04
MECHANISM = "更数学的分发/参数化"实现使一套代码适配多个不同形状（疑似多形状复用）
AFFECTED_CASES = case4, case7（及"三四个形状"）
SUPPORTING_RECORDS = L156563,L156566,L156567,L156585
CONTRADICTING_RECORDS = L156676（当事人坚称全 NPU，未证实"参数化"）
NEGATIVE_EVIDENCE = L156563 的怀疑本身即"若真能复用则可能非纯 NPU"这一质疑
CONFIDENCE = LOW-MEDIUM
KNOWN_FACTS = 有人观察到"三四个形状不一样也能同时复用"；第三方推测分发方式"更数学"
UNKNOWN_FACTS = 该复用是合法的多分支 tile 还是 host 参与；具体参数化维度
MINIMUM_FACT_NEEDED_NEXT = 复用实现是否在单一 kernel 内、是否随 shape 编译期分派；能否给出 shape 列表

MECHANISM_CLUE
CLUE_ID = C-LATE-05
MECHANISM = 后期评测存在显著跨提交噪声（同/近源码波动），且本地环境结果不可迁移到 Judge
AFFECTED_CASES = case5, case1, 泛指 Tiny 类
SUPPORTING_RECORDS = L156578,L156579,L156580,L156582,L156587,L155110,L155113,L155116,L155120,L154960,L154964
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = L154144/L154149（40 次提交无提升且不可复现）
CONFIDENCE = HIGH
KNOWN_FACTS = case5 有 5.5–6us 提交波动；A2 本地提升在 Judge 消失；早期同源码 20 次差 3–4 分
UNKNOWN_FACTS = 噪声幅度分布的定量模型；是否为调度/频率/多租干扰
MINIMUM_FACT_NEEDED_NEXT = 同源码 N 次提交的时延分布（均值/方差）；本地 vs Judge 同源码对照

MECHANISM_CLUE
CLUE_ID = C-LATE-06
MECHANISM = 计分：接近/命中 tbest 会显著改变相对分（命中 tbest 会压缩他人在该点的分差上限）
AFFECTED_CASES = 泛指（case1/2/5/7 等被频繁刷点）
SUPPORTING_RECORDS = L155437,L155439,L155440,L154225,L154229,L154238,L155449,L156513
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无
CONFIDENCE = MEDIUM
KNOWN_FACTS = 玩家共识"离 tbest 越近分越高"；多人观察到刷点导致他人掉分
UNKNOWN_FACTS = 官方公式；是否为 max/归一化
MINIMUM_FACT_NEEDED_NEXT = 官方计分公式或足够多的 (时延, 分数) 对照点

---

HIGH_CONFIDENCE_SIGNALS
- case4 与 case7 同簇、可合并攻击（L156612/L156614 + L156368–L156379 + L156413）：后期最强可执行发现。
- case11 与 case13 关键点相同（L156172/L156180/L156181）。
- 后期存在显著跨提交噪声与本地不可迁移：case5 波动 5.5–6us（L156578–L156582）、A2 提升在 Judge 消失（L155110/L155116）、40 次无提升且不可复现（L154144/L154149）。
- 官方专家背书：官方库并非对所有 case 最优，允许自研（L155284/L155287）。
- 评测镜像 CANN 9.0.0（L155955/L155956）。
- 后期 "探针/时间反推取形状" 是真实活跃方法论（L157056/L156935 + 早期 L148633/L148824/L151430）。

MEDIUM_CONFIDENCE_SIGNALS
- case4 于 10-01 被破（L156368–L156379）；case7 同日连锁（L156413）。
- case2 tbest 后期下探（2.2→2.12→1.95）（L155779/L155841/L157027–L157042）。
- case1 后期可达 1.24~1.27us（L155798/L155935）；但伴随"探针/空核/host"争议（L155785/L155793/L155802）。
- case14 平台最优量级 3750→3665（L155449，但为 AI 总结、口径不明）；case14 形状后期仍未知（L156925/L156926）。
- case11/13 中 13 受显存带宽限制（L156635）。
- 架构为 950 而非 910B 的说法（L155567，存疑）。

LOW_CONFIDENCE_SIGNALS
- "流水线/全向量归约/零冲突"作为机制口号（L155885 段子；L156621 kernel 融合仅意图；L156916 流水线耗时"仅供参考"）。
- "更数学的分发方式 / 多形状复用"（L156563/L156567）——纯推断。
- 高分归因给特定模型（6.1sol/Astra）（L156192 vs L155967）。
- 计分机制细节（命中 tbest 压缩他人分差上限）（L155437/L155439）——玩家推断。

NEGATIVE_EVIDENCE
- case5 提交间噪声 5.5–6us，每次提交都不一样（L156578–L156582）。
- 本地 A2 提升到 Judge 全无（L155110/L155116/L155120）。
- 40 次提交无提升，且该 case 后续无法复现（L154144/L154149）。
- 早期同源码 20 次提交差 3–4 分（提分原文 L49）。
- 早期 cube+vector 混合"效果很不好"（L151430/L151431，单次负实现，勿外推）。
- AI 声称已做流水线并行但实际未做（L156522/L156523）。
- 优化 case5/3 产生副作用、他人掉分（L156089–L156099）。

CONTRADICTIONS
- 同一选手 L156192「感谢 gpt6.1sol 助我破鼎」 vs L155967「证明了不是 6.1sol 的功劳」——归因自相矛盾。
- L155802 称"v392 的 case1 是纯 NPU，host 无计算" vs L155785/L155793 怀疑探针直接输出、空核 vs 死代码值不同——同一事件两种相反叙事。
- L156676 当事人坚称"全 AscendC NPU kernel" vs L156563/L156668 反复质疑是否用 host cpu。
- L155567"架构是 950 不是 910B" vs 社群广泛讨论本地 910B（L156995）——架构口径混乱。

UNKNOWN_BUT_IMPORTANT
- case14 的真实 shape/dtype（后期仍无人给出）——最高 ROI 但信息真空。
- case4/case7"同簇"的具体技术含义与可迁移手段的可复现性。
- "三四个形状复用"是否为合法单 kernel 多分支，还是 host 参与。
- 探针方法的实现与合规边界（被 AI 判"不合法"，但无官方口径）。
- 官方计分公式（tbest 影响分差/上限的确切机制）。
- 评测噪声的定量分布，及本地↔Judge 的系统性偏差。

RAW_FILES_READ
- group_901064769.csv（后期切片 L154014–L157067；另按关键词在全文 1–157067 命中处做定位读取）
- group_901064769_text.txt（格式核对，未全量）
- 提分技术讨论_原文.csv（77 行，全文，作为二级交叉核对）
- 提分技术讨论_清单.md（只读，遗漏检查）
- CANN西南赛区_群聊情报交接文档.md（只读，仅 grep 关键段落做遗漏检查；未逐行通读）

UNREAD_RANGES_IF_ANY
- CSV/text 中非后期（2026-09-10~09-25）的绝大多数消息未逐行通读，仅按指定关键词命中定位（属窗体设计，非遗漏）。
- CSV 内含转义换行/引号的复杂记录，本文以物理行号定位；个别多行消息的续行未纳入摘录（正文内容以首行为准）。
- 提分技术讨论_清单.md 与交接文档为二级材料，仅做定向核对，未全文精读。
- 未读取 worktrees/ 下任何内容（约束要求）。

RESEARCH_LIMITATIONS
- grep 对 UTF-8 中文不匹配（二进制误判 + locale），已改用 Python 处理；行号以 CSV 物理行为准。
- 语料为 QQ 群转储，含大量玩梗/复读/撤回/系统消息，噪声极高；"AI 总结""转述""自报"三类证据占比大，需降权。
- 无任何源码、shape、dtype 细节流入后期聊天（HAS_SOURCE_CODE 全为 NO），机制结论均为线索级，不可当作事实。
- case14 157us、case1 "纯NPU神迹"等离群值均**未获独立验证**，本文按 CONTRADICTED/SPECULATIVE 处理，不采信为有效基准。
- 时间切片仅覆盖 09-26~10-02；早期（09-12~09-25）的关键方法论（探测指令/带宽/核数、cube+vector 负结果、D↑→UB 吞吐↑）仅在窗体外交叉中标注，不属于本文后期结论。
- 本文只做取证与线索标注，不含路线选择、不含 CREATE_ROUTE/ONLINE_DECISION 等决策字段。
