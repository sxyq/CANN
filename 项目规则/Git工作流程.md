# Git 工作流程

本文件是实验 Git 顺序和安全边界的唯一说明。

## 实验版本顺序

```text
EDIT
→ COMPILE
→ CORRECTNESS
→ LOCAL
→ COMMIT RESULT
```

不要求 Compile 前建立 checkpoint commit。每个实验/版本至少一个独立 commit；编译、Correctness、Local 的事实可随该实验结果一起提交，失败和负结果同样提交并保留。

Local 改善时：

```text
COMMIT RESULT → PUSH PROMPTLY → CURRENT_LOCAL_BEST
```

Local 未改善时：

```text
COMMIT NEGATIVE RESULT → KEEP HISTORY → LATER PUSH OR HANDOFF PUSH
```

下一次小变化从当前 `CURRENT_LOCAL_BEST` 继续，不把多个独立变化压进一个 Revision。

## 暂存范围

- 只使用 `git add <specific paths>`；
- 禁止 `git add .`、`git add -A`、`git add --all`；
- Route 分支只提交自己的 Route 文件；共享记录由 Record Owner 单独提交；
- 规则迁移、Candidate、实验证据和共享记录分开组织；
- 一个提交不混入无关路线、无关文件或未授权的用户改动。

## Route 隔离

```text
1 Route = 1 Agent = 1 Context = 1 Branch = 1 Worktree
```

Route Agent 不提交其他 Route；Main 不运行 Git；Record Owner 只提交自己负责的共享记录；Online Owner 不改共享成绩记录。

## 禁止操作

禁止 force push、reset、clean、历史改写、覆盖已有证据和删除失败版本。不得因整理文档删除源码、凭据、SQLite、Keychain、LaunchAgent、运行缓存或用户需要的数据。外部 push 需按用户授权执行；本次规则迁移不自动 push。

未经用户明确要求，不创建 automation、scheduled task、cron、crontab、at、systemd timer、launchd timer、watchdog 或 detached sleep loop。
