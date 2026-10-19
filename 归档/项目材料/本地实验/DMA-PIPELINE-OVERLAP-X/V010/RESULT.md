# N04 V010 进展

## 假设与单一变化

- 父 Candidate：V009，源码提交 `77703ce3`；V009 Compile 提交 `ef182ff3`，设备结果提交 `3337b1ae`。
- 目标：FP16、rows=40、D=32768。Runner 传入 8 个 core，每个 core 处理 5 行；宽行 Pass2 的 tileWidth=4096、tileCount=8、batchLimit=1。
- V009 的 `outputBuf_` 在该构建宏下已有两个 tile 容量，但 `ProcessWideLowPrecision` 始终只用首槽，并在再次写入前等待上一笔 MTE3 完成。
- V010 仅对目标 shape 在 Pass2 按 tile 轮换两个输出槽。上一 tile 的 MTE3 写回可与下一 tile 的 Vector 输出计算并行；再次复用某槽前只等待该槽自己的 `MTE3_V` 完成事件。
- V009 的跨 batch 首个 x/residual tile 预取、参数加载次序、运算顺序及其他 shape 保持不变。复用一个在每次 Store 前已消费的 `V_MTE3` 事件，并为两个输出槽分别追踪 `MTE3_V` 完成。

## 已有结果背景

- V009 四组 Correctness PASS、每组 21 对；目标 FP16 D32768 配对差中位数 -6.520 μs，P10–P90 为 [-7.160, -5.240] μs，但 min/max 为 [-24.500, +16.960] μs。其余三组区间跨零，`LOCAL_SCORE=NONE`；不重跑 V009。
- Main 提供的既有 Official 汇总：R31B V011 对 N04 V003 为 7/15 项更快、8/15 项更慢；对 N04 V005 为 4/15 项更快、11/15 项更慢。V003 第14/15项分别 -30.12/+3.23 μs；V005 第14/15项分别 +113.32/+45.64 μs。
- Case→shape 映射仍为 UNKNOWN；不据此推断 shape 或机制表现，也不重跑 Official。

## 服务器事务实际结果

- Compile：PASS。沿用本路线 `support/paired/CMakeLists.txt`、`paired_runner_v016` 和已有 CANN 8.5.0.alpha002 环境；远端目录为 `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V010`。
- Correctness：PASS，FP16/BF16、D=12288/32768 四组均通过，Parent/Candidate failures=0。
- Local：PASS，21 对/组。paired median（Candidate−Parent）依次为 FP16 D12288 `-1.520 us`、FP16 D32768 `-1.520 us`、BF16 D12288 `-2.420 us`、BF16 D32768 `-9.340 us`；各组 P10-P90 与原始样本保存在远端日志 `device7-correctness-local-20261017.log`。
- 设备：NPU7；Runner 返回 `RC=0`，未调用设备 Reset，未触碰其他进程。
- Official：`Pass 15/15`，score `40.87`，Submission ID `6ad26b56193e507d8eb8bbfa`；线上结果见 `线上结果/NEW-N04-DMA-PIPELINE-OVERLAP-X/V010/result.json`。
- 源码身份：本地 SHA256 `1d6551208af95c75d01f44218c57f2ff5f6553d6eb38542e58a5c5da983a6e72`，平台返回 `LOCAL_SHA_EQ_REMOTE_SHA`。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MIXED_OFFICIAL_PASS
LOCAL_SCORE: NONE
OFFICIAL_SCORE: 40.87
