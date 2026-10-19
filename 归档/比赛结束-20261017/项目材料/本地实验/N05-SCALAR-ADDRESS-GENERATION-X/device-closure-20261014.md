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

### 2026-10-14 11:06Z V014 Local continuation

- 本次日志目录：`logs/n05-local-20261014T110428Z/`；目录名沿用用户指定值，连接尝试于 `2026-10-14T11:06:11Z` 结束。
- 计划复用 `/tmp/cann-n05-v2-20261014.sIM7dE/build/w4r07_runner`，以及上文记录的同一 Candidate/Parent；未修改 kernel 源码、未重建、未调用子 Agent、未访问其他 Clone 或会话日志。
- 用户指定参数：FP16、BF16 各一组 `same parent`（2 blocks）与一组 `paired -`（4 blocks），`rows=128,width=12416,device=2,warmups=45,samples=21`。这只是代理形状，Official 用例映射仍为 `UNKNOWN`。
- 准备命令通过 `cann-server3` 读取远端 N05 runner 参数入口和 CANN 环境设置，SSH 返回 `255`：`Connection closed by UNKNOWN port 65535`。远端标准输出为空；设备执行前失败，未重复该命令。命令、原始标准输出/错误和返回码分别保存在 `prepare.command.txt`、`prepare.stdout.log`、`prepare.stderr.log`、`prepare.status.txt`。
- 四组测量均未启动；未获得本次 d2 实时 npu-smi、进程列表、route-runner 状态或执行后快照。不能以先前快照代替本次状态。未产生 raw TSV，robust stats 无样本可计算，`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`LOAD_QUALITY=UNKNOWN`。当前连接失败的具体原因尚未确认，不据此评价 Candidate 性能。
- 剩余工作：连接恢复后完成上述指定设备测量及统计。本次仅保存实际连接失败；未改动任何远端进程、服务或数据。
- 本地 Git 保存未完成：精确路径 `git add` 返回 `128`，原始错误为 `fatal: Unable to create '/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation/.git/index.lock': Operation not permitted`。后接的 `git commit --only` 未执行；未创建提交、未推送。当前运行环境不允许写入本 Clone 的 Git 元数据，记录与日志保留在工作目录。

### 2026-10-14 11:16Z V014 Local result

- 实际测试由 Main 经 `cann-server3` 完成，Runner、源码、对象和链接库 SHA256 与本记录上方 Build 身份一致。未改 Kernel、未重编译。
- 使用 128x12416 诊断输入，FP16/BF16 各完成 45 warmups、21 samples、2 个 Parent same-binary blocks 和 4 个 paired blocks；四阶段均 RC=0。该代理 Shape 的 Official 映射仍为 UNKNOWN。
- FP16 same-binary 两块 Parent median 为 20.16/27.30 us，合并 median 20.50 us、MAD 1.35 us；包含 224.72 us 离群样本。配对块的 Candidate-Parent median 为 +2.08/-3.20/-0.16/-0.88 us，合并 paired median delta -0.63 us（-2.61%），块方向不一致。
- BF16 same-binary 两块 Parent median 为 26.96/32.80 us，合并 median 30.77 us、MAD 9.29 us。配对块 Candidate-Parent median 为 +0.64/+1.00/+0.44/+3.12 us，合并 paired median delta +0.88 us（+4.07%）；四块 Candidate 都较慢。
- 前后 d2 均有 Python PID 278936（penghao+）运行，HBM 96%；AICore/AIVector/HBM 带宽由 8%/6%/1% 变为 9%/9%/12%。样本分布与负载变化不足以支持稳定性能判断。`LOCAL_SCORE=NONE`、`LOCAL_DELTA=NONE`、`LOAD_QUALITY=CONTAMINATED_SHARED_DEVICE`，不晋升。
- 完整统计见 `logs/n05-local-20261014T111213Z/local-summary.md`；原始 TSV、阶段状态、身份和设备快照在同目录。首次受限 CLI SSH 失败及首次环境加载失败保存在 `environment-attempt-failures.txt`。
- 最新实际状态：现有 V014 保留为待进一步验证版本；本次未创建下一版。
