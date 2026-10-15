# N01 ROW-CORE-GEOMETRY-X V001

## HYPOTHESIS

在 `D >= 16384` 的宽行条件下，把目标分块数改为 `min(availableCoreNum, ceil(Rows / 2))`，让每个 Vector Core 处理约两行；保持行归属公式、算术、Buffer、Tile、参数加载和同步不变。`D < 16384` 保持父版本分块方式，作为控制组。

## ONE CHANGE

- Parent：`线上结果/R31B/V011/submission.asc`
- Parent copy：`本地实验/N01-ROW-CORE-GEOMETRY-X/V001/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`
- Candidate：`本地实验/N01-ROW-CORE-GEOMETRY-X/V001/candidate/N01-ROW-CORE-GEOMETRY-H1_kernel.asc`
- 唯一 kernel 差异：`run_kernel` 的 `blockCount` 计算，Candidate 在 `rowWidth >= 16384 && rowCount > 1` 时使用目标两行/核。
- D32768、8 核时的几何映射：Rows 1→1/1、2→2/1、4→4/2、8→8/4（Parent/Candidate blocks）。

## COMPILE

- 本机命令：`cmake -S 本地实验/N01-ROW-CORE-GEOMETRY-X/V001/paired -B 本地实验/N01-ROW-CORE-GEOMETRY-X/V001/paired/build`；返回码 127；证据：本机 `cmake: command not found`。
- server3 Configure：CMake + Toolkit 8.5.0.alpha002；返回码 0。
- server3 首次 Build：返回码 2；证据：`ASCEND_HOME_PATH` 未传入独立 Build 进程，编译器报 `can not find ASCEND_HOME_PATH`。
- server3 第二次 Build：返回码 2；证据：插件注册代码报 `'vector' file not found`。
- 当前 CMake 增加 C++11 include 环境变量，并在 Build 进程显式导出同一变量后重新 Build：返回码 0，`Built target paired_runner_n01`。
- 最终构建对象：server3 `/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V001/paired/build/paired_runner_n01`。

## CORRECTNESS

- Runner：`paired_runner.cpp`，device 0，Parent 资格 12 组，每组 62 个设备事件样本；随后 Parent/Candidate 各 21 对交错样本。
- 用例：FP16/BF16；D=12288 控制组 Rows=2/8；D=32768 宽行组 Rows=1/2/4/8。
- Parent 资格：12/12 `SAME_BINARY_QUAL=PASS`，12/12 Parent Correctness=PASS。
- 配对运行：12/12 Parent Correctness=PASS，12/12 Candidate Correctness=PASS，进程返回码 0。
- 原始日志：[qualify-v011.log](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V001/qualify-v011.log)、[paired.log](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V001/paired.log)。

## LOCAL

设备事件中位数及配对差值（Candidate - Parent）：

| 用例 | Parent us | Candidate us | 配对差值 us | 中位数相对变化 |
|---|---:|---:|---:|---:|
| fp16-tail-d12288-r2 | 7.200 | 6.920 | +0.140 | -3.89% |
| fp16-tail-d12288-r8 | 6.800 | 14.900 | +0.380 | +119.12% |
| fp16-wide-d32768-r1 | 10.180 | 9.820 | -0.300 | -3.54% |
| fp16-wide-d32768-r2 | 25.320 | 56.100 | +20.720 | +121.56% |
| fp16-wide-d32768-r4 | 10.240 | 17.280 | +7.120 | +68.75% |
| fp16-wide-d32768-r8 | 16.640 | 17.280 | +4.320 | +3.85% |
| bf16-tail-d12288-r2 | 18.160 | 16.140 | -0.960 | -11.12% |
| bf16-tail-d12288-r8 | 30.740 | 18.700 | -8.340 | -39.17% |
| bf16-wide-d32768-r1 | 13.800 | 13.720 | -0.060 | -0.58% |
| bf16-wide-d32768-r2 | 13.960 | 24.660 | +10.680 | +76.65% |
| bf16-wide-d32768-r4 | 14.220 | 24.820 | +10.620 | +74.54% |
| bf16-wide-d32768-r8 | 17.300 | 25.340 | +9.660 | +46.47% |

负载上下文：device 0；开测前 HBM 使用 59%、AICore 0%、Aivector 0%、HBM 带宽 0%；结束后 HBM 使用 59%、AICore 0%、Aivector 0%、HBM 带宽 0%；两次均存在其他进程。原始结束快照：[load-after-device0.txt](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V001/load-after-device0.txt)。

## RESULT

H1 Correctness 通过。Local 观察中，未改变分块的 Rows=1 宽行接近 Parent；真正改变为两行/核的 Rows=2/4/8 宽行均出现正的配对设备时间差，当前窗口不支持 H1 的性能收益。控制组方向混合，且存在其他进程，因此保留原始数据，不将单次控制组差异归因于几何变化。

## COMMIT

本地提交，PUSH=NO；不执行 Official。

## NEXT

停止 H1 当前版本，不新增重复 Runner。后续若继续 N01，应沿用本目录和同一 paired 入口，优先选择更细的 rows-per-core 几何点，并保持 Dtype/D 控制组不变。
