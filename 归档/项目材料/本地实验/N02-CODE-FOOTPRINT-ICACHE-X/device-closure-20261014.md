# N02 当前版本设备闭环记录

日期：2026-10-14

## 源码身份

- Candidate：`本地实验/N02-CODE-FOOTPRINT-ICACHE-X/candidate.asc`
- Candidate SHA-256：`45ab8784ab33d1c668c958fdf577084f7349e23bef37dce4d68368ee21fb2082`
- Parent：`线上结果/R31B/V011/submission.asc`
- Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- 已确认源码差异：Pass2 的 `batchRow` 循环前新增 `#pragma clang loop unroll(disable)`，其余内容与 Parent 的文本差异未见。

## 构建身份

- Full-link：PASS；本轮未重新编译。Candidate 对象时间为 `2026-10-14 04:38:57 UTC`，早于源码文件上传时间 `04:05:30 UTC`。
- 实际 Candidate 输入：`/tmp/cann-n02-v2-20261014.AvJAjF/Candidate.asc`，SHA256 与本地 Candidate 同为 `45ab8784ab33d1c668c958fdf577084f7349e23bef37dce4d68368ee21fb2082`。
- 构建依赖 `candidate_kernel.asc.o.d` 指向 `support/../Candidate.asc`；`support/candidate_kernel.asc` 是包含 `../Candidate.asc` 的构建包装文件，故该对象对应上述完整 Candidate。
- Parent 输入 `/tmp/cann-n02-v2-20261014.AvJAjF/Parent.asc` 与 Champion 源码 SHA256 相同；Parent 依赖文件指向 `support/../Parent.asc`。
- 构建包装文件 SHA256：Parent `bf47c912ab197a4838e121d765acc6f3f53b85c5f4bf793257864fb32cb87e25`；Candidate `3175fd4aefa9fa49270c9088b6f9f731315585e105a94a1944e97eebad4818ec`。
- 对象 SHA256：Parent `a8d7738508bf4d6a61eb6651104396ce85be36a8834e7505aaf8fbb77d200b14`；Candidate `d2595de6b15f2bd0616a9efca14e12e7e0e733715409d57d274fc0907d4220ba`。
- 链接库 SHA256：Parent `89a86ee883bb96259de41595e3deb05a148c22bc9d59576a4fb780fb04dea47f`；Candidate `abde3abf38b2d48a908e21cf343633ba38f01bb7309cb875054650e479806ec1`。
- Runner SHA256：`f2e576e88d733b56908196c2d9ca8e28d622eed73eb1369ad7e54ce660f44c9b`。Runner 内的 `W4-R07-V003` 标签不作为源码身份依据。
- 远端临时目录已确认存在：`/tmp/cann-n02-v2-20261014.AvJAjF`。

## 远端访问记录

- 命令：`ssh -o BatchMode=yes -o ConnectTimeout=8 cann-server3 'find /tmp/cann-n02-v2-20261014.AvJAjF -maxdepth 3 -type f -print'`
- 返回码：`255`
- 输出：`ssh: Could not resolve hostname cann-server3: nodename nor servname provided, or not known`
- 命令：`ssh -G -F /Users/sunyiyang/.ssh/config cann-server3`
- 返回码：`255`
- 输出：`Can't open user config file /Users/sunyiyang/.ssh/config: Operation not permitted`
- 远端命令未执行；未读取其他主机、Clone 或路线目录。
- 以上两次失败来自受限 V2 会话；Main 使用已配置的 `cann-server3` 入口核对了专用目录、编译依赖、对象身份并执行设备 Correctness。未更改 V2 权限档案。
- 初次加载 CANN 环境的尝试返回码 `1`：在 `set_env.sh` 前启用 `set -u`，导致 `LD_LIBRARY_PATH: unbound variable`，Runner 未启动。按既有构建脚本顺序将 `set -u` 放到环境脚本之后，再执行下列 Correctness。
- 首次 SCP 通配符尝试因本机 zsh 展开远端路径而返回码 `1`；给远端路径加引号后传输成功。24 个原始 stdout/stderr/TSV 文件均已在本地证据目录确认存在。

## Correctness 与 Local

| Shape | Dtype | Correctness | Same-binary / paired Local | Raw 数据与统计 |
|---|---|---|---|---|
| 128x12288 | FP16 | Parent/Candidate PASS，mismatch=0 | 未运行 | 原始输出见 `logs/corr-128x12288-fp16-*` |
| 128x12288 | BF16 | Parent/Candidate PASS，mismatch=0 | 未运行 | 原始输出见 `logs/corr-128x12288-bf16-*` |
| 128x32768 | FP16 | Parent/Candidate PASS，mismatch=0 | Parent same-binary 62 条，中位 67.47 µs、MAD/中位 8.17%；配对 Parent/Candidate 中位 66.45/66.59 µs，观测比值 +0.21%，受并发负载影响，不计 Local | 原始样本及统计见 `logs/n02-v2-local-20261014T083927Z/` |
| 128x32768 | BF16 | Parent/Candidate PASS，mismatch=0 | Parent same-binary 62 条，中位 92.42 µs、MAD/中位 10.11%；配对 Parent/Candidate 中位 96.57/96.04 µs，观测比值 -0.55%，受并发负载影响，不计 Local | 原始样本及统计见 `logs/n02-v2-local-20261014T083927Z/` |

- 每个 shape/dtype 的 Parent 与 Candidate 均单独运行既有 `w4r07_runner` 的 `correctness-only`，Runner 返回码均为 `0`，共 8 次；FP16 最大绝对误差 `0.001953125`，BF16 `0.0078125`，均为 0 mismatch、0 nonfinite。
- Correctness 原始 stdout/stderr/TSV 和源码/对象身份记录均保存在 `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/logs/`。
- 开始快照 `2026-10-14T05:12:10Z`：NPU6 HBM `59876/65536 MB`（5660 MB 可用）、AICore `0%`；VLLMWorker_TP PID `3357697` 使用 `56355 MB`，未触碰。
- 结束快照 `2026-10-14T05:20:23Z`：NPU6 HBM `59875/65536 MB`（5661 MB 可用）、AICore `0%`；同一 VLLMWorker 仍在，Runner 精确进程查询返回 `1`（无 w4r07_runner 运行）。
- `2026-10-14T08:39:27Z` 开始一次有限 Local：复用既有 Runner，在 `rows=128,width=32768` 上测 FP16、BF16；每种 dtype Parent same-binary 为 45 warmups、31 samples×2 blocks，paired 为 45 warmups、31 pairs×4 blocks。四次 Runner 均返回 `0`；每个 dtype 保存 62 条 same-binary 及 124 对 Parent/Candidate 原始样本。结束时间 `08:43:35Z`。
- 设备前快照 `08:43:03Z` 已显示 NPU6 HBM `91%`、AICore `41%`、AIVector `28%`、HBM 带宽 `44%`；VLLMWorker_TP PID `3357697` 使用 `56355 MB`。各阶段结束快照保持 AICore `41–42%`、AIVector `27–28%`、HBM 带宽 `44%`。该进程未触碰；测量受并发负载影响，不能作为 Candidate 性能结论。
- FP16 配对样本中，Candidate 较快 55/124 对，Parent/Candidate 中位数比值 `+0.21%`；BF16 较快 67/124 对，中位数比值 `-0.55%`。PC/CP 次序的差异和高设备负载均保留在统计文件中；`LOCAL_SCORE=NONE`。
- 本轮计时前直接调用 Runner `--help` 因未加载 CANN 环境返回 `127`（缺少 `libruntime.so`），未进入设备。随后按既有环境脚本顺序加载 CANN 后完成上述实验，没有重复该失败命令。
- Local 原始 TSV、stdout/stderr、负载快照、命令返回码及统计汇总保存在 `logs/n02-v2-local-20261014T083927Z/`。本轮未重新编译，Parent、Candidate、动态库与 Runner 的 SHA 均与已保存构建身份相符。

## 当前判断

- 建议：`INVESTIGATE_NEXT`。
- 依据：Parent/Candidate 在四个 shape/dtype 组合全部 Correctness PASS，且当前 Candidate 已完成一次 same-binary 与 paired 采集；但计时窗口开始前和全过程均有明显设备负载，故这些延迟只作原始观察，不能判定关闭展开是否改变了有效设备热点或性能。
- 现有 Parent/Candidate 设备对象的 `.aicore_binary` 段各导出 `44,248` 字节，逐字节比较相同，记录见 `logs/n02-v2-local-20261014T083927Z/device-code-section-comparison.txt`。`#pragma clang loop unroll(disable)` 没有改变 AICore 二进制；当前 Candidate 作为设备代码布局优化予以 REJECT。Host 侧对象与动态库身份不同，但本轮负载不支持延迟归因。
- 下一项 N02 研究应避开等价 unroll 写法，围绕 Pass2 热点代码寻找另一项能真实改变 AICore 指令布局的单因素假设，再做新的设备测量。当前源码不变。

## N02 V016 — BF16 重复转换移除

### 假设与源码身份

- Direct Parent commit：`c68ce4ababde3c7472147549a9105e9f7311e398`；Parent source SHA-256：`45ab8784ab33d1c668c958fdf577084f7349e23bef37dce4d68368ee21fb2082`。
- Candidate source SHA-256：`9abee2c9c4b296efd46a8b892c5fc873a8c9c2bd480cd5bd4c86aad3fd2676ab`。
- 唯一源码变化：移除 BF16 Pass2 batch-row 循环前重复的两次 `ToFloat` 和一个 `PipeBarrier`；保留前一组相同转换。预期减少热路径 Vector 转换和等待，不把收益归因于 I-cache。

### Build 与 Correctness

- 复用 server3 上的 CANN 8.5 构建；Full-link PASS。依赖文件分别指向本轮 Parent/Candidate 输入，远端源码 SHA 与本地一致。
- Runner SHA-256：`b2d686b0baf10555e8f832deda6863d86b91fceb89b387b630c8dbf342b90bde`。Parent/Candidate `.aicore_binary` 长度分别为 `44,248` / `44,336` 字节，二进制不同；Candidate 多 88 字节，不能据此声称设备指令段缩短。
- `128x12288`、`128x32768` 的 FP16/BF16 Parent 与 Candidate 共 8 次 Correctness 均 PASS，mismatch=0、nonfinite=0。
- 编译、对象、源码、Correctness 原始记录位于 `logs/n02-duplicate-cast-20261014/`。

### Local

- 设备：server3 NPU6 / Ascend 910B3。rows=128。每个 case 使用 45 warmups；same-binary 为 2x31，paired 为 4x31。每个配对 case 共 124 对 Parent/Candidate device-event 样本。
- `BF16 128x12288`：same Parent median `19.05 us`，MAD/median `1.21%`，两块中位数漂移 `0.10%`；paired Parent/Candidate median `19.73/19.24 us`，配对比值中位数 `-2.528%`，4/4 块 Candidate 更快，88/124 对更快。
- `BF16 128x32768`：same Parent median `50.39 us`，MAD/median `0.54%`，两块中位数漂移 `0.16%`；paired Parent/Candidate median `51.73/47.88 us`，配对比值中位数 `-7.766%`，4/4 块 Candidate 更快，108/124 对更快。
- `FP16 128x32768` 对照：same Parent median `34.33 us`，MAD/median `0.79%`，两块中位数漂移 `0.06%`；paired Parent/Candidate median `34.89/35.00 us`，配对比值中位数 `0.000%`，4 块方向混合，Candidate/Parent 各快 62/124 对。
- d6 preflight `2026-10-14T13:19:19Z`：HBM 94%，AICore 0%，AIVector 39%，HBM 带宽 12%；VLLMWorker_TP PID `3357697` 占用 `56,355 MB`，Python PID `3335972` 占用 `1,915 MB`，均未触碰。测量各阶段 AIVector 快照为 0–43%，原始样本有显著长尾。
- Local 数字保留为负载下的观察值，`LOCAL_SCORE=NONE`。BF16 两形状的配对方向值得保留，但本轮不作为可晋升的 Local 成绩，也不据此推算 Official 分数。
- 首次启动的六个 Runner 因 CANN 环境未在进程中生效而返回 `127`，缺少 `libruntime.so`，没有产生设备样本。改用普通 Bash 在同一进程加载现有 `set_env.sh` 后，仅完成一次有效测量；失败日志也保留。
- 有效 Local 原始 TSV、标准输出/错误、逐阶段设备快照及返回码位于 `logs/n02-duplicate-cast-local-envfix-20261014/`；首次环境失败位于 `logs/n02-duplicate-cast-local-20261014T1313Z/`。

### Result

- 判断：`INVESTIGATE_NEXT`。BF16 配对样本显示同向改善，FP16 对照中位持平；共享负载与长尾使本轮 Local 仍记 `NONE`。Candidate 可交 N06 按当前 Judge 规则评估；当前未提交 Official，也没有新 SID。
- 下一项 N02 假设应落在可解释的热点设备代码路径，并避免复测等价的 inline/unroll 写法。当前源码在本次结果提交后保持不变。
