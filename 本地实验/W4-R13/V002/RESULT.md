# W4-R13 V002 结果

```text
ROUTE=W4-R13-WIDE-FP32-CACHE-TAIL-X
REVISION=V002
DIRECT_PARENT=V001 at 65a9912b6c3e59d1722ad921a9bd0c737aec6237
PARENT_SOURCE=研究/W4-R13/parent-tail-probe/submission_v001.asc
CANDIDATE_SOURCE=研究/W4-R13/parent-tail-probe/submission_v002.asc
SOURCE_SHA256=3ebe51ced0cdcf6b960765c8a43c4595dc9e445362586e3bc15cbf128723afc0
FOCUS_AXIS=wide-FP32 inverse-RMS evaluation
SINGLE_CHANGE=FullCacheRows 使用 AscendC::Rsqrt，并对近似倒数平方根执行一次标量 Newton 精度补偿；V001 的连续 Store 未改动
```

## 变化依据

R13 已有 `ProcessWideFp32FullCacheRows` 与 `121x9216 FP32` 实测入口。V001 唯一性能变化是每批一次连续 Store。本版只改该路径的 inverse-RMS 求值：用向量 `Rsqrt` 加一次标量 Newton 精度补偿替代 `Sqrt` 加标量倒数。tile、参数加载、归约、同步、Store 粒度及其它 dtype 路径保持不变。未从 Official 分数推测任何输入映射。

首轮 `Rsqrt` 结果未达到本地参考容差，失败日志和输出已保留。抽样行反推的近似比例约为 `0.99934–1.00243`；对同一算法加入一次 Newton 精度补偿后，完整 `121x9216` 输入通过。该补偿只处理本次变化的 FP32 数值精度。

## Compile

```text
HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3 / dav-2201
TARGET=/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/build/w4r13_ref_candidate_v002_probe
COMPILE=PASS, exit code 0
```

首轮构建日志为 `evidence/candidate-v002/compile.log`；加入 Newton 精度补偿后的最终构建日志为 `evidence/candidate-v002/compile-fix01.log`。输出中保留 runner 原有的 `GM_ADDR` 属性与 printf 格式告警，目标均成功链接。

## Correctness

设备 3，FP32 `121x9216`，runner 使用相同输入生成方式并逐元素比较。首次执行 `bad=788838`、`max_abs=0.00730801`，日志、统计、raw TSV 与输出文件均保留。加入一次 Newton 精度补偿后，完整 `1,115,136` 个元素 `bad=0`，`max_abs=2.36034e-05`，runner 返回码 0；该输入容差为 `1e-4 + 1e-4*abs(reference)`。

首轮失败证据在 `evidence/candidate-v002/candidate-v002-correctness-*`；最终通过证据在 `evidence/candidate-v002/candidate-v002-correctness-fix01-*`。

## Local

同设备、同 runner、同输入 `121x9216 FP32`，device event 计时；每次 warmup 5 次、采样 25 次。顺序为 V001 A、V002 A、V001 B、V002 B。V001 和 V002 各 50 个原始样本。

| 分段 | V001 median | V002 median | V002 相对变化 |
|---|---:|---:|---:|
| A | 39.52 us | 38.32 us | -3.04% |
| B | 34.58 us | 40.36 us | +16.72% |
| 合计 50 样本 | 38.11 us | 38.60 us | +1.2858% |

`LOCAL_SCORE=38.60 us`，`LOCAL_DELTA=+1.2858%`（正值代表更慢）。合计设备样本 CV：V001 `0.3456`、V002 `0.3663`。两个分段方向相反，因此重复性不足；`CURRENT_LOCAL_BEST=NONE`。

四份 raw TSV、逐段统计、命令日志与运行前后快照均保存在 `evidence/candidate-v002/local/`：`v001-a-*`、`v002-a-*`、`v001-b-*`、`v002-b-*`。设备为 NPU 3；测量期间 HBM 容量 `65536 MB`、使用率 `90%`、折算 `FREE_HBM=6553 MB`；AICore `66–67%`、AIVector `34–35%`、HBM 带宽 `70%`，load average 约 `47.57 / 35.67 / 39.99`。快照显示 VLLMEngineCor 进程占用 `56142 MB`；未对其它进程或服务作任何操作。

四次远端运行均为退出码 0。运行结束后的本地 console 汇总因 heredoc 结束行写法错误而返回 1；四份逐次命令日志、raw TSV、统计和设备快照已完整保存，Local 数字取自这些逐次文件。

## Official 与交接

V001 Official Pass `15/15`、`42.45` 为用户提供的事实。V002 尚未提交 Official，Official Score=`NONE`。V002 源码身份与 V001 不同，且本版未复用 V001 连续 Store 变化。

```text
READY_STATE=SOURCE_AND_EVIDENCE_COMPLETE
COMPILE=PASS
CORRECTNESS=PASS_AFTER_ONE_SAME_VERSION_PRECISION_FIX
LOCAL_SCORE=38.60 us
LOCAL_DELTA=+1.2858%
CURRENT_LOCAL_BEST=NONE
OFFICIAL_SUBMITTED=NO
ONLINE_OWNER_NEXT_ACTION=按候选就绪时间进入唯一 Official 队列
```
