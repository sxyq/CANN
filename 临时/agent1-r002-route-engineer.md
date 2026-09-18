# FULL-R002-RETAINED-Y V001

## Hypothesis

对每个展平后的 outer 行，第一阶段按 D 方向分段只从 x 和 residual 各搬入一次，在 FP32 中形成 y，并把 y 写入 UB 的 retained-y cache。第一阶段同时累加 y*y，得到该行的 RMS；第二阶段从 retained-y cache 直接读取 y，完成归一化、gamma/bias 和输出。

当一行超过 UB cache 容量时，cache 只保留当前分段；第一阶段把已经形成的 y 暂存到 output GM，第二阶段只回读该 retained-y 暂存到 UB 再消费，仍不回读 x/residual。output GM 的暂存生命周期覆盖当前行的两个阶段，第二阶段最终写回 output。

主要技术点：

- 单核拥有完整行，按 blockIdx 循环分配 outer 行，不在行内跨核归约。
- `kTileElems=4096`，`kRetainedYElems=24576`；小行保留整行，多段行复用一个 UB 分段。
- x/residual native buffer、FP32 转换/平方工作区和 retained-y cache 各自有明确生命周期，不使用 ping-pong 输入流水。
- 第一阶段先形成 y，再计算 y*y 和 ReduceSum；平方工作区不覆盖 retained-y。
- 第二阶段先用 retained-y 做除法，再复用输入与 FP32工作区搬入 gamma/bias；尾段只按有效元素处理。
- FP16/BF16 的中间归约保持 FP32；宽行暂存到 output GM 时使用目标 dtype，舍入影响需要真机精度证据。

## Minimum acceptance cases

1. FP16、2D、outer=2、D=64：确认两行各自只映射到一个 core，第二阶段不存在 x/residual 搬运。
2. FP32、2D、outer=1、D=4097：确认完整段 + 尾段、cache 内整行保留和输出边界。
3. BF16、3D/4D 展平 outer、D=24577：确认 cache 窗口切换、output GM retained-y 暂存和尾段不越界。
4. FP16/FP32、D=32768：确认多段 sum 合并、宽行只回读 retained-y，不回读 x/residual。
5. D=64/67/4097 等非 32B 行宽：确认 `DataCopyPad` 有效字节数与 UB 偏移分离，写回不覆盖相邻行。

## Scope and status before editing

- 只允许修改 `提交/单方案/FULL-R002-RETAINED-Y/V001/kernel.txt` 与本文件。
- R013 的 ping-pong pipeline、R014 的 parameter residency、multi-row DMA、reciprocal、wide cached-row、manual-tail、alignment row-group 和 R031 multimode 不作为本路线的主要机制。
- 当前 Mac 未发现 CANN/Ascend C 编译器与 `kernel_operator.h`；源码完成后仅能做静态结构核对，CANN/NPU/性能状态待真机环境确认。
