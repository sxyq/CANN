---
name: cann-route-executor
description: CANN AddRmsNormBias Route Agent Skill。沿用已有源码入口完成路线实验、记录和本地提交。
---

# Route Executor

Route Agent 负责已有路线的源码、构建、正确性、性能实验和结果记录。路线信息以当前源码、实验目录和用户指令为准，不预设路线数量、版本数量或结果方向。

## 本地工作

- 先阅读当前路线的源码、历史提交、实验资料和已有入口。
- 优先直接修改已有实现；不为同一功能创建重复脚本、Runner、状态系统或数据账本。
- 每个变化记录其技术假设、父版本、源码路径、命令、返回码、设备、负载、结果和解释。
- 可按实际影响选择 Compile、Correctness、Local、Profile 或资料整理；未执行项目直接记录。
- 失败、负结果、未执行和资源不足均保留原始证据。
- 本地提交使用精确路径；默认 PUSH=NO。

常用结果字段：

ROUTE
REVISION
PARENT
CHANGE
COMPILE
CORRECTNESS
LOCAL_SCORE
LOCAL_DELTA
DEVICE_ID
FREE_HBM_MB
GIT_COMMIT
EVIDENCE_NOTE

字段用于记录事实，不形成额外的执行条件。普通下一版本沿用已有路线、分支和工作树；需要切换时使用已有工作树，避免覆盖未提交内容。

## 资源安全

资源不足、OOM、分配失败、真实运行时错误、服务器不可连接或可能影响其他用户任务时停止当前操作并记录原因。负载、其他进程、lease 和设备非空闲只作为测量上下文。

不删除、覆盖、伪造或重写其他人的 lease，不停止或修改其他用户任务。凭据、运行缓存、实验结果、失败材料和 Git 历史保持原样。

## 结果保存

Local 是本地测量结果，Official 是平台返回结果；两者分开记录。实验命令和原始输出保存到已有目录，缺失值使用 NONE 或 UNKNOWN。

没有用户明确要求时，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
