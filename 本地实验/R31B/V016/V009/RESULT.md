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

- Main 分配 NPU3 lease：`N04-V009-D3-CORR-LOCAL-20261016T124459Z`；本路线未改共享调度记录。
- 运行前快照 `2026-10-16T12:57:53Z`：NPU3 Health=OK，HBM=65536 MiB、Usage Rate=5%，AICore=0%；`proc-mem` 返回 `No process in device.` 原始记录：[server3-before-npu3-20261016.txt](server3-before-npu3-20261016.txt)。此前 `npu-smi info proc -i 3` 返回设备不支持该查询，随后使用 `proc-mem` 获取进程状态；未据不支持的查询推断空闲。
- CANN 环境来自 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`；复用构建目录 `/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build` 中的 `paired_runner_v016`。
- Correctness 命令：`./paired_runner_v016 --correctness --device 3`。开始/结束：`2026-10-16T12:58:34Z`；Runner RC=1。动态加载器在 Runner 进入测试前报错：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64/libstdc++.so.6: version 'GLIBCXX_3.4.29' not found (required by ./paired_runner_v016)`。完整原始输出：[server3-v009-correctness-device3-20261016.txt](server3-v009-correctness-device3-20261016.txt)。
- 四组 Correctness 均未开始，结果为 NONE；未重跑失败命令。Local 未启动，结果为 NONE。
- Postflight `2026-10-16T12:59:19Z`：NPU3 Health=OK、HBM Usage Rate=5%、AICore=0%，`No process in device.` 原始记录：[server3-after-npu3-20261016.txt](server3-after-npu3-20261016.txt)。未调用设备 Reset，未触碰其他进程。

## 设备资源快照

- Main 于 `2026-10-16T12:18:39Z` 报告 NPU0–7 均有现存 Python/VLLM 进程，其中 NPU3 AICore=29%。
- NPU7：Health=OK，HBM=5298/65536 MiB，AICore=8%；Python PID 1712355 使用 1921 MiB。
- Main 约于 `2026-10-16T12:39Z` 的最新只读快照仍显示 NPU0–7 均有现存进程；该状态来自 Main 报告，本路线未连接服务器或采样设备。
- 上述为历史快照；本次 lease 和运行前/后状态见“Correctness 与 Local”。
