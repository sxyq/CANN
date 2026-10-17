# N03 REDUCTION-TOPOLOGY-X V015

日期：2026-10-17（服务器 UTC 运行窗口：2026-10-16）
父版本：V011 Candidate（源码提交 `fe8495fd`）
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，四组差值区间均跨零，`LOCAL_SCORE=NONE`。

## 单一机制

V011 保留四 tile lane block，并在每个 block 结束后把 partial 写入 32B 对齐槽，再做 Vector-side 归约。V015 保留 lane block、partial 槽宽度、每次读取一个 FP32 槽和同步顺序，只改变归约依赖组织：

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V015/Candidate.asc`。
- `kWideN03LaneBlockTiles=4`、`kWideN03PartialStride=8`、`kWideFullYReduceStride=32` 保持 V011。
- 归约不再按 `treeStride=1,2,...` 的平衡树配对；改为 `rightBlock=1..blockCount-1`，将每个后续 partial 依次累加到当前 batchRow 的首槽。
- 每次首槽 `Add(..., 1)` 后保留 `PIPE_V` barrier；最终 RMS 仍从每行首槽读取。
- tile 计算、FP32 平方、RMS、输出路径、Parent 和 paired Runner 均未改动。

## Compile

- 主机：`hwnput3`；SSH 入口：`cann-server3`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V015`。
- Toolkit：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- Candidate SHA256：`d035bcc92684066f792b8203b049b0f12229db259444a86ba242d94c77e50014`。
- 复用入口：现有 `paired_runner_v016`、`runner`、`parent`；没有创建新的 Runner 或执行入口。

命令：

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V015/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V015
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V015/build --target paired_runner_v016 --parallel 1
```

- Configure：返回码 0。
- Build：返回码 0，输出 `[100%] Built target paired_runner_v016`。
- Runner：AArch64 ELF，SHA256 `e399c358c7bb51148b40b4c4a6cf36580e59fb6d56636f91bf1ee760a8fa0a4d`。
- Compile 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V015/compile.log`。

## Correctness 与 paired Local

- 设备：NPU7；主机：`hwnput3`；运行前 Health=OK、HBM 使用率 5%、AICore=0%；运行后 Health=OK、HBM 使用率 5%、AICore=0%，设备无进程。
- 环境：`source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，随后 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}`。
- 命令：`./paired_runner_v016 --paired --device 7`。
- Runner：`warmup_each=45`、`pairs_each=21`；四组 Correctness 全部 PASS，`failures=0`；Runner 返回码 0。
- Runner 中 `binary=V011` 为 Parent，`binary=V016` 为本版本 Candidate。

| Shape | Parent median us | Candidate median us | Candidate-Parent median us | Delta P10-P90 us | Local |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 24.180 | 24.580 | -0.040 | [-13.700, 13.780] | NONE |
| FP16 D32768 | 26.840 | 26.840 | 0.620 | [-7.560, 4.240] | NONE |
| BF16 D12288 | 26.300 | 25.780 | -0.520 | [-11.000, 3.140] | NONE |
| BF16 D32768 | 26.720 | 30.800 | 0.420 | [-4.580, 13.720] | NONE |

- 四组配对差值区间均跨零；BF16 D12288 的 Candidate 中位数低于 Parent，BF16 D32768 高于 Parent，FP16 两组分别接近和高于 Parent，未形成稳定 Local 收益。
- Correctness 误差：FP16 D12288 `max_abs=0.0009765625`、`max_rel=0.0015923567`；FP16 D32768 `0.001953125`、`0.010989011`；BF16 D12288 `0.0078125`、`0.0073529412`；BF16 D32768 `0.0078125`、`0.01754386`。
- paired 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V015/device7-correctness-local-20261017.log`。

## RESULT

V015 的首槽线性 fan-in 通过 Compile 和四组 Correctness，但 paired Local 未形成稳定收益。该归约依赖组织变化停止继续扩大。

下一实验：回到 V011，保留四 tile lane block、32B partial 槽和 `Add(..., 1)`，后续只在现有 Vector tree/partial 代码中找到尚未覆盖的单一依赖组织入口后再决定是否开展 V016。

PUSH=NO；线上提交=NO；共享记录=未修改。
