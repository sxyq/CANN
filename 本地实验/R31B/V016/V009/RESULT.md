# N04 V009 进展

## 假设与改动

- 父版本：V008，当前结果提交 `67f58ec8`。
- 目标范围：FP16、rows=40、D=32768，且当前 Vector Core 有下一批次。
- 单一变化：在 Pass2 最后一块参数已就绪后，等待已不用的 A 参数槽释放，把下一批次首个 x/residual tile 搬入 A；MTE2 搬运与当前最后 tile 的 Vector 应用重叠。下一批次 Pass1 等待该搬运事件后直接消费 A。
- 此 shape 的 UB 预算让 `wideFullYRows_` 取 1，tile 宽 4096，D=32768 共 8 个 tile。现有 Pass1 的双缓冲预取只覆盖本批 `units`，最后一个 unit 没有下一 unit 可预取；V009 把下一批首 tile 的 GM→UB 搬运提前到 Pass2 末 tile。
- V006 的输出写回事件、V007 的首参数加载与 V008 的后续参数提交保持原样。

## Compile

- 状态：未运行。
- 当前本机为 Darwin；PATH 中没有 `cmake`，路线配置的 CANN `bisheng` 不存在于本机。已有 CMake 入口要求 CANN 8.5.0.alpha002 的 Linux/AArch64 编译器；Clone 中没有 V008 本地构建缓存。
- 未连接 cann-server3；未运行任何设备。当前环境没有可用的本地 Ascend C 编译入口，因此没有编译返回码或编译日志。

## Correctness 与 Local

- 未运行；候选尚未 Compile。
- 当前不需要设备。若编译环境恢复且需要实测，再由 Main 分配设备窗口。
