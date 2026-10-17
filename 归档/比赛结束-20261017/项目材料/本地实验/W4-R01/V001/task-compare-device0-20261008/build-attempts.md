# host 构建尝试

本轮只在远端 `W4-R01/V001` 将原 host runner 链接至既有 Parent/Candidate kernel 库；所有尝试均未编译或写入 kernel 库。

| UTC | 结果 | 诊断 |
|---|---|---|
| 12:18:11 | FAIL | 首次 `-O2 -std=c++17` host 编译：`runner_main.cpp:536: error: ‘trace’ was not declared in this scope`，位置为 `RunBalancedCalls`。 |
| 12:20:33 | PASS | 将两个字段引用改用当前 `activeTrace` 后编译通过；GCC 提醒 `Sample.order` 的目标缓冲长度不足。 |
| 12:22:09 | PASS | 将该缓冲扩为3字节后再次编译通过，无警告；此 runner 用于唯一一次采集。 |

首次错误没有触发设备执行。编译命令见 `compile-command.txt`，最终成功命令和返回码见 `compile.log`。早先两次输出由后续同目录正常编译日志替代；本表保留原诊断和状态。
