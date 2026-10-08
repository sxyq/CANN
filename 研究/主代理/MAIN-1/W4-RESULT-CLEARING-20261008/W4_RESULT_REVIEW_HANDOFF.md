# W4 结果清算接手摘要

## 状态

- 工作树：`/Users/sunyiyang/Desktop/Project/cann`，分支 `main`，审计开始时 HEAD=`07662d7b96e9beaaa0f56d97c9cf346b86081eb3`。
- 仅新增本清算目录内五份文件；四份共享记录未改。
- 原主树17项未提交内容保留；本任务提交只包括五份新文件。提交 SHA 见本文件所在提交及本轮回执。
- PUSH=NO；未运行设备、Candidate、Compile、Correctness、Local 或 Online 操作。

## 可复用统计

- Official JSON：116条；Pass 65、Runtime Error 19、Wrong Answer 18、Time Limit Exceeded 6、Compile Error 8；65条含官方分数；submission ID 唯一。
- `source-meta.json`：实点24份；23份与结果JSON同目录，另有 `线上结果/R31A/V024/source-meta.json` 没有同目录结果JSON。22项可比较的 source 文件摘要值一致。
- W4当前账本：28条，9条 `REAL_EXECUTED_REVISION`、19条 `ROUTE_RESEARCH_EVENT`；15条授权路线均出现在矩阵。
- 五项当前待分类来源：R05研究、R05 V002性能事件、R11研究、R11 V002同版补充、R12研究。来源和分类见完整报告。
- R04新增 receipt：768样本、reference通过、输出逐位一致、计时波动大、`MEASUREMENT_BLOCKED`；submission ID与source commit均 `UNKNOWN`。已有0.992171392044 / +0.789037863747%继续标观察值。

## 后续核验事项

1. 按 Planning/Main 授权把五项待分类来源同步到四份共享记录；同版补充沿用原V002性能来源，R11研究事件 ID继续 `UNKNOWN`。
2. 确认R04 receipt对应的提交身份及是否有正式原始结果文件；确认前不把768样本值转成 Local 分数。
3. R01 Route 工作树有3项未提交文件。R04 最新 receipt 确认计时分析、原始采样/profiler 数据、研究稿及两个派发日志仍留在 Route 工作树；精确文件数 UNKNOWN，较早快照曾报4项。本次未读取这些文件。
4. Official 表保留每份结果JSON的case数组；报告已区分官方分数、绝对耗时及 `CALCULATED_COMPARISON`。不根据case ID推断shape或dtype。

## 文件

- `OFFICIAL_RESULTS_AUDIT.tsv`
- `LOCAL_RESULTS_AUDIT.tsv`
- `W4_ROUTE_RESULT_MATRIX.tsv`
- `W4_FULL_RESULT_AUDIT.md`
- `W4_RESULT_REVIEW_HANDOFF.md`
