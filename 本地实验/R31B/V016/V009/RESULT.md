# N04 V009 进展

## 假设与改动

- 父版本：V008，当前结果提交 `67f58ec8`。
- 目标范围：FP16、rows=40、D=32768，且当前 Vector Core 有下一批次。
- 单一变化：在 Pass2 最后一块参数已就绪后，等待已不用的 A 参数槽释放，把下一批次首个 x/residual tile 搬入 A；MTE2 搬运与当前最后 tile 的 Vector 应用重叠。下一批次 Pass1 等待该搬运事件后直接消费 A。
- 此 shape 的 UB 预算让 `wideFullYRows_` 取 1，tile 宽 4096，D=32768 共 8 个 tile。现有 Pass1 的双缓冲预取只覆盖本批 `units`，最后一个 unit 没有下一 unit 可预取；V009 把下一批首 tile 的 GM→UB 搬运提前到 Pass2 末 tile。
- V006 的输出写回事件、V007 的首参数加载与 V008 的后续参数提交保持原样。

## Compile

- 状态：PASS，CANN 8.5.0.alpha002，目标配置 Ascend910B3。
- 主机：`hwnput3`，通过 `cann-server3` 访问。
- 复用远端 paired 源目录 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/本地实验/R31B/V016/support/paired` 和已有构建目录 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build`。源目录中的 `V009/Candidate.asc` 已对应本地版本；paired CMake include 指向 `../../V009`。
- 环境：`set +u` 后 source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，再恢复 `set -u`；`CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward`。`LD_LIBRARY_PATH` 将 `/usr/lib/aarch64-linux-gnu` 放在工具链路径前。
- 开始：`2026-10-16T12:11:27Z`；结束：`2026-10-16T12:11:54Z`。
- 配置命令：

  ```sh
  /usr/bin/cmake -S /tmp/cann-n04-dma-pipeline-overlap-20261016/V008/本地实验/R31B/V016/support/paired -B /tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build
  ```

  配置 RC=0。
- 构建命令：

  ```sh
  /usr/bin/cmake --build /tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build --target paired_runner_v016 --parallel 1 -- -B
  ```

  Build RC=0，Compile RC=0；构建产物为 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build/paired_runner_v016`。
- CMake 输出 `libstdc++.so.6` runtime search-path warning；目标完成编译与链接。完整命令记录和原始输出：[server3-compile.log](server3-compile.log)。

## Correctness 与 Local

- 首轮 lease：`N04-V009-D3-CORR-LOCAL-20261016T124459Z`。`./paired_runner_v016 --correctness --device 3` 于 `2026-10-16T12:58:34Z` 以 RC=1 在动态加载阶段退出，所需 `GLIBCXX_3.4.29` 未从 CANN 工具链 `libstdc++.so.6` 找到；四组测试未开始，Local 未运行。首轮输出：[server3-v009-correctness-device3-20261016.txt](server3-v009-correctness-device3-20261016.txt)；运行前快照：[server3-before-npu3-20261016.txt](server3-before-npu3-20261016.txt)；postflight：[server3-after-npu3-20261016.txt](server3-after-npu3-20261016.txt)。
- 本轮 lease：`N04-V009-D3-ENVFIX-20261016T130447Z`。运行前快照 `2026-10-16T13:07:57Z`：NPU3 Health=OK、HBM 容量=65536 MiB、Usage Rate=5%、AICore=0%；`proc-mem` 返回 `No process in device.` 原始记录：[server3-before-npu3-envfix-20261016.txt](server3-before-npu3-envfix-20261016.txt)。
- source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh` 后，将 `/usr/lib/aarch64-linux-gnu` 前置到 `LD_LIBRARY_PATH`；复用 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build/paired_runner_v016`，源码未修改。
- 单次命令：`./paired_runner_v016 --paired --device 3`。开始 `2026-10-16T13:08:37Z`，结束 `2026-10-16T13:08:45Z`，RC=0；每组 45 次 warmup、21 对。完整 Correctness/Local 原始输出：[server3-v009-paired-device3-envfix-20261016.txt](server3-v009-paired-device3-envfix-20261016.txt)。
- Runner 每组先执行配对采样，再做该组 Correctness；若比对失败会退出，不再运行后续 shape。本轮四组均通过。
- Parent 与 Candidate 的 Correctness：4/4 shape PASS，failures=0。

Local delta 为 Candidate−V011，单位 μs；负值表示 Candidate 用时较短。

| Shape / dtype | Parent median | Candidate median | Delta median | Delta P10–P90 |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 20.540 | 20.900 | -0.040 | -1.800–4.820 |
| rows=40, D=32768 / FP16（目标） | 49.060 | 42.780 | -6.520 | -7.160–-5.240 |
| rows=40, D=12288 / BF16 | 29.500 | 27.180 | -2.180 | -5.780–7.560 |
| rows=40, D=32768 / BF16 | 69.600 | 61.540 | -8.260 | -10.940–2.300 |

- 目标 FP16 D=32768 的 P10–P90 全负；21 对全样本 min/max 为 -24.500/+16.960 μs。其余三组 P10–P90 跨零。此次目标 shape 显示负向中位数和负向 P10–P90，但样本尾部有较大波动；未运行 Official。
- Postflight `2026-10-16T13:09:48Z`：NPU3 Health=OK、HBM Usage Rate=5%、AICore=0%，`No process in device.` 原始记录：[server3-after-npu3-envfix-20261016.txt](server3-after-npu3-envfix-20261016.txt)。未调用设备 Reset，未触碰其他进程；未修改共享调度记录。

## 设备资源快照

- Main 于 `2026-10-16T12:18:39Z` 报告 NPU0–7 均有现存 Python/VLLM 进程，其中 NPU3 AICore=29%。
- NPU7：Health=OK，HBM=5298/65536 MiB，AICore=8%；Python PID 1712355 使用 1921 MiB。
- Main 约于 `2026-10-16T12:39Z` 的最新只读快照仍显示 NPU0–7 均有现存进程；该状态来自 Main 报告，本路线未连接服务器或采样设备。
- 上述为历史快照；本次 lease 和运行前/后状态见“Correctness 与 Local”。
