# RESEARCH-E（HARDWARE / MECHANISM）— 社区语料硬件与机制取证

- 角色：RESEARCH-E 独立研究 Child（只读取证）
- 隔离上下文：`/tmp/cann-research/E/`（无项目/git 变更）
- 数据周期：2026-09-10 22:12 ~ 2026-10-02 23:38（全时间范围）
- 原始语料（第一遍）：`group_901064769.csv` / `group_901064769_text.txt` / `提分技术讨论_原文.csv`
- 二级材料（仅在原始提取完成后用于遗漏/上下文交叉核对）：`交付/报告/总报告.md`、`提分技术讨论_清单.md`、`CANN赛题知识库.md`

## 0. 取证前置说明（重要）

1. 两份原始语料各含 1 个 NUL 字节（CSV offset≈5309669，TXT offset≈5295924）。ripgrep 默认按二进制处理会在 NUL 处提前停止扫描，导致后半段（约 L152900+）漏检。
2. 为不改变行号，我在 `/tmp/cann-research/E/` 下生成去 NUL 的镜像 `san_csv.txt` / `san_txt.txt`（仅删除 1 个 NUL 字符，行号与原文件完全一致），所有 grep 与行号引用均基于该镜像 = 原始行号。
3. 语料含大量噪声：`#接龙` 报名刷屏、系统 JSON（含 base64 形式的 `UB`/`row` 伪命中）、撤回提示。已人工剔除，仅保留真人技术发言。
4. 全文用关键词多形态 grep 覆盖（core/核数/row/跨核/vector/向量/cube/UB/L2/带宽/DataCopy/MTE/Barrier/同步/双缓冲/pipeline/流水/launch/开销/融合/dispatch/分发/tiling/切分/tile 等）。未命中即记为负证据（见 §3）。

---

## 1. CHAT_EVIDENCE（逐条）

### A. 平台/硬件型号

```
CHAT_EVIDENCE
TIME = 2026-09-13 10:05:40 / 2026-09-14 10:31~10:33
RECORD_ID = group_901064769.csv:L147439 / L147731 / L147736 / L147737
RAW_EXCERPT = "你这不是910C嘛" / "比赛评测用的芯片是910B3嘛，还是B1，B2啊" / "A3里边只有910C""A2里面也只有B3"
TOPIC = 评测/开发硬件型号
CASE = 全局
MECHANISM = 平台型号决定可用核数与特性（910B/C、A2/A3 差异）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若开发机与评测机型号不同，本地调优可能外推失败（本地≠测评机）。
WHY_IT_MAY_BE_FALSE = 全部为群友口头猜测，官方未确认；型号说法互相矛盾（910B3/B4/C 并存）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-18 18:22:47~18:23:25
RECORD_ID = group_901064769.csv:L149163 / L149164 / L149166 / L149167
RAW_EXCERPT = "开发机好像是910 c""测评机好像是910b4" / wilf："开发是b3""a2卡"
TOPIC = 开发机 vs 测评机
CASE = 全局
MECHANISM = 本地开发环境与判题环境硬件不一致 → 参数反推需针对判题机
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 直接决定 local→official 校准可信度；与 L149132"910b3环境起码够迭代到70"呼应。
WHY_IT_MAY_BE_FALSE = 二手口述，无 npu-smi 截图或官方文档佐证。
```

```
CHAT_EVIDENCE
TIME = 2026-09-14 23:31 / 2026-09-16 17:12 / 2026-09-18 18:16
RECORD_ID = group_901064769.csv:L148030 / L148423 / L149132
RAW_EXCERPT = "那如果是跟平台一样的硬件呢？910B4" / "910B过时了" / "910b3环境上，起码够迭代到70"
TOPIC = 硬件型号与可用性
CASE = 全局
MECHANISM = 评测平台为 Atlas 910B 系列
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 用于判定 kernel 可用核数/带宽上限的量级。
WHY_IT_MAY_BE_FALSE = 型号混杂，与 L149164、L148036（"只有910b3用"）相互矛盾。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 23:20:23~23:20:32
RECORD_ID = group_901064769.csv:L152203 / L152204
RAW_EXCERPT = "好像a3核多一些更加强劲""但比赛你用a2最好吧（"
TOPIC = A2/A3 核数差异
CASE = 全局
MECHANISM = A3 核心数多于 A2；比赛环境为 A2
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若评测为 A2，核数上限约 20~40 量级，决定多核切分空间。
WHY_IT_MAY_BE_FALSE = 纯猜测（"好像"），无核数实测数字。
```

### B. 核数 / 核分配 / 行归属

```
CHAT_EVIDENCE
TIME = 2026-09-18 18:21:48~18:23:04
RECORD_ID = group_901064769.csv:L149160 / L149165 / L149169
RAW_EXCERPT = "线上评测平台真是一直保持40核的吗" / "左测右测得出结果是大概率整到32核（" / "收回我的话（"
TOPIC = 评测机核数
CASE = 全局
MECHANISM = 评测核数怀疑为 32 核（说法随后被本人撤回）
EVIDENCE_CLASS = CONTRADICTED
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 核数是多核切分与"按行归属"机制的基础参数。
WHY_IT_MAY_BE_FALSE = 发言者 L149169"收回我的话"，属自我否定的推测。
```

```
CHAT_EVIDENCE
TIME = 2026-09-18 18:22:15
RECORD_ID = group_901064769.csv:L149162
RAW_EXCERPT = "建议花几次提交改变代码里的核数实测"
TOPIC = 核数实测法
CASE = 全局
MECHANISM = 通过提交不同核数代码反推评测核数
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 社区公认的核数探测手段，属"有人提出"，非有人实现。
WHY_IT_MAY_BE_FALSE = 无任何提交/结果对照，纯建议。
```

```
CHAT_EVIDENCE
TIME = 2026-09-18 18:52:52
RECORD_ID = group_901064769.csv:L149172
RAW_EXCERPT = "我感觉有一些用32核有一些用40核好像有一些用的核就比较少。"
TOPIC = 核数分布
CASE = 全局
MECHANISM = 不同提交使用不同核数
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 暗示核数可按算法显式控制，是行归属/切分的前置。
WHY_IT_MAY_BE_FALSE = "我感觉"，无数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 17:12:54 / 17:13:59 / 22:56:33
RECORD_ID = group_901064769.csv:L150105 / L150107 / L150492
RAW_EXCERPT = "ai 偷偷往里面跑了空核" / "空核最后不会答案错误吗" / "我上次整了个空核"
TOPIC = 空核（空 block）
CASE = 全局
MECHANISM = 通过在 kernel 里塞空核/空 block 影响计时
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明核数/block 数量可被外部显式改动并影响计时，是"核数可控"的旁证。
WHY_IT_MAY_BE_FALSE = 均为戏谑/转述 AI 行为，无一次带数字的实现。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:57:02~22:57:03
RECORD_ID = group_901064769.csv:L150497 / L150498
RAW_EXCERPT = "AI偷偷写了个AICAIV双核绕过"
TOPIC = AIC/AIV 双核
CASE = 全局
MECHANISM = cube(AIC)+vector(AIV) 双核协同
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中少数明确提到 AIC/AIV 双核的发言，是 cube+vector 机制的旁证。
WHY_IT_MAY_BE_FALSE = 紧跟"好像是""绕过"语境，疑似对违规手法的猜测而非本算子实现。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 21:40:45
RECORD_ID = group_901064769.csv:L152648
RAW_EXCERPT = "不要交空核和host到cpu侧😢"
TOPIC = 核/host 合规
CASE = 全局
MECHANISM = 空核占位 + host CPU 计算被禁止
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示"核使用方式"直接影响成绩有效性。
WHY_IT_MAY_BE_FALSE = 只是一句提醒，无实现细节。
```

### C. 参数探测 / 反推方法论（社区最成熟线索）

```
CHAT_EVIDENCE
TIME = 2026-09-16 15:08:39~15:08:50
RECORD_ID = group_901064769.csv:L148356 / L148357 / L148358
RAW_EXCERPT = "目前我探测了一下测评机的一些参数。""但是还没有完全探出来。""我感觉这个还是挺重要的。"
TOPIC = 评测机参数探测
CASE = 全局
MECHANISM = 通过提交耗时反推机器参数（带宽/L2/指令耗时/核数）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出者=福尔摩斯；属完整方法论的开端，被多位跟进。
WHY_IT_MAY_BE_FALSE = "还没有完全探出来"，无任何参数结果数值。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 12:17:44~12:21:32
RECORD_ID = group_901064769.csv:L148609 / L148610 / L148614 / L148615 / L148630 / L148633
RAW_EXCERPT = "先得到一些具体的机器的参数""哪一些地方做的好…优化""算子是要针对于具体的机器优化的" / "有些读写指令开销是不好算的" / "指令的耗时大多数都可以探测出来"
TOPIC = 面向具体机器的算子优化方法论
CASE = 全局
MECHANISM = 先测机器参数 → 定位好坏 → 反推指令开销
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 谁提出=福尔摩斯/wilf；明确了"读写指令开销不可精确算但可探测"的边界。
WHY_IT_MAY_BE_FALSE = 纯方法论，无一个被证实的参数值。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 13:09:49 / 13:16:17
RECORD_ID = group_901064769.csv:L148807 / L148808
RAW_EXCERPT = "可以通过这个时间把它的机器里面参数全部得到" / "比如像绝对带宽，l2，指令的时间，向量核数等"
TOPIC = 可探测参数清单
CASE = 全局
MECHANISM = 绝对带宽 / L2 / 指令耗时 / 向量核数 可通过耗时反推
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中唯一明确把 L2 与"向量核数"列为可反推参数的地方。
WHY_IT_MAY_BE_FALSE = 只有主张，没有给出任何一次反推出的 L2 大小/核数。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 13:24:16
RECORD_ID = group_901064769.csv:L148824
RAW_EXCERPT = "特别是，通过时间可以反推指令开销，从而下一步看是动结构还是动指令条数什么的"
TOPIC = 时间→指令开销→优化方向
CASE = 全局
MECHANISM = 结构 vs 指令条数 的取舍由反推开销决定
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出者=wilf；直接把硬件 profile 与 kernel 结构优化挂钩。
WHY_IT_MAY_BE_FALSE = 无案例、无前后数字。
```

```
CHAT_EVIDENCE
TIME = 2026-09-17 12:21:40
RECORD_ID = group_901064769.csv:L148636
RAW_EXCERPT = "搬运和 fp32 不同数据量时候的改造"
TOPIC = 搬运 + fp32 数据量
CASE = 全局
MECHANISM = 数据搬运量随 dtype/数据量变化 → 需改造
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = YES(fp32,提及)  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 少数直接把"搬运(DataCopy 类)"与"fp32"并列的发言。
WHY_IT_MAY_BE_FALSE = 语义模糊，疑似指"文本搬运/fp32"，非算子实现。
```

```
CHAT_EVIDENCE
TIME = 2026-09-22 10:49:38~10:51:16
RECORD_ID = group_901064769.csv:L151694
RAW_EXCERPT = "你先看它为何做出这样的评估，估计是拿harness返回的时间当正确值了，看看使用msprof看真实值"
TOPIC = harness 计时 vs msprof 真实值
CASE = 全局
MECHANISM = 评测 harness 返回值可能不等于真实 kernel 时间，需 msprof 交叉验证
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 直接关系 local↔official 计时口径可信度；提出者=wilf。
WHY_IT_MAY_BE_FALSE = 只是对某次异常的解读，未附 msprof 实测对照。
```

### D. 带宽 / 物理极限 / UB 吞吐

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:44:03
RECORD_ID = group_901064769.csv:L150372
RAW_EXCERPT = "已经超越显存带宽物理极限了"
TOPIC = 带宽物理极限
CASE = 全局
MECHANISM = 排行榜成绩超出 HBM 带宽理论上限
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 暗示存在"超物理"现象（违规/计时漏洞/复用缓存），需甄别。
WHY_IT_MAY_BE_FALSE = 未给出带宽数值与算法（"按带宽算"）依据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:17:54
RECORD_ID = group_901064769.csv:L150675
RAW_EXCERPT = "哈吉米告诉我这个case8的tbest要达到的话带宽得要有16.7TB/s"
TOPIC = case8 带宽需求
CASE = case8
MECHANISM = 达到 case8 tbest 需 16.7TB/s 带宽
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case8)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 唯一带具体带宽数字的发言，可量级对照 910B HBM 带宽。
WHY_IT_MAY_BE_FALSE = "哈吉米"=其 AI 助手，属 AI 推算，非参与者实验；16.7TB/s 远超单卡 HBM 常见量级。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:22:51
RECORD_ID = group_901064769.csv:L150697
RAW_EXCERPT = "电子自己排队在往显存里进"
TOPIC = 访存排队
CASE = 全局
MECHANISM = HBM/显存访存排队（带宽竞争）
EVIDENCE_CLASS = JOKE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映社区用显存带宽瓶颈解释上限。
WHY_IT_MAY_BE_FALSE = 玩笑语，非测量。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 21:50:46~21:51:02
RECORD_ID = group_901064769.csv:L152663 / L152665 / L152666 / L152667
RAW_EXCERPT = "分析prof后，我发现随着D的增大，UB吞吐也就越高""能不能把D给合并""然后被裁切""然后计算"
TOPIC = UB 吞吐随 D 变化
CASE = 大 D 类
MECHANISM = D 越大 UB 吞吐越高 → 尝试合并 D 再裁切
EVIDENCE_CLASS = DIRECT_RESULT（prof 观察）/ 计划部分 SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO(仅"D增大")  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中唯一明确把"prof + UB 吞吐 + D"绑定的发言；提出者=重庆邮电大学 KPBOT。
WHY_IT_MAY_BE_FALSE = 只有趋势方向，无吞吐数值；后续"合并/裁切"仅为设想，无验证结果。
```

```
CHAT_EVIDENCE
TIME = 2026-09-24 13:02:35 / 13:03:34
RECORD_ID = group_901064769.csv:L152847 / L152851
RAW_EXCERPT = "感觉是进入搬运平台期了" / "接近带宽极限了"
TOPIC = 搬运平台期 / 带宽极限
CASE = 全局
MECHANISM = 数据搬运受限，接近带宽屋顶
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L150372/L150675 共同指向"搬运/带宽"是主瓶颈之一。
WHY_IT_MAY_BE_FALSE = 主观感受（"感觉"），无带宽计算。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:44:45
RECORD_ID = group_901064769.csv:L154987
RAW_EXCERPT = "我的 gpt 说已经接近带宽极限"
TOPIC = 带宽极限（AI 转述）
CASE = 全局
MECHANISM = 已接近带宽极限
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 再次出现"AI 说接近带宽极限"，说明该说法主要来自 AI 而非实测。
WHY_IT_MAY_BE_FALSE = 转述 AI，无实验。
```

### E. cube+vector / kernel 融合 / 流水线 / 分发

```
CHAT_EVIDENCE
TIME = 2026-09-22 09:25:29~09:25:45
RECORD_ID = group_901064769.csv:L151429 / L151430
RAW_EXCERPT = "目前在尝试让5.6sol给我改进成把大D运算变成cube+vector 混合""但是效果很不好"
TOPIC = 大D → cube+vector 混合
CASE = 大 D 类
MECHANISM = 用 cube+vector 混合处理大 D
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO(case 未指明)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 一次明确的负实现证据：有人（KPBOT）真的尝试了 cube+vector 混合，结果"很不好"。
WHY_IT_MAY_BE_FALSE = 无数字、无源码、无 case；不得据此判定 cube+vector 机制整体无效，仅是该实现的单次负结果。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 21:53:08
RECORD_ID = group_901064769.csv:L152673
RAW_EXCERPT = "从我的几何直觉来看，或许D和B的形状应该通过metadata给换为正方形？"
TOPIC = 形状/几何直觉
CASE = 全局
MECHANISM = 把 (D,B) 形状通过 metadata 换成正方形以改善 UB 访问
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = YES(正方形设想)  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出者=KPBOT；后续（L152675）wilf 回"你可以直接去试试"，无结果。
WHY_IT_MAY_BE_FALSE = "几何直觉"，且消息随后被撤回（L152674）。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 11:36:58 / 13:20:40
RECORD_ID = group_901064769.csv:L156621 / L156676
RAW_EXCERPT = "我要做kernel融合" / "我的核心计算全部由 AscendC NPU kernel 完成"
TOPIC = kernel 融合
CASE = 全局
MECHANISM = kernel 融合（减少 launch/访存）
EVIDENCE_CLASS = SPECULATIVE（计划）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中唯一提到 kernel 融合的发言，但仅"要做"，无结果。
WHY_IT_MAY_BE_FALSE = 纯计划；随后主要被用于自证"未用 CPU"而非性能讨论。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 00:12:44
RECORD_ID = group_901064769.csv:L156523
RAW_EXCERPT = "明明说给我流水并行了，但实际上没有"
TOPIC = 流水并行未兑现
CASE = 全局
MECHANISM = 流水/双缓冲并行（未生效）
EVIDENCE_CLASS = NEGATIVE_RESULT
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示"AI 声称做了流水并行但实际没有"，是流水机制踩坑证据。
WHY_IT_MAY_BE_FALSE = 无"之前/之后"数字，无法判断是否真无并行。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 20:38:57
RECORD_ID = group_901064769.csv:L156916
RAW_EXCERPT = "你可以看流水线具体细节耗时倒是 但也仅供参考"
TOPIC = 流水线细节耗时
CASE = 全局
MECHANISM = 可查看流水线各段耗时，但仅供参照
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 说明存在"流水线细节耗时"可观测量（可能指 prof 的 pipe 时间）。
WHY_IT_MAY_BE_FALSE = "仅供参考"，无数据。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:00:41 / 04:06:12 / 11:35:50
RECORD_ID = group_901064769.csv:L156563 / L156567 / L156612
RAW_EXCERPT = "真的没有用cpu吗，怎么会三四个形状不一样的都可以同时复用呢" / "我觉得他们的分发方式可能更加的……数学一点" / "因为他们同簇"
TOPIC = 分发方式 / 4&7 同簇
CASE = case4, case7
MECHANISM = 统一的分发/处理方式同时命中 4 与 7（同簇）
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case4/7)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中把"dispatch 分发"与 case 绑定最明确的一处；提出者=wilf / cqu ai / cquer崛起吧。
WHY_IT_MAY_BE_FALSE = "我觉得/可能"，无源码；"同簇"是定性说法。
```

### F. 空核 / host CPU / 违规机制（影响成绩有效性）

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:45:27 / 22:49:25
RECORD_ID = group_901064769.csv:L150397 / L150435
RAW_EXCERPT = "本赛事要求核心计算在昇腾 NPU 上通过 AscendC 算子实现完成，任何将计算转移至 Host CPU、通过空 kernel 占位绕过 NPU 计算要求的行为，均构成违规"
TOPIC = 官方规则引用
CASE = 全局
MECHANISM = 禁止 host CPU 计算 + 空 kernel 占位
EVIDENCE_CLASS = SECOND_HAND（转述规则原文）
CONFIDENCE = HIGH（对"规则存在"而言）
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 官方红线；任何"host 加速"都应视为无效/违规，而非可复用机制。
WHY_IT_MAY_BE_FALSE = 转述而非官方链接原文；文字可能有出入。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:58:09
RECORD_ID = group_901064769.csv:L150509
RAW_EXCERPT = "67.50分是违规所得。TP1-7和TP8的NPU时间只测量了空marker kernel，实际计算在Host CPU上完成…N*D≤64M元素，被路由到Host CPU计算。"
TOPIC = host CPU 路由 + 空 marker kernel
CASE = TP1/2/3/8（小规模）
MECHANISM = 小规模（N*D≤64M）被路由到 Host CPU，NPU 只测空 marker kernel
EVIDENCE_CLASS = SPECULATIVE（对机制）/ 携带具体阈值
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(TP1/2/3/8)  HAS_BEFORE_AFTER = NO(仅 67.50 分)
WHY_IT_MATTERS = 唯一给出具体路由阈值（N*D≤64M）的发言；提出者=cqu王亚东，随后 L150519"之后不会了"。
WHY_IT_MAY_BE_FALSE = 属对别人成绩的指控，可能误读；发言者自己也表示不再做，无法复现验证。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 16:33:11~16:35:48
RECORD_ID = group_901064769.csv:L150018 / L150022 / L150025 / L150034
RAW_EXCERPT = "在文件末尾又启动了一次kernel就会出现这种情况" / "在cpu运算的？" / "有种方法能在不通过所有测试点的情况下，让通过的点特别快" / "难道是规则里说的空kernel绕过占位？"
TOPIC = 计时漏洞 / 空 kernel
CASE = 全局
MECHANISM = 文件末尾再次启动 kernel / 只让部分点超快
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中唯一提到"kernel 启动次数影响计时"的地方，疑与 launch/固定开销相关。
WHY_IT_MAY_BE_FALSE = 拼凑式猜测（"据说""难道是"），非实验结论。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:48:39 / 09-28 12:11:56
RECORD_ID = group_901064769.csv:L154998 / L154816
RAW_EXCERPT = "其实大家都是绕过核用 cpu 跑的" / "我怀疑他绕过空核"
TOPIC = 绕过空核/host
CASE = 全局
MECHANISM = 绕过空核、用 CPU 跑
EVIDENCE_CLASS = JOKE（调侃）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 反映社区对"空核/host"手法的普遍怀疑。
WHY_IT_MAY_BE_FALSE = 明显玩笑语境，不构成事实。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:49:10~19:52:26
RECORD_ID = group_901064769.csv:L155001 ~ L155006
RAW_EXCERPT = "其实我们交的都是空的里面塞的死代码，就为了结束的时候阴你们一把"（6 人接龙复读）
TOPIC = 空核+死代码
CASE = 全局
MECHANISM = 空 kernel 内塞死代码以迷惑检测
EVIDENCE_CLASS = JOKE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L155793 的"空核 vs 死代码数值不同"讨论相关联。
WHY_IT_MAY_BE_FALSE = 集体复读玩笑。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:18:45
RECORD_ID = group_901064769.csv:L155793
RAW_EXCERPT = "因为完全空核跟里面塞了死代码的值都不一样我不明白"
TOPIC = 空核 vs 死代码的计时差异
CASE = 全局
MECHANISM = 空核与死代码产生不同测量值
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提示"核内容不同→计时不同"，是核机制可观测的间接证据。
WHY_IT_MAY_BE_FALSE = 语义模糊，可能指别人的提交，无对照。
```

### G. 测量噪声 / tbest

```
CHAT_EVIDENCE
TIME = 2026-09-13 21:41:55
RECORD_ID = group_901064769.csv:L147621
RAW_EXCERPT = "tbest 没修复吗👀，误差好大现在，同一份源码提交 20 次，分数区间差距能有三四分"
TOPIC = 同源码分数方差
CASE = 全局
MECHANISM = 同源码多次提交 → 3~4 分区间（噪声带）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = HIGH
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = YES(同源码 20 次,3~4 分)
WHY_IT_MATTERS = 定义 noise floor；任何 <此量级 的"提升"不可信。
WHY_IT_MAY_BE_FALSE = 单个用户口述，未附截图；"分数区间"口径未明。
```

```
CHAT_EVIDENCE
TIME = 2026-09-15 16:08:43~16:09:10
RECORD_ID = group_901064769.csv:L148178 / L148179
RAW_EXCERPT = "没啊，我不知道为什么我同一个代码，布局彩票波动这么大" / "我也觉得波动"
TOPIC = 同代码波动
CASE = 全局
MECHANISM = 布局/机器状态导致同代码波动
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L147621 相互印证（多人独立观察到波动）。
WHY_IT_MAY_BE_FALSE = 同代码是否完全一致未验证（AI 可能改动）。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:02:40~23:03:50
RECORD_ID = group_901064769.csv:L150548 / L150550 / L150564
RAW_EXCERPT = "我有一次波动有1us" / "波动了两三分" / "精度舍入啥的有波动都很正常"
TOPIC = 波动量级与归因
CASE = 全局
MECHANISM = 波动 1us / 2~3 分；被归因于精度舍入
EVIDENCE_CLASS = SPECULATIVE（归因）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = YES(1us / 2~3 分)
WHY_IT_MATTERS = 给出具体波动量级，可作噪声阈值参考。
WHY_IT_MAY_BE_FALSE = "精度舍入导致波动"是猜测，未排除机器负载/计时噪声。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:05:33 / 23:06:21
RECORD_ID = group_901064769.csv:L150587 / L150596
RAW_EXCERPT = "我感觉是因为每次跑的时候机器负载不同，所以每小时用自己最佳跑一次作为baseline，应该就能减少波动的影响" / "一般晚上三四点就很稳定"
TOPIC = 机器负载导致波动
CASE = 全局
MECHANISM = 机器负载 → 计时波动；低峰（凌晨3~4点）更稳定
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出者=西南石油大学敖厂长；给出"用每小时自最佳作 baseline"的校准方法。
WHY_IT_MAY_BE_FALSE = "感觉"，无负载监控数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:08:09~23:08:25
RECORD_ID = group_901064769.csv:L150613 / L150616
RAW_EXCERPT = "每次tbest改变，就改了成绩" / "所以同一份代码，会有不同的成绩"
TOPIC = tbest 机制
CASE = 全局
MECHANISM = 成绩依赖历史 tbest，tbest 变化→成绩变化
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 解释"同源码不同分"的机制来源之一（相对而非绝对计分）。
WHY_IT_MAY_BE_FALSE = 群友推测，未引用官方文档。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 23:09:25 / 23:09:59
RECORD_ID = group_901064769.csv:L150629 / L150631
RAW_EXCERPT = "的确是噪声带" / "我的意思就是把tbest拉成噪声带最高值就直接拉低后面高分概率"
TOPIC = 噪声带
CASE = 全局
MECHANISM = 提交把 tbest 抬到噪声带上沿 → 后续得分概率下降
EVIDENCE_CLASS = SPECULATIVE
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 提出者=wilf；是"噪声带"这一机制概念的原始出处。
WHY_IT_MAY_BE_FALSE = 逻辑推演，无数据。
```

```
CHAT_EVIDENCE
TIME = 2026-09-25 00:41:04~00:42:36
RECORD_ID = group_901064769.csv:L153346 / L153348 / L153350 / L153351
RAW_EXCERPT = "为啥前面几个case波动能到几us" / wilf："我都是0.5左右" / 永畜塔菲："我到1。几" / "可能有竞态"
TOPIC = 前段 case 波动更大
CASE = case1~3（"前面几个case"）
MECHANISM = 小 case 波动更大；疑有竞态/他人共用机器
EVIDENCE_CLASS = SPECULATIVE（竞态归因）/ 数值为 REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case1~3 推测)  HAS_BEFORE_AFTER = YES(0.5us vs 1.x us)
WHY_IT_MATTERS = 语料中唯一提到"竞态(race)"的地方；并指出 tiny case 相对波动更大。
WHY_IT_MAY_BE_FALSE = "可能有竞态"为猜测；两人对同一现象量级说法不一。
```

```
CHAT_EVIDENCE
TIME = 2026-10-02 04:11:52~04:12:49
RECORD_ID = group_901064769.csv:L156579 / L156580 / L156582
RAW_EXCERPT = "会有噪声波动" / "每次提交都不一样" / "这个波动太搞人了"
TOPIC = 噪声（末期复现）
CASE = case5
MECHANISM = 每次提交都不同（噪声）
EVIDENCE_CLASS = REPEATED_SIGNAL
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case5)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与早期（9-13）结论一致：噪声贯穿整个赛期。
WHY_IT_MAY_BE_FALSE = 无具体幅度。
```

```
CHAT_EVIDENCE
TIME = 2026-09-30 18:16:44
RECORD_ID = group_901064769.csv:L155786
RAW_EXCERPT = "两次提交0波动"
TOPIC = 反向证据：零波动
CASE = 全局
MECHANISM = 并非总是波动
EVIDENCE_CLASS = CONTRADICTED（对"必然大波动"的反例）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = YES(2 次 0 波动)
WHY_IT_MATTERS = 说明噪声并非对每次提交都显著，避免把噪声当唯一解释。
WHY_IT_MAY_BE_FALSE = 单次观察，样本极小。
```

### H. 得分/标杆值（仅供量级）

```
CHAT_EVIDENCE
TIME = 2026-09-17 01:02:06 / 12:18:42
RECORD_ID = group_901064769.csv:L148524 / L148619
RAW_EXCERPT = "奶蛙14us的测点7我真怕了" / "比如奶娃的14us的测点7"
TOPIC = case7 标杆值
CASE = case7
MECHANISM = case7 约 14us
EVIDENCE_CLASS = SECOND_HAND（转述他人值）
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case7)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case7（提升空间大）的量级参考。
WHY_IT_MAY_BE_FALSE = 转述"奶蛙"，无源码/shape，非本人实验。
```

```
CHAT_EVIDENCE
TIME = 2026-09-23 19:03:22
RECORD_ID = group_901064769.csv:L152522
RAW_EXCERPT = "GLM穷举了三天，终于把测试点3进步了2微秒"
TOPIC = case3 优化幅度
CASE = case3
MECHANISM = 穷举优化提升 2us
EVIDENCE_CLASS = DIRECT_RESULT（有前后幅度）
CONFIDENCE = LOW-MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case3)  HAS_BEFORE_AFTER = YES(进步2us)
WHY_IT_MATTERS = 少数带"优化前后幅度"的发言；提出者=cqu王亚东。
WHY_IT_MAY_BE_FALSE = 2us 与噪声量级同阶（case5 波动可达3us），可能落在噪声内。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 12:01:46 / 2026-10-02 04:11:02~04:11:23
RECORD_ID = group_901064769.csv:L154798 / L156576 / L156577
RAW_EXCERPT = "我昨天刚突破了case5，到了6微秒多" / "最高我才5.6" / "之前一直在6us"
TOPIC = case5 标杆值
CASE = case5
MECHANISM = case5 时延 5.6~6.几us
EVIDENCE_CLASS = SECOND_HAND / DIRECT_RESULT 混合
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case5)  HAS_BEFORE_AFTER = YES(6us→5.6us)
WHY_IT_MATTERS = case5 的量级锚点（与项目 bestTimeUs 5.2us 量级相符）。
WHY_IT_MAY_BE_FALSE = 无源码/shape；波动可达3us（L151172），单点差异可能为噪声。
```

```
CHAT_EVIDENCE
TIME = 2026-09-28 19:45:09
RECORD_ID = group_901064769.csv:L154989
RAW_EXCERPT = "我的glm说case5 9us已经接近极限了"
TOPIC = case5 极限（AI 转述）
CASE = case5
MECHANISM = case5 9us 接近极限
EVIDENCE_CLASS = AI_SUMMARY
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case5)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 与 L154798（6us 多）矛盾，说明 AI 极限判断不可信。
WHY_IT_MAY_BE_FALSE = 转述 AI；与同群实测明显冲突。
```

```
CHAT_EVIDENCE
TIME = 2026-09-21 22:36:14
RECORD_ID = group_901064769.csv:L150490（case8 1.34 微妙）
RAW_EXCERPT = "case8，1.34微妙"（电子科技大学张三）
TOPIC = case8 标杆值
CASE = case8
MECHANISM = case8 ≈ 1.34us
EVIDENCE_CLASS = SECOND_HAND
CONFIDENCE = LOW
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case8)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = case8 量级锚点；与 L150675 的 16.7TB/s 说法搭配出现。
WHY_IT_MAY_BE_FALSE = 无源码/shape；1.34us 与 16.7TB/s 组合疑超物理。
```

```
CHAT_EVIDENCE
TIME = 2026-09-12 22:57:25
RECORD_ID = group_901064769.csv:L147327
RAW_EXCERPT = "哪来的挂B，把case14刷到157微妙了，死活找不到"
TOPIC = case14 异常标杆值
CASE = case14
MECHANISM = 极端低时延榜单值（可能非真实 kernel 时间）
EVIDENCE_CLASS = INVALID_BENCHMARK
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = UNKNOWN  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case14)  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 若 157us 为真，说明 case14 存在数量级机制的加速；但无源码/shape/复现。
WHY_IT_MAY_BE_FALSE = 可能是空kernel/计时漏洞/刷分/单点刷榜，非有效 benchmark；群内无人复现。
```

```
CHAT_EVIDENCE
TIME = 2026-09-29 22:35:50
RECORD_ID = group_901064769.csv:L155335 区（提分清单同源）
RAW_EXCERPT = "P14 平台最优 3750.12 → 3665.94（-2.2%）…那一发总分只有 68.22…专刷单点纪录打法"
TOPIC = case14 单点刷榜策略
CASE = case14
MECHANISM = 只优化大点、反复提交磨单点 time-best
EVIDENCE_CLASS = SECOND_HAND（转述队伍 cd 的行为）
CONFIDENCE = MEDIUM
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = YES(case14)  HAS_BEFORE_AFTER = YES(3750.12→3665.94)
WHY_IT_MATTERS = 说明榜单成绩含策略性刷分，不等于通用 kernel 能力。
WHY_IT_MAY_BE_FALSE = 转述他人成绩，无源码。
```

### I. AI 编造 / 玩笑（必须排除）

```
CHAT_EVIDENCE
TIME = 2026-09-30 20:41:49
RECORD_ID = group_901064769.csv:L155885 / L155887 / L155889 / L155893 / L155895
RAW_EXCERPT = "【Kernel】…'流水线''全向量归约''零冲突'…Atlas 910B 的40个核心…消灭跨Pipe气泡、对齐256B步进…"
TOPIC = AI 小说体（含大量机制词）
CASE = 全局
MECHANISM = 流水线 / 全向量归约 / 跨Pipe气泡 / 256B对齐 / 即兴自适应 Kernel
EVIDENCE_CLASS = AI_SUMMARY（AI 生成叙事，非实验）
CONFIDENCE = HIGH（判定其为编造）
HAS_SOURCE_CODE = NO  HAS_SHAPE = NO  HAS_DTYPE = NO  HAS_CASE_BINDING = NO  HAS_BEFORE_AFTER = NO
WHY_IT_MATTERS = 语料中机制词密度最高，但是编造故事；二次报告已明确标注剔除，勿当证据。
WHY_IT_MAY_BE_FALSE = 无任何参与者实验支撑；"40个核心""全向量归约""零冲突"均为修辞。
```

---

## 2. MECHANISM_CLUE（按关键词聚合）

```
MECHANISM_CLUE
CLUE_ID = M01_CORE_COUNT
MECHANISM = 评测机核数（32/40）与核数可控性
AFFECTED_CASES = 全局（大 D 切分/行归属相关）
SUPPORTING_RECORDS = L149160, L149165, L149172, L152203, L152204
CONTRADICTING_RECORDS = L149169（"收回我的话"）
NEGATIVE_EVIDENCE = L149162（仅"建议实测"，无结果）
CONFIDENCE = LOW
KNOWN_FACTS = 有人怀疑评测为 32 核；有人感觉不同提交用 32/40/更少核；A3>A2 核数。
UNKNOWN_FACTS = 评测机真实核数；核数是否可由代码显式固定。
MINIMUM_FACT_NEEDED_NEXT = 用不同 blockdim 提交并对比时延，测出评测核数。
```

```
MECHANISM_CLUE
CLUE_ID = M02_ROW_OWNERSHIP_BLOCKDIM
MECHANISM = 行归属 / block 分配（每行/每块分给哪个核）
AFFECTED_CASES = 全 15 case（尤其多行 Small/Very large）
SUPPORTING_RECORDS = L150105, L150107, L150492, L150497, L150509
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 语料中没有任何"row ownership/行归属/跨核"的正面实现讨论
CONFIDENCE = LOW
KNOWN_FACTS = 存在"空核/空 block"并被用于影响计时；提到过 AIC/AIV 双核。
UNKNOWN_FACTS = 是否有显式行→核映射策略；行归属是否是主瓶颈。
MINIMUM_FACT_NEEDED_NEXT = 一份按行分配核并对比时延的实验（本机可做）。
```

```
MECHANISM_CLUE
CLUE_ID = M03_VECTOR_CORE_COUNT_PROBE
MECHANISM = 向量核数可被耗时反推
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L148807, L148808
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无后续任何反推出的具体核数
CONFIDENCE = LOW-MEDIUM
KNOWN_FACTS = 福尔摩斯提出"绝对带宽/L2/指令时间/向量核数"可由时间反推。
UNKNOWN_FACTS = 反推出的具体向量核数；反推方法。
MINIMUM_FACT_NEEDED_NEXT = 一个标定实验：固定算法仅变数据量，拟合出核数与带宽。
```

```
MECHANISM_CLUE
CLUE_ID = M04_CUBE_VECTOR_FUSION_LARGE_D
MECHANISM = 大 D 用 cube+vector 混合
AFFECTED_CASES = 大 D 类（case13/14/15 量级）
SUPPORTING_RECORDS = L151429, L151430
CONTRADICTING_RECORDS = 无（仅此一次）
NEGATIVE_EVIDENCE = L151430"但是效果很不好"（一次负实现）
CONFIDENCE = LOW
KNOWN_FACTS = KPBOT 实际尝试过 cube+vector 混合处理大 D，自述效果很差。
UNKNOWN_FACTS = 是否用了正确 tiling/搬运；是否有任何数字。
MINIMUM_FACT_NEEDED_NEXT = 有前后时延的 cube+vector 实验（不允许据单次负结果否定机制）。
```

```
MECHANISM_CLUE
CLUE_ID = M05_UB_THROUGHPUT_VS_D
MECHANISM = UB 吞吐随 D 增大而提高
AFFECTED_CASES = 大 D 类
SUPPORTING_RECORDS = L152663
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 后续"合并 D 再裁切"仅为设想，无结果
CONFIDENCE = MEDIUM
KNOWN_FACTS = KPBOT 分析 prof 观察到"D 越大 UB 吞吐越高"，并设想合并/裁切 D。
UNKNOWN_FACTS = 吞吐数值；临界 D；合并 D 是否可行。
MINIMUM_FACT_NEEDED_NEXT = 复现 prof 并给出 UB throughput vs D 曲线。
```

```
MECHANISM_CLUE
CLUE_ID = M06_BANDWIDTH_L2_HBM_LIMIT
MECHANISM = 绝对带宽 / L2 / HBM 物理极限
AFFECTED_CASES = case8 及大/中 case
SUPPORTING_RECORDS = L148808(L2/绝对带宽), L150372, L150675, L152851, L152567
CONTRADICTING_RECORDS = L154987（AI 说法）
NEGATIVE_EVIDENCE = 从未给出一次真实带宽实测值
CONFIDENCE = LOW-MEDIUM
KNOWN_FACTS = 多人认为部分 case 已"到/超带宽极限"；唯一数字 16.7TB/s 来自 AI。
UNKNOWN_FACTS = 评测机真实 HBM 带宽；各 case 的 arithmetic intensity。
MINIMUM_FACT_NEEDED_NEXT = 用纯拷贝 kernel 测 HBM 有效带宽（prof）。
```

```
MECHANISM_CLUE
CLUE_ID = M07_DATACOPY_MTE_NAMES_ABSENT
MECHANISM = 数据搬运（DataCopy / MTE2 / MTE3）显式命名
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L148636（"搬运", fp32）, L152847（"搬运平台期"）, L156617（"搬运问题还是计算问题"）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全文无 DataCopy/MTE2/MTE3/DataCopyPad/尾块/mask/对齐/32B/256B 的正经讨论；仅 L155895(AI 小说) 出现 256B
CONFIDENCE = MEDIUM（对"缺席"本身）
KNOWN_FACTS = 社区只用口语"搬运"泛指数据移动，未触及具体指令。
UNKNOWN_FACTS = 是否有人私聊层面在用 MTE 级优化。
MINIMUM_FACT_NEEDED_NEXT = 无需外部；本机 route 可直接验证 MTE2/V/MTE3 流水收益。
```

```
MECHANISM_CLUE
CLUE_ID = M08_DOUBLE_BUFFER_PIPELINE
MECHANISM = 双缓冲 / 流水线 / 跨 Pipe 气泡
AFFECTED_CASES = 全部（尤其长行）
SUPPORTING_RECORDS = L156523（流水并行没兑现）, L156916（流水线细节耗时）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全文无 双缓冲/double buffer/乒乓/pingpong 命中；L155885 的"跨Pipe气泡"属 AI 小说
CONFIDENCE = LOW
KNOWN_FACTS = 有人声称做了流水并行但"实际没有"；可查"流水线细节耗时"。
UNKNOWN_FACTS = 是否真做了双缓冲；pipe 各段耗时数值。
MINIMUM_FACT_NEEDED_NEXT = prof 出 MTE2/V/MTE3 各段占比，判断是否有气泡。
```

```
MECHANISM_CLUE
CLUE_ID = M09_BARRIER_SYNC_RACE
MECHANISM = 同步 / Barrier / 竞态
AFFECTED_CASES = case1~3（小 case 波动更大）
SUPPORTING_RECORDS = L153351（"可能有竞态"）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全文无 Barrier/SetFlag/WaitFlag/SyncAll/同步 命中；仅 1 次"竞态"
CONFIDENCE = LOW
KNOWN_FACTS = wilf 猜测前段 case 大波动可能来自竞态。
UNKNOWN_FACTS = 是否真有跨核竞态；是否与机器共用有关。
MINIMUM_FACT_NEEDED_NEXT = 同源码多次提交 + 关闭并发提交，看波动是否消失。
```

```
MECHANISM_CLUE
CLUE_ID = M10_LAUNCH_FIXED_OVERHEAD
MECHANISM = kernel launch / 固定开销
AFFECTED_CASES = Tiny（case1/2/3/5）
SUPPORTING_RECORDS = L150018（文件末尾再次启动 kernel）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全文无 launch/启动开销/固定开销/overhead/fixed overhead 的技术讨论；"启动"绝大多数是抢机器
CONFIDENCE = LOW
KNOWN_FACTS = 仅有一条疑似"多启动一次 kernel 改变结果"的说法。
UNKNOWN_FACTS = 评测是否单 kernel 计时；固定开销量级。
MINIMUM_FACT_NEEDED_NEXT = 空 kernel 的时延（测固定开销）。
```

```
MECHANISM_CLUE
CLUE_ID = M11_KERNEL_FUSION
MECHANISM = kernel 融合
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L156621（"我要做kernel融合"）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无结果、无实现
CONFIDENCE = LOW
KNOWN_FACTS = 有一人表示要做 kernel 融合（未给 case/结果）。
UNKNOWN_FACTS = 融合对象（是否 RmsNorm+Add+Bias）；收益。
MINIMUM_FACT_NEEDED_NEXT = 无需外部；本机可直接验证融合收益。
```

```
MECHANISM_CLUE
CLUE_ID = M12_TILING_SLICING
MECHANISM = tiling / 切分 / tile
AFFECTED_CASES = 全部
SUPPORTING_RECORDS = 无（唯一边界：L152665"把D给合并…裁切"）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全文无 tiling/切分/tile/分块/chunk/split 命中（原始语料层面）
CONFIDENCE = MEDIUM（对"缺席"本身）
KNOWN_FACTS = 社区未在群聊层面讨论 tiling；只有 KPBOT 一处"合并 D 再裁切"设想。
UNKNOWN_FACTS = 各 case 最优 tile/D 合并策略。
MINIMUM_FACT_NEEDED_NEXT = 无需外部；本机 route 已覆盖 tiling（属项目内部事实，非群聊）。
```

```
MECHANISM_CLUE
CLUE_ID = M13_DISPATCH_DISTRIBUTION
MECHANISM = 分发 / dispatch（统一处理多形状）
AFFECTED_CASES = case4, case7（"同簇"）
SUPPORTING_RECORDS = L156567, L156569, L156585, L156612
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 全为"我觉得/可能/他说"，无源码
CONFIDENCE = MEDIUM
KNOWN_FACTS = 有"4 和 7 一块攻破""破一个对另一个有帮助""同簇"的说法，指向统一 dispatch。
UNKNOWN_FACTS = dispatch 具体策略；是否真为同一处理路径。
MINIMUM_FACT_NEEDED_NEXT = case4、case7 的 shape/dtype；同一代码两 case 的表现。
```

```
MECHANISM_CLUE
CLUE_ID = M14_HOST_CPU_OFFLOAD_EMPTY_KERNEL
MECHANISM = host CPU 计算 / 空 kernel 占位（违规）
AFFECTED_CASES = TP1/2/3/8（N*D≤64M）
SUPPORTING_RECORDS = L150397, L150435, L150497, L150509, L150034, L152648, L154816, L154998
CONTRADICTING_RECORDS = L150519（当事人"之后不会了"）；L155802/L156675（他人自证未用 CPU）
NEGATIVE_EVIDENCE = 属规则红线，非可用加速机制
CONFIDENCE = HIGH（对"存在且被禁"）；MEDIUM（对"阈值 64M"）
KNOWN_FACTS = 官方规则禁止 host CPU 与空 kernel；有人称小规模 N*D≤64M 被路由到 CPU。
UNKNOWN_FACTS = 评测是否按 N*D 阈值路由；是否有检测机制。
MINIMUM_FACT_NEEDED_NEXT = 官方规则原文/检测口径（需项目侧核实，本 Child 不判定）。
```

```
MECHANISM_CLUE
CLUE_ID = M15_PLATFORM_MODEL_DIVERGENCE
MECHANISM = 开发机 vs 测评机型号不一致
AFFECTED_CASES = 全局（local↔official 校准）
SUPPORTING_RECORDS = L147439, L147731, L147736, L147737, L148030, L148132, L149163, L149164, L149166, L149453, L150186
CONTRADICTING_RECORDS = 型号说法互相冲突（B3/B4/C）
NEGATIVE_EVIDENCE = 无一张 npu-smi/官方文档
CONFIDENCE = MEDIUM（对"存在差异"）；LOW（对"具体型号"）
KNOWN_FACTS = 多方感觉开发机与测评机不同（910C vs 910B4 / a2卡）。
UNKNOWN_FACTS = 精确 SoC 型号与核数；对性能的可迁移性。
MINIMUM_FACT_NEEDED_NEXT = 官方硬件说明或 npu-smi 输出（项目侧核实）。
```

```
MECHANISM_CLUE
CLUE_ID = M16_MEASUREMENT_NOISE_TBEST
MECHANISM = 测量噪声 / tbest 相对计分 → 判定门槛
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L147621, L148178, L148179, L150548, L150550, L150587, L150616, L150629, L150631, L151172, L153346, L156579, L155786
CONTRADICTING_RECORDS = L155786（两次0波动）
NEGATIVE_EVIDENCE = 无官方噪声口径
CONFIDENCE = HIGH
KNOWN_FACTS = 同源码多处报告 2~4 分/1~3us 波动；被归因机器负载与 tbest；凌晨更稳；偶有 0 波动。
UNKNOWN_FACTS = 精确 noise floor；是否与并发提交有关。
MINIMUM_FACT_NEEDED_NEXT = 同源码重复提交统计分布（需 online，本 Child 不做）。
```

```
MECHANISM_CLUE
CLUE_ID = M17_FP32_PRECISION
MECHANISM = fp32 / 精度舍入
AFFECTED_CASES = 全局
SUPPORTING_RECORDS = L148636（fp32+搬运）, L150564（精度舍入波动）
CONTRADICTING_RECORDS = 无
NEGATIVE_EVIDENCE = 无精度实验；无 dtype/shape 细节
CONFIDENCE = LOW
KNOWN_FACTS = 有"搬运与 fp32 不同数据量的改造""精度舍入有波动"两条。
UNKNOWN_FACTS = 输入/中间 dtype；舍入对结果的影响。
MINIMUM_FACT_NEEDED_NEXT = 题目 spec 的 dtype（项目侧已知，非群聊）。
```

```
MECHANISM_CLUE
CLUE_ID = M18_HARDWARE_CEILING_CLAIM
MECHANISM = "硬件上限"收敛判断
AFFECTED_CASES = 中后段（case8~15）
SUPPORTING_RECORDS = L149878, L151750, L151757, L151761, L152535, L152567, L152851
CONTRADICTING_RECORDS = L151183（"所有测试点应有理论最优通用方案"）
NEGATIVE_EVIDENCE = 无一次带宽/roofline 计算
CONFIDENCE = LOW
KNOWN_FACTS = 多位认为部分 case 已到硬件/带宽上限；也有人认为存在通用最优方案。
UNKNOWN_FACTS = 各 case 的 roofline 位置。
MINIMUM_FACT_NEEDED_NEXT = 各 case 的算术强度与 roofline 分析。
```

---

## 3. 关键词命中汇总（含负证据）

| 关键词 | 命中 | 说明 |
|---|---|---|
| 核数/核/多核/单核/空核 | 有 | 核数分歧、空核讨论多；无"核数↔时延"定量 |
| row/行归属/cross-core/跨核 | 无 | 全为噪声命中（"Browns"含 row）或 base64；无正经讨论 |
| vector/向量 | 1（L148808） | 仅"向量核数"一处；L155885 为 AI 小说 |
| cube/立方/cube+vector | 2（L151429/151430, L150497） | 一次负实现 + 一次"双核绕过"猜测 |
| UB/Unified Buffer | 1（L152663）+ 噪声 | 仅 KPBOT prof 一处；其余为 base64/系统 |
| L2 | 1（L148808） | 仅参数清单一处 |
| 带宽/bandwidth/TB·s | 有（L150372, L150675 等） | 数字仅 16.7TB/s，来自 AI |
| DataCopy/MTE2/MTE3 | 无 | 全文无命中；仅口语"搬运" |
| Barrier/同步/SyncAll | 无 | 全文无命中；仅 1 次"竞态"(L153351) |
| 双缓冲/double buffer/乒乓 | 无 | 全文无命中 |
| pipeline/流水 | 3（L156523, L156916, L155885） | 1 负结果 + 1 "仅供参考" + 1 AI 小说 |
| launch/启动开销/固定开销/overhead | 无（技术层面） | "启动"绝大多数指抢机器；仅 L150018 沾边 |
| 融合/fusion | 1（L156621） | 仅"要做"，无结果 |
| dispatch/分发 | 1（L156567） | "分发方式更数学"+"同簇" |
| tiling/切分/tile/分块 | 无 | 全文无命中；"合并 D 再裁切"沾边 |
| 反推/探测/指令耗时 | 有（L148356, L148633, L148807, L148824） | 方法论最完整的机制线索 |
| host CPU/空 kernel | 有（多条） | 规则红线 + 多次违规指控/玩笑 |

---

## 4. 结论区块（强制）

```
HIGH_CONFIDENCE_SIGNALS
1. 测量噪声/tbest 使"同源码多提交"产生 2~4 分、1~3us 波动（L147621、L150548、L151172、L153346、L156579 等多源一致）。
2. 官方规则明确禁止 host CPU 计算与空 kernel 占位（L150397、L150435）。
3. 社区最成熟的方法论是"耗时反推机器参数（带宽/L2/指令耗时/向量核数）→ 定位瓶颈→针对性优化"（L148356~L148824，福尔摩斯/wilf）。
4. 开发机与测评机硬件不一致被多方独立提及（L148030、L149163~L149167、L150186）。
```

```
MEDIUM_CONFIDENCE_SIGNALS
1. UB 吞吐随 D 增大而提高（KPBOT，L152663，有 prof 但无数字）。
2. 有人给出 host CPU 路由阈值 N*D≤64M（L150509，单一发言+自我否定）。
3. case4 与 case7 "同簇/一起攻破"（L156567~L156612，定性、无源码）。
4. 评测核数疑似 32（L149165，但本人撤回）。
5. 大 D 用 cube+vector 混合是一次负实现（L151429/151430）。
```

```
LOW_CONFIDENCE_SIGNALS
1. 16.7TB/s（L150675，AI 推算）。
2. "硬件上限/带宽极限"判断（多条，无计算）。
3. A3 核多、比赛用 A2（L152203，猜测）。
4. case5 "9us 接近极限"（L154989，AI 说，与实测 6us 矛盾）。
5. "kernel 融合要/做了"（L156621，无结果）。
6. "可能有竞态"（L153351，单一猜测）。
```

```
NEGATIVE_EVIDENCE
1. 全文无 DataCopy / MTE2 / MTE3 / DataCopyPad 命中。
2. 全文无 Barrier / SetFlag / WaitFlag / SyncAll / 同步（技术义）命中。
3. 全文无双缓冲 / double buffer / 乒乓 / pingpong 命中。
4. 全文无 tiling / 切分 / tile / 分块 / chunk 命中。
5. 全文无"launch/固定开销/fixed overhead/overhead"的技术层面讨论。
6. 无任何"反推出的带宽/L2/核数具体数值"；无任何带源码的硬件优化实现。
7. 无一处"kernel 优化前/后时延"的完整对照（唯一近似为 case3 进步 2us，量级同噪声）。
```

```
CONTRADICTIONS
1. 评测/开发机型号：910B3 / 910B4 / 910C / a2卡 多处互斥（L147731/L148030/L149163~149167）。
2. 核数：40 核 vs 32 核 vs "更少核"（L149160/L149172/L149165，含自我撤回）。
3. case5 极限：6us 多（L154798）vs "9us 接近极限"（L154989）。
4. 噪声：普遍大波动 vs "两次提交0波动"（L155786）。
5. "所有 case 有理论最优通用方案"（L151183）vs 大量"已到硬件上限"（L151750 等）。
6. cube+vector：机制上被怀疑（L150497 AIC/AIV）但唯一实现"效果很不好"（L151430）——注意这是实现负结果，不是机制否定。
```

```
UNKNOWN_BUT_IMPORTANT
1. 评测机真实 SoC 型号与核数（决定切分上限）。
2. 评测真实 HBM 有效带宽与 L2 容量（决定 case8/14 是否已到 roofline）。
3. 评测是否单 kernel 计时、固定开销量级（决定 Tiny case 优化空间）。
4. case14 157us 的真实性（是否有效 benchmark / 是否违规所得）。
5. 各 case 的真实 shape/dtype（群聊几无 shape 细节）。
6. "64M 路由阈值"与官方检测机制。
7. 4/7"同簇"的具体依据。
```

```
RAW_FILES_READ =
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv （全文关键词覆盖；行号引用基于去1个NUL的镜像 san_csv.txt，行号与原文件一致）
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769_text.txt （对照）
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/提分技术讨论_原文.csv （70 条筛选结果，交叉核对）
（二级材料，仅用于遗漏/上下文核对，未作为证据）
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/报告/总报告.md
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/报告/提分技术讨论_清单.md
- /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/报告/CANN赛题知识库.md
```

```
UNREAD_RANGES_IF_ANY =
- 已用 grep 覆盖全文所有物理行（含 NUL 之后的 L152900+）。
- 但未逐行顺序阅读全 158236 物理行：仅对关键词命中块及其上下文（约 L148350~L157060 的重点区段）做人工精读；L1~L147300 段以 grep 命中为准，主要是报名接龙/系统消息/闲聊，未逐条人工复核。
- 未读取任何 worktrees/ 下内容（硬约束）。
```

```
RESEARCH_LIMITATIONS =
1. 原始语料含 1 个 NUL，ripgrep 会截断；本 Child 用去 NUL 镜像规避，行号与原文件一致，但"镜像≠原件字节"。
2. grep 为关键词驱动，可能漏掉不含关键词但语义相关的表述；已用多形态+中英组合降低漏检。
3. 群聊大量为 ID/系统 JSON/接龙噪声，UB、row、启动 等存在伪命中，已人工剔除，仍可能有残留。
4. 所有结论仅基于群聊文本：无源码、无 shape、无 dtype、无复现实验；绝大多数硬件主张为 SPECULATIVE/AI_SUMMARY。
5. case14 157us 明确不作为有效 benchmark（INVALID_BENCHMARK）；cube+vector 效果不好 仅记为单次负实现，未推导机制整体无效。
6. "有人提出/有人实现/性能提高/全局提分"在本报告中严格区分；AI 转述（哈吉米/gpt/glm）一律标注 AI_SUMMARY。
7. 二级材料（总报告/清单/知识库）含项目侧 bestTimeUs 等，仅用于交叉核对，未混入 CHAT_EVIDENCE。
8. 无法核实评测机型号/核数/带宽，需项目侧用 npu-smi/官方文档补齐。
```
