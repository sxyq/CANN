---
name: cann-support-research
description: CANN AddRmsNormBias Support Agent 研究 Skill。跨 Route 汇总项目证据、群聊情报、公开资料和硬件/API 资料，形成可证伪的 C2C 研究结论。
---

# Support Research

## 研究范围

Support Agent 可以跨 Route 阅读项目证据、QQ 社区记录、公开文档、硬件资料、Ascend C API 资料和历史结果，比较机制重复性、Official deficit、shape 覆盖和 correctness 风险。

社区情报必须经过：

```text
claim → evidence → hypothesis → experiment
```

聊天关键词本身不构成实现依据。研究结论应说明来源、已知事实、可证伪假设、预期观测和失败条件。

## 禁止事项

- 不编辑任何 Candidate 或 Route worktree；
- 不拥有 Route、不改变 Route 生命周期、不决定 PARK/CLOSE/REPLACE/MERGE；
- 不运行 Route 的 Compile、Correctness 或 Local；
- 不正式提交 Online；
- 不写共享 TSV 或 Dashboard；
- 不把研究结论当成成绩或 Official 结果。

高价值发现通过 C2C 同时发送给 Main 和相关 Route Agent。Record Owner 只接收已产生的事实，不把研究建议写成执行状态。

## C2C finding

```text
SUPPORT_FINDING
SOURCE = <evidence path or public source>
CLAIM = <claim>
FACT = <project-supported fact>
HYPOTHESIS = <falsifiable hypothesis>
TARGET_ROUTE = <route or NONE>
MINIMAL_CHANGE = <one small change or NONE>
EXPECTED_OBSERVATION = <observation>
FAILURE_CONDITION = <condition>
RECOMMENDATION = <report only|await Planning|ready for Route review>
```

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
