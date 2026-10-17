# R11 V002 同协议 P/P 诊断

## 事先声明

本次只处理 V002 的 host 测量工具，不新建性能版本，不改 Parent、Candidate、
kernel 动态库、参考公式或性能因子。来源为 `4243f4e9e90a56cc12cf4adcbd7282149238b8f3`。
工作树为 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R11-row-remainder-balance-x`，
分支为 `w4/r11-row-remainder-balance-x`，开始时无未提交内容。设备固定为 server3
device 1，Online=PAUSED，PUSH=NO，CURRENT_LOCAL_BEST=NONE。

规则来源为 `9f91895506023d917637f707bb3f61cd9d9f8765` 的九项指定入口，
并已读取本工作树 AGENTS、Route Skill、已安装 ops-profiling Skill 与 msprof guide。
共享状态从 `main` 读取，读取时提交为 `285e7b4c2a6810774ca46c053bb7cac0f0b52223`；
R11 已登记，4243f4e9 的同版补充事实仍待 Record 同步。

## 重复范围

```text
DUPLICATE_AUDIT
MECHANISM=RunLocal 内双方调用 Parent，保留双输出地址及原采样顺序
SEARCHED_HISTORY=R11 V002@4243f4e9；R10 V002@1441fc72
MATCH_FOUND=R11 same 与 local 协议有差异；R10 使用单输出地址和不同预热/采样次数
WHY_NEW_OR_DUPLICATE=R11 尚无本次同协议 P/P；这是测量工具研究，无新性能概念
REVISION=V002
DIRECT_PARENT=R31B V011
NEW_PERFORMANCE_REVISIONS=0
```

R11 旧 same 每样本 flush、两组之间暂停两秒；旧 local 使用两个输出地址、45 对
预热和 4x11 交错对；Measure 的第一个样本含诊断打印。它们是已见源码差异，
现有证据未确定单一原因。R10 的 task 次序差提供研究线索，不作为 R11 资格证明。
本次不研究宽行内部余行归属，也不开展 W3/R31/MIX/STORE 的新性能轴审计。
W3 当前 refs 已确认存在，旧表端点不被当作最新版本。

## 固定次数与比较指标

第一项实验只启动一次 `local-pp`，在同一进程依次处理 `64x8192 FP32` target
和 `12x8192 FP32` control。每个输入保持 45 对预热、4 组各 11 对，逐对按
`(block + pair) % 2` 交替 P-C/C-P 槽位次序。P 槽位始终写原 Parent 输出地址，
C 槽位始终写原 Candidate 输出地址；本模式两个槽位都调用 Parent 函数。
每输入 88 个计时调用，总计 176 个。每输入在计时前以两个实际输出地址对
既有 CPU FP32 reference 验证，计时结束后再次读回输出验证，不追加 kernel 调用。
原随机种子、输入范围、epsilon、容差 `2e-5 + 1e-4 * abs(reference)` 均不变。

保持原 RunLocal 的写盘位置与 TSV 列，不增加每样本 flush 或组间暂停。
Measure 中 start/stop/reset/synchronize 与首样本打印保持原样。输出地址和槽位
函数只在每个输入开始前记录。task ID 由本次 op_summary/task_time 和事件相邻
关系逐调用对应，包含精度与预热调用的原 CSV 也保留。

分别对 device-event 和 kernel-task 报告：全部样本中位数、均值、标准差、CV、
MAD/中位数、p10/p90、min/max；四组中位数与 `(max-min)/全样本中位数`；
两个地址各自的统计；第一/第二次调用、P-C/C-P 次序下的配对差；首样本与长尾。
原 10% 要求用于全部 P/P 样本以及每个地址：MAD/中位数 <= 0.10 且四组中位数
相对范围 <= 0.10。任何样本均不删除。两个输入在两种计时范围均满足后，才启动
一次同版 P/C；否则保留数值并报告 MEASUREMENT_BLOCKED，不将 P/P 数值称为收益。
位置和地址差值为诊断指标，不从一次相关性指定根因。

采集仅使用 `msprof --ai-core=off --task-time=on --ascendcl=on --runtime-api=on
--aicpu=off`；不采多组硬件指标。若需要另一项有限实验，先根据新增证据说明唯一
对照变量与固定次数；不以无变化重复采样取得通过结果。

## 产物与保留方式

原位修改 `本地实验/W4-R11/V002/support/probe.cpp`，旧源码可从 4243f4e9 读取。
原 `build-server3-v002/adaptive_probe` 与两份 kernel 库不重建；只生成
`build-server3-v002/adaptive_probe_local_pp`，编译选项沿用原 host 的 gnu++17，
不新增优化参数。构建/采集脚本为同目录的 `support/host_diagnostic.sh`。
本次证据写到 V002 的 `results/local-protocol-20261008/`，旧结果保持原样。

远端唯一操作范围为：

```text
/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x/V002/
```

## 实际结果

本次只执行上述一次 P/P，共 176 个计时调用；两地址采样前后的 reference 均通过。
device-event 与 kernel-task 的资格均未满足，未启动 P/C，未追加第二项实验。
全部数值、真实地址、task ID、首样本、长尾和精确下一动作见
`本地实验/W4-R11/V002/RESULT.md` 的“RunLocal 双地址 P/P 有限诊断”节。
初次环境路径失败、随后成功构建及原 runner/两库未重建的来源均已保留。
新增性能版 0、有效 Local 0、连续无改善贡献 0，CURRENT_LOCAL_BEST=NONE。
事件在提交后发送；不写共享 TSV。
