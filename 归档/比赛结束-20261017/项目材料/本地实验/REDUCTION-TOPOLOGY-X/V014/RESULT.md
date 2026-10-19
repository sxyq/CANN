# N03 REDUCTION-TOPOLOGY-X V014

日期：2026-10-17（服务器 UTC 运行窗口：2026-10-16）
父版本：V011 Candidate（源码提交 `fe8495fd`）
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，四组差值区间均跨零，`LOCAL_SCORE=NONE`。

## 单一机制

V011 保留四 tile lane block，并把每行的 block partial 写入 32B 对齐槽后执行 Vector-side 二叉树。V014 保留 lane block、partial 槽宽度和每条 tree 边的 Vector `Add`，只调整 tree 的依赖发射顺序：

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V014/Candidate.asc`。
- `kWideN03LaneBlockTiles=4`、`kWideN03PartialStride=8`、`kWideFullYReduceStride=32` 保持 V011。
- `Add(..., 1)` 保持不变；没有改变 partial 槽的读取宽度。
- tree 外层改为 `treeStride`，内层遍历当前 batch 的全部 `batchRow`；每个依赖层只执行一次 `PIPE_V` barrier。
- tile 计算、FP32 平方、RMS、输出路径、Parent 和 paired Runner 均未改动。

## Compile

- 主机：`hwnput3`；SSH 入口：`cann-server3`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V014`。
- Toolkit：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- Candidate SHA256：`fe1afe9f5ec53b406aacf2dcc418c87e35c4a4d689abfb828c0c8ca60a5f8c2d`。
- 复用入口：现有 `paired_runner_v016`、`runner`、`parent`；没有创建新的 Runner 或执行入口。

命令：

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V014/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V014
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V014/build --target paired_runner_v016 --parallel 1
```

- Configure：返回码 0。
- Build：返回码 0，输出 `[100%] Built target paired_runner_v016`。
- Runner：AArch64 ELF，SHA256 `79500af3d5104743195160fb889796703ed3f4b5d093158b01823018568d0ba3`。
- Compile 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V014/compile.log`。

## Correctness 与 paired Local

- 设备：NPU7；主机：`hwnput3`；运行前 Health=OK、HBM `3434/65536 MB`、AICore=0%。预采样曾看到既存 `clx_ref_candida` 进程 104 MB，未触碰；Runner 自身开始前设备无进程。
- 环境：`source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，随后 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}`。
- 命令：`./paired_runner_v016 --paired --device 7`。
- Runner：`warmup_each=45`、`pairs_each=21`；四组 Correctness 全部 PASS，`failures=0`；Runner 返回码 0。
- Runner 中 `binary=V011` 为 Parent，`binary=V016` 为本版本 Candidate。

| Shape | Parent median us | Candidate median us | Candidate-Parent median us | Delta P10-P90 us | Local |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 16.520 | 20.180 | 0.380 | [-1.380, 5.560] | NONE |
| FP16 D32768 | 22.500 | 22.660 | 0.160 | [-12.220, 11.800] | NONE |
| BF16 D12288 | 14.300 | 8.220 | -0.540 | [-12.600, 12.020] | NONE |
| BF16 D32768 | 23.600 | 22.240 | -0.140 | [-20.960, 7.840] | NONE |

- 四组配对差值区间均跨零，Candidate median 在两组 FP16 上略高、两组 BF16 上略低，跨 dtype 没有稳定收益方向。
- Correctness 误差：FP16 D12288 `max_abs=0.0009765625`、`max_rel=0.0015923567`；FP16 D32768 `0.001953125`、`0.010989011`；BF16 D12288 `0.0078125`、`0.0073529412`；BF16 D32768 `0.0078125`、`0.01754386`。
- 运行后：NPU7 Health=OK、HBM `3434/65536 MB`、AICore=0%、设备无进程；未 Reset、未触碰既存进程。
- paired 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V014/device7-correctness-local-20261017.log`。

## RESULT

V014 的跨 batchRow tree 层发射通过 Compile 和四组 Correctness，但 paired Local 未形成稳定收益。该依赖组织变化停止继续扩大。

下一实验：回到 V011，保持四 tile lane block、32B partial 槽和 `Add(..., 1)` 不变；继续前先读取尚未使用的 Vector tree 代码入口，再选择单一机制。

PUSH=NO；线上提交=NO；共享记录=未修改。
