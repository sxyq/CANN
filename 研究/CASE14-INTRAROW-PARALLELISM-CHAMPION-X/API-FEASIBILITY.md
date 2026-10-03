# CASE14 行内跨核归约 API 可行性事实补充

日期：2026-10-03
状态：只读 Track-B 事实补充；`MAIN_SELECTED=NONE`

## 结论

当前 direct-invoke host ABI 能表达 kernel 内的分组跨核归约、host 侧分配 workspace 并作为 kernel 参数传入；C001 exact source 在 CANN 8.5.0.alpha002、DAV_C220 Vector 目标上编译通过。C001 Official 记录为首个 testcase TLE，其余包含 case14 的条目均为 Skipped。由此只能确认源码/API 可表达且曾编译，不能确认该实现完成了任何 testcase，也不能确认 case14 命中 D-slice 热路径。

多次 kernel launch 也能在 `run_kernel` 内按同一 `aclrtStream` 写出。R31B V008 保存了四次连续 launch 的 exact source，Official 结果为 0/15 Runtime Error；记录没有给出出错点。因此多次 launch 的运行正确性仍未获证明，错误也不能归因到 launch 数量本身。

case14 的 rows、D、dtype、实际分支和活跃 Vector Core 数均为 `UNKNOWN`。`timeUs=16486.82` 与 `bestTimeUs=3750.12` 的约 4.40 倍只描述耗时比，不用于推断上述字段。

## Direct-invoke ABI 与 Parent

保留的调用样本 `线上结果/R31A/V016/judge-main-template.asc` 中，`run_kernel` 接收 x、residual、gamma、bias、output 的 `GM_ADDR` 与 `TensorGroupInfo`，以及 `availableCoreNum`、`aclrtStream`、`epsilon`；签名没有 workspace 入参。样本在 `run_kernel` 返回后同步该 stream。R31B V011 的 `run_kernel` 采用相同参数形式，只发起一个 kernel launch，不申请 device workspace。

V011 exact source SHA256 为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，与其 `submission.sha256`、`source-meta.json` 相符；Official 为 15/15、45.16。其 host 端把输入末维作为 `rowWidth`，其余维度乘积作为 `rowCount`，`blockCount=min(availableCoreNum,rowCount)`。device 端仅当 `rowWidth>8192` 进入 wide path。每个 block 独立拥有分配到的整行，不在 block 间交换 row 内 partial。以上均为源码通用行为，不能替代 case14 的输入形状与 dispatch 记录。

## C001 exact source、workspace 与同步

Git 中留存的 `归档/历史工作区/C001/kernel.txt` 为 359 行、12798 字节，SHA256=`e13e9b242d475d9552b4d2739db5bcd48e9378d58badb690bb878993167ce720`，与 `线上结果/C001/result.json` 的 source 元数据一致。其 `run_kernel` 从 `TensorGroupInfo` 取得 rows、D、dtype，并依据 `availableCoreNum` 计算分组数。device kernel 参数包含 workspace 地址和 tiling 地址；host 函数内部调用 `aclrtMalloc`，再将这两个地址传入 launch。

C001 使用四个 Vector Core 协作一行：各 lane 计算各自连续 D 区间的 FP32 partial，写入分开的 32-byte 槽位；`GroupBarrier<PipeMode::MTE3_MODE>` 第一轮后由 lane 0 合并 partial 并写 inverse RMS；第二轮后各 lane 读取该值并处理自己的输出区间。host 端令 `blockDim=4*group_count`，其中 `group_count` 不大于 rows、`availableCoreNum/4` 和 10。每个参与 lane 对每行执行相同的两轮 barrier；不参加热路径的 fallback 由 block 0 单独执行且不进入 barrier。

C001 只有在 `64<=D<=32768` 且 `D%32==0` 时进入上述协作热路径；其他 D 条件走 block 0 完整遍历的 fallback。case14 是否满足该条件为 `UNKNOWN`。

C001 workspace 为 `16 MiB + group_count * 2208 bytes`；device 代码用 `SetSysWorkspaceForce` / `GetUserWorkspace` 跳过 framework 保留区，再使用每组的 barrier 状态、FP32 partial 槽和 RMS 槽。该尺寸只复述 C001，不构成新实现的分配建议。

`归档/历史工作区/C001/architecture-metadata.md` 记载 DAV_2201 上的 `GroupBarrier` 到达数和等待数不得大于本次 `GetBlockNum()`，并说明 barrier workspace 按参与数乘 512 bytes 规划。`归档/历史工作区/C001/compile.log` 显示 FP16、BF16、FP32 模板编译成功，编译参数为 `--cce-aicore-arch=dav-c220-vec`。这确认该源文件在记录的工具链与目标上可编译，不代表其运行通过。

## Official 失败边界

`线上结果/C001/result.json` 记整体 `Time Limit Exceeded`、`passCount=0`。唯一实际开始的条目是 index 1 / testcaseId `6a9a9a99bf41025d6013eb8a`，状态 TLE；index 2–15 均为 `Skipped`，包括 index 14 / testcaseId `6a9a9a99bf41025d6013ebbe`。JSON 没有 rows、D、dtype、timeout 控制台记录或 device trace。因此 C001 是 D-slice 机制的失败 Official 记录，不能称作 case14 的失败样本；TLE 原因 `UNKNOWN`。

R31B V008 exact source `归档/历史工作区/R31B/R31B-V008-DSLICE-SMALL-R_kernel.asc` 在同一 `run_kernel` 形式下连续发起四个 kernel；`线上结果/R31B/V008/result.json` 记 0/15 Runtime Error，包括 case14。结果未附失败阶段或 runtime 诊断。它证明历史多 launch 实现未取得可用 Official 结果，不单独证明 stream 排序或 workspace 接口不合法。

## 与 Main-2 方向的关系

分支 `origin/w2/m2/crossrow` 的 `研究/CROSSROW-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` 边界是相邻 row 的 stage overlap，保持 row 到 core 的归属且不切 D；并记 V011 Pass-1 已在 `(row,tile)` 间交叠 MTE2 与 Vector。它与同一 row 的 D 区间分给多个 core 属不同分工维度。

分支 `origin/w2/m2/interpass` 的 `研究/INTERPASS-PIPELINE-CHAMPION-X/TRACK-B-HANDOFF.md` 只覆盖同一 logical row 的 Pass-1/Pass-2 内或边界的 MTE2、Vector、MTE3 时序，不改变 row ownership。C001 改动的是 row 内 partial reduction 的跨 core 协作与 barrier；两者机制不同，但都可能影响 RMS 标量阶段的关键路径，不能把 interpass 结果当作 D-slice 结果。

`REDUCE-HIER-X` 的共享版本记录列有五种 Parent=V011 的单核 reduction 变体，均未形成可信的大 D 收益；记录把 reduction V/S 判为非主要瓶颈。跨核 D-slice 改变执行分工，不等同于这些单核求和次序变化；但它触及同一 RMS reduction 区段，故已有结果提高了性能风险，尚不能单凭它判作 API 不可用或机制已被同一实现覆盖。

## 精度与运行风险

- C001 把输入转换为 FP32，使用固定 lane 区间、lane 内顺序累加及 lane 0 固定次序合并。固定次序有利于复现同一实现的求和顺序；分组方式改变后，FP32 舍入仍可能与 V011 不同。
- C001 最终由 `FromScalar<T>` 转回输入 dtype；V011 对宽行低精度路径另有 y 暂存与 tile 流程。没有 exact case14 dtype、输入值及逐例容差数据，低精度舍入是否与 Parent 对齐为 `UNKNOWN`。
- host 内分配后在 `aclrtSynchronizeStream` 返回后释放，见 C001 source。每次调用分配、同步与释放对 Judge 计时边界的影响，当前留存的完整 Judge runner 与计时 trace 不足以判定。
- 采用同 stream 多 launch 可把阶段依赖放在 kernel 边界；当前仓内 V008 runtime 失败，未给出可以引用的成功多 launch 对照。采用单 launch 则需所有同组参与者完整执行 barrier 序列，不能让任一参与者提前退出。

## 可证伪条件与精确缺件

以下数据齐全后可判断 case14 是否满足跨核分行前提；获取前维持 `UNKNOWN`，不以耗时比替代：

1. Official testcaseId `6a9a9a99bf41025d6013ebbe` 对应的输入元数据：完整 shape、折叠后的 rows、末维 D、dtype、stride/layout。
2. 该 testcase 在 V011 中的 host dispatch 值与实际路径：`availableCoreNum`、`blockCount`、wide-path 分支、每行 tile 数及实际参与 core 数。
3. 绑定到同一 testcaseId 的原始 timeline/profile，至少覆盖 MTE2、Vector、MTE3、reduction 与等待区间；现有研究中 `1.23/4` 等汇总值没有 case14 原始 profile 可复算。
4. C001 testcase 1 的 runtime 标准输出/错误输出、超时来源、device trace 与输入元数据；现存编译日志不能解释 Official TLE。
5. Judge 上传包对应的完整 `main.asc`、`CMakeLists.txt`、`data_utils.h`、`run.sh` 及实际 CANN 版本记录。V011 metadata 指向 `/tmp/cannjudge_template.json`，该包未随仓库保存；当前留存的 R31A V016 wrapper 样本只够确认 `run_kernel` 参数形状与返回后 stream sync。
6. 针对实际 testcase dtype 的 Parent/C001 数值对照，包含 RMS partial、最终 inverse RMS、输出误差与容差；目前只有 Official PASS/TLE 状态，没有逐例误差数据。

`ASC_DEVKIT_DIR` 在当前 shell 未设置；CANN API 本机正式文档目录及 `GroupBarrier` 头文件未随本路线 worktree 提供。现有 GroupBarrier 版本约束来自 C001 的 architecture metadata 与已存 compile log，尚无独立的当前工具包文档复核。

### 当前判定

| 项目 | 状态 |
|---|---|
| `run_kernel` ABI 内 host workspace 分配与 kernel 参数传递 | `SOURCE_CONFIRMED` |
| C001 DAV_C220 单 launch GroupBarrier 编译 | `COMPILE_CONFIRMED` |
| C001 任一 Official testcase 正确性或性能通过 | `NOT_CONFIRMED`；首例 TLE，余例 Skipped |
| 多 launch 同 stream 的成功运行 | `UNKNOWN`；V008 为 0/15 Runtime Error，原因缺失 |
| case14 rows / D / dtype / dispatch / 活跃 core | `UNKNOWN` |
| 精度等价、case14 收益与实施资格 | `UNKNOWN` |
| `MAIN_SELECTED` | `NONE` |

本轮未创建 Revision、未写 Candidate、未构建、未运行正确性、未运行设备、未测时、未更改路线生命周期或共享调度。
