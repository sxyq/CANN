# W4-R04 V002：NarrowMid FP32 residual 子视图

Agent：`01a119cc-fa30-7740-8fd2-9cdfab6e6c46`。分支 `w4/r04-ub-bank-xr-layout-x`。
Direct Parent 为 `de70b634813dea80783fc57716d6e95c158edeec:线上结果/R31B/V011/submission.asc`。
V001 保留为旧研究标签；V002 是本轮第一个真实性能版本，不替代失败样本。

唯一变化：在 `ProcessNarrowMidOverlap` 的 FP32 分支中，D<=4032 时将 residual
Load/Add 的同一视图向后移动 64 float；D>4032、其他 dtype 和路径保持原样。
全部 InitBuffer、其他 buffer 视图、DMA 次数、事件和算术顺序不变。

研究依据为 `8c3b6659014e7904c1b14371e24045e68aaa8bee:研究/W4-R04/UB-BANK-EVIDENCE.md`。
174 份已提交源码的专项结果未发现该输入视图变化。官方 2201 地址模型给出
256 B 对应 8 个 group 相位差；容量条件是 `64+round_up(D,8)<=4096`。
实际 UB 指针与周期数尚未实测，本版不增加 bank 探测。

## 运行与精度

server3 唯一版本目录：`/home/data4t2/lelinfeng/cann/server_runs/W4-R04/V002`。
使用设备 3、CANN 8.5.0.alpha002、Ascend910B3、dav-2201。
复用 W3 R2 V040 的 CMake、直调包装和 45 warmups / 4×11 对 event 计时结构；
源码来自 `6321ad4427b1819d172c719ebde190eb447ce9ae`，没有读取其远端目录。

正式精度对 Parent/Candidate 分别与 CPU FP64 reference 比较，最终 reference
仅在输出处转换为 FP32。混合容差为 `1e-5 + 1e-4*abs(reference)`，要求全部元素通过，
并要求 max_abs<=1e-2。另要求双方逐元素字节一致，以核对纯布局变化的数值语义。
测试包含有来源的 16×2048、16×2056 FP32；构造的边界测试为 2×128、2×129、
2×4032、2×4033、2×4096，以及验证多行复用的 81×2056。后六项只用于精度，
不作为隐藏测试形状或 Local 成绩来源。

用户在首次 Compile 通过后要求增加来源明确的 guard-OFF 对照，故在本版内只补
runner 输入，再编译验证：`12×8192 FP32`，来源为
`712e47230287182bc65ab433a3ce714e6fdafd9f:研究/W4-R03/HOST-OCCUPANCY-STUDY.md`
中绑定 R11 V002 的输入与 host 分派记录。它进入 GenericRow，不进入 NarrowMid，
本版子视图变化不生效。它用于同二进制与 P/C 的环境对照，不进入下述两项受影响
输入的汇总 score。它与受影响输入路径不同，这项限制保留在结果解释中。

## Local 方法与解释条件

先用同一 Parent 动态库作为 A/B 两侧，保留 same-binary 原始样本；随后用
同样的 runner 与输入交错采 Parent/Candidate。两项性能输入都受变化影响。
每项每侧 45 warmups、4 块×11 对；块内按 block+pair 奇偶交替 P-C/C-P。
分配、输入准备与 warmup 不进入 event 样本，所有样本永久保留。

运行前固定的解释条件：same-binary 两侧 median 差绝对值<=2%，两侧
MAD/median<=5%；P/C 测量也要求两侧 MAD/median<=5%。有效改善还需幅度超过
对应 same-binary 漂移，并且四块的 median 变化方向一致。条件不成立时仍完成
采样并保留数字，状态为 MEASUREMENT_BLOCKED，不提升 Local Best。

报告每项 Parent/Candidate median、`(C/P-1)*100`、配对差值 median 和波动。
汇总 Local score 定义为两个 `P_median/C_median` 的几何均值，汇总 delta 为
`(1/score-1)*100`；这些仅为本地代理，不代表 Official。

阶段命令为 `bash support/run_stage.sh compile`、`correctness`、`same-binary`、`local`。
每阶段记录 UTC、主机、用户、FREE_HBM、设备负载、进程内存和返回码。
FREE_HBM>=100 MB 即执行，不等待独占设备；不改其他用户任务或 lease。

`OFFICIAL=NONE`，`ONLINE=PAUSED`，`PUSH=NO`。结果随 RESULT 与原子提交给出。
