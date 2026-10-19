# W3 R1/R3/R4/R5 历史证据归档（Batch-09）

本批只收录由 Integration worktree 中 Git 提交对象定位的历史文件。来源 Route 分支均以 `503b98cb22ae88798bb6bfca83a46676933f3600` 为共同 base；R4/R5 Official 文件来自 `origin/main` 的提交 `1ea9677ba8e7307a73f13041c7b639ab6a96425c`。

| Route | 来源 HEAD | 纳入 | 排除 |
|---|---|---:|---:|
| W3 R1 | `64e32f535348cdde1828cd8a8fd89fad100aaa33` | 13 | 80 |
| W3 R3 | `584c590cef81349969c4dcf014a5c04d05032e9d` | 12 | 80 |
| W3 R4 | `ce6c6dc568256ac8b80b674096bd7ca081b897cb` | 616 | 248 |
| W3 R5 | `1efa0863611f1a9b76ab13dd361eba161b32b46d` | 263 | 252 |
| R4/R5 Official 附件 | `1ea9677ba8e7307a73f13041c7b639ab6a96425c` | 6 | 0 |
| 合计 |  | 910 | 660 |

`manifest.tsv` 为 1,570 条来源路径逐条记录 Route、Revision、原提交说明、source ref、commit/parent/tree、source path/blob、归档位置或排除类别。R1/R3 已由 Batch-07 收录的来源路径不在本批重复登记；本批补入其余 25 个日志文件。`commits.tsv` 保留四条 Route 的 87 个分支提交及 Official 来源提交。

纳入内容包括 Local 原始表、测量上下文、Correctness/Compile/运行日志、R5 的修订声明、R4 V001/V002 的 UTC 时间记录，以及 R4 V020、R5 V012 的 Official JSON 和提交日志。来源提交中没有独立的 R4/R5 研究报告路径。R5 worktree 的 ignored profile 文件不在本次读取范围内，其状态仍为 `UNKNOWN`。

660 个排除项为 Candidate/Parent/kernel/support 源码、构建配置和执行脚本；只保留来源路径与 Git 对象标识，不复制文件。纳入文件逐项按来源 commit:path 取 blob，归档内容保留原始字节。未执行实验或 Judge，未改共享账本；Route 的 Agent 与运行命令状态仍为 `UNKNOWN`，未关闭或移除任何分支、工作树或证据。`PUSH=NO`。
