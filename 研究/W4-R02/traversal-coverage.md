# W4-R02：五 tile 的选择性跨行遍历

## 本轮范围

仅操作 `w4/r02-selective-tile-traversal-x`。初始提交为 `de70b634813dea80783fc57716d6e95c158edeec`，初始工作树无未提交内容。Online=PAUSED，PUSH=NO。

最新历史登记来自 `w3/m1/record-owner@cecec26b`。其 R02 V001 行只有编辑前研究，Candidate、Compile、Correctness、Local、Git commit 均不存在。为保留该行并避免编号重用，本轮首个真实性能实验使用 V002。旧控制状态不作为停止理由。

## DUPLICATE_AUDIT

MECHANISM：只在 FP16、tileWidth=4096、tileCount=5、当前 batchRows=2 时，将 ProcessWideLowPrecision 的 pass-1 从 row-major 改为 tile-major。当前单元与下一预取单元使用同一映射。其他路径保留 Parent 映射。

SEARCHED_HISTORY：

| 来源 | 已读证据 | 对当前想法的约束 |
|---|---|---|
| 本 Route | 本地分支及所有本地 Git 对象中的 R02 路径；Record Owner 的 R02 V001 行 | 无真实性能版；旧研究仅否定 12288 列 |
| W3 R4，ref w3/m1/multirow-panel-rms，至 V031 | V003、V020、V031 完整 summary；V003–V031 的单变化和路径字段；Record Owner V014–V031 行 | V003：4096/3/3；V020：6144/2/2。其后宽度、预算及归约间距实验仍为 128×12288 |
| W3 R5，ref w3/m1/crossrow-full-pipeline，至 V028 | V002–V016 声明；V006 Parent/Candidate diff 与 Local 日志；V017–V028 登记；V001–V028 的实际 Local 命令字段 | V006 已测 16×16384 FP16、8 blocks、4096/4/2 的 tile-major；不能重做 |
| W3 R2，ref w3/m1/adaptive-core-ownership，至 V040 | V001–V016 Parent/Candidate diff；V017–V040 登记与提交序列 | 核数量、行归属及其排列；不提供五 tile 的遍历结果 |
| R031、R31A、R31B | 技术路线总表 R031 项、R031 原始源码、R31A/R31B 全版本机制登记；R31B V011 Init、ChooseWideFullYRows、pass-1、Host dispatch | R31B V016 改 tile 宽度，后续改 store 或输入队列深度；没有当前选择性五 tile 条件的 Local 证据 |
| MIX、STORE、EPILOGUE | 本工作树和 Record Owner 的机制登记 | 分派、算术与写出机制；未覆盖当前输入遍历组合 |
| 相关 W4 | Record Owner 的 R01/R02/R03/R06/R07/R08/R10/R13/R14/R15 行 | 参数相位、输入次序、跨行预取、活跃核数等，未记录当前五 tile 的选择条件 |

MATCH_FOUND：4-tile 想法 YES（R5 V006）；5-tile 选择性想法 NO。

WHY_NEW_OR_DUPLICATE：R4 V003/V020 的已测 tileCount 分别为 3/2；R5 V006 为 4。本轮五 tile 的两行输入流具有不同的奇偶交替相位，且明确排除以上几何。复用历史 tile-major 映射，新增内容是未测的几何启用条件，不声称 tile-major 本身首次提出。

HISTORICALLY_UNCOVERED=YES，范围限定为上述可取得的源码与 Local 证据。隐藏测试点的 shape 未知，OFFICIAL_COVERAGE=UNKNOWN。

## 几何推导

Parent 常量为 176×1024 字节预算、4096 元素 tile、16 个 FP32 归约槽。FP16 两行路径的选择式为：

`need = rows × (2 × D + 64) + 4 × 4096 × 2 + 3 × 4096 × 4`

| D | tileWidth | tileCount | batchLimit | 最后 tile 元素数 | V002 条件 |
|---:|---:|---:|---:|---:|---|
| 12288 | 4096 | 3 | 3 | 4096 | false，已有历史，不重测 |
| 16384 | 4096 | 4 | 2 | 4096 | false，已有 R5 V006，不重测 |
| 18432 | 4096 | 5 | 2 | 2048 | 两行 batch 为 true |
| 20480 | 4096 | 5 | 2 | 4096 | 两行 batch 为 true |
| 22528 | 4096 | 6 | 2 | 2048 | false，负对照 |
| 24576 | 4096 | 6 | 1 | 4096 | false，单行对照 |

runner 保留历史 R4 的 128 rows、40 available cores。Host 实际 blockCount=40；8 个核各 4 行，32 个核各 3 行。前者是两个两行 batch，后者是一个两行 batch 加单行收尾。单行收尾保留 Parent 映射。均为明确声明的 Local 探针，不代表 Official 测试点。

两行五 tile 的 row-major 顺序为 (r0,t0)…(r0,t4),(r1,t0)…(r1,t4)；tile-major 为 (r0,t0),(r1,t0)…(r0,t4),(r1,t4)。每行归约槽的 tile 下标和最终 ReduceSum 输入次序不变。

## 验证约定

Parent=本工作树的 `线上结果/R31B/V011/submission.asc`；原样保存到实验目录 parent.asc。runner 与构建结构复用 W3 R4 V031 的已用实现，移除来源摘要计算，保留 device-event 计时边界、同步方式和输入模式。

Correctness：Parent 与 Candidate 分别对独立 CPU 公式进行比较。输入先按 dtype 量化，CPU 用 double 累积 RMS、按公式求输出并量化回输出 dtype。FP16 使用 atol=rtol=0.001，每个元素均须通过，非有限输出计失败。该标准取自题面/模板的本地历史记录，属于 Local reference 验证，不能写成 Official 通过。没有把 Parent 的输出充当 reference。

范围：18432、20480、22528、24576 列 FP16。未改变的 FP32/BF16 不在本次运行矩阵内。原 R4 的固定绝对容差 0.0025 不作为本轮通过依据。

Local：每个 shape 先取 Parent same-binary，45 warmups、21 samples × 4 blocks；再按 PC/CP 交错取 Parent/Candidate，各 84 个样本。计时循环外分配与搬运。主 shape=20480，尾 tile=18432，条件为 false 的对照=22528。保存全部 device_event_us、wall_us、顺序和设备负载。

数值解释预先固定：Local score=Candidate device-event 中位数；delta=(Candidate median / Parent median - 1)×100；同时报告配对差中位数。Parent same-binary 或任一侧 MAD/median>0.10、block 中位数相对范围>0.10 时，记 MEASUREMENT_BLOCKED，不宣布改善。仍照常完成已经允许的测量并保留所有样本。只有资源规则中的真实失败才停止对应服务器动作。

## 运行位置

本地：`本地实验/W4-R02/V002/`。

远端：`cann-server3:/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/`。已确认规范父目录存在，现有目录中未发现 R02 专用对象。本轮仅在此位置创建必要源码、build 与日志，不安装服务，不改变系统配置，不操作其他 Route。

设备：1。工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`，编译目标 dav-2201。开始时 HBM 65536 MB、使用率 14%，据此可用 56360 MB；各运行阶段另保留快照。服务器已有用户负载，未作任何停止或迁移动作。

## V002 实际结果

V002 已完成 Compile、独立 reference 比较和 Local。结果为 `MEASUREMENT_BLOCKED`；实测数值全部保留，不宣布提升，不更新 Local Best，`STAGNATION_INCREMENT=0`。本轮只实现这一个性能版本，不启动 V003。

Compile：首次在 2026-10-08T03:44:48Z 因工具链目录没有 `set_env.sh` 而退出，编译器尚未启动。留在 V002，只把构建环境设置改为历史使用的 PATH、库路径和 include 路径；Candidate 未再改动。第二次构建在 03:46:12Z 开始、03:46:32Z 结束，`COMPILE_RC=0`。两个日志均保留。

Correctness：FP16，128 rows，D=18432、20480、22528、24576。每个宽度分别运行 Parent 与 Candidate，共 8 次，均对独立 CPU reference PASS；每次 mismatches=0、nonfinite=0、max_abs_error=0.001953125。混合容差为 `error <= 0.001 + 0.001 × abs(reference)`，没有仅凭 Parent/Candidate 一致认定正确。实际输入为 runner 的固定确定性模式；未执行 FP32/BF16、其他输入分布或正式 Judge 验证。

Local：每个 shape 有 84 条 Parent same-binary、84 条配对 Parent、84 条配对 Candidate，共 756 条。无样本被删除。下表的 delta 使用两侧中位数之比；配对差是另一项统计量，二者不混用。

| FP16 shape | Parent same-binary 中位数 µs | 配对 Parent 中位数 µs | Candidate 中位数 µs | delta % | 配对差中位数 µs |
|---|---:|---:|---:|---:|---:|
| 128×20480 | 26.200001 | 26.230000 | 25.770001 | -1.753715 | -0.420001 |
| 128×18432 | 24.650000 | 32.410000 | 28.990001 | -10.552296 | -2.150000 |
| 128×22528 | 39.920001 | 35.629999 | 35.660000 | +0.084203 | +0.640000 |

主 shape 的配对 Parent/Candidate MAD/median 分别为 0.110941632、0.109817654，均超过预定 0.10。PC 次序的配对差中位数为 +0.200000 µs，CP 为 -2.540000 µs；四个 block 的配对差中位数依次为 +0.020001、-1.560001、-1.060000、+0.219999 µs，方向不一致。

18432 的 Parent same-binary block 中位数相对范围为 0.199594320，配对两侧 MAD/median 为 0.213514335、0.187650928。22528 的 same-binary MAD/median 为 0.212424885，配对两侧为 0.209093452、0.197980942。该条件未启用的对照也出现明显波动，当前数据不足以归因五 tile 遍历的速度变化。

Local 前后 FREE_HBM 均为 55705 MB；AICore 17%→6%，AIVector 21%→11%。host load 三个值由 60.24/58.47/54.20 变为 48.54/55.83/53.46。负载未被当作拒绝运行的理由；`BLOCKER=NONE`。这里的 `MEASUREMENT_BLOCKED` 只表示当前数值不能用于提升判断。

统计脚本使用所有原始观测；CV 使用标准差除以均值绝对值，配对差保留正负号。完整统计位于 `本地实验/W4-R02/V002/result.json`，可仅在本机运行 `python3 本地实验/W4-R02/V002/support/summarize.py` 复算，不触发任何设备操作。

## 实际入口与证据

服务器主机 `hwnput3`，用户 `lelinfeng`，SSH 入口 `cann-server3`。设备操作仅涉及前述 R02 V002 专用目录。

| 内容 | 入口或文件 |
|---|---|
| Compile 入口 | 远端 V002/support/build_server3.sh；CMake Release、dav-2201、parallel=4 |
| Correctness/Local 入口 | 远端 V002/support/run_server3.sh |
| Correctness 参数 | `build/r02_runner 1 D fp16 correctness-only SIDE 0 0 1 OUTPUT.tsv`；D 与 SIDE 见上述 8 个组合 |
| Parent same-binary 参数 | `build/r02_runner 1 D fp16 same parent 45 21 4 OUTPUT.tsv`；D=20480、18432、22528 |
| 配对参数 | `build/r02_runner 1 D fp16 paired - 45 21 4 OUTPUT.tsv`；相同三个 D，逐样本交替 PC/CP |
| 构建日志 | `本地实验/W4-R02/V002/logs/compile-01.log`、`compile-02.log` |
| 精度结果 | `本地实验/W4-R02/V002/logs/correctness-D-SIDE.tsv` 及各自 `.log` |
| 原始样本 | `本地实验/W4-R02/V002/local/same-parent-D.tsv`、`paired-D.tsv` 及各自 `.log` |
| 资源和结束记录 | `本地实验/W4-R02/V002/logs/validation-01.log`、`stages.txt`、各阶段 `.usages.txt`、`.load.txt`、`.npu.txt` |

Correctness/Local 命令于 2026-10-08T03:47:35Z 开始，03:48:54Z 结束，`RUN_COMPLETE RC=0`。全部前台命令已退出；结束后的本 Route runner/build/run 进程查询无匹配。`RUNNING_DEVICE_OPERATION=NONE`。已构建二进制留在远端 V002/build，供需要时复用。本轮没有清理文件、安装服务、改变系统配置、操作其他用户任务或写入共享记录。

本地 Parent 与 `线上结果/R31B/V011/submission.asc` 逐字节一致。Parent/Candidate 源码差异仅为 pass-1 的五 tile 两行启用条件，以及当前/下一单元的行列映射；没有修改 tile 宽度、行归属、归约顺序、事件或输出流程。

## 交付状态与后续动作

`DIRECT_PARENT=R31B-V011`；`CURRENT_LOCAL_BEST=NONE`（W4-R02 本轮没有新增可接受 Best），保留 R31B-V011。`OFFICIAL_SCORE=NONE`、`ONLINE_STATE=PAUSED`、`PUSH=NO`。结果、源码、支持文件及全部失败/原始证据随 V002 单一实验提交保存；实际 commit 与最终工作树状态由提交后的版本事件提供。

本次达到一个真实性能版本的完整闭环后返回 Main，不改变路线生命周期。首要后续动作是复用现有 V002 二进制研究主 shape 的 PC/CP 次序偏差：先对照 runner 的单次 event 起止位置与已存 PC/CP 样本；若 Main 再次安排设备时间，用同二进制 Parent 对照配合有限 kernel-duration 采集，区分 host launch 间隙与 kernel 时长。新采集必须使用独立输出路径，保留本次所有观测；本次不执行该采集，不创建后台任务。

仍值得审计的独立轴是分段遍历：每行连续处理两个 tile 后，再切换到另一行，对五 tile 的末段单独处理。该想法的历史覆盖状态仍为 UNKNOWN，尚不具备开版依据。精确研究入口是本版 ProcessWideLowPrecision 的单元映射、R5 V006/V028 与 R4 V021–V031 的 Candidate diff；先确定是否已有同等分段映射及对应几何，再决定是否构成独立 OFAT 实验。隐藏测试点覆盖和 Official 相关性均未确认。
