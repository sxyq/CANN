# N04 V019 结果

## 假设与源码

- 父版本：V018；父源码为本路线 `本地实验/R31B/V016/V018/Candidate.asc`。
- Candidate：`本地实验/R31B/V016/V019/Candidate.asc`。
- 单一机制：保持 V018 的 x-first/residual-second 提交顺序、双完成事件、下一批首 tile 消费方式和尾部 A/B 参数搬运；仅把下一批首 tile 的 slot C 输入 DMA 放到当前 tile 的 `SyncVToMTE2()` 与输出 Store 之间，使 MTE2 输入搬运尝试与 MTE3 输出重叠。
- 服务器 staging：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V019/`。

## Compile

- 入口：复用既有 `paired_runner_v016`、`paired_runner.cpp`、ABI、wrapper 和 CMake；未创建新 Runner、提交脚本或执行链。
- Configure：`CONFIGURE_RC=0`；记录：[server3-configure-v019-20261017.log](server3-configure-v019-20261017.log)。
- Compile：三次真实构建均 `BUILD_RC=0`，目标为 `[100%] Built target paired_runner_v016`。原始记录：[server3-compile-v019-20261017.log](server3-compile-v019-20261017.log)、[server3-compile-v019-rerun-20261017.log](server3-compile-v019-rerun-20261017.log)、[server3-compile-v019-rerun2-20261017.log](server3-compile-v019-rerun2-20261017.log)。

## Correctness

- 设备：主机 `hwnput3`，登录入口 `cann-server3`，NPU1；未使用 NPU4。
- 命令：`build-v019/paired_runner_v016 --correctness --device 1`。
- 第一次真实事务：日志只有 `CORRECTNESS_RC=143`，无任何用例输出；原始日志：[server3-v019-correctness-device1-20261017.txt](server3-v019-correctness-device1-20261017.txt)。运行前快照：[server3-before-v019-correctness-device1-20261017.txt](server3-before-v019-correctness-device1-20261017.txt)；运行后快照：[server3-after-v019-correctness-device1-20261017.txt](server3-after-v019-correctness-device1-20261017.txt)。
- 第二次真实事务：日志只有 `CORRECTNESS_RC=143`，无任何用例输出；原始日志：[server3-v019-correctness-device1-rerun-20261017.txt](server3-v019-correctness-device1-rerun-20261017.txt)。运行前快照：[server3-before-v019-correctness-device1-rerun-20261017.txt](server3-before-v019-correctness-device1-rerun-20261017.txt)；运行后快照：[server3-after-v019-correctness-device1-rerun-20261017.txt](server3-after-v019-correctness-device1-rerun-20261017.txt)。
- 两次运行均未产生四组 Parent/Candidate 结果，不能记为 Correctness PASS。运行期间只读状态显示 Runner 主线程等待设备回执，NPU1 Aivector Usage 为 20%；结束后快照为 HBM 5%、Aicore 0%、Aivector 0%，没有残留本次 V019 Runner。

## Local / Official

- Local：未执行；没有四组中位数或 P10-P90，`LOCAL_SCORE=NONE`。
- Official：未执行；没有线上 SID、分数或回执。
- V019 不计为有效性能结果，不与 V003 或 Champion 做性能比较。

STATUS: COMPILE_PASS_CORRECTNESS_RC143_TWICE_LOCAL_NOT_EXECUTED_OFFICIAL_NOT_EXECUTED
LOCAL_SCORE: NONE
OFFICIAL: NOT_EXECUTED
