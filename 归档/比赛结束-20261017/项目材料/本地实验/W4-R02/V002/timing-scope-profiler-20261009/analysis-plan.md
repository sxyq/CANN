# W4-R02 V002 profiler 开关诊断计划

## 范围与假设

- 仅诊断 128x20480 FP16 下 profiler 对 ACL device-event 与 host-wall 观测的影响；不形成 Local 成绩，不改 Candidate、不新增性能版。
- 假设：profiler 开启会扩大 event 相对 kernel-task 的额外区间，或改变 event 样本的离散程度；若开关两侧 event 分布相近且开 profiler 时 task 仍稳定，则该假设不获支持。
- Parent 为 `R31B-V011`，Candidate 为 `W4-R02-V002`；Candidate source commit 固定为 `60ca277d2afc3d985965e16ca09d1ff9b03f8c10`。源码 SHA-256：Parent `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，Candidate `b512579409b6614002f4bd7463d9a243b13d2bec0723094e52cc8c2b5c35bf38`。

## 固定条件与顺序

- 仅用现有 `support/runner_main.cpp` 对应 runner；shape 128x20480、FP16、相同确定性输入生成器、相同计时边界、同一目标设备。按资源协调选择 device 1；device 3 的旧分配不用于本次对照。
- 每个逻辑槽预热 45 次；每条件 4 组，每组 21 对；PC/CP 次序按 runner 的 `(block + sample) % 2` 交替，不删样本、不挑窗口。
- P/P 使用 `paired-parent`，P/C 使用 `paired`。计划顺序：profiler-on P/P、profiler-off P/P、profiler-off P/C、profiler-on P/C。所有条件均由同一 runner 顺序执行，每次启动前记录 HBM 与负载。
- profiler-on 使用现有 msprof 参数：AI-core off、task-time on、AscendCL/runtime API on、AICPU off；profiler-off 直接运行同一 runner，不另造采时器。

## 预先声明的统计

- 保留每个条件全部 event 与 wall 样本、完整 runner 日志、profiler 原始 CSV、主机负载、设备 usages/HBM 和指针/stream 元数据；任何失败输出也保留。
- 每条件分别报告 event/wall 的样本数、中位数、MAD/中位数、四组中位数及其极差/总体中位数；P/C 再报告配对差、PC 与 CP 分层中位数、先后位置差。
- 沿用既有 10% MAD/中位数和 10% 四组极差/总体中位数界值，仅作诊断说明；不移除异常样本、不调整统计定义。
- profiler-on 用现有 task/API 与 event-call 对照资料核对调用序号、stream ID、输出关系及 task 时间。profiler-off 只具备 runner event/wall 行，不会生成 task/API 轨迹；不为缺失项补值。

## 解释边界

- runner 每次进程启动都会新建 ACL stream 与 device allocation；它不支持从外部传入或跨 profiler 状态复用同一 `aclrtStream` 句柄/物理输出指针。各次启动会记录实际地址；即使地址相同，也只按观测报告，不宣称句柄由 runner 保证复用。
- 因此比较仅用于判断同设备、同输入、同 shape、同调用顺序下 profiler 对 event/wall 观测的影响；不能视作同一 stream/地址下的严格因果隔离，也不能比较两侧 task/API 时间。
- 只运行这一组预定条件一次；不重试、不另选窗口。任何不满足固定 shape、设备或原始输出完整性的条件均保留，并按实际范围说明。

## 启动前记录

- 首次远端包装命令返回码 1；确认新远端采集目录尚未创建，未调用 runner、未产生设备样本。原因未由该次命令保存的输出确认。后续正式条件只在调整 shell 初始化顺序后各执行一次；此启动记录保留，不作为测量样本。
