---
name: cann-online-owner
description: CANN AddRmsNormBias Online Owner Skill。使用现有工具提交用户指定的 Candidate 并保存真实结果。
---

# Online Owner

Online Owner 负责调用已有的 工具/cannjudge-submit.mjs，保存平台响应，并把结果写入既有的 线上结果/<ROUTE>/<REVISION>/ 位置。

## 范围

- 处理用户当前指令中点名的 Candidate、文件和顺序。
- 记录 Route、Revision、源码提交、文件路径、提交时间、Submission ID、状态和平台返回值。
- Candidate 由其现有工作树维护；Online Owner 不复制源码、不创建替代提交工具。
- Local、Compile、Correctness 和研究结果与 Official 结果分开保存；缺失字段写 UNKNOWN 或 NONE。
- 平台强制要求、账号权限和运行环境要求按平台实际返回处理，不绕过平台，也不伪造结果。

## 操作记录

每项结果尽可能包含：

ROUTE
REVISION
SOURCE_IDENTITY
GIT_COMMIT
SOURCE_PATH
SUBMISSION_ID
SUBMISSION_TIME
OFFICIAL_STATUS
OFFICIAL_CORRECTNESS
OFFICIAL_SCORE
RESULT_JSON_PATH
PLATFORM_FAILURE_REASON

成功、失败、平台错误和未完成结果均保留。重复提交由用户指令和平台返回共同决定。

没有用户明确要求时，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
