# N03 REDUCTION-TOPOLOGY-X V010

日期：2026-10-17  
父版本：V009 Candidate（源码提交 `6ef2b016`）  
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，但四组差值区间均跨零，Local=NONE。

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

## 服务器执行记录

- 服务器：`hwnput3`；SSH 入口：`cann-server3`；用户：`lelinfeng`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03`。
- 工具链：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；`set_env.sh` 返回码：0。
- 源码 Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V010/Candidate.asc`；源码提交：`82dda27b`；SHA256：`bcd8f6ebf54ac334735cb5bd608a499e6be0c2fa67970738639af0a75513fcf1`。
- 复用入口：`本地实验/R31B/V016/support/paired/CMakeLists.txt`、`paired_runner_v016.asc`；两者 SHA256 分别为 `2bf75ef42740e2d04f155848c7276fccf5751a89b747fe78f0e2c9157b5e21e8`、`8a4489c960415039fb4f6cfd601fb8929de3ee6617b8a2f84ba63cbb8068c9b6`，与远端 runner 入口一致。

### Compile

- Configure：`cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V010/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V010`；返回码：0。
- Build：`cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V010/build --target paired_runner_v016 --parallel 1`；返回码：0；日志末行为 `[100%] Built target paired_runner_v016`。
- 产物：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V010/build/paired_runner_v016`，AArch64 ELF，587376 bytes。

### Correctness 与 Local

- 设备：NPU7；Runner：`paired_runner_v016`；模式：`PAIRED`；四组 Correctness 全部 PASS，`failures=0`，Runner 返回码：0。
- Correctness 结果：FP16 D12288 `max_abs=0.0009765625, max_rel=0.0015923567`；FP16 D32768 `0.001953125, 0.010989011`；BF16 D12288 `0.0078125, 0.0073529412`；BF16 D32768 `0.0078125, 0.01754386`。

| Shape | Parent median us | Candidate median us | Delta median us | Delta P10–P90 us | Local 判断 |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 15.840 | 7.260 | -0.220 | [-10.280, 10.880] | 跨零 |
| FP16 D32768 | 17.420 | 18.040 | 0.860 | [-5.080, 6.520] | 跨零 |
| BF16 D12288 | 7.300 | 9.480 | -0.200 | [-1.880, 14.360] | 跨零 |
| BF16 D32768 | 17.760 | 16.540 | -0.340 | [-8.280, 1.900] | 跨零 |

- 每组 `pairs=21`、`warmup_each=45`；计时主指标为 `device_us`，wall-clock 仅作诊断。
- Local：四组区间均跨零，`LOCAL_SCORE=NONE`；重点形状 FP16/BF16 D32768 没有明确稳定收益，记录负结果，不推荐线上提交。
- 设备负载：运行前 HBM 5%、AICore 0%、AIVector 0%、CtrlCPU 12%；运行后 HBM 5%、AICore 0%、AIVector 0%、CtrlCPU 4%；结束后无运行进程。
- `507035`：未出现；日志中无该返回码。SLOG：V010 目录未生成独立 SLOG 文件；已保留完整 Runner stdout/stderr 与设备负载记录。
- 证据：`compile.log`、`device7-correctness-local-20261017.log`；两份文件已从远端目录原样保存到本目录。
- 远端未执行线上提交。
