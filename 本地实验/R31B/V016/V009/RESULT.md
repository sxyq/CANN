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

- 未运行；本次只 Compile，未启动 Runner，也未使用设备。Correctness 和 Local 结果仍为 NONE。
- 未申请设备；本轮没有可用设备窗口。

## 设备资源快照

- Main 于 `2026-10-16T12:18:39Z` 报告 NPU0–7 均有现存 Python/VLLM 进程，其中 NPU3 AICore=29%。
- NPU7：Health=OK，HBM=5298/65536 MiB，AICore=8%；Python PID 1712355 使用 1921 MiB。
- 因设备均有现存进程，V009 的 Correctness 与 Local 未运行；没有启动 Runner，也没有申请设备。
- 等 Main 后续分配空闲设备窗口后再进行设备测量。
