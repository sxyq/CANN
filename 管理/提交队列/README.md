# 提交队列

线上提交由主控 Agent 审核、唯一 Submission Worker 执行。路线 Agent 只能创建申请，不能直接操作浏览器或固定像素。

目录状态：

- `queued/`：已绑定 commit、SHA-256 且通过本地门禁的待提交任务。
- `running/`：Worker 当前持有的任务。
- `completed/`：已返回提交 ID、15 个 Case 状态和官方分数的任务。

当前队列没有可提交任务。精度门禁未通过、源码身份不确定或缺少官方结果的版本不得进入 `queued/`。
