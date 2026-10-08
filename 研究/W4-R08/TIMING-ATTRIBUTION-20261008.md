# W4-R08 V001 计时归因：采集前声明

本次只补充 V001，新增性能版 0。不修改 Parent、Candidate 或预取逻辑，不创建 V002。
规则来源为 9f918955，当前共享事实另从 main@4959725e 的三份 TSV 读取。
接手 HEAD 为 fa9b19bb，分支 w4/r08-crossrow-prefetch-x，工作树初始无未提交内容。

## 输入与代码复用

仅使用已有独立 CPU FP32 reference 通过的 17x257 FP32 sin/cos 输入、epsilon=1e-5。
来源：fa9b19bb 的 RETEST-20261008.md、local-result.json、support/paired_runner.asc。
availableCoreNum 固定为 runner 参数 8；blockCount=8，行数为 3/2/2/2/2/2/2/2。
257 不满足按 8 对齐的多行路径，128<257<=4096，进入 ProcessNarrowMidOverlap。
既有 Candidate 每 launch 静态累计 9 次跨行 x 预取；8 不代表硬件核数。

远端唯一位置为 /home/data4t2/lelinfeng/w4-r08/V001。实际构建是单可执行文件，
Parent/Candidate 同时包含在 paired_runner.asc.o 内，无两份独立 kernel 动态库。
复用 2026-10-08 04:12:03 UTC 的原对象和原程序；仅用 W4R08_HOST_ONLY 编译 host
部分，并以 --wrap=main 接入旧对象。新 host 使用独立 C++ 命名空间，避免与旧对象的
Runtime 定义混用。原程序和对象均不覆盖，不使用 ELF 改写工具，不重新编译设备代码。
因此两侧代码的链接驻留方式保持一致；不能据此声称两个独立动态库存在。

## 旧数据与方法来源

先分析已有 372 条样本：原 Parent 62、修复后的 Parent 62、P/C 248。
原 RunPaired 每侧连续 31 个 timed call，四组 PC/CP/PC/CP；并非逐次交替。
RunBlock 每样本写 TSV 并 flush，组后写统计并打印。TimedLaunch 从 start-event 记录前
取 wall 时间，到 stop-event 同步返回后取 wall 时间；ElapsedTime 查询在 wall 结束之后。
输入生成、分配、搬运、reference、warmup、raw 写盘不在 event 范围内，但可影响后续调用。
旧样本包含非首样本长尾与块内明显变化，无法仅由 event/wall 数据拆出 kernel task。

方法参考从本工作树 Git 对象读取：

- 1b528176920d57936a9a889c13c3af8293b2a85b:本地实验/W4-R11/V002/RESULT.md
- 同提交的 本地实验/W4-R11/V002/support/probe.cpp
- 1a31a3b527d81d4db0fce20dda091b85889330b6:本地实验/W4-R05/gamma-view-20261008/RESULT.md

R11 相同协议 P/P 的 task 与 event 均可波动；删除打印或 flush 没有必然改善的证据。
R05 的双库驻留、进程与 ELF 封装差异必须保留为限制，不借用其样本资格。
本次无新性能概念；复用 R08 既有重复性研究，不将测量支撑记成新性能版。
W3 R2/R4/R5 的现有 ref 已确认分别到 V040/V031/V028。

## 一次有限 P/P

DEVICE=2，CPU affinity=72，warmup=45 对，samples=31，pair groups=4，batch_n=1。
两个逻辑槽位均调用同一 Parent，沿用一个共享输出地址、同一 stream、同一对 event。
按 P1P2/P2P1/P1P2/P2P1 顺序采 248 条 timed call；每侧每组连续 31 次。
每对 warmup 仍逐次同步，原 raw/flush/组统计位置不移动，不加主动暂停。
计时前真实 launch 一次 Parent 并与原 reference 比较；计时结束时再次比较最后输出。
全部首样本、长尾与负值差均保留。Candidate 本阶段不执行。

仅做一轮 msprof task-time，ai-core=off、aic-mode=task-based、task-time=on、
ascendcl=on、runtime-api=on、aicpu=off。不采多组硬件计数，不按结果追加重复轮次。
预计 339 个 kernel：1 次 reference、90 次 warmup、248 次计时。
按 raw 顺序逐个对应 op_summary 与 task_time 的 device/stream/task ID、开始时间和时长，
再对应同 stream 的 start/stop EVENT_RECORD。所有非计时调用也保留在原始导出。

## 事前判读与结束位置

分别报告 event 与 task 的全样本、两槽位、各块、块先后位置的 median、MAD/median、
CV、p10/p90、范围；报告 event-task、start-event 到 task、task 到 stop-event 间隔。
两槽位的差称为槽位差，不称为 Candidate delta。

沿用既有 0.10 的 MAD/median 与块间中位数漂移要求，全部块与两槽位分别判断。
块间漂移同时列出原首块相对差及 max-min 相对范围；不只选较小的一项。
还报告两种先后次序的槽位差，范围满足时也不能保证能识别微小收益。
P/P 不可靠则本次 Candidate=NOT_EXECUTED、LOCAL_SCORE=NONE，不追加 P/C。
只有 P/P 在目标计时范围可靠，才考虑同一支撑入口的一轮有限 P/C；跨进程与真实地址
重新分配仍须明确记录，不能把前一进程资格扩大成无条件资格。
停止后提交本次 host 支撑、完整样本与分析，发研究及 V001 同版补充事件。
不改变 Route 生命周期，不切换 Route，不创建计时或后台任务。
