# online — 线上结果快照

存放每次 CANNJudge 提交的结果快照，每份至少包含：

- submission id
- 状态（CompileError / Pass / WA / Runtime 等）
- 15 个 case 的逐点结果
- Official Score（只抄平台，不自行计算）

当前已保存 FULL-R013 V001 线上快照：`FULL-R013-DOUBLE-BUFFER-PIPELINE/V001/6aacb325b0477ec41e1707d2/`。其中 `result.json` 是脚本原始结果，`kernel.txt` 是提交源码，`manifest.json` 绑定源码 SHA-256、Git commit 和 submission ID，`result.md` 保存 15 个点位与环境状态。

提交方式与工具边界见 `文档/CANNJudge流程.md`。
