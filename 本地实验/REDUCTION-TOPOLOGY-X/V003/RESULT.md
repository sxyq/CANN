# N03 REDUCTION-TOPOLOGY-X V003

日期：2026-10-16
状态：Compile PASS；Candidate 的完整 Correctness 与 Local 未完成，FP16 D32768 在 warmup 同步时返回 507035。保留本次未完成 revision 和原始日志。

## V002_EVIDENCE

本 Clone 的 V002 源码、paired 输出及项目内 API 资料提供了一个可核对的归因线索：

- `/Users/sunyiyang/Desktop/Project/cann/.agents/skills/ascendc-api-best-practices/references/api-reduce.md:204-224` 说明 `ReduceSum<float>` 的 dst 起始地址需 8 字节对齐；每个结果占 4 字节时，stride=1 的奇数槽不满足要求。
- V002 在 `Candidate.asc:3240-3243` 把块结果依次写到 `xFp32[block]`。
- 固定 tile 宽度 4096 下，D12288 有 3 个 tile、1 个块，写入 `xFp32[0]`；D32768 有 8 个 tile、2 个块，第二个块写入 `xFp32[1]`，字节偏移为 4，违反上述 API 对齐要求。
- V002 的两次 paired 日志均显示 D12288 通过，随后 Candidate 在 D32768 warmup 的流同步返回 507035：`build_out/n03-reduction-topology/local-v002-server3.log`、`local-v002-server3-retry.log`。

这将故障具体关联到第二个块归约结果的目的槽。日志没有 kernel 级指令回溯；API 资料说明未对齐可能导致静默结果异常，并未直接断言它必然触发 507035，因此该线索支持做一次修订验证，不能单独作为已解码设备故障指令的结论。

## HYPOTHESIS

保持 tile partial 计算及每 4 个 partial 的首层 `ReduceSum` 不变；块和写入 stride=2 的 FP32 槽，满足 8 字节 dst 对齐；最后仅对块和列表做平衡二叉 `Add` 合并。

与此前两版的区别：V001 对全部 tile partial 做二叉 `Add`；V002 对 4-tile 块和列表再调用一次 `ReduceSum`；V003 保留 4-tile 局部归约，在对齐槽中仅对块和做二叉 `Add`。不改变分核、tile 宽度、DMA、参数加载、FP32 数学路径或输入输出路径。

## ONE_CHANGE

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V003/Candidate.asc`
- 现有 paired 工程绑定：`本地实验/R31B/V016/support/paired/CMakeLists.txt`
- Runner 源码和 ABI 未改：`paired_runner_v016.asc`、`paired_runner.cpp`、`paired_runner_abi.h`
- 父源码只读：`线上结果/R31B/V011/submission.asc`

Candidate SHA-256：`e11f537581e2e65ae6ac41a3f47f85eb475b24ae05a6f5701a8ee0d45a5d5062`。跨机传输后，server3 上 Candidate 与本 Clone 的 SHA-256 相同。
Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。

## COMPILE

- 主机：`hwnput3`；SoC：Ascend 910B3；工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；目标：`paired_runner_v016`。
- 命令：

  `cmake -S /tmp/cann-n03-reduction-topology-v002-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v002-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V016_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v002-20261016/本地实验/REDUCTION-TOPOLOGY-X/V003 && cmake --build /tmp/cann-n03-reduction-topology-v002-20261016/build --target paired_runner_v016 --parallel 1`

- 执行前 source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，并设置 `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward`。

- 返回码：0；日志末行为 `[100%] Built target paired_runner_v016`。
- 日志：`build_out/n03-reduction-topology/compile-v003-server3.log`。

## CORRECTNESS

- 使用现有 paired Runner 的 `--paired` 模式，设备 NPU 4，rows=2。
- FP16 D12288 Parent/Candidate 均 PASS；Candidate 最大绝对误差 0.0009765625，最大相对误差 0.0015923567。
- FP16 D32768 在 Candidate warmup 的 `aclrtSynchronizeStream` 返回 507035；剩余 shape 未执行，完整 Candidate Correctness 为 NONE。
- 本轮没有采集运行时 NPU 负载快照，当前负载记录为 NONE。

## LOCAL

- 命令：`/tmp/cann-n03-reduction-topology-v002-20261016/build/paired_runner_v016 --paired`
- 执行前 source 同一 `set_env.sh`，并设置上述 `CPLUS_INCLUDE_PATH` 与 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64`。
- 协议：每个 shape warmup=45、交错 pairs=21，device event 为主计时。
- D12288 已完成 21 对：Parent median 15.340 us，Candidate median 6.800 us，配对 delta median -0.440 us。样本波动大（Parent CV=0.99022、Candidate CV=1.27580），仅属部分结果，不据此判断收益。
- 进入 FP16 D32768 后 Candidate warmup 同步返回 507035，进程返回码 2；完整 paired Local 为 NONE。
- 原始日志：`build_out/n03-reduction-topology/local-v003-server3.log`。

## RESULT

对齐槽符合 API 文档，但 D32768 仍在包含两个块和的路径触发 507035。V002 的 stride=1 目的槽是已确认的 API 约束违例，却不足以解释全部运行时异常。当前故障与多块合并路径相关；现有材料仍不能在 Add 树、设备运行时或其他向量核异常之间定出具体指令根因。无性能结论。

## COMMIT

本地保存 Candidate、CMake 绑定和本结果；不推送、不做 Official 操作。

## NEXT

在可取得 HIIPU 指令回溯或其他直接设备故障定位资料前，暂停新归约拓扑设计。不要重跑相同 V003 Candidate。
