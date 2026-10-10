---
name: cann-main-orchestrator
description: CANN AddRmsNormBias Main 协调 Skill。汇总本地路线、源码、实验和记录状态。
---

# Main Orchestrator

Main 用于汇总任务范围、路线信息、实验结果和资料来源。路线名称、Candidate、实验成绩和后续安排以现有文件、实际结果和用户指令为准。

## 工作方式

- 开始或恢复任务时读取根 AGENTS.md、相关 Skill、项目规则和需要的共享记录。
- 可以读取 Git、源码、实验资料和提交记录，也可以在用户指定范围内直接修改已有本地文件。
- 编译、正确性、性能、设备运行和线上提交都沿用已有入口；缺失某项记录时如实写明，不把缺失信息当成结果。
- 状态记录使用已有文件和字段，不创建重复账本、重复脚本或平行执行链。
- 研究结果、实验数字和平台返回值按来源保存，不凭空补齐。

## 路线信息

Route、Agent、Context、Branch 和 Worktree 是现有对象的描述。它们之间可以按当前任务需要协作，工作树中的未提交内容需避免互相覆盖。

常见路线记录字段如下：

ROUTE
REVISION
LAST_ACTION
NEXT_ACTION
CHANGE
COMPILE
CORRECTNESS
FREE_HBM_MB
DEVICE_ID
LOCAL_SCORE
LOCAL_DELTA
CURRENT_LOCAL_BEST
GIT_COMMIT
PUSH
STATUS
EVIDENCE_NOTE

这些字段用于记录事实，不形成额外的执行条件。实验可以按实际影响选择编译、正确性、性能或资料整理步骤。

## 资源安全

目标 NPU 的 FREE_HBM、真实运行错误、服务器连通性和对其他用户任务的影响共同决定当前操作是否继续。设备占用、AICore、其他进程、负载和 lease 只作为上下文保存，不要求额外的独占时段或人工确认。

不停止、杀掉、重启、迁移或修改其他用户的进程、服务、容器、数据和系统配置。凭据、实验结果和失败材料保持原样。

## 本地保存

使用精确路径分批本地提交；默认不推送、不创建远端对象、不添加远端审查、自动构建或自动发布配置。没有用户明确要求时，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。