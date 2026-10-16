# N03 REDUCTION-TOPOLOGY-X V012

日期：2026-10-17（服务器 UTC 运行窗口：2026-10-16）  
父版本：V011 Candidate（源码提交 `fe8495fd`）  
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，四组差值区间均跨零，Local=NONE。

## 假设

V011 使用四 tile lane block，并在本行全部 block 完成后用 Vector-side 二叉树合并 partial。V012 将 lane block 改为两个连续 tile，保留 V011 的 Vector tree、partial 对齐布局和输出路径。

2-tile block 会增加 block partial 数量，目标是缩短每条 lane 内的连续 Add 链，并观察更多 block partial 对 Vector tree 和 Scalar 归约尾部的影响。当前 Runner 的最大形状为 BF16 D32768：tileWidth=2560、tileCount=13、blockCount=7。

## 源码变化

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V012/Candidate.asc`，由 V011 复制后修改宽行低精度归约参数。
- `kWideN03LaneBlockTiles` 从 4 改为 2；同一行的 block partial 数从最多 4 个增加到最多 7 个。
- `kWideFullYReduceStride` 从 V011 的 32 个 FP32 槽扩为 64 个 FP32 槽，保留 8 个 FP32（32B）partial 间距，覆盖 block 偏移 0、8、16、24、32、40、48。
- Vector-side 二叉树、tile 计算、输入双缓冲、FP32 平方、RMS、输出路径和 paired Runner 保持不变。
- 没有修改 Runner、CMake、Parent 或共享记录。

## 服务器 Compile

- 主机：`hwnput3`；SSH 入口：`cann-server3`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V012`。
- Toolkit：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；`set_env.sh` 返回 0。
- Candidate SHA256：`86adcb778b1ac15c73b672196871007232665a1dd8c1297db285fd0660f1c444`。
- 复用入口：`本地实验/R31B/V016/support/paired/CMakeLists.txt`、`paired_runner_v016.asc` 和现有 `paired_runner_v016`。

命令：

```text
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V012/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V012
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V012/build --target paired_runner_v016 --parallel 1
```

- Configure：PASS，返回码 0。
- Build：PASS，返回码 0；输出 `[100%] Built target paired_runner_v016`。
- 产物：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V012/build/paired_runner_v016`，AArch64 ELF，SHA256 `126b242b25c90e7569ed6953d743c64be09b7bdbfe5a344568c0ef64567661da`。
- Compile 证据：`本地实验/REDUCTION-TOPOLOGY-X/V012/compile.log`。

## Correctness 与 paired Local

- 设备：NPU7；命令：`./paired_runner_v016 --paired --device 7`。
- 运行前：Health=OK、HBM 使用率 5%；NPU7 的 `proc-mem` 显示既存 `clx_ref_candida` 进程 122 MB，未触碰该进程。
- Runner：`warmup_each=45`、`pairs_each=21`；四组 Correctness 全部 PASS，`failures=0`；Runner 返回码 0。
- Runner 日志中的 `binary=V011` 是 Parent，`binary=V016` 是本版本 Candidate；下表已按 Parent/Candidate 展示。

| Shape | Parent median us | Candidate median us | Candidate-Parent delta median us | Delta P10–P90 us | Local 判断 |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 20.900 | 22.640 | 1.220 | [-13.100, 16.960] | 跨零 |
| FP16 D32768 | 13.000 | 21.440 | -0.060 | [-8.300, 11.600] | 跨零 |
| BF16 D12288 | 7.520 | 19.320 | 0.580 | [-13.480, 13.120] | 跨零 |
| BF16 D32768 | 21.400 | 22.580 | 0.100 | [-6.300, 9.160] | 跨零 |

- Correctness 误差与 V011 一致：FP16 D12288 `max_abs=0.0009765625`、`max_rel=0.0015923567`；FP16 D32768 `0.001953125`、`0.010989011`；BF16 D12288 `0.0078125`、`0.0073529412`；BF16 D32768 `0.0078125`、`0.01754386`。
- 四组配对差值区间均跨零，`LOCAL_SCORE=NONE`；Candidate median 四组均高于 Parent，2-tile block 未形成收益证据。
- 运行后：Health=OK、HBM 使用率 5%、NPU7 无进程；未 Reset、未触碰既存进程。
- 原始设备证据：`本地实验/REDUCTION-TOPOLOGY-X/V012/device7-correctness-local-20261017.log`。

## RESULT

V012 的 2-tile lane block 通过 Compile 和四组 Correctness，但 paired Local 未形成稳定收益。FP16 D32768 的 Candidate median 为 `21.440 us`，Parent 为 `13.000 us`；BF16 D12288 的 Candidate median 为 `19.320 us`，Parent 为 `7.520 us`。四组 raw median 都显示 Candidate 高于 Parent，但配对区间均跨零，不能形成稳定 Local 结论。

## NEXT

停止继续扩大低精度 lane block 变化。下一实验若继续 N03，应回到已通过 Correctness 的 V011 结构，改测 partial 槽读取/Vector tree 的单一指令组织，避免继续叠加 2-tile 的额外 block 数量。
