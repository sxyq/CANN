# N05 当前候选设备记录

日期：2026-10-14

## 源码与构建

- Clone 起点与直接 Parent 提交：`3f5e67d4a3a2a8a39a99399e9a226ec39e847bf8`
- Parent：R31B V011，源码 SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate：`本地实验/N05-SCALAR-ADDRESS-GENERATION-X/candidate.asc`，SHA256 `3346b12b7f14e88e5188080f5810ec6327cd25338354f709435e4d676e58d817`
- 单因素变化：在 Pass1 预计算 `lastTile` 和 `tailValid`，供当前 tile 与预取 tile 选择有效元素数；Tile/Row 遍历、地址、数学、DMA 与 Vector 调度保持不变。
- 适用路径：`rowWidth > 8192` 的 FP16/BF16 宽行；本路线保存的 Judge Shape/Dtype 映射仍为 UNKNOWN。
- CANN 8.5 Full-link：PASS，`2026-10-14T05:49:49Z`。
- 依赖文件指向本目录的 Candidate 与 Champion Parent。对象 SHA256：Parent `a8d7738508bf4d6a61eb6651104396ce85be36a8834e7505aaf8fbb77d200b14`，Candidate `c75fcd8eebd10c1c93ddc7d5ca68e772fd5efce2baace404441341cef578f5ed`。链接库 SHA256：Parent `89a86ee883bb96259de41595e3deb05a148c22bc9d59576a4fb780fb04dea47f`，Candidate `d1c9a5ebb1c537f3cd4f88044eab230766042fbcef9643cdf7d03b74302bc1e0`。
- Runner SHA256：`40e16bede8f55e2cf0fb09b35488bf3f3bc9cd210db3d1f4cf11bb4bab9fa6ae`。Runner 内路线标签不作为源码身份依据。
- 复用 N02 已有 CANN 8.5 runner 源码与构建脚本；在 N05 专用临时目录完成编译，未新增 Runner 实现。

## Correctness 与 Local

N05 的 d2 设备快照 `2026-10-14T05:52:38Z`：HBM `63413/65536 MB`，AICore `12%`；Python PID `278936` 使用 `60043 MB`，并与其它卡上的进程同时运行。该设备有用户任务在执行，因此本轮未运行 NPU Correctness，也未做 Same-binary 或 paired Local。`CORRECTNESS=NONE`、`LOCAL_SCORE=NONE`；无性能结论。快照、构建日志和源码/对象身份记录保存在 `本地实验/N05-SCALAR-ADDRESS-GENERATION-X/logs/`。

本路线记录显示旧 Cursor 变体主要改地址推导；当前 Candidate 预计算尾 tile 有效长度，机制不同。但旧版本源码未包含在本 Clone，不能逐版本排除所有相同实现。当前状态：`INVESTIGATE_NEXT`，待设备可用后先完成 Candidate Correctness，再决定是否做 Local。
