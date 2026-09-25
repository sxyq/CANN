# R31A V021 Main Handoff

## 状态

`NEEDS_ONE_MORE_LOCAL`

停在 Main Review。当前没有 Main 授权给 R31A 的独占设备租约；本轮未查询 NPU、启动 probe 或执行测量。不得创建 V022，也不得提交 CANNJudge。

## 版本与来源

| 项目 | 值 |
|---|---|
| Route / revision | R31A / V021 |
| 分支 / V021 既有证据提交 | `exec/sixlane-20260923-r31a` / `d38328aed787dfce6ad687821223d87db04f7e43` |
| Direct Parent | R31A-V016 |
| Parent source SHA-256 | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |
| Parent Official Score | 45.00，15/15 PASS |
| Candidate | `phase4/local/R31A/V021/submission.asc` |
| Candidate source SHA-256 | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` |

V016 的 Online 源、Route 工作区源及 `support/parent_v016_submission.asc` 三者的 SHA-256 相同。V021 的实算 SHA-256 与 `submission.sha256` 相同。V017 不属于 V021 的来源链；`diff.patch` 的直接比较对象是 V016。

## 直接差异与既有验证

Candidate 源仅把 `SyncMTE3ToV()` 从 FP32 CachedRows 输出的 tile 循环体内移到循环之后，位置见 `submission.asc:2053-2055`。记录的单一假设是延后每个 tile 的 MTE3 完成等待，使后续 tile 运算与前一 tile 的输出写入重叠；其他计算路径保持不变。

V021 的编译、链接记录均为 PASS。设备 4 上记录的正确性结果：D=32768 与控制 D=24576 均 PASS，最大绝对误差分别为 `4.83928943e-07`、`3.48268474e-07`。已有两组延迟记录标为 `LOAD_CONTAMINATED`，方向相反，不构成性能结论。

## 可执行文件身份

现有 `local-result.json` 留存的 server3 构建记录：

| 文件 | 记录的二进制 SHA-256 | 编译输入源 SHA-256 |
|---|---|---|
| `probe_v016` | `5513076e24b067864ea625ff9a0cfe9e8d1adb89262de3cebd9a3ab198df3f78` | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |
| `probe_v021` | `62092bb490d61e2604e6561e2971856aace504d7875427dceb06039ace163bf0` | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` |

依赖记录显示 Parent probe 包含 `support/parent_v016_submission.asc`，Candidate probe 包含 `../submission.asc`。本轮只核对了 Route 内保留的构建与依赖记录；二进制本体位于本 Route 工作区之外，未读取或重新计算其摘要。若更换 runner，以上二进制身份随即失效，须按新 runner 重建并记录。

## Harness 输入与状态

既定形状输入为 FP32、rows=2、blocks=1：目标宽度 D=32768，控制宽度 D=24576；两者均覆盖末尾非满 7680-element tile。设备编号由 Main 的有效独占租约指定，不能沿用旧准备记录中的 device 4 假设。

当前 `support/probe_main.inc` 与旧二进制不符合统一计时 protocol：warmup=2；每次启动聚合 10 次 launch，只输出一个平均 `device_us`；P/C 各自运行进程；没有逐样本 `wall_us`、样本文件或 jitter 统计。旧准备命令仅有每种形状两对，也低于新要求。因此这些二进制和延迟记录不得用于下一次统一 paired run。

## 新 paired runner

`support/paired_probe.asc` 在一个 ASC 翻译单元中分别包含 V016 与 V021；两个版本放在独立命名空间，并使用不同 kernel/host entry 名。`support/paired_probe_main.inc` 生成一个进程：同一 ACL stream、同一组一次分配并 H2D 的输入，先分别完成 V016/V021 correctness 与计时外 D2H，再各做 10 次 launch+stream sync warmup，之后执行 21 个相邻 P/C 配对，先后顺序逐对交替。每个单次 launch 记录 device event 时间和 host wall 时间。

每次调用只接受 D=32768 或 D=24576，固定 FP32、rows=2、blocks=1；成功后生成 `<prefix>-samples.tsv` 和 `<prefix>-jitter.tsv`，后者分别汇总两个版本的 device/wall 样本。文件使用独占新建模式，已有路径会报错，不覆盖旧结果。

构建目标为 `r31a_paired_probe`；原 `probe_v016` 与 `probe_v021` CMake 目标保留。本机 C++ stub 语法编译通过，CMake/Ascend C/ACL 开发组件未安装，因此目标二进制尚未构建。没有有效租约，runner 未启动。

Main 安排有效租约、在具备该 Route 源码和 CANN 工具链的构建位置生成目标后，从构建目录执行下面两条命令；`DEVICE` 必须取本轮租约设备号，`RUN_ID` 每轮唯一：

```sh
RUN_ID=$(date +%Y%m%dT%H%M%S)
DEVICE=<LEASE_DEVICE_ID>
./r31a_paired_probe "$DEVICE" 32768 "r31a-v021-${RUN_ID}-D32768"
./r31a_paired_probe "$DEVICE" 24576 "r31a-v021-${RUN_ID}-D24576"
```

取得 Main 独占租约并确认可比负载后，统一 harness 必须满足：

- P/C 使用同一版 runner，按相邻交错顺序执行，至少 4 对。
- 每进程先做至少 10 次 launch+完整 stream sync warmup；每个计时样本读取前完成 event wait 或 stream sync。
- 进程内至少 21 个计时样本；device event 记录每个样本的 `device_us`，同时记录 `wall_us`，保存原始样本及 median、mean、stdev、CV、min、max、max/min、MAD、p10、p90 和绝对跨度。
- 输入、分配和 H2D 位于计时区间外；D2H 与正确性比较在计时区间之后。
- 执行前后保存完整设备负载快照；没有有效租约或负载窗口不可比时停止，不启动 probe。

新 runner 尚未完成 Ascend C 目标构建，二进制身份尚未验证。本文件只交接现状与下一次获 Main 授权后的输入要求，不替代 Main 的租约安排。

## 本轮边界

未改 Candidate source、build wiring 或旧 harness；未改共享控制文件；未读取或修改其他 Route 的文件内容，也未读取其他 Main 的工作文件；未运行设备测量、CANNJudge 或清理操作。后续任何新实验仍以 R31A-V016 为 Direct Parent。
