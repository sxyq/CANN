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

## 编译

- 状态：未运行。当前工作范围只允许项目目录内操作；现有 CMake 配置引用项目目录外的 CANN Toolkit，且本轮未获 server3 访问确认。
- 构建入口仍为本路线 `support/paired/CMakeLists.txt` 与 `paired_runner_v016.asc`；CMake 候选 include 已指向 V010。

## Correctness 与 Local

- Correctness：未运行。
- Local：未运行；没有新设备结果。
- 当前等待用户确认 server3/NPU 访问后，再沿用路线已有构建目录和 Runner。运行前需确认 Runner 不触发设备 Reset，并按 Main 的设备安排执行。
