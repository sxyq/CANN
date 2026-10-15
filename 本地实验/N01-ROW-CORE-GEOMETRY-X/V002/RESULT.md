# N01 ROW-CORE-GEOMETRY-X V002

## HYPOTHESIS

V001 的两行/核在 D32768、Rows=2/4/8 均出现正的配对设备时间差。V002 改测更粗的四行/核：对 `D >= 16384` 使用 `min(availableCoreNum, ceil(Rows / 4))`，观察更少活跃核与更大行组是否能摊薄宽行固定开销。Dtype、D、Rows 控制组沿用 V001。

## V001 INPUT

- 已读取 [V001 RESULT](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V001/RESULT.md)。
- 已读取 V001 Parent/Candidate diff：唯一变化为 `D>=16384` 时目标两行/核的 `blockCount` 计算。
- V001 提交：`4c4fe193`。

## ONE CHANGE

- Parent：`本地实验/N01-ROW-CORE-GEOMETRY-X/V002/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`
- Candidate：`本地实验/N01-ROW-CORE-GEOMETRY-X/V002/candidate/N01-ROW-CORE-GEOMETRY-H2_kernel.asc`
- 唯一 kernel 差异：`run_kernel` 的宽行 `blockCount` 目标从 V001 的两行/核改为四行/核。
- D32768、8 核时的几何映射：Rows 1→1/1、2→2/1、4→4/1、8→8/2（Parent/Candidate blocks）。
- 算术、Tile、同步、参数加载、设备侧行归属公式均未改动。

## COMPILE

- server3 Configure + Build 命令：
  `export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake -S /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V002/paired -B /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V002/paired/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 && cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V002/paired/build --parallel 2`
- 返回码：0。
- 产物：`paired_runner_n01` 成功生成。

## CORRECTNESS

- Runner：复用 V001 paired Runner/ABI，device 0；12 组用例。
- 用例：FP16/BF16；D=12288 控制组 Rows=2/8；D=32768 宽行组 Rows=1/2/4/8。
- Parent 资格：12 组均完成 Correctness=PASS；其中多组同二进制稳定性受负载影响，原始状态保留。
- 配对运行：12/12 Parent Correctness=PASS，12/12 Candidate Correctness=PASS，进程返回码 0。
- 原始日志：[qualify-v011.log](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V002/qualify-v011.log)、[paired.log](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V002/paired.log)。

## LOCAL

设备事件中位数及配对差值（Candidate - Parent）：

| 用例 | Parent us | Candidate us | 配对差值 us | 中位数相对变化 |
|---|---:|---:|---:|---:|
| fp16-tail-d12288-r2 | 19.100 | 17.400 | -1.020 | -8.90% |
| fp16-tail-d12288-r8 | 12.300 | 10.840 | -0.220 | -11.87% |
| fp16-wide-d32768-r1 | 10.360 | 15.740 | -0.220 | +51.93% |
| fp16-wide-d32768-r2 | 17.540 | 17.300 | +6.900 | -1.37% |
| fp16-wide-d32768-r4 | 17.860 | 30.920 | +17.560 | +73.12% |
| fp16-wide-d32768-r8 | 10.980 | 31.460 | +20.660 | +186.52% |
| bf16-tail-d12288-r2 | 17.780 | 16.820 | -0.780 | -5.40% |
| bf16-tail-d12288-r8 | 7.280 | 7.300 | -0.040 | +0.27% |
| bf16-wide-d32768-r1 | 13.820 | 13.740 | -0.160 | -0.58% |
| bf16-wide-d32768-r2 | 13.920 | 24.720 | +10.920 | +77.59% |
| bf16-wide-d32768-r4 | 29.440 | 56.680 | +30.840 | +92.53% |
| bf16-wide-d32768-r8 | 16.240 | 46.940 | +31.520 | +189.04% |

负载上下文：device 0；开测前 HBM 使用 59%、AICore 0%、Aivector 0%、HBM 带宽 0%；结束快照已保存；两次均存在其他进程。[开测负载](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V002/load-before-device0.txt)、[结束负载](/Users/sunyiyang/Desktop/Project/cann/worktrees/new-n01-row-core-geometry/本地实验/N01-ROW-CORE-GEOMETRY-X/V002/load-after-device0.txt)。

## RESULT

H2 Correctness 通过，但 Local 没有支持四行/核收益的证据。D32768 的 Rows=2/4/8 在 FP16、BF16 中配对差值全部为正，且四行/核相较 V001 的两行/核没有改善。部分 Parent 同二进制资格出现 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`，因此不把控制组波动归因于 H2；宽行回退趋势在多组配对结果中仍一致。

`LOCAL_SCORE=NONE`。不重复 V001，不创建 V003；本路线当前没有新的证据支持的分核几何机制。

## COMMIT

本地提交，PUSH=NO；不执行 Official。

## NEXT

NONE。保留 V002 的源码、日志和负载快照，等待新的独立几何证据。
