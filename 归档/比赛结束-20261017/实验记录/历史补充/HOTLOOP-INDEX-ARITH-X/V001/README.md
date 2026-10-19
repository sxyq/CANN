# HOTLOOP-INDEX-ARITH-X / V001 历史补充

本目录只保存从 `cann-studio-hotloop` 工作树找回的五份文本诊断日志。它们记录形式化编译、CMake 配置和 probe 重跑过程，不是 Kernel 源码、Correctness 结果或 Canonical Local 分数。

## 文件

- `compile-formal-20261006.log`：正式编译/设备状态文本日志，SHA256 `8f6f07cd00cf8c17753d5df54fa361fde7ae34a5e569f7b133bc9cf8f8abbbdb`。
- `support/probe-build-rerun.log`：构建重跑诊断，SHA256 `8e1b0172ac133d14993079b97cc2154313f0272ba9cfbb046f60786b50f6b602`。
- `support/probe-build.log`：构建失败诊断，SHA256 `183a24ca292a33b5892e8776370fbc41c0cd52ec819f7abfc755a73a5872de5f`。
- `support/probe-configure-rerun.log`：CMake 配置重跑日志，SHA256 `6f6ee05b91afddd0354c90ae4644063417a122453b83668754e05dea6e438417`。
- `support/probe-configure.log`：CMake 配置日志，SHA256 `dde769e031918812d2492150b4cb00c1e791ffdd11ac8fbd7c223ac068f2563d`。

工作树中的 `build-env/`、`build-formal-20261006/` 和 `support/build-rerun/` 等 CMake 生成目录未纳入归档；原始工作树仍保留在仓外恢复目录。
