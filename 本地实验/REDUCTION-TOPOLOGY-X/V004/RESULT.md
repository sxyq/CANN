# N03 REDUCTION-TOPOLOGY-X V004

日期：2026-10-16
状态：Compile PASS；配对 Runner 已执行。FP16 D12288、FP16 D32768、BF16 D12288 的 Correctness PASS；BF16 D32768 未完成。受设备负载影响，LOCAL_SCORE=NONE。

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

## 设备验证与配对结果

- Main 分配：NPU2，lease 起始 `2026-10-16T06:25:43Z`；主机 `hwnput3`，SoC Ascend910B3。
- 执行时间：2026-10-16 `06:37:12Z` 至 `06:37:21Z`。
- 环境：CANN 8.5.0.alpha002；沿用现有构建记录中的初始化方式，临时关闭 nounset 后 source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，返回码 0；`LD_LIBRARY_PATH` 前置 `/usr/lib/aarch64-linux-gnu`。
- Runner：`/tmp/cann-n03-reduction-topology-v004-20261016/build/paired_runner_v016`，由现有 V004 编译输出运行；代码执行 `aclrtSetDevice(2)`，不调用设备 reset。
- 命令：`./paired_runner_v016 --paired --device 2`。
- Runner 返回码：2。执行器完成前三种 shape，在 `bf16-wide-d32768` 输出 `CASE_BEGIN` 后退出；未重跑、未 reset。
- 原始 stdout（含逐样本输出、命令、返回码和前后设备快照）：[`correctness-local-device2-20261016T0635Z.log`](correctness-local-device2-20261016T0635Z.log)。Runner 的 stderr 未进入该文件，故保存记录无法确认具体 ACL 错误码；本轮不据此声称再次出现 507035。

### 设备负载

| 快照 | Health | HBM | AICore | AIVector | HBM 带宽 | 进程 |
|---|---|---:|---:|---:|---:|---|
| Pre `06:37:12Z` | OK | 19% / 65536 MiB | 0% | 2% | 0% | Python PID 1583166，9662 MiB；无 N03 Runner |
| Post `06:37:21Z` | OK | 19% / 65536 MiB | 0% | 1% | 46% | 同一 Python PID，9662 MiB；无 N03 Runner |

更早的 `06:35:30Z` 快照曾读到 AICore 27%、AIVector 18%；运行前后同一 Python 任务仍在使用 NPU2。样本中可见明显离群值，因此下表只保留采样统计，不作性能结论，所有 shape 的 `LOCAL_SCORE=NONE`。

### Correctness 与 Local 摘要

| Shape | Correctness | V011 device_us median | V004 device_us median | 配对 delta median（V004−V011） | Local |
|---|---|---:|---:|---:|---|
| FP16 D12288 | V011 / V004 PASS，failures=0 | 7.780 | 8.080 | -0.040 | 21 pairs；受负载影响，NONE |
| FP16 D32768 | V011 / V004 PASS，failures=0 | 12.720 | 12.860 | 0.060 | 21 pairs；受负载影响，NONE |
| BF16 D12288 | V011 / V004 PASS，failures=0 | 8.540 | 8.220 | -0.200 | 21 pairs；受负载影响，NONE |
| BF16 D32768 | 未完成；进入 shape 后 Runner 返回 2，未见样本或 Correctness 行 | NONE | NONE | NONE | 未完成，NONE |

前三种 shape 均按每个 kernel 45 次 warmup、21 组 P/C 配对执行。逐样本 device/wall 时间及完整统计见上述 stdout 日志。BF16 D32768 的退出错误细节未保存在 stdout 日志中，具体 ACL 状态为 UNKNOWN；设备健康在 post 快照仍为 OK。没有性能结论。
