# N03 REDUCTION-TOPOLOGY-X V010

日期：2026-10-17  
父版本：V009 Candidate（源码提交 `6ef2b016`）  
状态：Candidate 已形成；Compile、Correctness、Local 均未执行。

## 假设

V009 每两个输入 tile 在 FP32 lane 上相加，再对该组做一次 `ReduceSum`；每组 partial 进入对齐槽，并通过 carry tree 合并。V010 将 lane block 扩为最多四个连续 tile：同一 lane 内依次累加最多四份 FP32 平方，再对该 block 调用一次 `ReduceSum`。标量 partial 仍由 V009 的 carry tree 平衡合并。

目标是长 D 下减少横向 `ReduceSum` 与标量合并次数。主要观察形状为 rows=2、D=32768；尾部控制形状为 rows=2、D=12288。此机制区别于 V002/V003：它先在向量 lane 上汇总四个 tile 再做一次 ReduceSum；V002/V003 是逐 tile ReduceSum 后再合并标量。

## 操作量预期

| Shape | tileWidth / tileCount | V009：ReduceSum / lane Add / 标量合并 Add | V010：ReduceSum / lane Add / 标量合并 Add |
|---|---:|---:|---:|
| FP16 D12288 | 4096 / 3 | 2 / 1 / 1 | 1 / 2 / 0 |
| FP16 D32768 | 4096 / 8 | 4 / 4 / 3 | 2 / 6 / 1 |
| BF16 D12288 | 4096 / 3 | 2 / 1 / 1 | 1 / 2 / 0 |
| BF16 D32768 | 2560 / 13 | 7 / 6 / 6 | 4 / 9 / 3 |

V010 以更多宽向量 Add 和最长三步 lane 累加链，换取更少的 ReduceSum 与标量合并。BF16 D32768 是主要分辨形状；若水平归约成本高于增加的向量 Add，预期收益更明显。此为待实测假设。

数值代价：平方值仍以 FP32 累加，y 的 FP16/BF16 生成及存储路径不变；每个四 tile block 改变 FP32 加法结合顺序，需重新做四组 Correctness。

## 源码变化

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V010/Candidate.asc`，由 V009 派生。
- 只改 `ProcessWideLowPrecision` 的 lane block 宽度及 partial 触发位置；尾 tile 仍在所在 block 结束时归约。
- `kWideFullYReduceStride=16`、partial stride=8、carry tree、双输入 staging、DataCopyPad、TBuf 分配和输出路径保持不变；未增加缓冲。
- 现有 `本地实验/R31B/V016/support/paired/CMakeLists.txt` 通过 `R31B_V016_SOURCE_DIR` 绑定 Candidate；Runner 沿用 `paired_runner_v016`，不改 Runner 或 CMake。

## 执行状态

- Compile：未执行。本轮仅授权在 `/Users/sunyiyang/Desktop/Project/cann` 内工作，未访问该目录外的 CANN 工具链或 server3。
- Correctness：未执行；需 Compile PASS 后由用户分配设备窗口，再沿用现有 Runner。
- Local：未执行，NONE。仅在 Correctness 通过且设备窗口分配后测量。
- 本轮没有连接 server、采样或运行 NPU，也未触碰 CANNJudge。

下一步：在用户授权的构建环境中沿用现有 paired CMake，设置 `R31B_V016_SOURCE_DIR` 指向 V010 并编译 `paired_runner_v016`。Compile 通过后申请设备窗口，先执行四组 Correctness，再执行 paired Local；记录实际结果后判断四 tile lane block 是否优于 V009。
