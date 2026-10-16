# N04 V008 进展

## 假设

- 父源码：V006，提交 `0dd0d828`；V007 的首 gamma/bias tile 提前加载不纳入此候选。
- 单一变化：在 `ProcessWideLowPrecision` 的 Pass 2 中，先向备用槽提交下一组 gamma/bias 的 GM→UB 搬运，再等待当前组的 `MTE2_V` 完成。
- 备用槽覆盖前仍等待对应 `V_MTE2` 释放事件；当前参数仍要等自己的 `MTE2_V` 后才由 Vector 读取。
- 机制判断：让下一组异步 MTE2 参数搬运的请求在当前组等待期间即可排队。它和 V005 的 Pass 1 x/residual 提交次序作用于不同数据流，也不复用 V007 的首组参数提前加载。
- 源码提交：`20ba6b60`。

## 编译

- CANN 8.5.0.alpha002，Ascend910B3；现有 paired Runner 与 CMake 入口复用成功。
- CMake 配置：PASS，RC=0；编译/链接：PASS，RC=0。
- 构建产物：`/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build/paired_runner_v016`。
- 原始记录：[server3-configure.log](server3-configure.log)、[server3-compile.log](server3-compile.log)。配置输出有 `libstdc++.so.6` runtime search-path warning，目标已成功链接。

## Correctness 与 Local

- 尚未运行。当前 Clone 的 `调度/服务器设备使用.tsv` 没有有效的 N04 V008 设备分配；因此未采 NPU 快照、未启动 Runner，也没有 Correctness 或 Local 样本。
- 后续计划沿用现有 Runner：rows=40，FP16/BF16，D=12288/32768，四组 Correctness 与每组 21 对 Local。需 Main 分配独占设备窗口后，再做启动前快照。
- 当前 `LOCAL_SCORE=NONE`；Official 未运行。
