# N04 V012 结果

## 假设与单一变化

- 父版本：V011，源码结果提交 `8fa17527`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行 Pass 1/Pass 2 交界。
- 单一变化：最后一个 Pass 1 输入 tile 已提交后，复用已完成的 `rd0` MTE2→V 事件，把 gamma/bias 的首个参数 tile 搬入第三输入槽；该搬运与最后输入 tile 的等待、Vector 计算及后续标量归约并行，Pass 2 直接消费该参数 tile。其余参数仍沿用 V011 的三槽预排，V009 的跨批次首输入预取和 V010 的双输出槽保持不变。
- 与既有版本的区别：V009 预取下一批 x/residual，V010 重叠输出 MTE3，V011 在 Pass 2 内预排参数；V012 将首参数 DMA 前移到 Pass 1 末端。

## 服务器事务

- Compile：待执行。
- Correctness：待执行。
- Local：待执行。
- Official：不执行。
- 设备：沿用已有 paired Runner；不调用设备 Reset。

STATUS: SOURCE_READY_COMPILE_PENDING
LOCAL_SCORE: NONE
