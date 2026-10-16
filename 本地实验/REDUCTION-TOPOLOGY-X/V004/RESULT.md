# N03 REDUCTION-TOPOLOGY-X V004

日期：2026-10-16
状态：Compile PASS；Correctness / Local 未执行，等待 Main 协调 NPU 资源。

## 观察与假设

V002、V003 的 FP16 D12288（3 个 tile，1 个块）Candidate 均通过；D32768（8 个 tile，2 个块）Candidate 均在 warmup 同步时返回 507035。两版的块间合并分别为 `ReduceSum` 与平衡 `Add`，现有证据尚不能确定故障指令或根因。

V004 将局部块宽从 4 个 tile 改为 8 个。对 D32768，八个 tile partial 由一次 `ReduceSum` 归约，不再产生第二块结果或块间合并。该实验检验故障是否与多块归并结构相关，不宣称已知 507035 的原因。其余算术、tile 宽度、输入输出及数据搬运保持不变。

## 文件与入口

- 父版本：V003。
- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V004/Candidate.asc`。
- 配对工程：`本地实验/R31B/V016/support/paired/CMakeLists.txt`，绑定到 V004。
- Runner：沿用 `paired_runner_v016`；移除退出时的设备 reset，新增可选 `--device ID` 参数，默认值仍为 4。
- NPU：尚未启动。启动前需 Main 分配设备并确认运行窗口。拟先验证 FP16 rows=2,D32768；若同步通过，再跑其余配对样例。现有 Runner 每个模式固定覆盖 FP16/BF16 的 D12288、D32768 四个样例，warmup=45；配对模式另有每样例 21 对测量。预计完整配对运行约 10–20 分钟，受设备负载影响。
- Correctness / Local：NONE，尚未执行。

## Compile

- 主机：`hwnput3`；SoC：Ascend 910B3；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- 命令：source 工具链 `set_env.sh`，设置与 V003 相同的 `CPLUS_INCLUDE_PATH`，随后执行：

  `cmake -S /tmp/cann-n03-reduction-topology-v004-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v004-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V016_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v004-20261016/本地实验/REDUCTION-TOPOLOGY-X/V004 && cmake --build /tmp/cann-n03-reduction-topology-v004-20261016/build --target paired_runner_v016 --parallel 1`

- 返回码：0；输出：`[100%] Built target paired_runner_v016`。
- 完整编译日志：`build_out/n03-reduction-topology/compile-v004-server3.log`。
- 构建目录与 Runner：`/tmp/cann-n03-reduction-topology-v004-20261016/build`。

## 结果

无性能结论。507035 的具体设备指令根因 UNKNOWN。V004 硬件正确性待设备资源协调后验证；Runner 已支持 `--device ID`，退出时不会 reset 设备。
