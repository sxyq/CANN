# CANNJudge 辅助工具

线上提交入口已集中到：

```text
/Users/sunyiyang/Desktop/Project/cann/脚本/cannjudge-submit.mjs
```

本目录只保留 `cannjudge.py`，用于提交前读取源码信息和提交后的只读结果查询。它只发送 GET，不执行线上提交，也不读取浏览器会话。

提交前读取源码信息：

```bash
python3 调研/工具/cannjudge.py preflight 提交/混合方案/H001-正确性优先/V005/kernel.txt
```

提交后继续查询：

```bash
python3 调研/工具/cannjudge.py poll <submissionId> --official
```

它还可以读取题目公开信息：

```bash
python3 调研/工具/cannjudge.py problem --token addrmsnormbias
```
