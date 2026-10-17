# 社区语料只读取证报告（2026-10-04）

阶段：COMMUNITY_RESEARCH_ONLY（仅社区研究）。
Campaign 未启动；Route 未选择；无新 Revision；无性能执行。

产出：6 个独立 Research Child 对 QQ 群 901064769 原始语料的只读取证报告，正文原样保留，未删改。
语料源：`group_901064769.csv` / `group_901064769_text.txt` / `提分技术讨论_原文.csv`（项目外原始导出）。
隔离方式：每个 Child 使用项目外临时目录，未创建 git worktree，未改动项目、未进入正式 Route worktree。

## 报告清单

| 报告 | 文件 | 范围 | CHAT_EVIDENCE | MECHANISM_CLUE |
|---|---|---|---|---|
| A | RESEARCH_A_EARLY.md | 2026-09-12~09-18 早期段 | 34 | 6 |
| B | RESEARCH_B_MID.md | 2026-09-19~09-25 中期段 | 40 | 12 |
| C | RESEARCH_C_LATE.md | 2026-09-26~10-02 后期段 | 43 | 6 |
| D | RESEARCH_D_CASE.md | 全时段 case-centric（case1/4/7/14） | 42 | 5 |
| E | RESEARCH_E_HARDWARE.md | 全时段硬件/机制关键词 | 43 | 18 |
| F | RESEARCH_F_SKEPTIC.md | 全时段怀疑论/负证据 | 58 | 5 |

## 证据格式

每条记录统一使用 `CHAT_EVIDENCE` / `MECHANISM_CLUE` 字段，含 `EVIDENCE_CLASS`（DIRECT_RESULT / REPEATED_SIGNAL / NEGATIVE_RESULT / SPECULATIVE / SECOND_HAND / AI_SUMMARY / JOKE / INVALID_BENCHMARK / CONTRADICTED）与 `CONFIDENCE`（HIGH / MEDIUM / LOW）。报告未输出任何路线决策字段（CREATE_ROUTE / MAIN_SELECTED / PARK / CLOSE / REPLACE / ONLINE_DECISION）。

## 覆盖与限制

- 全体均未读取 `worktrees/` 下任何内容；均未修改项目文件。
- E/F/D 报告原始语料含 1 个 NUL 字节（ripgrep 会提前截断），已用去 NUL 镜像或 Python 规避；A/B/C 未显式标注该风险。
- 全部结论来自群聊文本：无源码、无 shape、无 dtype、无 before/after 配对；`case14 157us` 等异常值已按 INVALID_BENCHMARK 处理。
- 本目录仅记录研究证据，不代表任何路线选择；后续 Route portfolio 由 Planning / Review Layer 统一决定。

## 2026-10-12 原文复核与路线对照

六份报告共标注 260 条 CHAT_EVIDENCE 与 52 条 MECHANISM_CLUE；条目互有重复，不代表去重后的独立样本数。报告引用的实际语料是 QQ 群 901064769 导出，不是 GitHub 聊天。主源为 /Users/sunyiyang/WorkBuddy/2026-10-02-22-49-56/交付/原始数据/group_901064769.csv，报告中的 RECORD_ID 对应该原文件物理行。GitCode 讨论 #15 与 CANNJudge 排名页只取得网页外壳，没有取得正文。

| CHAT_SOURCE | CLAIM | SHAPE / DTYPE / HARDWARE | OPTIMIZATION_MECHANISM | CODE_OR_PROFILE_EVIDENCE | MEASURED_GAIN / FAILURE_EVIDENCE | MATCHED_OFFICIAL_CASE / ROUTE | EVIDENCE_LEVEL |
|---|---|---|---|---|---|---|---|
| QQ L150397；[官方赛题页](https://gitcode.com/cann-competition/026CANNChallenge_chengdu)；[集中答疑 #36](https://gitcode.com/cann/cann-ops-competitions/issues/36) | 计算必须由 Ascend C 在 NPU 执行；Host CPU 回退或空 kernel 占位违规；允许按 shape/dtype 走通用分支，不允许读取输入值按单个 case 编程。 | shape/dtype 未限定；硬件为 NPU，具体型号 UNKNOWN | 合规边界，不是提速方案 | 有规则/答疑文本；无社区源码/profile | 无增益数字；违规成绩应取消 | 全局规则；不匹配单一路线 | OFFICIAL_RULE |
| QQ L147327、L147621；报告 F | 群友报告同源码重复提交分数波动约 3–4 分，并质疑 Case 14 的 157 us 来源。 | UNKNOWN / UNKNOWN / UNKNOWN | 噪声或评分参考值变动只是解释假设 | 没有原始重复序列、源码、shape 或 profile | 无可归因收益；157 us 未能绑定有效 SID/回执 | Case 14 仅编号对应；无路线匹配 | REPEATED_REPORT / CAUSE_UNVERIFIED |
| QQ L150509；规则讨论见 L150397；报告 B/E/F | 声称小规模 TP 在 N*D<=64M 时走 Host CPU、NPU 仅运行 marker，并出现 67.50 分。 | 只报乘积阈值，无具体 Rows/D/dtype；型号 UNKNOWN | CPU 回退与空 kernel 的违规指控，不是可采信优化 | 无源码、设备轨迹或 profile | 67.50 是聊天中的指控，不是有效分数；若为真即违反规则 | TP 与官方 Case 无映射；无路线匹配 | HYPOTHESIS / INVALID_BENCHMARK_CLAIM |
| QQ L149163–149167、L152203–152204；报告 E | 对评测硬件型号的说法互相冲突，包含 910B3/B4、910C、A2/A3。 | 未提供 | 无可确认机制 | 无设备信息、npu-smi 或官方平台记录 | 无有效比较或失败复现；不能据此推断开发机与 Judge 环境差 | 全局；无路线匹配 | HYPOTHESIS / CONTRADICTED |
| QQ L152663–152667；报告 E | 发言者称 profile 看到 D 增大时 UB 吞吐上升，并提出先合并 D 再裁切。 | 只提 D 增大；dtype、硬件、具体 shape UNKNOWN | UB 数据布局/吞吐的假设 | 仅自述看过 profile；没有 profile 文件、计数器或源码 | 无增益数值、无实现结果 | 未绑定 Case；不能直接匹配 N01–N05 | SELF_REPORTED_PROFILE / UNVERIFIED |
| QQ L150675；报告 E/F | Case 8 达到 tbest 需 16.7 TB/s。 | shape/dtype/硬件口径 UNKNOWN | 带宽上限估算 | 二手 AI 估算，无推导、测量或 profile | 不是实测收益；量级被报告标为可疑 | Case 8 仅编号对应；N04 仅主题弱相关，不构成证据 | AI_SUMMARY / HYPOTHESIS |
| QQ L156563、L156567、L156612、L157056；报告 E/F | Case 4/7 可能走相似分发路径或“同类”处理。 | shape/dtype/hardware UNKNOWN | 共享路径猜测 | 无源码、profile 或 dispatch 记录 | 没有实现及前后数据；发言者也表示不了解队友实现 | Case 4、7 仅编号对应；无路线匹配 | HYPOTHESIS |
| QQ L151429–151430；报告 E | 一次大 D 的 cube+vector 混合尝试被描述为“效果不好”。 | 仅“大 D”；shape/dtype/hardware UNKNOWN | Cube+Vector 混合计算 | 无源码、计时或 profile | 单次口述负反馈，无数值，不能泛化为该机制无效 | 未绑定 Case；不能因出现 Vector 就对应 N03 | SINGLE_NEGATIVE_REPORT |

**路线结论：**社区语料没有可复核的微观提速机制直接匹配 N01–N05。N04 与带宽/L2 话题只有主题上的弱关联；N01/N02/N03/N05 均没有足够的源码、shape 或 profile 证据可用。路线实验优先级应由项目内真实 Case 回执、路线自身设备证据和正确性决定。社区中的分数变化、CPU 回退、case4/7 同路、UB 吞吐与 16.7 TB/s 都不得作为已验证成绩或机制。

**原文访问限制：**可读取 WorkBuddy 原始 CSV 与文本导出；引用行号使用报告标注的原始物理行。GitCode discussion #15 和公开排名页正文不可获取，相关内容不作为独立佐证。此处未复制或改写六份原始报告正文。

## 零变更清单（研究阶段）

    PERFORMANCE_EXPERIMENTS_RUN = 0
    CANDIDATE_FILES_MODIFIED = 0
    NEW_REVISIONS_CREATED = 0
    FORMAL_ROUTES_CREATED = 0
    ROUTE_LIFECYCLE_DECISIONS = 0
    ONLINE_SUBMISSIONS = 0
    HASH_CALCULATIONS = 0
    AUTONOMOUS_SCHEDULED_TASKS_CREATED = 0
