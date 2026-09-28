# Phase4 Round1 Architecture Plan

目标平台：Ascend 910B3 / DAV_2201 / CANN 8.5.0.alpha002。

Round1 固定六条路线：

- A001：Row-Resident Fused Streaming。单 Core 独占完整 row，使用一维 row slots 和核内 reduction。
- B001：True 2D Panel DMA and Broadcast。二维 panel 是基本计算单位，使用 panel-level DMA、参数广播和多行 reduction。
- C001：Cooperative D-Slice Reduction。row-group 内多个 Core 协作同一 row，D slice 产生 partial sum 后合并。
- D001：Persistent Channel-Stripe Sweep。固定 D stripe 跨 row batch 持久运行，参数按 stripe 长期复用。
- E001：Primitive-Assisted RmsNorm。验证 DAV_2201 上 RmsNorm 高阶 API，外围融合 residual add 和 bias add，并为不支持域建立 private fallback。
- F001：Unconstrained DAV_2201 Clean-Slate。Fresh Child 只接收共同事实，自行决定全部执行拓扑。

共同执行限制：

- 每个候选覆盖完整合法输入域。
- 每个候选独立实现 private generic fallback。
- C/D 自行选择实际可编译的跨核同步 API。
- 每个候选生成自己的 `architecture-metadata.md`。
- 六个 terminal 以前 Main 不做横向 metadata 或源码分析。
- Round1 禁止 A002-F002、Mix、Router 和参数 sweep。

