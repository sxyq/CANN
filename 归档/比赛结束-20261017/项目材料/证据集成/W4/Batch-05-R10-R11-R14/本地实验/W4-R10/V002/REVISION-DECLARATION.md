# W4-R10 V002

AGENT_ID=01a119de-51ab-7810-9515-c9c388f70ac3；SLOT=5。
Direct Parent 为 R31B V011；复用 `../V001/parent.asc`，它与本工作树已保存的
`线上结果/R31B/V011/submission.asc` 比较无差异。V001 不作为 Parent，也不计入本次新增。

## 单一变化

只改变 BF16 宽 D 的 host blockCount 策略。设 Parent 核数 C、每核最大行数
Q=ceil(M/C)，R 沿用 Parent 的 `ChooseWideFullYRows(D,4,4,2,2)`。
仅在 BF16、D>8192、R>1、Q>R、Q%R=0、M%Q=0 时选择 M/Q 个核。
其余分支保持 Parent。owner 公式、tile、DMA、算术、流水和 device 函数均不变。

128x12288 BF16 在 availableCoreNum=40 时预计 40→32 核、Q=4 不变、
驻留批 80→64、参数 DMA 命令 480→384。以上为静态推导，未作为测量结果。

## 规则与重复核对

已完整读取自己的 AGENTS/Route Skill，再用本工作树 git show 读取
`9f91895506023d917637f707bb3f61cd9d9f8765` 的 AGENTS、Route Skill、W4 控制文件、
实验总则、执行约定、服务器规范、Local 规范、Git 流程、资源脚本，并发 RULE_REFRESH_RECEIPT。
本工作树缺少性能/环境/精度 Skill；改读已安装的对应 Skill、msprof 说明与浮点/CPU 参考说明。

复用 `5b7b9215:研究/W4-R10/ACTIVE-CORE-STUDY.md` 的完整历史审计。
W3 端点已再次确认：R2 `6321ad44` V040，R4 `ce6c6dc5` V031，R5 `1efa0863` V028。
Parent 的驻留批选择、行分配、参数读取循环与 host launch 已核对。
旧 V001 只含单批上限；想法 B 没有相同已测变化。
共享提交 `285e7b4c` 仍列出 V001 旧事件及两个研究事件，V002 尚无既有结果。

```text
DUPLICATE_AUDIT
MECHANISM=保持 Q 的 BF16 多完整驻留批缩核
SEARCHED_HISTORY=5b7b9215 审计的 R31/R31A/R31B、MIX、STORE/EPILOGUE、W4；W3 R2 V001-V040、R4 V001-V031、R5 V001-V028
MATCH_FOUND=NO
WHY_NEW_OR_DUPLICATE=V001 在 128x12288 BF16 保持 40 核；本版为 32 核且不影响 V001 的单批目标
```

## 输入与 reference

目标几何来自 W3 R4，BF16 域及固定输入生成沿用 R10 V001。所有输入均为本地测试，
不声称对应隐藏测试。已有 device 2 Parent reference 结果继续保留，不重跑该旧探测。
本版在 device 4 上分别执行 Parent/Candidate，与独立 CPU FP64 参考逐元素比较。
BF16 保留 FP64 参考直到比较；FP16 按原 runner 的逐步 half 舍入语义。
容差采用精度 Skill 的 BF16 2^-6/2^-6、FP32 2^-16/2^-10、FP16 2^-9/2^-9，
另用各 dtype 的绝对误差上限；本版要求每个元素通过并拒绝非有限输出。
原始输出保存为二进制；reference TSV 按行保留失败数、最大误差和对应元素。

| 输入 | 用途 |
|---|---|
| 128x12288 BF16 | guard ON，目标 |
| 48x12288 BF16 | Q=R，既有单批输入，计时对照 |
| 127x12288 BF16 | 从目标构造的 M%Q!=0 边界 |
| 120x12288 BF16 | 从目标构造的 Q%R!=0 边界 |
| 128x8192 BF16 | 从目标构造的 D=8192 边界 |
| 1x32768 BF16 | 既有 R=1 输入 |
| 12x8192 FP32 | 既有短 D 输入 |
| 2x12288 FP16 | 既有 dtype 对照 |

## 构建与计时

沿用 R11 `4243f4e9` 的双库构建与独立 host runner 方法；Parent 只引用已有源码。
沿用 R12 `a018f702` 的同输出地址、交替次序、内存保存 raw 和最小 task-time 采集。
不使用其他 Route 的时间或稳定性结论。CANN 8.5.0.alpha002，Ascend910B3，dav-2201。
本地构建支持文件准备好后才写 Candidate；其后下一实验动作是 server3 Compile。

Correctness 全部通过后，对目标和 BF16 对照分别进行一次采集。
每次同一进程先 warmup Parent 60 次，P/P 两组各 31 对，P1/P2 与 P2/P1 交替；
保存 P/P，再各 warmup Parent/Candidate 60 次，P/C 两组各 31 对，P-C/C-P 交替。
两侧使用同一输入输出地址，batch_n=1；分配、数据搬运、预热、写盘在计时外。
每次共 428 个 kernel 调用，其中 P/P 与 P/C 各 124 条计时样本。全部样本保留。

kernel task 为主要比较范围，device event 与 host wall 同时保留作诊断；
按 launch ordinal 对应 op_summary/task_time 和前后 EVENT_RECORD。
沿用既有 MAD/median<=0.10、两组中位数相对差<=0.10 的稳定性解释。
同时报告 P/P 配对差 p90、P/C 次序和分组差。波动超过这些范围时，
保留数字并标 MEASUREMENT_BLOCKED，不更新 Local Best，不增加有效或连续次数。
Local score 为目标逐对 `(C/P-1)*100` 的中位数，正数代表更慢；另保留总体中位比。

## 初始状态

BRANCH=w4/r10-active-core-d-aware-x；HEAD_AT_START=5b7b92150136513c30eb4af47c6d3d395377d94a。
CURRENT_LOCAL_BEST=R31B V011；本轮起始新增/有效/连续次数=0/0/0。
Online=PAUSED；Official=NONE；PUSH=NO。不创建其他 Route、子代理、线程或持续定时任务。
