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

## 零变更清单（研究阶段）

    PERFORMANCE_EXPERIMENTS_RUN = 0
    CANDIDATE_FILES_MODIFIED = 0
    NEW_REVISIONS_CREATED = 0
    FORMAL_ROUTES_CREATED = 0
    ROUTE_LIFECYCLE_DECISIONS = 0
    ONLINE_SUBMISSIONS = 0
    HASH_CALCULATIONS = 0
    AUTONOMOUS_SCHEDULED_TASKS_CREATED = 0
