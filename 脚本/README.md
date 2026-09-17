# CANNJudge 自动提交脚本

`cannjudge-submit.mjs` 是当前唯一的一键线上提交入口。它读取源码、获取题目和用户信息、发送一次提交请求、轮询测试结果，并输出官方分数与本地复算分数。

## 首次登录

```bash
npm run cannjudge:login
```

在打开的项目专用 Microsoft Edge 窗口中完成登录，回到终端按 Enter。浏览器配置目录为：

```text
管理/提交队列/runtime/cannjudge-browser-profile/
```

## 一键提交

交互粘贴提交（省略 `--source` 时）：

```bash
npm run cannjudge:submit -- --yes
```

命令会等待你粘贴完整代码；粘贴完成后按 Enter，脚本才会开始提交。

提交 V005 文件：

```bash
npm run cannjudge:submit -- --yes --source 提交/混合方案/H001-正确性优先/V005/kernel.txt
```

提交指定源码：

```bash
npm run cannjudge:submit -- --yes --source /绝对路径/代码.txt
```

也可以从标准输入传入代码，例如：

```bash
pbpaste | npm run cannjudge:submit -- --yes --source -
```

`--yes` 确认一次线上提交。脚本默认每 2 秒轮询，最长等待 30 分钟；提交完成后输出 15 个测试点、官方分数和本地复算分数。

普通文本输出按测试点 `1` 到 `15` 排列，包含状态、失配率、本次用时、该点最优用时和单点分数。单点分数按以下公式复算，总分是 15 个单点分数的平均值：

```text
s_i = 100 / (1 + log_1.5(time_i / best_time_i))
total = mean(s_i)
```

脚本使用浏览器自己的项目会话，不读取、导出或写入 Cookie、密码和授权头。
