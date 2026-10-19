# CANNJudge 流程

## 工具边界（唯一权威口径）

| 工具 | 职责 | 明确禁止 |
| --- | --- | --- |
| `脚本/cannjudge-submit.mjs`（`npm run cannjudge:login` / `npm run cannjudge:submit`） | **唯一**线上执行器：登录、提交、等待、取回结果 | 不绕过它另写第二条提交通道 |
| `调研/工具/cannjudge.py` | 提交前本地核对（preflight）+ 只读 GET 轮询结果 | **永不** POST 提交；不发凭据、不读浏览器配置 |

除以上两者外，不再新增任何线上提交方式。

## 本次清理（cleanup）期间

- **本次清理不发起任何新的线上提交。** 只整理文档与目录，登录、提交、轮询一律暂停，等清理完成、上游交接包到位后再按 [实验纪律](实验纪律.md) 恢复。

## 标准提交流程（清理结束后适用）

1. 确认候选：源码文件 + SHA-256 + git commit 三者一致。
2. 本地核对：`python3 调研/工具/cannjudge.py preflight <文件>`。
3. 服务器侧验证：编译、NPU 正确性、性能，证据落盘 `实验/` 对应目录。
4. 提交：`npm run cannjudge:submit -- --yes --source <文件>`（需已通过 `npm run cannjudge:login` 登录）。
5. 轮询：脚本内置等待；也可用 `cannjudge.py poll <submissionId>` 只读查看。
6. 抄录结果：submission id、状态、15 个 case、Official Score（只抄平台，不自行计算）。
7. 快照落盘 `实验/online/`，再更新 [当前状态](当前状态.md) 与 [路线树](路线树.md)。

## 结果解释

- CompileError → CompileFix：同一路线同一个 V001，不新开版本。
- WA / Runtime：只要实际执行了判题，计为该路线的一次广度证据。
- 线上返回的 Official Score 是最终依据；本地复算分数只作参考，不入权威表。
