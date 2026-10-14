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

N05 的 d2 设备快照 `2026-10-14T05:52:38Z`：HBM `63413/65536 MB`，AICore `12%`；Python PID `278936` 使用 `60043 MB`，并与其它卡上的进程同时运行。

再次准备 Correctness 前的快照 `2026-10-14T06:37:54Z`：HBM `63412/65536 MB`（2124 MB 可用），AICore `11%`；同一 Python PID `278936` 使用 `60043 MB`，Runner 精确进程查询返回 1。由于设备已有计算负载，本轮没有启动 NPU Runner。Same-binary 与 paired Local 也未运行。`CORRECTNESS=NONE`、`LOCAL_SCORE=NONE`；无性能结论。两份快照、构建日志和源码/对象身份记录保存在 `本地实验/N05-SCALAR-ADDRESS-GENERATION-X/logs/`。

本路线记录显示旧 Cursor 变体主要改地址推导；当前 Candidate 预计算尾 tile 有效长度，机制不同。但旧版本源码未包含在本 Clone，不能逐版本排除所有相同实现。当前状态：`INVESTIGATE_NEXT`；诊断 Shape Correctness 已通过，Official Shape 映射仍未知，Local 为 `NONE`。

### 2026-10-14 07:24Z Correctness follow-up

- 使用已编译的 N05 Runner，未重编译。Parent/Candidate 的编译输入、对象与链接库身份沿用上方记录；Candidate 源码仍为 `3346b12b7f14e88e5188080f5810ec6327cd25338354f709435e4d676e58d817`。
- 先前一次直接启动返回 `127`：Shell 未载入 CANN 运行库，`libruntime.so` 找不到；Runner 未进入 NPU。随后沿用 `build_server3.sh` 中的 CANN 环境初始化和库路径，`ldd` 已解析全部依赖。
- 同一输入生成器 `R4_MULTIROW_PROXY_128XWIDTH_SEED0`，`rows=128,width=12416`，分别验证 FP16 与 BF16 的 Parent/Candidate。该宽度为 4096 元素 tile 的非整除尾块，尾块有效长度为 128；用例是诊断输入，不代表 Official Shape。
- FP16 Parent/Candidate 均 `PASS`，`max_abs_error=0.001953125`、`mismatches=0`、`nonfinite=0`；BF16 Parent/Candidate 均 `PASS`，`max_abs_error=0.0078125`、`mismatches=0`、`nonfinite=0`。完整原始 TSV 分别保存在 `logs/n05-corr-20261014T0724Z-width12416-fp16-parent.tsv`、`logs/n05-corr-20261014T0724Z-width12416-fp16-candidate.tsv`、`logs/n05-corr-20261014T0724Z-width12416-bf16-parent.tsv`、`logs/n05-corr-20261014T0724Z-width12416-bf16-candidate.tsv`。
- 测量前 d2：HBM 使用率 96%（约 2621 MB 可用）、AICore 8%、AIVector 7%、HBM 带宽 2%；Python PID `278936` 占用约 `60043 MB`。测量后该进程仍在，HBM 使用率 96%（约 2125 MB 可用）、AICore/AIVector 0%、HBM 带宽 14%；Runner 已退出。未触碰该进程。
- Same-binary 与 paired Local 未运行。Local 保持 `NONE`：设备有其他用户的长期 HBM 占用，未取得可解释本 Candidate 的配对计时。Official Shape 映射仍为 `UNKNOWN`，这些诊断正确性数据不计为 Official 结果。
- 后续考虑 Local 时 d2 仍为 HBM 使用率 96%、AICore 9%、AIVector 7%、HBM 带宽 30%，Runner 进程查询为 1；因此没有启动配对计时。详细设备快照见 `logs/n05-d2-corr-load-20261014T0724Z.txt`。
