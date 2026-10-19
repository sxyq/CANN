# N03 REDUCTION-TOPOLOGY-X V002

日期：2026-10-16
状态：Compile PASS；Correctness 只有 Parent 完整通过，Candidate 仅首个样例完成；Local 因设备同步错误未完成，性能结果记为 NONE。

## V001_READBACK

V001 的 [RESULT.md](../V001/RESULT.md) 和 [Candidate.asc](../V001/Candidate.asc) 已读取并确认：

- V001 把低精度宽行 partial 槽步长设为 8 个 FP32 元素，reduce stride 设为 128。
- 每个 tile 仍先做 FP32 `ReduceSum`，随后用 `Add` 循环做全 tile 的平衡二叉合并。
- V001 没有改变分核、tile 长度、DMA 或参数驻留。
- V001 Local 结果跨形状和 dtype 不一致，因此 V002 不复用槽步长或二叉 `Add` 机制。

## HYPOTHESIS

V002 改用局部归约块长度机制：保持 Champion 的 16-float partial 槽布局和 FP32 算术，每 4 个 tile partial 先由一次硬件 `ReduceSum` 汇成块和，再对块和列表做一次硬件 `ReduceSum`。目标是把长 tile 列表的单次归约拆成固定短块与短列表两层，缩短单次归约输入长度；不改变分核、tile 长度、DMA、参数驻留或输入输出路径。

设备指令证据：NONE。生成的 AICore ELF 可提取为 `elf64-hiipu`，但当前 toolkit disassembler 对 `.text` 输出 `<not available>`，通用 AArch64 反汇编也无法可靠识别 AICore 指令，因此本假设没有可引用的设备指令支持。相关日志为 `build_out/n03-reduction-topology/device-instruction-v002.log`、`device-instruction-v002-disasm.log` 和 `device-aicore-v002.log`。

## ONE CHANGE

- 父源码只读保留：`线上结果/R31B/V011/submission.asc`。
- V002 路线源码：`本地实验/REDUCTION-TOPOLOGY-X/V002/Candidate.asc`。
- Candidate 由父源码复制后只在 `ProcessWideLowPrecision` 的最终 RMS partial 合并处加入 `kWideN03BlockTiles = 4` 的两层 `ReduceSum`。
- paired wrapper 和 ABI 未改；CMake 绑定从 V001 Candidate 路径切换到 V002 Candidate 路径。

源码身份：

- Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- V002 Candidate SHA-256：`de6ec07d406d70e29870b3716e9baca239bf30e1e6846b9f6bd209c01d672ba3`
- server3 编译输入的 Parent 与 Candidate SHA-256 与本 Clone 相同。

## COMPILE

server3：`hwnput3`；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；目标：`paired_runner_v016`。

真实命令序列：

1. 首次尝试 source 了不存在的版本目录 `set_env.sh`，Configure 仍完成，Compile 返回 2；日志证据为 `ASCEND_HOME_PATH` 缺失及插件崩溃。
2. 使用实际存在的 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh` 重跑，返回 2；日志证据为插件生成代码找不到 `<vector>`。
3. 沿用公开 paired 构建使用的 C++ include 环境重跑：

   `cmake -S /tmp/cann-n03-reduction-topology-v002-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v002-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 && cmake --build /tmp/cann-n03-reduction-topology-v002-20261016/build --target paired_runner_v016 --parallel 1`

   返回码：0；末尾为 `[100%] Built target paired_runner_v016`。

Compile 日志：

- `build_out/n03-reduction-topology/compile-v002-server3.log`
- `build_out/n03-reduction-topology/compile-v002-server3-retry.log`
- `build_out/n03-reduction-topology/compile-v002-server3-retry2.log`

生成 ELF SHA-256：`aa9142a28a5dcaa2cfa94ab4bba34db0a60f2b57d0ad0d6bddc4fa3785ef242d`。

## CORRECTNESS

设备：NPU 4；Runner：现有 `paired_runner_v016 --qualify-v011` 与 `--paired`。

- Parent 完整 `--qualify-v011` 返回码 0，4/4 样例 PASS：FP16 D12288、FP16 D32768、BF16 D12288、BF16 D32768。
- Parent 误差上界：FP16 最大绝对误差 0.001953125；BF16 最大绝对误差 0.0078125。
- Candidate 在 `--paired` 首个样例 FP16 D12288 完成，Candidate Correctness PASS，误差为最大绝对误差 0.0009765625、最大相对误差 0.0015923567；随后进入 FP16 D32768 warmup 时发生设备同步错误，未完成 Candidate 的 4/4 Correctness。因此完整 Candidate Correctness：NONE。

日志：

- Parent：`build_out/n03-reduction-topology/correctness-v002-server3-retry.log`
- Candidate 部分样例：`build_out/n03-reduction-topology/local-v002-server3-retry.log`

## LOCAL

目标命令：`/tmp/cann-n03-reduction-topology-v002-20261016/build/paired_runner_v016 --paired`。

协议为 warmup=45、交错 pairs=21、device-event 主指标。第一次 Local 返回码 2；第二次按一次瞬态失败重跑仍返回码 2。两次均在 FP16 D12288 完成后，进入 FP16 D32768 warmup 时报告 `candidate warmup synchronize failed: 507035 (null)`。

运行负载记录：NPU 4 AICore 57%，温度 60 C，VLLMEngineCor 进程占用 55,686 MB。该外部负载和同步错误使完整 paired Local 无法完成，故 V002 Local 结果为 `NONE`，不从首个样例的中位数推断收益。

日志：

- `build_out/n03-reduction-topology/local-v002-server3.log`
- `build_out/n03-reduction-topology/local-v002-server3-retry.log`
- `build_out/n03-reduction-topology/device-v002-local-retry-pre.log`

## RESULT

H2 已形成不同于 V001 的局部块长度归约实现，并通过 server3 Compile。当前没有完整 Candidate Correctness 和 paired Local 证据；设备指令证据为 NONE，性能结论为 NONE。

## COMMIT

待将 V002 Candidate、CMake V002 绑定和本记录作为一个本地提交；不推送远端，不提交 Official。

## NEXT

停止继续扩大 V002 修改范围。后续若继续，应先解决设备同步错误或获得可解析的 AICore 指令证据，再提出新的单因素归约组织假设。

## 2026-10-17 服务器事务

- Compile：PASS；远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V002`。
- Correctness：FP16 D32768 Candidate 在 warmup 同步返回 `507035`，Runner RC=2；本轮未完成完整矩阵。
- Local：NONE；未产生完整配对样本。
- 原始日志：`server-device7-correctness-local-20261017.log`；NPU7 前后 Health=OK、HBM 约 5%、无进程，未 Reset。
