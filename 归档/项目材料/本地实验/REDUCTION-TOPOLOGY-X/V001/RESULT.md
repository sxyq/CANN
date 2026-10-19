# N03 REDUCTION-TOPOLOGY-X V001

日期：2026-10-16
状态：本轮 Compile、Correctness、Local 已完成；结果不支持稳定提速，等待本地提交。

## HYPOTHESIS

对 FP16/BF16 宽行 RMS Sum(y²)，保持输入 tile 长度 4096、每个 tile 的 FP32 `ReduceSum`、MTE/参数阶段和分核方式不变，把 tile partial 写入 8-float 对齐槽，再在同一 UB 中用二叉 pairwise `Add` 合并。假设更短的部分和依赖链能降低长行归约的 Vector 延迟。

## ONE CHANGE

- 父源码只读保留：`线上结果/R31B/V011/submission.asc`。
- 路线源码：`本地实验/REDUCTION-TOPOLOGY-X/V001/Candidate.asc`。
- Candidate 只改低精度宽行归约的 partial 槽步长和合并组织：partial stride=8、reduce stride=128；tile 宽度、行分配、MTE、参数驻留和算术精度路径保持原样。
- 沿用公开 paired 入口：`本地实验/R31B/V016/support/paired/paired_runner.cpp`、`paired_runner_abi.h` 和 `CMakeLists.txt`。绑定已改为本 Clone 内的父源码和 Candidate 路径；wrapper 仍以 `submission.asc` 与 `Candidate.asc` 分别编译两套符号。

源码身份：

- Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA-256：`09610030dec1e74905c63349bc846dc3ce2449bbf9aa74f48b5848f80a7b3ab2`

## COMPILE

- 目标：`paired_runner_v016`
- 设备编译输出：`build_out/n03-reduction-topology/compile-h1-aligned.log`
- 返回码：0；日志末行为 `Built target paired_runner_v016`。
- 输入身份：上述 Parent 与 Candidate；父文件在本轮保持未修改。
- 已记录的 server3 构建输出只保存了 CMake 构建阶段，未保存远端 shell wrapper 的完整路径。
- 本 Clone 内复现命令：`cmake -S 本地实验/R31B/V016/support/paired -B build_out/n03-reduction-topology/cmake-build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；随后执行 `cmake --build build_out/n03-reduction-topology/cmake-build --target paired_runner_v016 --parallel 1`。两条命令均返回 127，原因是当前本地 shell 没有 `cmake`；这不改变已有 server3 Compile PASS 记录。

## CORRECTNESS

- 入口：现有 paired Runner；另有 `build_out/n03-reduction-topology/correctness-v011.log` 的同二进制重复运行。
- 设备：NPU 4。
- 4 个样例均通过：FP16 D12288、FP16 D32768、BF16 D12288、BF16 D32768。
- paired 日志中 Parent/Candidate 每个样例均为 `CORRECTNESS ... result=PASS failures=0`。
- 同二进制重复运行也为 4/4 PASS；主要误差记录为 FP16 最大绝对误差 0.001953125、BF16 最大绝对误差 0.0078125。

## LOCAL

- 入口：现有 paired Runner 的 `--paired` 模式。
- 设备：NPU 4；每个样例 warmup=45、交错 pairs=21，每个二进制 21 个 device-event 样本；device-event 为主指标，wall time 仅作诊断。
- 负载：运行时观察到 NPU 4 有 VLLM 进程，且样本中存在明显离群值；因此只报告中位数，不把单次下降当作稳定收益。

| 样例 | Parent median (us) | Candidate median (us) | Candidate - Parent (us) |
|---|---:|---:|---:|
| FP16 D12288 | 11.800 | 12.220 | +0.460 |
| FP16 D32768 | 25.500 | 20.300 | -4.240 |
| BF16 D12288 | 12.260 | 12.460 | 0.000 |
| BF16 D32768 | 21.500 | 25.380 | +2.060 |

结果文件：`build_out/n03-reduction-topology/local-v001-h1-aligned.log`。

## RESULT

H1 通过编译和精度运行，但 Local 没有形成跨 dtype/shape 的稳定收益：仅 FP16 D32768 的中位数下降 4.240 us，BF16 D12288 持平，另外两项上升。当前结论为“正确性通过，性能假设未证实”。

## COMMIT

待把本轮 Candidate、paired 绑定和本记录作为一个本地提交；不推送远端，不提交 Official。

## NEXT

停止本 V001 的代码扩展。下一轮若继续，只能基于新的一因素假设，并保持父源码路径与 paired ABI 不变。
