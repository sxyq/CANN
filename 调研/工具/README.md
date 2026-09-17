# CANNJudge 工具

本目录集中放置 AddRmsNormBias 的提交前核对、浏览器内提交和结果查询工具。

## 文件

- `cannjudge.py`：本地文件核对、公开接口结果查询和官方分数读取。只发送 GET，不执行线上提交。
- `cannjudge-browser-submit.js`：在已登录的 CANNJudge 页面控制台运行，使用当前页面登录态提交一个本地源码文件，并轮询返回结果。

## 使用顺序

```bash
python3 调研/工具/cannjudge.py preflight 提交/混合方案/H001-正确性优先/V005/kernel.txt
```

然后在 `https://cannjudge.cn` 的已登录页面控制台运行 `cannjudge-browser-submit.js`。脚本会先显示文件名、行数、字节数、SHA-256 和入口检查结果，只有输入精确确认文本后才发送一次提交请求。

拿到 `submissionId` 后，可以在终端查询：

```bash
python3 调研/工具/cannjudge.py poll <submissionId> --official
```

浏览器脚本不会读取或保存 Cookie、密码、授权头，也不会自动重复提交。
