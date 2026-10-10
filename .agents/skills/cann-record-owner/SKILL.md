---
name: cann-record-owner
description: CANN AddRmsNormBias shared-record Skill。维护已有规则和共享记录，按真实来源同步事实。
---

# Record Owner

Record Owner 负责维护已有的共享记录和项目规则，不创建新的账本、脚本或执行链。历史 W2/W3/W4 记录保持原样，当前记录以实际文件、提交、实验结果和平台响应为准。

## 记录范围

可维护的正式记录包括：

- 技术路线/全版本记录.tsv
- 技术路线/路线成绩表.tsv
- 技术路线/技术路线图.md
- 调度/当前任务.tsv

规则入口包括根 AGENTS.md、.agents/skills/ 和 项目规则/ 中已有文件。修改时只改用户当前任务涉及的内容，不覆盖其他用户改动。

## 记录内容

保留来源路径、Route、Revision、结果类型、源码提交、实验日期和已有事件标识。缺失字段使用 UNKNOWN 或 NONE，不补造数据。Local 与 Official 分开记录，原始输出和失败材料不删除。

## 工作方式

记录可以与实验并行进行，不改变源码执行路径。Record Owner 可以读取本地 Git、实验资料和既有工作树状态，也可以在用户指定范围内直接编辑共享记录。

实验资源安全、凭据保护、用户数据保护和 Git 历史保护沿用根 AGENTS.md 与 项目规则/服务器实验规范.md。

没有用户明确要求时，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
